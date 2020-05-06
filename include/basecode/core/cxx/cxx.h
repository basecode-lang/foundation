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

#include <basecode/core/bass.h>
#include <basecode/core/types.h>
#include <basecode/core/stack.h>
#include <basecode/core/array.h>
#include <basecode/core/intern.h>
#include <basecode/core/format.h>
#include <basecode/core/symtab.h>

#define MAKE_TYPE(m, d) ((((u32) m) << (u32) 8) | ((u32) d))
#define POSTFIX(x)      (((u32)x) | (u32) 0b100000000000000000000000)
#define PREFIX(x)       (((u32)x) | (u32) 0b010000000000000000000000)
#define BASE_TYPE(x)    ((((u32) x) & (u32) 0b1111111100000000) >> (u32) 8)
#define SUB_TYPE(x)     (((u32)  x) & (u32) 0b0000000011111111)
#define POS_TYPE(x)     ((((u32) x) & (u32) 0b110000000000000000000000) >> (u32) 22)

namespace basecode::cxx {
    struct scope_t;
    struct module_t;
    struct program_t;

    using id_array_t        = array_t<u32>;
    using scope_array_t     = array_t<scope_t>;
    using scope_stack_t     = stack_t<u32>;
    using module_array_t    = array_t<module_t>;

    enum class status_t : u8 {
        ok,
        error,
        lhs_not_found,
        rhs_not_found,
        pgm_not_found,
        list_not_found,
        scope_not_found,
        label_not_found,
        child_not_found,
        invalid_pp_type,
        intern_not_found,
        invalid_revision,
        invalid_pos_type,
        element_not_found,
        invalid_meta_type,
        invalid_list_entry,
        invalid_def_element,
        invalid_decl_element,
        invalid_expr_element,
        unsupported_revision,
        not_implemented,
    };

    enum class revision_t : u8 {
        c99                 = 1,
        c11,
        cpp17,
        cpp20,
    };

    enum class meta_type_t : u8 {
        none                = 1,
        alias,
        void_,
        array,
        boolean,
        pointer,
        bit_mask,
        function,
        reference,
        aggregate,
        signed_integer,
        floating_point,
        unsigned_integer,
    };

    enum class aggregate_type_t : u8 {
        enum_               = 1,
        union_,
        class_,
        struct_,
        enum_class,
    };

    enum class integral_size_t : u8 {
        zero                = 1,
        byte,
        word,
        dword,
        qword,
    };

    enum class statement_type_t : u8 {
        empty               = 1,
        pp,
        raw,
        if_,
        do_,
        for_,
        expr,
        decl,
        case_,
        goto_,
        break_,
        while_,
        using_,
        return_,
        public_,
        switch_,
        default_,
        private_,
        continue_,
        using_ns_,
        protected_,
        definition,
        line_comment,
        block_comment,
    };

    enum class preprocessor_type_t : u8 {
        if_                 = 1,
        pragma,
        define,
        endif_,
        local_include,
        system_include,
    };

    enum class expression_type_t : u8 {
        raw                 = 1,
        unary,
        binary,
        assignment,
        initializer,
    };

    enum class initializer_type_t : u8 {
        direct              = 1,
        list,
    };

    enum class assignment_type_t : u8 {
        direct              = 1,
        bor,
        sum,
        shl,
        shr,
        diff,
        band,
        bxor,
        product,
        quotient,
        remainder,
    };

    enum class unary_op_type_t : u8 {
        neg                 = 1,
        inc,
        dec,
        lnot,
        bnot,
        deref,
        addrof,
        addrof_label,
    };

    enum class position_type_t : u8 {
        none,
        prefix,
        postfix
    };

    enum class binary_op_type_t : u8 {
        eq                  = 1,
        lt,
        gt,
        lor,
        add,
        sub,
        mul,
        div,
        mod,
        shl,
        shr,
        bor,
        neq,
        lte,
        gte,
        band,
        bxor,
        land,
        cast,
        comma,
        scope,
        range,
        member,
        subscript,
    };

    namespace var {
        [[maybe_unused]] static constexpr u8 none           = 0b00000000;
        [[maybe_unused]] static constexpr u8 const_         = 0b00000001;
        [[maybe_unused]] static constexpr u8 static_        = 0b00000010;
        [[maybe_unused]] static constexpr u8 volatile_      = 0b00000100;
        [[maybe_unused]] static constexpr u8 register_      = 0b00001000;
        [[maybe_unused]] static constexpr u8 constexpr_     = 0b00010000;
    }

    namespace aggregate {
        [[maybe_unused]] static constexpr u8 none           = 0b00000000;
        [[maybe_unused]] static constexpr u8 final_         = 0b00000001;
        [[maybe_unused]] static constexpr u8 packed_        = 0b00000010;
    }

    namespace element {
        namespace field {
            [[maybe_unused]] static constexpr u8 none       = 0b00000;
            [[maybe_unused]] static constexpr u8 id         = 0b00001;
            [[maybe_unused]] static constexpr u8 lhs        = 0b00010;
            [[maybe_unused]] static constexpr u8 rhs        = 0b00011;
            [[maybe_unused]] static constexpr u8 lit        = 0b00100;
            [[maybe_unused]] static constexpr u8 init       = 0b00101;
            [[maybe_unused]] static constexpr u8 list       = 0b00110;
            [[maybe_unused]] static constexpr u8 type       = 0b00111;
            [[maybe_unused]] static constexpr u8 child      = 0b01000;
            [[maybe_unused]] static constexpr u8 scope      = 0b01001;
            [[maybe_unused]] static constexpr u8 radix      = 0b01010;
            [[maybe_unused]] static constexpr u8 ident      = 0b01011;
            [[maybe_unused]] static constexpr u8 intern     = 0b01100;
            [[maybe_unused]] static constexpr u8 parent     = 0b01101;
            [[maybe_unused]] static constexpr u8 revision   = 0b01110;
            [[maybe_unused]] static constexpr u8 label      = 0b01111;
            [[maybe_unused]] static constexpr u8 tbranch    = 0b10000;
            [[maybe_unused]] static constexpr u8 fbranch    = 0b10001;

            [[maybe_unused]] static constexpr u32 count     = fbranch;

            str::slice_t name(u8);
        }

        namespace header {
            [[maybe_unused]] static constexpr u8 none       = 0b00000;
            [[maybe_unused]] static constexpr u8 type       = 0b00001;
            [[maybe_unused]] static constexpr u8 list       = 0b00010;
            [[maybe_unused]] static constexpr u8 scope      = 0b00011;
            [[maybe_unused]] static constexpr u8 ident      = 0b00100;
            [[maybe_unused]] static constexpr u8 module     = 0b00101;
            [[maybe_unused]] static constexpr u8 num_lit    = 0b00110;
            [[maybe_unused]] static constexpr u8 str_lit    = 0b00111;
            [[maybe_unused]] static constexpr u8 program    = 0b01000;
            [[maybe_unused]] static constexpr u8 variable   = 0b01001;
            [[maybe_unused]] static constexpr u8 statement  = 0b01010;
            [[maybe_unused]] static constexpr u8 expression = 0b01011;
            [[maybe_unused]] static constexpr u8 char_lit   = 0b01100;
            [[maybe_unused]] static constexpr u8 label      = 0b01101;

            str::slice_t name(u8);
        }
    }

    struct ident_t final {
        u32                     record_id;
        u32                     intern_id;
    };

    using symbols_t = symtab_t<ident_t>;

    struct scope_t final {
        program_t*              pgm;
        scope_stack_t           stack;
        id_array_t              types;
        symbols_t               labels;
        id_array_t              children;
        id_array_t              statements;
        symbols_t               identifiers;
        u32                     module_idx;
        u32                     parent_idx;
        u32                     idx;
        u32                     id;
    };

    struct module_t final {
        program_t*              program;
        scope_array_t           scopes;
        u32                     root_scope_idx;
        u32                     filename_id;
        u32                     idx;
        u32                     id;
        revision_t              revision;
    };

    struct program_t final {
        bass_t                  storage;
        module_array_t          modules;
        intern_t                intern;
        str_t                   scratch;
        alloc_t*                alloc;
        u32                     id;
    };

    struct serializer_t final {
        intern_t*               intern;
        bass_t*                 store;
        alloc_t*                alloc;
        symtab_t<str_t>         modules;
        str_t                   scratch;
        u32                     line;
        u16                     indent;
        u16                     column;
        u16                     margin;
        u16                     tab_width;
    };

    struct type_info_t final {
        str_t*                  name;
        u32                     size;
        integral_size_t         size_type;
        meta_type_t             meta_type;
    };

    namespace scope {
        namespace expr {
            namespace unary {
                u32 neg(scope_t& scope, u32 expr_id);

                u32 bnot(scope_t& scope, u32 expr_id);

                u32 lnot(scope_t& scope, u32 expr_id);

                u32 deref(scope_t& scope, u32 expr_id);

                u32 addrof(scope_t& scope, u32 expr_id);

                str::slice_t name(unary_op_type_t type);

                u32 addrof_label(scope_t& scope, u32 expr_id);

                str::slice_t position_name(position_type_t type);

                u32 inc(scope_t& scope, u32 expr_id, position_type_t pos = position_type_t::prefix);

                u32 dec(scope_t& scope, u32 expr_id, position_type_t pos = position_type_t::prefix);
            }

            namespace init {
                u32 direct(scope_t& scope, u32 expr_id);

                str::slice_t name(initializer_type_t type);

                u32 list(scope_t& scope, u32 expr_ids[], u32 size);
            }

            namespace assign {
                str::slice_t name(assignment_type_t type);

                u32 sum(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 shl(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 shr(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 bor(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 band(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 bxor(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 diff(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 direct(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 product(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 quotient(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 remainder(scope_t& scope, u32 lhs_id, u32 rhs_id);
            }

            namespace binary {
                str::slice_t name(binary_op_type_t type);

                u32 eq(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 lt(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 gt(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 mul(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 neq(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 lte(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 gte(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 add(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 sub(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 div(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 mod(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 shl(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 shr(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 bor(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 lor(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 cast(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 band(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 bxor(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 land(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 comma(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 range(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 member(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 scope_res(scope_t& scope, u32 lhs_id, u32 rhs_id);

                u32 subscript(scope_t& scope, u32 lhs_id, u32 rhs_id);
            }

            str::slice_t name(expression_type_t type);

            u32 raw(scope_t& scope, str::slice_t source);

            u32 ident(scope_t& scope, str::slice_t name);

            u32 list(scope_t& scope, u32 id_list[], u32 size);

            u32 var(scope_t& scope, u32 type_id, u32 ident_id, u32 init_expr_id = 0, u8 flags = var::none);
        }

        namespace type {
            u32 ptr(scope_t& scope, u32 type_id);

            u32 u0_(scope_t& scope, u32 ident_id);

            u32 s8_(scope_t& scope, u32 ident_id);

            u32 u8_(scope_t& scope, u32 ident_id);

            u32 b8_(scope_t& scope, u32 ident_id);

            u32 ref(scope_t& scope, u32 ident_id);

            u32 f32_(scope_t& scope, u32 ident_id);

            u32 f64_(scope_t& scope, u32 ident_id);

            u32 s16_(scope_t& scope, u32 ident_id);

            u32 s32_(scope_t& scope, u32 ident_id);

            u32 s64_(scope_t& scope, u32 ident_id);

            u32 u16_(scope_t& scope, u32 ident_id);

            u32 u32_(scope_t& scope, u32 ident_id);

            u32 u64_(scope_t& scope, u32 ident_id);

            str::slice_t meta_name(meta_type_t type);

            u32 bit_mask(scope_t& scope, u32 type_id, u8 bits);

            str::slice_t aggregate_name(aggregate_type_t type);

            u32 array(scope_t& scope, u32 type_id, u32 size = 0);

            str::slice_t integral_size_name(integral_size_t size);

            u32 enum_(scope_t& scope, u32 block_id, u32 ident_id = 0, u8 flags = 0);

            u32 class_(scope_t& scope, u32 block_id, u32 ident_id = 0, u8 flags = 0);

            u32 union_(scope_t& scope, u32 block_id, u32 ident_id = 0, u8 flags = 0);

            u32 struct_(scope_t& scope, u32 block_id, u32 ident_id = 0, u8 flags = 0);

            u32 enum_class_(scope_t& scope, u32 block_id, u32 ident_id = 0, u8 flags = 0);

            u32 func(scope_t& scope, u32 block_id, u32 return_type_id, u32 ident_id, u32 params_list_id = 0);
        }

        namespace lit {
            u32 chr(scope_t& scope, s8 value);

            u32 str(scope_t& scope, str::slice_t value);

            u32 float_(scope_t& scope, f64 value, integral_size_t size);

            u32 signed_(scope_t& scope, u64 value, integral_size_t size, u32 radix = 10);

            u32 unsigned_(scope_t& scope, u64 value, integral_size_t size, u32 radix = 10);
        }

        namespace stmt {
            namespace pp {
                str::slice_t name(preprocessor_type_t type);

                u32 pragma(scope_t& scope, str::slice_t expr);

                u32 include_local(scope_t& scope, str::slice_t path);

                u32 include_system(scope_t& scope, str::slice_t path);
            }

            namespace comment {
                u32 line(scope_t& scope, str::slice_t value);

                u32 block(scope_t& scope, str::slice_t value);
            }

            u32 if_(
                scope_t& scope,
                u32 predicate_id,
                u32 true_expr_id,
                u32 false_expr_id = 0,
                u32 label_id = 0);

            u32 for_(
                scope_t& scope,
                u32 predicate_id,
                u32 expr_id,
                u32 init_expr_id = 0,
                u32 post_expr_id = 0,
                u32 label_id = 0);

            u32 public_(scope_t& scope);

            u32 private_(scope_t& scope);

            u32 protected_(scope_t& scope);

            u32 def(scope_t& scope, u32 expr_id);

            str::slice_t name(statement_type_t type);

            u32 using_ns(scope_t& scope, u32 expr_id);

            u32 empty(scope_t& scope, u32 label_id = 0);

            u32 break_(scope_t& scope, u32 label_id = 0);

            u32 raw(scope_t& scope, str::slice_t source);

            u32 continue_(scope_t& scope, u32 label_id = 0);

            u32 using_(scope_t& scope, u32 ident_id, u32 type_id);

            u32 expr(scope_t& scope, u32 expr_id, u32 label_id = 0);

            u32 decl(scope_t& scope, u32 expr_id, u32 label_id = 0);

            u32 goto_(scope_t& scope, u32 expr_id, u32 label_id = 0);

            u32 return_(scope_t& scope, u32 expr_id = 0, u32 label_id = 0);

            u32 default_(scope_t& scope, u32 expr_id = 0, u32 label_id = 0);

            u32 do_(scope_t& scope, u32 predicate_id, u32 expr_id, u32 label_id = 0);

            u32 case_(scope_t& scope, u32 predicate_id, u32 expr_id, u32 label_id = 0);

            u32 while_(scope_t& scope, u32 predicate_id, u32 expr_id, u32 label_id = 0);

            u32 switch_(scope_t& scope, u32 predicate_id, u32 expr_id, u32 label_id = 0);
        }

        u0 pop(scope_t& scope);

        u0 free(scope_t& scope);

        u32 push(scope_t& scope);

        status_t finalize(scope_t& scope);

        u32 label(scope_t& scope, str::slice_t name);

        u0 init(program_t* pgm, module_t* module, scope_t& scope, scope_t* parent, alloc_t* alloc = context::top()->alloc);
    }

    namespace module {
        u0 init(
            module_t& module,
            program_t& pgm,
            str::slice_t& filename,
            cxx::revision_t rev,
            alloc_t* alloc = context::top()->alloc);

        u0 free(module_t& module);

        status_t finalize(module_t& module);

        scope_t& get_scope(module_t& module, u32 scope_idx);
    }

    namespace program {
        u0 free(program_t& pgm);

        status_t finalize(program_t& pgm);

        str::slice_t status_name(status_t status);

        str::slice_t revision_name(revision_t rev);

        u0 debug_dump(program_t& pgm, fmt_buf_t& buf);

        u32 integral_size_in_bits(integral_size_t size);

        u32 integral_size_in_bytes(integral_size_t size);

        module_t& get_module(program_t& pgm, u32 module_idx);

        module_t& add_module(program_t& pgm, str::slice_t filename, cxx::revision_t rev);

        u0 init(program_t& pgm, alloc_t* alloc = context::top()->alloc, u32 num_modules = 16);
    }

    namespace serializer {
        u0 free(serializer_t& s);

        status_t serialize(serializer_t& s);

        status_t expand_type(bass_t& storage, intern_t& intern, u32 type_id, type_info_t& type_info);

        u0 init(serializer_t& s, program_t& pgm, alloc_t* alloc = context::top()->alloc, u16 margin = 160, u16 tab_width = 4);
    }
}

