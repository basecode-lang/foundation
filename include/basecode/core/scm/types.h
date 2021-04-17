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

#include <basecode/core/string.h>
#include <basecode/core/scm/scm.h>
#include <basecode/core/hashtab.h>
#include <basecode/core/digraph.h>
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
#define IS_SCM_ERROR(x)         ((x) && scm::obj_type_t((x)->hdr.type) == scm::obj_type_t::error)
#define TYPE(x)                 (scm::obj_type_t((x)->hdr.type))
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
#define ENV(x)                  ((env_t*) ctx->native_ptrs[FIXNUM((x)) - 1])
#define PROC(x)                 ((proc_t*) ctx->native_ptrs[FIXNUM((x)) - 1])
#define CFUNC(x)                ((native_func_t) ctx->native_ptrs[FIXNUM((x)) - 1])
#define PROTO(x)                ((proto_t*) ctx->native_ptrs[FIXNUM((x)) - 1])
#define NATIVE_PTR(x)           (ctx->native_ptrs[FIXNUM((x)) - 1])
#define EVAL(o)                 eval(ctx, (o))
#define EVAL_ARG()              eval(ctx, next_arg(ctx, &arg))
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
    struct comment_t;
    struct operand_t;
    struct emitter_t;
    struct mem_area_t;
    struct var_access_t;
    struct var_version_t;
    struct encoded_inst_t;
    union  encoded_operand_t;
    struct liveliness_range_t;

    using reg_t                 = u8;
    using trap_t                = b8 (*)(vm_t& vm, u64 arg);
    using op_code_t             = u8;
    using bb_array_t            = stable_array_t<bb_t>;
    using obj_stack_t           = stack_t<obj_t*>;
    using ptr_array_t           = array_t<u0*>;
    using env_array_t           = array_t<env_t>;
    using var_table_t           = symtab_t<var_t*>;
    using var_array_t           = stable_array_t<var_t>;
    using bb_digraph_t          = digraph_t<bb_t>;
    using inst_array_t          = array_t<inst_t>;
    using proc_array_t          = array_t<proc_t>;
    using bind_table_t          = hashtab_t<u32, obj_t*>;
    using trap_table_t          = hashtab_t<u32, trap_t>;
    using var_digraph_t         = digraph_t<var_version_t>;
    using symbol_table_t        = hashtab_t<u32, obj_t*>;
    using string_table_t        = hashtab_t<u32, obj_t*>;
    using access_array_t        = array_t<var_access_t>;
    using keyword_table_t       = hashtab_t<str::slice_t, obj_t*>;
    using comment_array_t       = array_t<comment_t>;
    using mem_area_array_t      = array_t<mem_area_t>;
    using interval_array_t      = array_t<liveliness_range_t*>;
    using liveliness_array_t    = stable_array_t<liveliness_range_t>;
    using var_version_array_t   = stable_array_t<var_version_t>;

    enum class status_t : u32 {
        ok                          = 0,
        fail                        = 300,
        error,
        unquote_invalid,
        unresolved_label,
        unknown_primitive,
        var_intern_failure,
        unquote_splicing_invalid,
        missing_sequential_branch,
    };

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
        is,
        gt,
        lt,
        gte,
        lte,
        add,
        sub,
        mul,
        div,
        mod,
        set,
        if_,
        or_,
        car,
        cdr,
        and_,
        not_,
        cons,
        eval,
        list,
        atom,
        begin,
        error,
        quote,
        while_,
        lambda,
        setcar,
        setcdr,
        define,
        unquote,
        quasiquote,
        define_macro,
        unquote_splicing,
        max,
    };

    enum class mem_area_type_t : u8 {
        heap,
        gc_stack,
        env_stack,
        code_stack,
        data_stack,
        register_file,
    };

    enum class comment_type_t : u8 {
        note,
        line,
        margin,
    };

    enum class operand_type_t : u8 {
        none,
        reg,
        var,
        trap,
        value,
        block,
    };

    enum class var_access_type_t : u8 {
        none,
        def,
        read,
        write
    };

    struct reg_pool_t final {
        u64                     slots;
        reg_t                   start;
        reg_t                   end;
        u32                     size;
        u32                     bit_count;
    };

    struct print_rule_t final {
        str::slice_t            symbol;
        u8                      skip_first_arg:     1;
        u8                      increase_indent:    1;
        u8                      pad:                6;
    };

    struct env_t final {
        obj_t*                  self;
        obj_t*                  parent;
        bind_table_t            bindings;
        u32                     native_ptr_idx;
        u8                      protect:    1;
        u8                      free:       1;
        u8                      pad:        6;
    };

    struct proc_t final {
        env_t*                  e;
        obj_t*                  env;
        obj_t*                  sym;
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

    struct var_access_t final {
        u32                     inst_id;
        var_access_type_t       type;
    };

    struct var_t final {
        str::slice_t            symbol;
        var_version_t*          first;
        var_version_t*          current;
    };

    struct var_version_t final {
        using Node_Type         = typename var_digraph_t::Node;

        var_t*                  var;
        Node_Type*              node;
        access_array_t          accesses;
        u32                     number;
        reg_t                   reg;
    };

    struct liveliness_range_t final {
        liveliness_range_t*     next;
        var_version_t*          version;
        u32                     start;
        u32                     end;
        u32                     id;
        reg_t                   reg;

        inline auto operator<=>(const liveliness_range_t& other) const {
            if (start < other.start)
                return std::strong_ordering::less;
            else if (start >= other.end)
                return std::strong_ordering::greater;
            else if (start == other.start
                 &&  end   == other.end) {
                return std::strong_ordering::equal;
            }
            return std::strong_ordering::equivalent;
        }
    };

    struct operand_t final {
        union {
            struct {
                bb_t*           bb;
                var_version_t*  args[4];
                u32             size;
            }                   branch;
            reg_t               reg;
            var_version_t*      var;
            s32                 s;
            u32                 u;
        }                       kind;
        operand_type_t          type;
    };

    struct inst_t final {
        alloc_t*                alloc;
        operand_t               operands[4];
        u32                     id;
        s32                     aux;
        u32                     block_id;
        op_code_t               type;
        u8                      encoding:   4;
        u8                      mode:       1;
        u8                      is_signed:  1;
        u8                      pad:        2;
    };

    struct comment_t final {
        u32                     id;
        u32                     line;
        u32                     block_id;
        comment_type_t          type;
    };

    struct bb_t final {
        using Node_Type         = typename bb_digraph_t::Node;

        emitter_t*              emit;
        Node_Type*              node;
        bb_t*                   prev;
        bb_t*                   next;
        u64                     addr;
        u32                     id;
        u32                     str_id;
        struct {
            u32                 sidx;
            u32                 eidx;
            inline u32 size() const     { return (eidx - sidx) + 1; }
        }                       notes;
        struct {
            u32                 sidx;
            u32                 eidx;
            inline u32 size() const     { return (eidx - sidx); }
        }                       insts;
        struct {
            var_version_t*      vars[4];
            u32                 size;
        }                       params;
        bb_type_t               type;
    };

    struct emitter_t final {
        alloc_t*                alloc;
        vm_t*                   vm;
        var_array_t             vars;
        bb_array_t              blocks;
        var_table_t             vartab;
        str_array_t             strtab;
        inst_array_t            insts;
        liveliness_array_t      ranges;
        comment_array_t         comments;
        bb_digraph_t            bb_graph;
        var_version_array_t     versions;
        var_digraph_t           var_graph;
        interval_array_t        intervals;
    };

    struct mem_area_t final {
        alloc_t*                alloc;
        vm_t*                   vm;
        u64*                    data;
        u64                     null;
        u32                     id;
        u32                     size;
        u32                     capacity;
        u32                     min_capacity;
        mem_area_type_t         type;
        reg_t                   reg;
        u8                      top:    1;
        u8                      pad:    7;

        u64& operator[](u32 idx) {
            return top ? (idx < capacity ? data[idx] : null) :
                   (idx < size ? data[idx] : null);
        }

        u64 operator[](u32 idx) const {
            return top ? (idx < capacity ? data[idx] : null) :
                   (idx < size ? data[idx] : null);
        }
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
        mem_area_t*             heap;
        mem_area_t*             reg_file;
        trap_table_t            traptab;
        mem_area_array_t        mem_map;
        u32                     area_by_reg[27];
        u8                      exited: 1;
        u8                      pad:    7;
    };

    struct compiler_t final {
        vm_t*                   vm;
        emitter_t               emit;
    };

    struct context_t final {
        bb_t*                   bb;
        ctx_t*                  ctx;
        obj_t*                  obj;
        obj_t*                  env;
        obj_t*                  sym;
        var_version_t*          target;
        b8                      is_macro;
        b8                      top_level;
    };

    struct compile_result_t final {
        bb_t*                   bb;
        var_version_t*          var;
        obj_t*                  obj;
        status_t                status;

        inline u32 u32() const  { return (uint32_t)status; }
    };

    struct ctx_t final {
        alloc_t*                alloc;
        mem_area_t*             gc_stack;
        mem_area_t*             env_stack;
        mem_area_t*             data_stack;
        vm_t                    vm;
        compiler_t              compiler;
        handlers_t              handlers;
        obj_stack_t             cl_stack;
        proc_array_t            procedures;
        env_array_t             environments;
        ptr_array_t             native_ptrs;
        string_table_t          strtab;
        symbol_table_t          symtab;
        obj_t*                  objects;
        obj_t*                  err;
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

FORMAT_TYPE(basecode::scm::bb_t,
            format_to(ctx.out(),
                        "{}_{}",
                        data.emit->strtab[data.str_id - 1],
                        data.id));

#pragma clang diagnostic pop
