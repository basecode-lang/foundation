// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//      F O U N D A T I O N   P R O J E C T
//
// Copyright (C) 2017-2021 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <basecode/core/env.h>
#include <basecode/core/buf.h>
#include <basecode/core/string.h>
#include <basecode/core/slice_utils.h>
#include <basecode/core/stable_array.h>

#ifdef _WIN32
#   define LIST_SEP ';'
#else
#   define LIST_SEP ':'
#endif

namespace basecode::env {
    using env_array_t           = stable_array_t<env_t>;
    using env_table_t           = hashtab_t<str::slice_t, env_t*>;

    struct system_t final {
        alloc_t*                alloc;
        env_array_t             envs;
        env_table_t             envtab;
    };

    system_t                    g_env_sys{};

    u0 free(env_t* env) {
        for (const auto& pair : env->vartab) {
            if (pair.value.type == env_value_type_t::array)
                array::free(const_cast<slice_array_t&>(pair.value.kind.list));
        }
        hashtab::free(env->vartab);
    }

    status_t init(env_t* env,
                  str::slice_t name,
                  env_t* parent,
                  alloc_t* alloc) {
        env->alloc  = alloc;
        env->name   = name;
        env->parent = parent;
        hashtab::init(env->vartab, alloc);
        return status_t::ok;
    }

    status_t parse(env_t* env, s8** pairs) {
        if (!pairs)
            return status_t::expected_non_null_pairs;
        s8** e = pairs;
        for (; *e; e++) {
            auto key_ptr = *e;
            auto p = key_ptr;
            while (*(p++) != '=');
            auto key = slice::make(key_ptr, (p - key_ptr) - 1);
            if (*p == '"')
                ++p;
            auto val_ptr = p;
            while (true) {
                auto ch = *(p++);
                if (!ch || ch == '"')
                    break;
            }
            auto val = slice::make(val_ptr, (p - val_ptr) - 1);
            if (slice::contains(val, LIST_SEP)) {
                slice_array_t fields{};
                array::init(fields, g_env_sys.alloc);
                slice::to_fields(val, fields, LIST_SEP);
                env::set(env, key, fields);
                array::free(fields);
            } else {
                env::set(env, key, val);
            }
        }
        return status_t::ok;
    }

    status_t load(env_t* env, const path_t& path) {
        buf_t buf{};
        buf::init(buf, g_env_sys.alloc);
        defer(buf::free(buf));

        if (!OK(buf::load(buf, path)))
            return status_t::load_config_error;

        buf::index(buf);

        buf_crsr_t c{};
        buf::cursor::init(c, buf);
        defer(buf::cursor::free(c));

        array_t<s8*> pairs{};
        array::init(pairs, g_env_sys.alloc);

        for (const auto& line : buf.lines) {
            if (line.len == 0)
                continue;
            if (!OK(buf::cursor::seek(c, line.pos)))
                return status_t::load_config_error;
            if (CRSR_READ(c) == '#')
                continue;
            array::append(pairs, (s8*) CRSR_PTR(c));
            while (CRSR_READ(c) != '=')
                CRSR_NEXT(c);
            // move past the =
            CRSR_NEXT(c);
            // skip the first "
            if (CRSR_READ(c) == '"')
                CRSR_NEXT(c);
            while (true) {
                auto ch = CRSR_READ(c);
                if (ch == '\n' || ch == '"')
                    break;
                CRSR_NEXT(c);
            }
            *CRSR_PTR(c) = '\0';
        }

        array::append(pairs, nullptr);
        auto status = parse(env, pairs.data);
        array::free(pairs);
        return status;
    }

    env_value_t* get(env_t* env, str::slice_t key) {
        auto curr = env;
        while (curr) {
            auto val = hashtab::find(env->vartab, key);
            if (val)
                return val;
            curr = curr->parent;
        }
        return nullptr;
    }

    status_t expand(env_t* env, env_value_t* value, str_t& expanded) {
        const u8* p;
        const u8* e;

        switch (value->type) {
            case env_value_type_t::string:
                p = value->kind.str.data;
                e = p + value->kind.str.length;
                break;
            case env_value_type_t::array:
                return status_t::ok;
        }

        const u8* last_stop = p;
        while (p != e) {
            if (*p != '$') {
                ++p;
                continue;
            }
            if (p > last_stop)
                str::append(expanded, last_stop, p - last_stop);
            ++p;
            auto key_ptr = p;
            while (true) {
                auto ch = *p;
                if (!isalnum(ch) && ch != '_')
                    break;
                ++p;
            }
            last_stop = p;
            auto key = slice::make(key_ptr, p - key_ptr);
            auto val = env::get(env, key);
            if (!val)
                return status_t::key_not_found;
            auto status = expand(env, val, expanded);
            if (!OK(status))
                return status;
        }
        str::append(expanded, last_stop, p - last_stop);
        return status_t::ok;
    }

    env_value_t* set(env_t* env, str::slice_t key, str::slice_t value) {
        auto interned_key = string::interned::fold(key);
        auto val = hashtab::find(env->vartab, interned_key);
        if (!val) {
            val = hashtab::emplace(env->vartab, interned_key);
            val->type = env_value_type_t::string;
        }
        val->kind.str = string::interned::fold(value);
        return val;
    }

    env_value_t* set(env_t* env, str::slice_t key, const slice_array_t& value) {
        auto interned_key = string::interned::fold(key);
        auto val = hashtab::find(env->vartab, interned_key);
        if (!val) {
            val = hashtab::emplace(env->vartab, interned_key);
            val->type = env_value_type_t::array;
            array::init(val->kind.list, g_env_sys.alloc);
        }
        array::append(val->kind.list, value);
        return val;
    }

    namespace system {
        u0 fini() {
            hashtab::free(g_env_sys.envtab);
            stable_array::free(g_env_sys.envs);
        }

        env_t* get_root() {
            return get("root"_ss);
        }

        env_t* get(str::slice_t name) {
            return hashtab::find(g_env_sys.envtab, name);
        }

        status_t init(alloc_t* alloc) {
            g_env_sys.alloc = alloc;
            hashtab::init(g_env_sys.envtab, g_env_sys.alloc);
            stable_array::init(g_env_sys.envs, g_env_sys.alloc);
            make("root"_ss, nullptr, environ);
            return status_t::ok;
        }

        env_t* make(str::slice_t name, env_t* parent, s8** pairs) {
            auto env = &stable_array::append(g_env_sys.envs);
            name = string::interned::fold(name);
            env::init(env, name, parent, g_env_sys.alloc);
            hashtab::insert(g_env_sys.envtab, name, env);
            if (pairs) {
                auto status = parse(env, pairs);
                if (!OK(status))
                    return nullptr;
            }
            return env;
        }
    }
}
