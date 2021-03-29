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

namespace basecode::scm {
    struct obj_t;
    struct ctx_t;

    using number_t              = f32;

    using read_func_t           = s8 (*)(ctx_t*, u0*);
    using error_func_t          = u0 (*)(ctx_t*, const s8*, obj_t*);
    using write_func_t          = u0 (*)(ctx_t*, u0*, s8);
    using native_func_t         = obj_t* (*)(ctx_t*, obj_t*);

    struct handlers_t {
        error_func_t            error;
        native_func_t           mark;
        native_func_t           gc;
    };

    enum class ffi_type_t : s32 {
        none                    = 0,
        object                  = 1,
        context                 = 2,
        boolean                 = 3,
    };

    enum class obj_type_t : u32 {
        pair,
        free,
        nil,
        number,
        symbol,
        string,
        func,
        macro,
        prim,
        cfunc,
        ptr,
        keyword,
        ffi
    };

    u0 free(ctx_t* ctx);

    obj_t* nil(ctx_t* ctx);

    u32 save_gc(ctx_t* ctx);

    obj_t* pop_gc(ctx_t* ctx);

    u0 collect_garbage(ctx_t* ctx);

    ctx_t* init(u0* ptr, u32 size);

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

    u0 error(ctx_t* ctx, const s8* msg);

    obj_t* read_fp(ctx_t* ctx, FILE* fp);

    obj_t* make_bool(ctx_t* ctx, b8 value);

    u32 to_integer(ctx_t* ctx, obj_t* obj);

    u0* to_user_ptr(ctx_t* ctx, obj_t* obj);

    obj_type_t type(ctx_t* ctx, obj_t* obj);

    u0 set(ctx_t* ctx, obj_t* sym, obj_t* v);

    obj_t* next_arg(ctx_t* ctx, obj_t** arg);

    obj_t* make_user_ptr(ctx_t* ctx, u0* ptr);

    number_t to_number(ctx_t* ctx, obj_t* obj);

    obj_t* make_number(ctx_t* ctx, number_t n);

    obj_t* make_ffi(ctx_t* ctx, proto_t* proto);

    u32 write_fp(ctx_t* ctx, obj_t* obj, FILE* fp);

    obj_t* get(ctx_t* ctx, obj_t* sym, obj_t* env);

    obj_t* next_arg_no_chk(ctx_t* ctx, obj_t** arg);

    obj_t* cons(ctx_t* ctx, obj_t* car, obj_t* cdr);

    obj_t* read(ctx_t* ctx, read_func_t fn, u0* udata);

    obj_t* make_list(ctx_t* ctx, obj_t** objs, u32 size);

    obj_t* make_native_func(ctx_t* ctx, native_func_t fn);

    u32 to_string(ctx_t* ctx, obj_t* obj, s8* dst, u32 size);

    obj_t* make_string(ctx_t* ctx, const s8* str, s32 len = -1);

    obj_t* find_symbol(ctx_t* ctx, const s8* name, s32 len = -1);

    obj_t* make_symbol(ctx_t* ctx, const s8* name, s32 len = -1);

    obj_t* make_keyword(ctx_t* ctx, const s8* name, s32 len = -1);

    u32 write(ctx_t* ctx, obj_t* obj, write_func_t fn, u0* udata, u32 qt);
}
