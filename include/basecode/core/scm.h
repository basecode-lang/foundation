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
//
// Based on the *fe* scheme interpreter at https://github.com/rxi/fe
//
// Copyright (c) 2020 rxi
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the MIT license.
//
// ----------------------------------------------------------------------------

#pragma once

#include <basecode/core/ffi.h>
#include <basecode/core/buf.h>

namespace basecode::scm {
    struct obj_t;
    struct ctx_t;

    using fixnum_t              = u32;
    using flonum_t              = f32;

    using obj_stack_t           = stack_t<obj_t*>;
    using error_func_t          = u0 (*)(ctx_t*, const s8*, obj_t*);
    using native_func_t         = obj_t* (*)(ctx_t*, obj_t*);

    struct handlers_t final {
        error_func_t            error;
        native_func_t           mark;
        native_func_t           gc;
    };

    struct printable_t final {
        ctx_t*                  ctx;
        obj_t*                  obj;
        b8                      quote;
    };

    enum class ffi_type_t : u8 {
        none                    = 0,
        list                    = 'L',
        string                  = 'S',
        object                  = 'O',
        context                 = 'C',
        boolean                 = 'B',
    };

    enum class obj_type_t : u32 {
        pair,
        free,
        nil,
        fixnum,
        flonum,
        symbol,
        string,
        func,
        macro,
        prim,
        cfunc,
        ptr,
        boolean,
        keyword,
        ffi,
        error,
    };

    u0 free(ctx_t* ctx);

    obj_t* nil(ctx_t* ctx);

    u32 save_gc(ctx_t* ctx);

    obj_t* pop_gc(ctx_t* ctx);

    obj_type_t type(obj_t* obj);

    fixnum_t to_fixnum(obj_t* obj);

    flonum_t to_flonum(obj_t* obj);

    u0 collect_garbage(ctx_t* ctx);

    const s8* type_name(obj_t* obj);

    u0 mark(ctx_t* ctx, obj_t* obj);

    handlers_t* handlers(ctx_t* ctx);

    b8 is_nil(ctx_t* ctx, obj_t* obj);

    b8 is_true(ctx_t* ctx, obj_t* obj);

    u32 length(ctx_t* ctx, obj_t* obj);

    u0 push_gc(ctx_t* ctx, obj_t* obj);

    u0 restore_gc(ctx_t* ctx, u32 idx);

    obj_t* car(ctx_t* ctx, obj_t* obj);

    obj_t* cdr(ctx_t* ctx, obj_t* obj);

    obj_t* eval(ctx_t* ctx, obj_t* obj);

    obj_t* make_bool(ctx_t* ctx, b8 value);

    u0* to_user_ptr(ctx_t* ctx, obj_t* obj);

    obj_t* make_bool(ctx_t* ctx, obj_t* obj);

    u0 set(ctx_t* ctx, obj_t* sym, obj_t* v);

    obj_t* next_arg(ctx_t* ctx, obj_t** arg);

    obj_t* read(ctx_t* ctx, buf_crsr_t& crsr);

    obj_t* error_args(ctx_t* ctx, obj_t* obj);

    obj_t* make_user_ptr(ctx_t* ctx, u0* ptr);

    obj_t* make_fixnum(ctx_t* ctx, fixnum_t n);

    obj_t* make_flonum(ctx_t* ctx, flonum_t n);

    obj_t* make_ffi(ctx_t* ctx, proto_t* proto);

    u0 write(FILE* file, ctx_t* ctx, obj_t* obj);

    u0 print(FILE* file, ctx_t* ctx, obj_t* obj);

    str::slice_t to_string(ctx_t* ctx, obj_t* obj);

    obj_t* get(ctx_t* ctx, obj_t* sym, obj_t* env);

    obj_t* next_arg_no_chk(ctx_t* ctx, obj_t** arg);

    obj_t* cons(ctx_t* ctx, obj_t* car, obj_t* cdr);

    u0 write(fmt_buf_t& buf, ctx_t* ctx, obj_t* obj);

    u0 print(fmt_buf_t& buf, ctx_t* ctx, obj_t* obj);

    obj_t* make_list(ctx_t* ctx, obj_t** objs, u32 size);

    obj_t* make_native_func(ctx_t* ctx, native_func_t fn);

    u0 fmt_error(ctx_t* ctx, fmt_str_t fmt_msg, fmt_args_t args);

    obj_t* make_symbol(ctx_t* ctx, const s8* name, s32 len = -1);

    obj_t* make_keyword(ctx_t* ctx, const s8* name, s32 len = -1);

    obj_t* make_error(ctx_t* ctx, obj_t* args, obj_t* call_stack);

    template <typename... Args>
    u0 error(ctx_t* ctx, fmt_str_t fmt_msg, const Args&... args) {
        fmt_error(ctx, fmt_msg, fmt::make_format_args(args...));
    }

    obj_t* find_string(ctx_t* ctx, const s8* str, u32& id, s32 len = -1);

    obj_t* find_symbol(ctx_t* ctx, const s8* name, u32& id, s32 len = -1);

    ctx_t* init(u0* ptr, u32 size, alloc_t* alloc = context::top()->alloc);

    obj_t* make_string(ctx_t* ctx, const s8* str, s32 len = -1, u32 id = {});
}

FORMAT_TYPE(
    basecode::scm::printable_t,
    {
        using namespace basecode;
        using namespace basecode::scm;

        auto o = ctx.out();
        switch (type(data.obj)) {
            case obj_type_t::nil:
                format_to(o, "nil");
                break;

            case obj_type_t::pair: {
                obj_t* p = data.obj;
                format_to(o, "(");
                for (;;) {
                    format_to(o, "{}", printable_t{data.ctx, car(data.ctx, p), true});
                    p = cdr(data.ctx, p);
                    if (type(p) != obj_type_t::pair)
                        break;
                    format_to(o, " ");
                }
                if (!is_nil(data.ctx, p))
                    format_to(o, " . {}", printable_t{data.ctx, p, true});
                format_to(o, ")");
                break;
            }

            case obj_type_t::fixnum:
                format_to(o, "{}", to_fixnum(data.obj));
                break;

            case obj_type_t::flonum:
                format_to(o, "{:.7g}", to_flonum(data.obj));
                break;

            case obj_type_t::error: {
                auto err_args   = error_args(data.ctx, data.obj);
                auto args       = car(data.ctx, err_args);
                auto call_stack = car(data.ctx, cdr(data.ctx, err_args));
                format_to(o, "error: ");
                for (; !is_nil(data.ctx, args); args = cdr(data.ctx, args)) {
                    auto expr = eval(data.ctx, car(data.ctx, args));
                    format_to(o, "{} ", printable_t{data.ctx, expr});
                }
                format_to(o, "\n");
                for (; !is_nil(data.ctx, call_stack); call_stack = cdr(data.ctx, call_stack))
                    format_to(o, "=> {}\n", printable_t{data.ctx, car(data.ctx, call_stack)});
                break;
            }

            case obj_type_t::symbol:
            case obj_type_t::keyword:
                format_to(o, "{}", to_string(data.ctx, data.obj));
                break;

            case obj_type_t::string:
                if (data.quote)
                    format_to(o, "\"{}\"", to_string(data.ctx, data.obj));
                else
                    format_to(o, "{}", to_string(data.ctx, data.obj));
                break;

            case obj_type_t::boolean:
                format_to(o, "{}", to_string(data.ctx, data.obj));
                break;

            default:
                format_to(o, "[{} {}]", type_name(data.obj), (u0*) data.obj);
                break;
        }
    });
