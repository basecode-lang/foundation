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

#pragma once

#pragma clang diagnostic push
#pragma ide diagnostic ignored "altera-struct-pack-align"

#include <bit>
#include <basecode/core/string.h>
#include <basecode/core/hashtab.h>
#include <basecode/core/scm/scm.h>

#define OBJ_AT(idx)             (&ctx->objects[(idx)])
#define OBJ_IDX(x)              ((x) - ctx->objects)
#define CAR(x)                  (&ctx->objects[(x)->pair.car_idx])
#define CDR(x)                  (&ctx->objects[(x)->pair.cdr_idx])
#define CAAR(x)                 CAR(CAR(x))
#define CADR(x)                 CAR(CDR(x))
#define CDDR(x)                 CDR(CDR(x))
#define CADDR(x)                CAR(CDR(CDR(x)))
#define SET_CAR(x, o)           ((x)->pair.car_idx = OBJ_IDX(o))
#define SET_CDR(x, o)           ((x)->pair.cdr_idx = OBJ_IDX(o))
#define IS_NIL(x)               ((x) == ctx->nil)
#define TYPE(x)                 (obj_type_t((x)->hdr.type))
#define IS_TRUE(x)              (!IS_NIL(x) && (x) == ctx->true_)
#define IS_FALSE(x)             (IS_NIL(x)  || (x) == ctx->false_)
#define IS_GC_MARKED(x)         ((x)->hdr.gc_mark)
#define SET_GC_MARK(x, v)       ((x)->hdr.gc_mark = (v))
#define SET_TYPE(x, t)          ((x)->hdr.type = u8((t)))
#define FIXNUM(x)               ((x)->number.value)
#define FLONUM(x)               (std::bit_cast<f32>(u32((x)->number.value)))
#define STRING_ID(x)            (FIXNUM((x)))
#define SET_FIXNUM(x, v)        ((x)->number.value = (v))
#define SET_FLONUM(x, v)        ((x)->number.value = std::bit_cast<u32>(f32(v)))
#define PRIM(x)                 (prim_type_t((x)->prim.code))
#define SET_PRIM(x, p)          ((x)->prim.code = fixnum_t((p)))
#define NATIVE_PTR(x)           (ctx->native_ptrs[FIXNUM((x))])
#define EVAL(o)                 eval(ctx, (o), env)
#define EVAL_ARG()              eval(ctx, next_arg(ctx, &arg), env)
#define SYM(o)                  make_symbol(ctx, (o))
#define CONS1(a)                cons(ctx, (a), ctx->nil)
#define CONS(a, d)              cons(ctx, (a), (d))
#define PRINT(h, o)             SAFE_SCOPE( \
    str_t str{};                            \
    str::init(str, ctx->alloc);             \
    {                                       \
        str_buf_t _buf{&str};                \
        format::format_to(_buf, "{}{}\n", (h), printable_t{ctx, (o), true});\
    }                                       \
    format::print(stdout, "{}", str);\
    fflush(stdout);)

namespace basecode::scm {
    struct env_t;

    using ptr_array_t           = array_t<u0*>;
    using env_array_t           = array_t<env_t>;
    using bind_table_t          = hashtab_t<u32, obj_t*>;
    using symbol_table_t        = hashtab_t<u32, obj_t*>;
    using string_table_t        = hashtab_t<u32, obj_t*>;

    enum class numtype_t : u8 {
        none,
        fixnum,
        flonum,
    };

    enum class prim_type_t : u8 {
        eval,
        let,
        set,
        if_,
        fn,
        mac,
        while_,
        error,
        quote,
        unquote,
        quasiquote,
        unquote_splicing,
        and_,
        or_,
        do_,
        cons,
        car,
        cdr,
        setcar,
        setcdr,
        list,
        not_,
        is,
        atom,
        print,
        gt,
        gte,
        lt,
        lte,
        add,
        sub,
        mul,
        div,
        mod,
        max,
    };

    struct env_t final {
        obj_t*                  params;
        obj_t*                  parent;
        bind_table_t            bindings;
        b8                      gc_protect;
    };

    struct obj_t final {
        union {
            struct {
                u64             type:       6;
                u64             gc_mark:    1;
                u64             pad:        57;
            }                   hdr;
            struct {
                u64             type:       6;
                u64             gc_mark:    1;
                u64             code:       32;
                u64             pad:        25;
            }                   prim;
            struct {
                u64             type:       6;
                u64             gc_mark:    1;
                u64             car_idx:    28;
                u64             cdr_idx:    28;
                u64             pad:        1;
            }                   pair;
            struct {
                u64             type:       6;
                u64             gc_mark:    1;
                u64             value:      32;
                u64             pad:        25;
            }                   number;
            u64                 full;
        };
    };
    static_assert(sizeof(obj_t) == 8, "obj_t is no longer 8 bytes!");

    struct ctx_t final {
        alloc_t*                alloc;
        ffi_t                   ffi;
        handlers_t              handlers;
        obj_stack_t             gc_stack;
        obj_stack_t             cl_stack;
        env_array_t             environments;
        ptr_array_t             native_ptrs;
        string_table_t          strtab;
        symbol_table_t          symtab;
        obj_t*                  objects;
        obj_t*                  env;
        obj_t*                  nil;
        obj_t*                  dot;
        obj_t*                  true_;
        obj_t*                  false_;
        obj_t*                  rbrac;
        obj_t*                  rparen;
        obj_t*                  call_list;
        obj_t*                  free_list;
        u32                     object_used;
        u32                     object_count;
    };

    struct ffi_type_map_t final {
        u8                      type;
        u8                      size;
    } __attribute__((aligned(2)));
}

#pragma clang diagnostic pop
