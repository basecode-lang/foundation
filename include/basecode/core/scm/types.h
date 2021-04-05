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
#include <basecode/core/scm/scm.h>
#include <basecode/core/hashtab.h>
#include <basecode/core/str_array.h>
#include <basecode/core/stable_array.h>

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
    struct bb_t;
    struct vm_t;
    struct ctx_t;
    struct obj_t;
    struct env_t;
    struct var_t;
    struct proc_t;
    struct inst_t;
    struct operand_t;
    struct emitter_t;
    struct reg_result_t;
    struct encoded_inst_t;
    union  encoded_operand_t;

    using reg_t                 = u8;
    using trap_t                = b8 (*)(vm_t& vm, u64 arg);
    using label_t               = u32;
    using op_code_t             = u8;
    using bb_array_t            = stable_array_t<bb_t>;
    using ptr_array_t           = array_t<u0*>;
    using env_array_t           = array_t<env_t>;
    using label_map_t           = hashtab_t<u32, bb_t*>;
    using var_table_t           = symtab_t<var_t*>;
    using var_array_t           = stable_array_t<var_t>;
    using note_array_t          = array_t<u32>;
    using inst_array_t          = array_t<inst_t>;
    using proc_array_t          = array_t<proc_t>;
    using bind_table_t          = hashtab_t<u32, obj_t*>;
    using trap_table_t          = hashtab_t<u32, trap_t>;
    using symbol_table_t        = hashtab_t<u32, obj_t*>;
    using string_table_t        = hashtab_t<u32, obj_t*>;
    using comment_table_t       = hashtab_t<u32, note_array_t>;

    enum class bb_type_t : u8 {
        code,
        data,
        empty
    };

    enum class num_type_t : u8 {
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

    enum class memory_area_t : u8 {
        code,
        heap,
        env_stack,
        code_stack,
        data_stack,
    };

    enum class operand_type_t : u8 {
        none,
        reg,
        var,
        trap,
        value,
        label,
        block,
    };

    [[maybe_unused]] constexpr u32 max_memory_areas = 6;

    struct env_t final {
        obj_t*                  parent;
        bind_table_t            bindings;
        b8                      gc_protect;
    };

    struct proc_t final {
        env_t*                  e;
        obj_t*                  env;
        obj_t*                  body;
        obj_t*                  params;
        union {
            bb_t*               bb;
            u32                 abs;
        }                       addr;
        u8                      is_tco:         1;
        u8                      is_macro:       1;
        u8                      is_compiled:    1;
        u8                      is_assembled:   1;
        u8                      pad:            5;
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

    union encoded_operand_t final {
        struct {
            u64                 src:        32;
            u64                 dst:        6;
            u64                 aux:        8;
            u64                 pad:        5;
        }                       imm;
        struct {
            u64                 dst:        6;
            u64                 pad:        45;
        }                       reg1;
        struct {
            u64                 src:        6;
            u64                 dst:        6;
            u64                 aux:        32;
            u64                 pad:        7;
        }                       reg2;
        struct {
            u64                 a:          6;
            u64                 b:          6;
            u64                 imm:        32;
            u64                 pad:        7;
        }                       reg2_imm;
        struct {
            u64                 a:          6;
            u64                 b:          6;
            u64                 c:          6;
            u64                 pad:        33;
        }                       reg3;
        struct {
            u64                 a:          6;
            u64                 b:          6;
            u64                 c:          6;
            u64                 d:          6;
            u64                 pad:        27;
        }                       reg4;
        struct {
            u64                 offs:       32;
            u64                 src:        6;
            u64                 dst:        6;
            u64                 pad:        7;
        }                       offset;
        struct {
            u64                 offs:       32;
            u64                 base:       6;
            u64                 ndx:        6;
            u64                 dst:        6;
            u64                 pad:        1;
        }                       indexed;
    };

    struct encoded_inst_t final {
        u64                     type:       8;
        u64                     is_signed:  1;
        u64                     encoding:   4;
        u64                     data:       51;
    };
    static_assert(sizeof(encoded_inst_t) <= 8, "encoded_inst_t is now greater than 8 bytes!");

    struct var_t final {
        const var_t*            succ;
        const var_t*            pred;
        intern_id               symbol;
        u32                     version;
    };

    struct operand_t final {
        union {
            reg_t               reg;
            var_t*              var;
            s32                 s;
            u32                 u;
            bb_t*               bb;
            label_t             label;
        }                       kind;
        operand_type_t          type;
    };

    struct inst_t final {
        alloc_t*                alloc;
        operand_t               operands[4];
        s32                     aux;
        op_code_t               type;
        u8                      mode:       1;
        u8                      encoding:   4;
        u8                      is_signed:  1;
        u8                      pad:        2;
    };

    struct bb_t final {
        emitter_t*              emitter;
        bb_t*                   prev;
        bb_t*                   next;
        u64                     addr;
        note_array_t            notes;
        inst_array_t            insts;
        comment_table_t         comments;
        label_t                 label;
        u32                     id;
        bb_type_t               type;
    };

    struct reg_alloc_t final {
        u64                     slots;
        u64                     prots;
        reg_t                   start;
        reg_t                   end;
    };

    struct memory_map_entry_t final {
        u64                     addr;
        u32                     offs;
        u32                     size;
        u8                      reg;
        b8                      top;
        b8                      valid;
    };

    struct memory_map_t final {
        u32                     heap_size;
        s32                     reg_to_entry[32];
        memory_map_entry_t      entries[max_memory_areas];
    };

    struct flag_register_t final {
        u64                     n:      1;
        u64                     z:      1;
        u64                     c:      1;
        u64                     v:      1;
        u64                     i:      1;
        u64                     pad:    58;
    };

    struct vm_t final {
        alloc_t*                alloc;
        u64*                    heap;
        trap_table_t            traptab;
        memory_map_t            memory_map;
        b8                      exited;

        u64& operator[](u32 idx)        { return heap[idx]; }
        u64 operator[](u32 idx) const   { return heap[idx]; }
    };

    struct emitter_t final {
        alloc_t*                alloc;
        vm_t*                   vm;
        u64                     addr;
        bb_array_t              blocks;
        var_array_t             vars;
        var_table_t             vartab;
        str_array_t             strtab;
        label_map_t             labtab;
        reg_alloc_t             gp;
    };

    struct compiler_t final {
        vm_t*                   vm;
        emitter_t               emitter;
        reg_t                   prot[16];
        u32                     prot_count;
        reg_t                   ret_reg;
    };

    struct context_t final {
        bb_t*                   bb;
        ctx_t*                  ctx;
        obj_t*                  obj;
        obj_t*                  env;
        reg_t*                  target;
        label_t                 label;
        b8                      top_level;
    };

    struct compile_result_t final {
        bb_t*                   bb;
        reg_t                   reg;
        b8                      should_release;
    };

    struct ctx_t final {
        alloc_t*                alloc;
        vm_t                    vm;
        ffi_t                   ffi;
        compiler_t              compiler;
        handlers_t              handlers;
        obj_stack_t             gc_stack;
        obj_stack_t             cl_stack;
        proc_array_t            procedures;
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

FORMAT_TYPE(basecode::scm::var_t,
            format_to(ctx.out(),
                      "{}@{}",
                      *basecode::string::interned::get_slice(data.symbol),
                      data.version));

#pragma clang diagnostic pop
