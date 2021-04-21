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

#include <basecode/core/string.h>
#include <basecode/core/filesys.h>
#include <basecode/core/obj_pool.h>
#include <basecode/core/scm/types.h>
#include <basecode/core/scm/system.h>
#include <basecode/core/scm/modules/basic.h>

namespace basecode::scm::module::basic {
    struct system_t final {
        scm::ctx_t*             ctx;
        alloc_t*                alloc;
        obj_t*                  current_user;
        obj_t*                  current_alloc;
        obj_t*                  current_logger;
        obj_t*                  current_command_line;
        obj_pool_t              storage;
    };

    system_t                    g_basic_sys;

    b8 adjust_load_path(path_t& path, str::slice_t file_name);

    u32 length(obj_t* obj) {
        return scm::length(g_basic_sys.ctx, obj);
    }

    obj_t* intern_get(u32 id) {
        auto s = string::interned::get_slice(id);
        return s ? make_string(g_basic_sys.ctx, *s) : nil(g_basic_sys.ctx);
    }

    obj_t* expand(obj_t* obj) {
        auto ctx = g_basic_sys.ctx;
        auto fn = CAR(obj);
        if (TYPE(fn) == obj_type_t::symbol)
            fn = get(ctx, fn);
        if (TYPE(fn) != obj_type_t::proc)
            return ctx->nil;
        auto args = CDR(obj);
        auto proc = PROC(fn);
        if (!proc->is_macro)
            return ctx->nil;
        push_env(ctx, make_environment(ctx, top_env(ctx)));
        args_to_env(ctx, proc->params, args);
        auto res  = ctx->nil;
        auto body = proc->body;
        auto save = save_gc(ctx);
        while (!IS_NIL(body)) {
            restore_gc(ctx, save);
            push_gc(ctx, body);
            push_gc(ctx, top_env(ctx));
            res = eval(ctx, CAR(body));
            body = CDR(body);
        }
        finalize_environment(ctx, pop_env(ctx));
        return res;
    }

    obj_t* reverse(obj_t* lst) {
        auto ctx = g_basic_sys.ctx;
        obj_t* res;
        for (res = ctx->nil; !IS_NIL(lst); lst = CDR(lst))
            res = cons(ctx, CAR(lst), res);
        return res;
    }

    scm::obj_t* current_user() {
        auto ctx = g_basic_sys.ctx;
        if (IS_NIL(g_basic_sys.current_user)) {
            g_basic_sys.current_user = scm::make_user_ptr(
                ctx,
                context::top()->user);
        }
        return g_basic_sys.current_user;
    }

    obj_t* current_environment() {
        return g_basic_sys.ctx->env;
    }

    scm::obj_t* current_alloc() {
        auto ctx = g_basic_sys.ctx;
        if (IS_NIL(g_basic_sys.current_alloc)) {
            g_basic_sys.current_alloc = scm::make_user_ptr(
                ctx,
                context::top()->alloc.main);
        }
        return g_basic_sys.current_alloc;
    }

    u0 print(rest_array_t* rest) {
        auto ctx = g_basic_sys.ctx;
        const auto& lst = *rest;
        for (u32 i = 0; i < lst.size; ++i) {
            if (i > 0)
                format::print(" ");
            format::print("{}", printable_t{ctx, lst[i]});
        }
        if (lst.size > 0)
            format::print("\n");
    }

    scm::obj_t* current_logger() {
        auto ctx = g_basic_sys.ctx;
        if (scm::is_nil(ctx, g_basic_sys.current_logger)) {
            g_basic_sys.current_logger = scm::make_user_ptr(
                ctx,
                context::top()->logger);
        }
        return g_basic_sys.current_logger;
    }

    obj_t* load(rest_array_t* rest) {
        obj_t* obj{};
        path_t load_path{};
        defer(path::free(load_path));
        for (auto path : *rest) {
            check_type(g_basic_sys.ctx, path, obj_type_t::string);
            const auto& file_name = *string::interned::get_slice(STRING_ID(path));
            if (!adjust_load_path(load_path, file_name)) {
                // XXX:
                return error(g_basic_sys.ctx,
                             "load cannot resolve file path: {}",
                             file_name);
            }
            auto status = scm::system::eval(load_path, &obj);
            if (!OK(status))
                return obj;
        }
        return obj ? obj : g_basic_sys.ctx->nil;
    }

    obj_t* fixnum_to_flonum(u32 num) {
        return make_flonum(g_basic_sys.ctx, flonum_t(num));
    }

    obj_t* flonum_to_fixnum(f32 num) {
        return make_fixnum(g_basic_sys.ctx, fixnum_t(num));
    }

    obj_t* append(rest_array_t* rest) {
        auto ctx = g_basic_sys.ctx;
        obj_t* head = ctx->nil;
        obj_t* tail = head;
        for (auto lst : *rest) {
            if (IS_NIL(lst))
                continue;
            if (is_list(ctx, lst)) {
                while (!IS_NIL(lst)) {
                    auto r = CONS1(CAR(lst));
                    if (IS_NIL(tail)) {
                        head = r;
                        tail = head;
                    } else {
                        SET_CDR(tail, r);
                        tail = r;
                    }
                    lst = CDR(lst);
                }
            } else {
                auto r = CONS1(lst);
                if (IS_NIL(tail)) {
                    head = r;
                    tail = head;
                } else {
                    SET_CDR(tail, r);
                    tail = r;
                }
            }
        }
        return head;
    }

    scm::obj_t* current_command_line() {
        auto ctx = g_basic_sys.ctx;
        if (IS_NIL(g_basic_sys.current_command_line)) {
            const auto argc = context::top()->argc;
            const auto argv = context::top()->argv;
            scm::obj_t* objs[argc];
            for (u32 i = 0; i < argc; ++i)
                objs[i] = scm::make_string(ctx, argv[i]);
            g_basic_sys.current_command_line = scm::make_list(ctx,
                                                              &objs[0],
                                                              argc);
        }
        return g_basic_sys.current_command_line;
    }

    obj_t* parent_environment(obj_t* env) {
        auto ctx = g_basic_sys.ctx;
        check_type(ctx, env, obj_type_t::environment);
        auto e = ENV(env);
        return e->parent;
    }

    b8 adjust_load_path(path_t& path, str::slice_t file_name) {
        path_t tmp_path{};
        path::init(tmp_path, file_name, g_basic_sys.alloc);
        defer(path::free(tmp_path));
        if (!path::absolute(tmp_path)) {
            auto eval_path = scm::system::current_eval_path();
            if (eval_path) {
                path::init(path, eval_path->str);
                path::parent_path(path, path);
                path::append(path, tmp_path);
                if (!OK(filesys::mkabs(path, path)))
                    return false;
            } else {
                if (!OK(filesys::bin_rel_path(path, tmp_path)))
                    return false;
            }
            if (!OK(filesys::exists(path)))
                return false;
        } else {
            path::init(path, file_name, g_basic_sys.alloc);
        }
        return true;
    }

    obj_t* format(str::slice_t* fmt_str, b8 quote, rest_array_t* rest) {
        auto ctx = g_basic_sys.ctx;
        str_t buf{};
        str::init(buf, ctx->alloc);  {
            str_buf_t str_buf{&buf};
            fmt::dynamic_format_arg_store<fmt::format_context> fmt_args{};
            for (auto arg : *rest) {
                auto va = EVAL(arg);
                switch (TYPE(va)) {
                    case obj_type_t::pair:
                        fmt_args.push_back(scm::printable_t{ctx, va, quote});
                        break;
                    case obj_type_t::nil:
                        fmt_args.push_back("nil");
                        break;
                    case obj_type_t::fixnum:
                        fmt_args.push_back(FIXNUM(va));
                        break;
                    case obj_type_t::flonum:
                        fmt_args.push_back(FLONUM(va));
                        break;
                    case obj_type_t::keyword:
                        fmt_args.push_back(format::format(
                            "#:{}",
                            *string::interned::get_slice(STRING_ID(va))));
                        break;
                    case obj_type_t::symbol:
                        fmt_args.push_back(*string::interned::get_slice(STRING_ID(va)));
                        break;
                    case obj_type_t::string:
                        if (quote) {
                            fmt_args.push_back(format::format(
                                "\"{}\"",
                                *string::interned::get_slice(STRING_ID(va))));
                        } else {
                            fmt_args.push_back(*string::interned::get_slice(STRING_ID(va)));
                        }
                        break;
                    case obj_type_t::ffi: {
                        auto proto = PROTO(va);
                        fmt_args.push_back(format::format(
                            "[ffi: {}/{} overloads]",
                            proto->name,
                            proto->overloads.size));
                        break;
                    }
                    case obj_type_t::prim:
                        fmt_args.push_back(format::format(
                            "[prim: {}]",
                            scm::prim_name(va)));
                        break;
                    case obj_type_t::proc: {
                        auto proc = PROC(va);
                        fmt_args.push_back(format::format(
                            "(lambda {}\n    {})",
                            printable_t{ctx, proc->params, true},
                            printable_t{ctx, proc->body, true}));
                        break;
                    }
                    case obj_type_t::cfunc:
                        fmt_args.push_back(format::format(
                            "[cfunc: {}]",
                            (u0*) NATIVE_PTR(va)));
                        break;
                    case obj_type_t::ptr:
                        fmt_args.push_back(format::format(
                            "[user ptr: {}]",
                            (u0*) NATIVE_PTR(va)));
                        break;
                    case obj_type_t::boolean:
                        fmt_args.push_back(IS_TRUE(va) ? true : false);
                        break;
                    case obj_type_t::error:
                        break;
                    case obj_type_t::environment:
                        break;
                    default:
                        break;
                }
            }
            try {
                fmt::vformat_to(str_buf, (std::string_view) *fmt_str, fmt_args);
            } catch (std::runtime_error& ex) {
                return error(ctx, ex.what());
            }
        }
        return make_string(ctx, (const s8*) buf.data, buf.length);
    }

#define EXPORTS EXPORT_PROC("load",                                             \
                    OVERLOAD(load, obj_ptr,                                     \
                        REST("rest", obj_ptr)))                                 \
                EXPORT_PROC("fl->fx",                                           \
                    OVERLOAD(flonum_to_fixnum, obj_ptr,                         \
                        REQ("num", f32)))                                       \
                EXPORT_PROC("fx->fl",                                           \
                    OVERLOAD(fixnum_to_flonum, obj_ptr,                         \
                        REQ("num", u32)))                                       \
                EXPORT_PROC("expand",                                           \
                    OVERLOAD(expand, obj_ptr,                                   \
                        REQ("obj", obj_ptr)))                                   \
                EXPORT_PROC("print",                                            \
                    OVERLOAD(print, u0,                                         \
                        REST("rest", obj_ptr)))                                 \
                EXPORT_PROC("format",                                           \
                    OVERLOAD(format, obj_ptr,                                   \
                        REQ("fmt_str", slice_ptr),                              \
                        OPT("quote", b8, false),                                \
                        REST("rest", obj_ptr)))                                 \
                EXPORT_PROC("append",                                           \
                    OVERLOAD(append, obj_ptr,                                   \
                        REST("rest", obj_ptr)))                                 \
                EXPORT_PROC("length",                                           \
                    OVERLOAD(length, u32,                                       \
                        REQ("obj", obj_ptr)))                                   \
                EXPORT_PROC("reverse",                                          \
                    OVERLOAD(reverse, obj_ptr,                                  \
                        REQ("lst", list_ptr)))                                  \
                EXPORT_PROC("intern/get",                                       \
                    OVERLOAD(intern_get, obj_ptr,                               \
                        REQ("id", u32)))                                        \
                EXPORT_PROC("current-environment",                              \
                    OVERLOAD(current_environment, obj_ptr))                     \
                EXPORT_PROC("parent-environment",                               \
                    OVERLOAD(parent_environment, obj_ptr,                       \
                        REQ("env", obj_ptr)))                                   \
                EXPORT_PROC("current-user",                                     \
                    OVERLOAD(current_user, obj_ptr))                            \
                EXPORT_PROC("current-alloc",                                    \
                    OVERLOAD(current_alloc, obj_ptr))                           \
                EXPORT_PROC("current-logger",                                   \
                    OVERLOAD(current_logger, obj_ptr))                          \
                EXPORT_PROC("current-command-line",                             \
                    OVERLOAD(current_command_line, obj_ptr))

namespace system {
        namespace exports {
            using namespace scm::kernel;

            static proc_export_t s_exports[] = {
#define EXPORT_PROC(n, ...)    basecode::scm::kernel::proc_export_t{    \
    n##_ss, \
    u32(VA_COUNT(__VA_ARGS__)), \
    __VA_ARGS__},
                EXPORTS
#undef EXPORT_PROC
                {str::slice_t{}},
            };
        }

        u0 fini() {
            obj_pool::free(g_basic_sys.storage);
        }

        status_t init(scm::ctx_t* ctx, alloc_t* alloc) {
            g_basic_sys.ctx   = ctx;
            g_basic_sys.alloc = alloc;

            g_basic_sys.current_user         = scm::nil(g_basic_sys.ctx);
            g_basic_sys.current_alloc        = scm::nil(g_basic_sys.ctx);
            g_basic_sys.current_logger       = scm::nil(g_basic_sys.ctx);
            g_basic_sys.current_command_line = scm::nil(g_basic_sys.ctx);

            obj_pool::init(g_basic_sys.storage, g_basic_sys.alloc);
            kernel::create_exports(g_basic_sys.ctx, exports::s_exports);

            return status_t::ok;
        }
    }
}
