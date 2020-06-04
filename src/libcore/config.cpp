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
// Copyright (C) 2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <basecode/core/log.h>
#include <basecode/core/array.h>
#include <basecode/core/defer.h>
#include <basecode/core/mutex.h>
#include <basecode/core/format.h>
#include <basecode/core/config.h>
#include <basecode/core/symtab.h>
#include <basecode/core/string.h>
#include <basecode/core/str_array.h>
#include <basecode/core/log/system/spdlog.h>
#include <basecode/core/log/system/syslog.h>

namespace basecode::config {
    struct system_t final {
        alloc_t*                alloc;
        fe_Context*             ctx;
        str_t                   buf;
        mutex_t                 lock;
        u32                     heap_size;
    };

    system_t                    g_system;

    struct kernel_func_t final {
        const s8*               symbol;
        fe_CFunc                func;
    };

    using arg_map_t             = symtab_t<fe_Object*>;

    static str_t& to_str(fe_Context* ctx, fe_Object* obj) {
        str::reset(g_system.buf);
        g_system.buf.length = fe_tostring(ctx, obj, (s8*) g_system.buf.data, g_system.buf.capacity);
        return g_system.buf;
    }

    static fe_Object* get_map_arg(arg_map_t& args, u32 pos) {
        fe_Object* arg;
        ++pos;
        if (symtab::find(args, slice::make((const u8*) &pos, sizeof(u32)), arg))
            return arg;
        return nullptr;
    }

    static fe_Object* next_arg(fe_Context* ctx, fe_Object** arg) {
        fe_Object* a = *arg;
        if (fe_type(ctx, a) != FE_TPAIR)
            return fe_bool(ctx, 0);
        *arg = fe_cdr(ctx, a);
        return fe_car(ctx, a);
    }

    static u32 make_arg_map(fe_Context* ctx, fe_Object* arg, arg_map_t& args) {
        u32 pos = 1;
        while (true) {
            auto obj = next_arg(ctx, &arg);
            auto type = fe_type(ctx, obj);
            if (type == FE_TNIL) {
                break;
            } else if (type == FE_TKEYWORD) {
                auto value_obj = next_arg(ctx, &arg);
                type = fe_type(ctx, value_obj);
                if (type == FE_TNIL)
                    fe_error(ctx, "keyword parameter requires value");
                else if (type == FE_TKEYWORD)
                    fe_error(ctx, "keyword parameter value cannot be another keyword");
                else
                    symtab::insert(args, to_str(ctx, obj), value_obj);
            } else {
                symtab::insert(args, slice::make((const u8*) &pos, sizeof(u32)), obj);
                ++pos;
            }
        }
        return pos;
    }

    static const s8* vlog(fe_Context* ctx, fe_Object* arg) {
        using fmt_ctx = fmt::format_context;
        using fmt_arg = fmt::basic_format_arg<fmt_ctx>;

        str_t fmt_str{};
        str::init(fmt_str, g_system.alloc);
        str::append(fmt_str, to_str(ctx, fe_nextarg(ctx, &arg)));

        str_array_t strs{};
        str_array::init(strs, g_system.alloc);
        defer(str_array::free(strs));

        while (true) {
            auto obj = next_arg(ctx, &arg);
            if (fe_type(ctx, obj) == FE_TNIL)
                break;
            str_array::append(strs, to_str(ctx, obj));
        }

        array_t<fmt_arg> args{};
        array::init(args, g_system.alloc);
        defer(array::free(args));

        for (u32 i = 0; i < strs.size; ++i)
            array::append(args, fmt::internal::make_arg<fmt_ctx>(strs[i].data));

        auto buf = g_system.buf;
        str::reset(buf); {
            str_buf_t str_buf{&buf};
            fmt::vformat_to(str_buf,
                            str::c_str(fmt_str),
                            fmt::basic_format_args<fmt_ctx>(args.data, args.size));
        }

        return str::c_str(g_system.buf);
    }

    static fe_Object* log_warn(fe_Context* ctx, fe_Object* arg) {
        scoped_lock_t lock(g_system.lock);
        log::warn(vlog(ctx, arg));
        return fe_bool(ctx, 0);
    }

    static fe_Object* log_info(fe_Context* ctx, fe_Object* arg) {
        scoped_lock_t lock(g_system.lock);
        log::info(vlog(ctx, arg));
        return fe_bool(ctx, 0);
    }

    static fe_Object* log_alert(fe_Context* ctx, fe_Object* arg) {
        scoped_lock_t lock(g_system.lock);
        log::alert(vlog(ctx, arg));
        return fe_bool(ctx, 0);
    }

    static fe_Object* log_debug(fe_Context* ctx, fe_Object* arg) {
        scoped_lock_t lock(g_system.lock);
        log::debug(vlog(ctx, arg));
        return fe_bool(ctx, 0);
    }

    static fe_Object* log_error(fe_Context* ctx, fe_Object* arg) {
        scoped_lock_t lock(g_system.lock);
        log::error(vlog(ctx, arg));
        return fe_bool(ctx, 0);
    }

    static fe_Object* log_notice(fe_Context* ctx, fe_Object* arg) {
        scoped_lock_t lock(g_system.lock);
        log::notice(vlog(ctx, arg));
        return fe_bool(ctx, 0);
    }

    static fe_Object* log_critical(fe_Context* ctx, fe_Object* arg) {
        scoped_lock_t lock(g_system.lock);
        log::critical(vlog(ctx, arg));
        return fe_bool(ctx, 0);
    }

    static fe_Object* log_emergency(fe_Context* ctx, fe_Object* arg) {
        scoped_lock_t lock(g_system.lock);
        log::emergency(vlog(ctx, arg));
        return fe_bool(ctx, 0);
    }

    // pattern: (string)
    // color-type: 'out, 'err
    // file-name: (string)
    // daily:
    //  hour, minute
    // rotating:
    //  max-size, max-files
    // name
    static fe_Object* log_create_color(fe_Context* ctx, fe_Object* arg) {
        arg_map_t args{};
        symtab::init(args, g_system.alloc);
        defer(symtab::free(args));
        auto pos_arg_n = make_arg_map(ctx, arg, args);

        spdlog_color_config_t config{};
        fe_Object* name;
        if (symtab::find(args, "#:name"_ss, name))
            config.name = string::interned::fold(to_str(ctx, name));
        else
            config.name = string::interned::fold("console"_ss);

        fe_Object* pattern;
        if (symtab::find(args, "#:pattern"_ss, pattern))
            config.pattern = string::interned::fold(to_str(ctx, pattern));

        auto color_type = to_str(ctx, get_map_arg(args, 0));
        if (color_type == "out") {
            config.color_type = spdlog_color_type_t::out;
        } else if (color_type == "err") {
            config.color_type = spdlog_color_type_t::err;
        } else {
            fe_error(ctx, "invalid color-type value; expected: 'out or 'err");
        }

        auto logger = log::system::make(logger_type_t::spdlog, &config);
        return fe_ptr(ctx, logger);
    }

    static fe_Object* log_create_daily_file(fe_Context* ctx, fe_Object* arg) {
        arg_map_t args{};
        symtab::init(args, g_system.alloc);
        defer(symtab::free(args));
        auto pos_arg_n = make_arg_map(ctx, arg, args);

        spdlog_daily_file_config_t config{};
        fe_Object* name;
        if (symtab::find(args, "#:name"_ss, name))
            config.name = string::interned::fold(to_str(ctx, name));
        else
            config.name = string::interned::fold("daily-file"_ss);

        fe_Object* pattern;
        if (symtab::find(args, "#:pattern"_ss, pattern))
            config.pattern = string::interned::fold(to_str(ctx, pattern));

        auto file_name = to_str(ctx, get_map_arg(args, 0));
        config.file_name = string::interned::fold(file_name);

        auto hour = get_map_arg(args, 1);
        if (fe_type(ctx, hour) != FE_TNUMBER)
            fe_error(ctx, "hour: expected number");
        config.hour = u32(fe_tonumber(ctx, hour));

        auto minute = get_map_arg(args, 1);
        if (fe_type(ctx, minute) != FE_TNUMBER)
            fe_error(ctx, "minute: expected number");
        config.minute = u32(fe_tonumber(ctx, minute));

        auto logger = log::system::make(logger_type_t::spdlog, &config);
        return fe_ptr(ctx, logger);
    }

    static fe_Object* log_create_basic_file(fe_Context* ctx, fe_Object* arg) {
        arg_map_t args{};
        symtab::init(args, g_system.alloc);
        defer(symtab::free(args));
        auto pos_arg_n = make_arg_map(ctx, arg, args);

        spdlog_basic_file_config_t config{};
        fe_Object* name;
        if (symtab::find(args, "#:name"_ss, name))
            config.name = string::interned::fold(to_str(ctx, name));
        else
            config.name = string::interned::fold("basic-file"_ss);

        fe_Object* pattern;
        if (symtab::find(args, "#:pattern"_ss, pattern))
            config.pattern = string::interned::fold(to_str(ctx, pattern));

        auto file_name = to_str(ctx, get_map_arg(args, 0));
        config.file_name = string::interned::fold(file_name);

        auto logger = log::system::make(logger_type_t::spdlog, &config);
        return fe_ptr(ctx, logger);
    }

    static fe_Object* log_create_rotating_file(fe_Context* ctx, fe_Object* arg) {
        arg_map_t args{};
        symtab::init(args, g_system.alloc);
        defer(symtab::free(args));
        auto pos_arg_n = make_arg_map(ctx, arg, args);

        spdlog_rotating_file_config_t config{};
        fe_Object* name;
        if (symtab::find(args, "#:name"_ss, name))
            config.name = string::interned::fold(to_str(ctx, name));
        else
            config.name = string::interned::fold("rotating-file"_ss);

        fe_Object* pattern;
        if (symtab::find(args, "#:pattern"_ss, pattern))
            config.pattern = string::interned::fold(to_str(ctx, pattern));

        auto file_name = to_str(ctx, get_map_arg(args, 0));
        config.file_name = string::interned::fold(file_name);

        auto max_size = get_map_arg(args, 1);
        if (fe_type(ctx, max_size) != FE_TNUMBER)
            fe_error(ctx, "max_size: expected number");
        config.max_size  = u32(fe_tonumber(ctx, max_size));

        auto max_files = get_map_arg(args, 2);
        if (fe_type(ctx, max_files)!= FE_TNUMBER)
            fe_error(ctx, "max_files: expected number");
        config.max_files = u32(fe_tonumber(ctx, max_files));

        auto logger = log::system::make(logger_type_t::spdlog, &config);
        return fe_ptr(ctx, logger);
    }

    struct named_flag_t final {
        const s8*               name;
        s32                     value;
    };

    static named_flag_t s_syslog_opts[] = {
        {"opt-pid",     opt_pid},
        {"opt-cons",    opt_cons},
        {"opt-odelay",  opt_odelay},
        {"opt-ndelay",  opt_ndelay},
        {"opt-nowait",  opt_nowait},
        {"opt-perror",  opt_perror},
        {nullptr,       0},
    };

    static named_flag_t s_syslog_facilities[] = {
        {"daemon",      facility_daemon},
        {"local0",      facility_local0},
        {"local1",      facility_local1},
        {"local2",      facility_local2},
        {"local3",      facility_local3},
        {"local4",      facility_local4},
        {"local5",      facility_local5},
        {"local6",      facility_local6},
        {"local7",      facility_local7},
        {nullptr,       0},
    };

    static s32 find_syslog_opt_value(const str_t& name) {
        for (u32 i = 0; s_syslog_opts[i].name != nullptr; ++i) {
            if (name == s_syslog_opts[i].name)
                return s_syslog_opts[i].value;
        }
        return -1;
    }

    static s32 find_syslog_facility_value(const str_t& name) {
        for (u32 i = 0; s_syslog_facilities[i].name != nullptr; ++i) {
            if (name == s_syslog_facilities[i].name) {
                return s_syslog_facilities[i].value;
            }
        }
        return -1;
    }

    static fe_Object* syslog_create(fe_Context* ctx, fe_Object* arg) {
        arg_map_t args{};
        symtab::init(args, g_system.alloc);
        defer(symtab::free(args));
        auto pos_arg_n = make_arg_map(ctx, arg, args);

        syslog_config_t config{};
        config.name = string::interned::fold("syslog"_ss);
        config.opts = {};
        auto ident_str = to_str(ctx, get_map_arg(args, 0));
        config.ident = (const s8*) string::interned::fold(ident_str).data;

        auto opts = get_map_arg(args, 1);
        if (fe_type(ctx, opts) != FE_TPAIR)
            fe_error(ctx, "opts argument must be a list");
        while (true) {
            auto obj = next_arg(ctx, &opts);
            auto type = fe_type(ctx, obj);
            if (type == FE_TNIL)
                break;
            else if (type != FE_TSYMBOL)
                fe_error(ctx, "syslog opts list may only contain symbols");
            auto opt_value = find_syslog_opt_value(to_str(ctx, obj));
            if (opt_value == -1)
                fe_error(ctx, "invalid syslog opt flag");
            config.opts |= opt_value;
        }

        auto facility = find_syslog_facility_value(to_str(ctx, get_map_arg(args, 2)));
        if (facility == -1)
            fe_error(ctx, "invalid syslog facility value");
        config.facility = facility;

        auto logger = log::system::make(logger_type_t::syslog, &config);
        return fe_ptr(ctx, logger);
    }

    static fe_Object* logger_append_child(fe_Context* ctx, fe_Object* arg) {
        auto parent = fe_nextarg(ctx, &arg);
        if (fe_type(ctx, parent) != FE_TPTR)
            fe_error(ctx, "parent: expected pointer argument");
        auto child = fe_nextarg(ctx, &arg);
        if (fe_type(ctx, child) != FE_TPTR)
            fe_error(ctx, "child: expected pointer argument");
        log::append_child((logger_t*) fe_toptr(ctx, parent), (logger_t*) fe_toptr(ctx, child));
        return fe_bool(ctx, 0);
    }

    static fe_Object* current_user(fe_Context* ctx, fe_Object* arg) {
        UNUSED(arg);
        return fe_ptr(ctx, context::top()->user);
    }

    static fe_Object* current_alloc(fe_Context* ctx, fe_Object* arg) {
        UNUSED(arg);
        return fe_ptr(ctx, context::top()->alloc);
    }

    static fe_Object* current_logger(fe_Context* ctx, fe_Object* arg) {
        UNUSED(arg);
        return fe_ptr(ctx, context::top()->logger);
    }

    static fe_Object* current_command_line(fe_Context* ctx, fe_Object* arg) {
        UNUSED(arg);
        const auto argc = context::top()->argc;
        const auto argv = context::top()->argv;
        fe_Object* objs[argc];
        for (u32 i = 0; i < argc; ++i)
            objs[i] = fe_string(ctx, argv[i]);
        return fe_list(ctx, &objs[0], argc);
    }

    static kernel_func_t s_kernel_funcs[] = {
        {"current-user",             current_user},
        {"current-alloc",            current_alloc},
        {"current-logger",           current_logger},
        {"current-command-line",     current_command_line},
        {"log-info",                 log_info},
        {"log-warn",                 log_warn},
        {"log-error",                log_error},
        {"log-alert",                log_alert},
        {"log-debug",                log_debug},
        {"log-notice",               log_notice},
        {"log-critical",             log_critical},
        {"log-emergency",            log_emergency},
        {"log-create-color",         log_create_color},
        {"log-create-daily-file",    log_create_daily_file},
        {"log-create-basic-file",    log_create_basic_file},
        {"log-create-rotating-file", log_create_rotating_file},
        {"logger-append-child",      logger_append_child},
        {"syslog-create",            syslog_create},
        {nullptr,                    nullptr}
    };

    namespace system {
        u0 fini() {
            fe_close(g_system.ctx);
            str::free(g_system.buf);
            mutex::free(g_system.lock);
            memory::free(g_system.alloc, g_system.ctx);
        }

        fe_Context* context() {
            return g_system.ctx;
        }

        status_t init(u32 heap_size, alloc_t* alloc) {
            g_system.alloc     = alloc;
            g_system.heap_size = heap_size;
            g_system.ctx       = (fe_Context*) memory::alloc(g_system.alloc, g_system.heap_size);
            str::init(g_system.buf, g_system.alloc);
            str::reserve(g_system.buf, 8192);
            mutex::init(g_system.lock);
            fe_open(g_system.ctx, g_system.heap_size);
            for (u32 i = 0; s_kernel_funcs[i].symbol != nullptr; ++i) {
                auto symbol = fe_symbol(g_system.ctx, s_kernel_funcs[i].symbol);
                auto func = fe_cfunc(g_system.ctx, s_kernel_funcs[i].func);
                fe_set(g_system.ctx, symbol, func);
            }
            return config::status_t::ok;
        }
    }

    status_t eval(const path_t& path, fe_Object** obj) {
        auto file = fopen(path::c_str(path), "r");
        if (!file) return status_t::bad_input;
        auto gc = fe_savegc(g_system.ctx);
        defer(
            fe_restoregc(g_system.ctx, gc);
            fclose(file)
        );
        while (true) {
            auto expr = fe_readfp(g_system.ctx, file);
            if (!expr) break;
            *obj = fe_eval(g_system.ctx, expr);
            fe_restoregc(g_system.ctx, gc);
        }
        return status_t::ok;
    }

    status_t eval(const u8* source, u32 len, fe_Object** obj) {
        auto gc = fe_savegc(g_system.ctx);
        defer(fe_restoregc(g_system.ctx, gc));
        auto file = ::fmemopen((u0*) source, len, "r");
        if (!file) return status_t::bad_input;
        defer(fclose(file));
        auto expr = fe_readfp(g_system.ctx, file);
        if (!expr) {
            *obj = fe_bool(g_system.ctx, 0);
            return status_t::ok;
        }
        *obj = fe_eval(g_system.ctx, expr);
        return status_t::ok;
    }
}

