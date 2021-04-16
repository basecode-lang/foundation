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
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
// ----------------------------------------------------------------------------

#include <basecode/core/scm/scm.h>
#include <basecode/core/scm/types.h>
#include <basecode/core/scm/compiler.h>

namespace basecode::scm {
    static const s8* s_type_names[] = {
        "nil",
        "ffi",
        "ptr",
        "pair",
        "free",
        "prim",
        "port",
        "proc",
        "cfunc",
        "error",
        "lambda",
        "fixnum",
        "flonum",
        "symbol",
        "string",
        "boolean",
        "keyword",
        "environment",
    };

    static const s8* s_prim_names[] = {
        "is",
        ">",
        "<",
        ">=",
        "<=",
        "+",
        "-",
        "*",
        "/",
        "mod",
        "set!",
        "if",
        "or",
        "car",
        "cdr",
        "and",
        "not",
        "cons",
        "eval",
        "list",
        "atom",
        "begin",
        "error",
        "quote",
        "while",
        "lambda",
        "setcar",
        "setcdr",
        "define",
        "unquote",
        "quasiquote",
        "define-macro",
        "unquote-splicing",
    };

    static const s8* s_delims   = " \n\t\r()[];";

    static ffi_type_map_t s_types[] = {
        [u32(obj_type_t::pair)]     = ffi_type_map_t{.type = 'P', .size = 'q'},
        [u32(obj_type_t::nil)]      = ffi_type_map_t{.type = 'P', .size = 'q'},
        [u32(obj_type_t::fixnum)]   = ffi_type_map_t{.type = 'I', .size = 'd'},
        [u32(obj_type_t::flonum)]   = ffi_type_map_t{.type = 'F', .size = 'd'},
        [u32(obj_type_t::symbol)]   = ffi_type_map_t{.type = 'S', .size = 'q'},
        [u32(obj_type_t::string)]   = ffi_type_map_t{.type = 'S', .size = 'q'},
        [u32(obj_type_t::boolean)]  = ffi_type_map_t{.type = 'B', .size = 'b'},
    };

    static obj_t* make_object(ctx_t* ctx);

    static u0 format_environment(ctx_t* ctx,
                                 obj_t* env,
                                 fmt_ctx_t& fmt_ctx,
                                 u32 indent);

    static num_type_t get_numtype(const s8* buf, s32 len);

    static obj_t* read_expr(ctx_t* ctx, buf_crsr_t& crsr);

    static obj_t* make_ffi_signature(ctx_t* ctx,
                                     obj_t* args,
                                     rest_array_t& eval_args,
                                     u8* buf,
                                     u32& len,
                                     keyword_table_t& keywords);

    u0 free(ctx_t* ctx) {
        collect_garbage(ctx);
        array::free(ctx->native_ptrs);
        stack::free(ctx->cl_stack);
        hashtab::free(ctx->strtab);
        hashtab::free(ctx->symtab);
        for (auto& env : ctx->environments)
            hashtab::free(env.bindings);
        array::free(ctx->environments);
        array::free(ctx->procedures);
        compiler::free(ctx->compiler);
        vm::free(ctx->vm);
    }

    obj_t* nil(ctx_t* ctx) {
        return ctx->nil;
    }

    u32 save_gc(ctx_t* ctx) {
        return ctx->gc_stack->size;
    }

    obj_t* pop_gc(ctx_t* ctx) {
        return vm::mem_area::pop<obj_t*>(*ctx->gc_stack);
    }

    obj_t* make_proc(ctx_t* ctx,
                     obj_t* sym,
                     obj_t* params,
                     obj_t* body,
                     b8 macro) {
        auto proc = &array::append(ctx->procedures);
        auto env  = top_env(ctx);
        auto begin_sym = SYM("begin");
        if (TYPE(body) != obj_type_t::pair
        || (!equal(ctx, CAAR(body), begin_sym))) {
            body = cons(ctx,
                        get(ctx, begin_sym),
                        is_list(ctx, body) ? body : CONS1(body));
        }
        proc->e            = ENV(env);
        proc->env          = env;
        proc->addr         = {};
        proc->sym          = sym;
        proc->body         = body;
        proc->params       = params;
        proc->is_tco       = false;
        proc->is_macro     = macro;
        proc->is_compiled  = false;
        proc->is_assembled = false;
        array::append(ctx->native_ptrs, proc);
        auto obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::proc);
        SET_FIXNUM(obj, ctx->native_ptrs.size);
        return obj;
    }

    obj_t* top_env(ctx_t* ctx) {
        return vm::mem_area::top<obj_t*>(*ctx->env_stack);
    }

    u0 clear_error(ctx_t* ctx) {
        ctx->err = ctx->nil;
    }

    obj_t* pop_env(ctx_t* ctx) {
        obj_t* tos = vm::mem_area::pop<obj_t*>(*ctx->env_stack);
        return !tos ? ctx->nil : tos;
    }

    obj_type_t type(obj_t* obj) {
        return TYPE(obj);
    }

    u0 collect_garbage(ctx_t* ctx) {
        auto& vm = ctx->vm;
        for (const auto& pair : ctx->strtab)
            mark(ctx, pair.value);
        for (const auto& pair : ctx->symtab)
            mark(ctx, pair.value);
        for (auto& env : ctx->environments) {
            if (!env.protect)
                continue;
            mark(ctx, env.self);
            mark(ctx, env.parent);
            for (const auto& pair : env.bindings)
                mark(ctx, pair.value);
        }
        auto addr = GP;
        for (u32 i = 0; i < ctx->gc_stack->size; i++) {
            mark(ctx, (obj_t*) HU(addr));
            addr += sizeof(u64);
        }
        for (u32 i = 0; i < ctx->object_count; i++) {
            obj_t* obj = &ctx->objects[i];
            if (TYPE(obj) == obj_type_t::free)
                continue;
            if (!IS_GC_MARKED(obj)) {
                switch (TYPE(obj)) {
                    case obj_type_t::ptr: {
                        auto chain = ctx->handlers.chain;
                        while (chain) {
                            if (chain->gc_enabled && chain->gc)
                                chain->gc(ctx, obj);
                            chain = chain->next;
                        }
                        break;
                    }

                    case obj_type_t::environment: {
                        auto e = ENV(obj);
                        e->free = true;
                        break;
                    }

                    default:
                        break;
                }
                SET_TYPE(obj, obj_type_t::free);
                SET_CDR(obj, ctx->free_list);
                ctx->free_list = obj;
                --ctx->object_used;
            } else {
                SET_GC_MARK(obj, false);
            }
        }
    }

    fixnum_t to_fixnum(obj_t* obj) {
        if (TYPE(obj) == obj_type_t::fixnum)
            return fixnum_t(u32(obj->number.value));
        else
            return fixnum_t(std::bit_cast<f32>(u32(obj->number.value)));
    }

    flonum_t to_flonum(obj_t* obj) {
        if (TYPE(obj) == obj_type_t::flonum)
            return std::bit_cast<f32>(u32(obj->number.value));
        else
            return flonum_t(u32(obj->number.value));
    }

    const s8* type_name(obj_t* obj) {
        return s_type_names[u32(TYPE(obj))];
    }

    const s8* prim_name(obj_t* obj) {
        return s_prim_names[u32(TYPE(obj))];
    }

    u0 mark(ctx_t* ctx, obj_t* obj) {
        if (!obj)
            return;
        obj_t* car;
    begin:
        if (IS_GC_MARKED(obj))
            return;

        car = CAR(obj);
        SET_GC_MARK(obj, true);

        switch (TYPE(obj)) {
            case obj_type_t::pair:
                mark(ctx, car);
                obj = CDR(obj);
                goto begin;

            case obj_type_t::ptr: {
                auto chain = ctx->handlers.chain;
                while (chain) {
                    if (chain->mark_enabled && chain->mark)
                        chain->mark(ctx, obj);
                    chain = chain->next;
                }
                break;
            }

            default:
                break;
        }
    }

    b8 is_nil(ctx_t* ctx, obj_t* obj) {
        return IS_NIL(obj);
    }

    b8 is_true(ctx_t* ctx, obj_t* obj) {
        return ctx->true_ == obj;
    }

    b8 is_list(ctx_t* ctx, obj_t* obj) {
        if (TYPE(obj) != obj_type_t::pair)
            return false;
        auto turtle = obj;
        auto hare = CDR(obj);
        for (; TYPE(hare) == obj_type_t::pair; turtle = CDR(turtle)) {
            if (hare == turtle)
                return false;
            hare = CDR(hare);
            if (TYPE(hare) == obj_type_t::pair)
                hare = CDR(hare);
        }
        return IS_NIL(hare);
    }

    u32 length(ctx_t* ctx, obj_t* obj) {
        switch (TYPE(obj)) {
            case obj_type_t::pair: {
                u32 len = 0;
                for (obj_t* pair = obj; !IS_NIL(pair); pair = CDR(pair))
                    ++len;
                return len;
            }
            case obj_type_t::symbol:
            case obj_type_t::string:
            case obj_type_t::keyword: {
                auto rc = string::interned::get(STRING_ID(obj));
                if (!OK(rc.status)) {
                    error(ctx,
                          "unable to find interned string: {}", STRING_ID(obj));
                    return 0;
                }
                return rc.slice.length;
            }
            default:
                break;
        }
        return 0;
    }

    u0 push_gc(ctx_t* ctx, obj_t* obj) {
        if (obj < ctx->objects
        ||  obj >= ctx->objects + ctx->object_count) {
            error(ctx,
                  "fatal: obj address outside of heap range! {}", (u0*) obj);
            return;
        }
        vm::mem_area::push(*ctx->gc_stack, obj);
    }

    obj_t* car(ctx_t* ctx, obj_t* obj) {
        // FIXME: check_type was removed
        return IS_NIL(obj) ? obj : CAR(obj);
    }

    obj_t* cdr(ctx_t* ctx, obj_t* obj) {
        // FIXME: check_type was removed
        return IS_NIL(obj) ? obj : CDR(obj);
    }

    obj_t* get(ctx_t* ctx, obj_t* sym) {
        auto env = top_env(ctx);
        const auto str_id = FIXNUM(sym);
        const auto name = *string::interned::get_slice(str_id);
        auto chain = ctx->handlers.chain;
        while (chain) {
            if (chain->get_enabled && chain->get) {
                auto res = chain->get(ctx, name, env);
                if (res)
                    return res;
            }
            chain = chain->next;
        }
        while (!IS_NIL(env)) {
            check_type(ctx, env, obj_type_t::environment);
            auto e = ENV(env);
            auto v = hashtab::find(e->bindings, str_id);
            if (v)
                return v;
            env = e->parent;
        }
        return ctx->nil;
    }

    u0 restore_gc(ctx_t* ctx, u32 idx) {
        vm::mem_area::shrink_to_size(*ctx->gc_stack, idx);
    }

    obj_t* eval2(ctx_t* ctx, obj_t* obj) {
        auto comp_result = compiler::compile(ctx->compiler, ctx, obj);
        UNUSED(comp_result);
        return ctx->nil;
    }

    static obj_t* make_object(ctx_t* ctx) {
        obj_t* obj;
        if (IS_NIL(ctx->free_list)) {
            collect_garbage(ctx);
            if (IS_NIL(ctx->free_list))
                return error(ctx, "out of memory");
        }
        obj = ctx->free_list;
        ctx->free_list = CDR(obj);
        push_gc(ctx, obj);
        ++ctx->object_used;
        return obj;
    }

    obj_t* make_bool(ctx_t* ctx, b8 value) {
        return value ? ctx->true_ : ctx->false_;
    }

    obj_t* push_env(ctx_t* ctx, obj_t* obj) {
        check_type(ctx, obj, obj_type_t::environment);
        vm::mem_area::push(*ctx->env_stack, obj);
        return obj;
    }

    u0* to_user_ptr(ctx_t* ctx, obj_t* obj) {
        const auto idx = FIXNUM(check_type(ctx, obj, obj_type_t::ptr));
        return ctx->native_ptrs[idx - 1];
    }

    static obj_t* make_ffi_signature(ctx_t* ctx,
                                     obj_t* args,
                                     rest_array_t& eval_args,
                                     u8* buf,
                                     u32& len,
                                     keyword_table_t& keywords) {
        len = 0;
        if (IS_NIL(args)) {
            buf[len++] = u8(param_cls_t::void_);
            return ctx->nil;
        }
        while (!IS_NIL(args)) {
            auto arg = CAR(args);
            if (TYPE(arg) == obj_type_t::keyword) {
                args = CDR(args);
                if (IS_NIL(args)) {
                    return error(ctx,
                                 "[ffi] argument value expected after keyword");
                }
                auto value = EVAL(CAR(args));
                if (IS_SCM_ERROR(value))
                    return value;
                hashtab::insert(keywords, to_string(ctx, arg), value);
            } else {
                auto eval_arg = EVAL(arg);
                if (IS_SCM_ERROR(eval_arg))
                    return eval_arg;
                const auto& mapping = s_types[u32(TYPE(eval_arg))];
                buf[len++] = mapping.type;
                buf[len++] = mapping.size;
                array::append(eval_args, eval_arg);
            }
            args = CDR(args);
        }
        return ctx->nil;
    }

    obj_t* make_bool(ctx_t* ctx, obj_t* obj) {
        return IS_TRUE(obj) ? ctx->true_ : ctx->false_;
    }

    obj_t* next_arg(ctx_t* ctx, obj_t** arg) {
        obj_t* a = *arg;
        if (TYPE(a) != obj_type_t::pair) {
            if (IS_NIL(a))
                return error(ctx, "too few arguments");
            return error(ctx, "dotted pair in argument list");
        }
        *arg = CDR(a);
        return CAR(a);
    }

    b8 equal(ctx_t* ctx, obj_t* a, obj_t* b) {
        if (a == b)
            return true;
        if ((TYPE(a) == obj_type_t::fixnum || TYPE(a) == obj_type_t::flonum)
        &&  (TYPE(b) == obj_type_t::fixnum || TYPE(b) == obj_type_t::flonum)) {
            switch (TYPE(a)) {
                case obj_type_t::fixnum: {
                    auto fa = to_fixnum(a);
                    auto fb = to_fixnum(b);
                    return fa == fb;
                }
                case obj_type_t::flonum: {
                    const auto e = std::numeric_limits<flonum_t>::epsilon();
                    auto x = fabs(to_flonum(a));
                    auto y = fabs(to_flonum(b));
                    auto largest = (y > x) ? y : x;
                    auto diff = x - y;
                    return diff <= largest * e;
                }
                default:                return false;
            }
        }
        if (TYPE(a) != TYPE(b))         return false;
        switch (TYPE(a)) {
            case obj_type_t::pair: {
                auto ka = CAR(a);
                auto kd = CDR(a);
                auto ja = CAR(b);
                auto jd = CDR(b);
                return equal(ctx, ka, ja) && equal(ctx, kd, jd);
            }
            case obj_type_t::prim:      return PRIM(a) == PRIM(b);
            case obj_type_t::ptr:
            case obj_type_t::cfunc:
            case obj_type_t::lambda:    return NATIVE_PTR(a) == NATIVE_PTR(b);
            default:                    break;
        }
        return false;
    }

    obj_t* make_user_ptr(ctx_t* ctx, u0* ptr) {
        obj_t* obj = make_object(ctx);
        array::append(ctx->native_ptrs, ptr);
        SET_TYPE(obj, obj_type_t::ptr);
        SET_FIXNUM(obj, ctx->native_ptrs.size);
        return obj;
    }

    obj_t* error_args(ctx_t* ctx, obj_t* obj) {
        return CDR(check_type(ctx, obj, obj_type_t::error));
    }

    obj_t* read(ctx_t* ctx, buf_crsr_t& crsr) {
        obj_t* obj = read_expr(ctx, crsr);
        if (obj == ctx->rparen) {
            return error(ctx, "stray ')'");
        }
        if (obj == ctx->rbrac) {
            return error(ctx, "stray ']'");
        }
        return obj;
    }

    obj_t* make_fixnum(ctx_t* ctx, fixnum_t n) {
        obj_t* obj  = make_object(ctx);
        SET_TYPE(obj, obj_type_t::fixnum);
        SET_FIXNUM(obj, n);
        return obj;
    }

    obj_t* make_flonum(ctx_t* ctx, flonum_t n) {
        obj_t* obj  = make_object(ctx);
        SET_TYPE(obj, obj_type_t::flonum);
        SET_FLONUM(obj, n);
        return obj;
    }

    obj_t* make_ffi(ctx_t* ctx, proto_t* proto) {
        obj_t* obj = make_object(ctx);
        array::append(ctx->native_ptrs, proto);
        SET_TYPE(obj, obj_type_t::ffi);
        SET_FIXNUM(obj, ctx->native_ptrs.size);
        return obj;
    }

    u0 print(FILE* file, ctx_t* ctx, obj_t* obj) {
        format::print(file, "{}\n", printable_t{ctx, obj});
    }

    u0 write(FILE* file, ctx_t* ctx, obj_t* obj) {
        format::print(file, "{}", printable_t{ctx, obj});
    }

    ctx_t* init(u0* ptr, u32 size, alloc_t* alloc) {
        namespace rf = scm::register_file;

        // init context struct
        auto ctx = (ctx_t*) ptr;
        std::memset(ctx, 0, sizeof(ctx_t));
        u32 align;
        ptr = (u8*) memory::system::align_forward(
            (u8*) ptr + sizeof(ctx_t),
            alignof(obj_t),
            align);
        size -= (sizeof(ctx_t) + align);

        // init objects memory region
        ctx->objects      = (obj_t*) ptr;
        ctx->object_used  = 0;
        ctx->object_count = (size / sizeof(obj_t)) - 1;
        if ((ctx->object_count % 2) != 0)
            --ctx->object_count;

        auto& vm = ctx->vm;
        ctx->alloc = alloc;
        vm::init(vm, ctx->alloc);
        vm::add_mem_area(vm,
                         scm::mem_area_type_t::heap,
                         register_file::hp,
                         ctx->alloc,
                         0);
        vm::add_mem_area(vm,
                         scm::mem_area_type_t::code_stack,
                         register_file::sp,
                         ctx->alloc,
                         1024,
                         true);
        ctx->data_stack = &vm::add_mem_area(vm,
                                           scm::mem_area_type_t::data_stack,
                                           register_file::dp,
                                           ctx->alloc,
                                           1024,
                                           true);
        ctx->gc_stack = &vm::add_mem_area(vm,
                                           scm::mem_area_type_t::gc_stack,
                                           register_file::gp,
                                           ctx->alloc,
                                           1024,
                                           true);
        ctx->env_stack = &vm::add_mem_area(vm,
                                           scm::mem_area_type_t::env_stack,
                                           register_file::ep,
                                           ctx->alloc,
                                           1024,
                                           true);
        vm::reset(vm);

        compiler::init(ctx->compiler, &vm, ctx->alloc);

        array::init(ctx->procedures, ctx->alloc);
        array::init(ctx->environments, ctx->alloc);

        array::init(ctx->native_ptrs, ctx->alloc);
        array::reserve(ctx->native_ptrs, 256);

        stack::init(ctx->cl_stack, ctx->alloc);
        stack::reserve(ctx->cl_stack, 1024);

        hashtab::init(ctx->strtab, ctx->alloc);
        hashtab::init(ctx->symtab, ctx->alloc);

        // init objects
        ctx->nil       = &ctx->objects[ctx->object_used++];
        ctx->nil->full = 0;
        SET_TYPE(ctx->nil, obj_type_t::nil);

        // init lists
        ctx->call_list = ctx->nil;
        ctx->free_list = ctx->nil;

        ctx->err = ctx->nil;

        // populate freelist
        for (s32 i = ctx->object_count - 1; i >= ctx->object_used; i--) {
            obj_t* obj = &ctx->objects[i];
            SET_TYPE(obj, obj_type_t::free);
            SET_CDR(obj, ctx->free_list);
            ctx->free_list = obj;
        }

        ctx->env = make_environment(ctx, ctx->nil);
        push_env(ctx, ctx->env);

        /* true */ {
            ctx->true_ = make_object(ctx);
            SET_TYPE(ctx->true_, obj_type_t::boolean);
            SET_FIXNUM(ctx->true_, 1);
            define(ctx, make_symbol(ctx, "#t", 2), ctx->true_);
        }

        /* false */ {
            ctx->false_ = make_object(ctx);
            SET_TYPE(ctx->false_, obj_type_t::boolean);
            SET_FIXNUM(ctx->false_, 0);
            define(ctx, make_symbol(ctx, "#f", 2), ctx->false_);
        }

        ctx->rbrac  = SYM("]");
        ctx->rparen = SYM(")");
        ctx->dot    = SYM(".");
        define(ctx, ctx->dot, ctx->dot);
        define(ctx, ctx->rbrac, ctx->rbrac);
        define(ctx, SYM("nil"), ctx->nil);
        define(ctx, ctx->rparen, ctx->rparen);

        // register built in primitives
        for (u32 i = 0; i < u32(prim_type_t::max); i++) {
            obj_t* v = make_object(ctx);
            SET_TYPE(v, obj_type_t::prim);
            SET_PRIM(v, i);
            define(ctx, make_symbol(ctx, s_prim_names[i]), v);
        }

        return ctx;
    }

    obj_t* cons(ctx_t* ctx, obj_t* car, obj_t* cdr) {
        obj_t* obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::pair);
        SET_CAR(obj, car);
        SET_CDR(obj, cdr);
        return obj;
    }

    s32 compare(ctx_t* ctx, obj_t* lhs, obj_t* rhs) {
        if (equal(ctx, lhs, rhs))
            return 0;

        const auto lt = TYPE(lhs);
        const auto rt = TYPE(rhs);

        if (lt == obj_type_t::free && rt == obj_type_t::free) {
            if (lhs < rhs) return -1;
            return 1;
        }

        if ((lt == obj_type_t::fixnum || lt == obj_type_t::flonum)
        &&  (rt == obj_type_t::fixnum || rt== obj_type_t::flonum)) {
            auto a = to_flonum(lhs);
            auto b = to_flonum(rhs);
            if (a < b) return -1;
            return 1;
        }

        if ((lt == obj_type_t::symbol  && rt == obj_type_t::symbol)
        ||  (lt == obj_type_t::keyword && rt == obj_type_t::keyword)) {
            auto a = *string::interned::get_slice(STRING_ID(lhs));
            auto b = *string::interned::get_slice(STRING_ID(rhs));
            if (a < b) return -1;
            return 1;
        }

        if (lt == obj_type_t::string && rt == obj_type_t::string) {
            auto a = *string::interned::get_slice(STRING_ID(lhs));
            auto b = *string::interned::get_slice(STRING_ID(rhs));
            if (a < b) return -1;
            return 1;
        }

        if (lt == obj_type_t::proc && rt == obj_type_t::proc) {
            auto a = PROC(lhs);
            auto b = PROC(rhs);
            if (a < b) return -1;
            return 1;
        }

        if (lt == obj_type_t::prim && rt == obj_type_t::prim) {
            auto a = PRIM(lhs);
            auto b = PRIM(rhs);
            if (a < b) return -1;
            return 1;
        }

        if (lt == obj_type_t::cfunc && rt == obj_type_t::cfunc) {
            if (CFUNC(lhs) < CFUNC(rhs)) return -1;
            return 1;
        }

        if (lt == obj_type_t::ptr && rt == obj_type_t::ptr) {
            if (NATIVE_PTR(lhs) < NATIVE_PTR(rhs)) return -1;
            return 1;
        }

        if (lt == obj_type_t::ffi && rt == obj_type_t::ffi) {
            if (PROTO(lhs) < PROTO(rhs)) return -1;
            return 1;
        }

        if (lt == obj_type_t::environment && rt == obj_type_t::environment) {
            auto a = ENV(lhs);
            auto b = ENV(rhs);
            if (a->parent < b->parent) return -1;
            return 1;
        }

        return -1;
    }

    obj_t* eval(ctx_t* ctx, obj_t* obj) {
        if (IS_SCM_ERROR(ctx->err))
            return ctx->err;

        if (ctx->env_stack->size == 0) {
            return error(ctx, "environment stack is empty: push an "
                              "environment before calling eval");
        }

        obj_t* fn   {};
        obj_t* kar  {};
        obj_t* arg  {};
        obj_t* res  {ctx->nil};
        obj_t* va   {};
        obj_t* vb   {};
        obj_t* cl   {};
        u32    gc   {save_gc(ctx)};

        defer(
            restore_gc(ctx, gc);
            if (cl)
                ctx->call_list = CDR(cl);
        );

        if (TYPE(obj) == obj_type_t::symbol) {
            return get(ctx, obj);
        }

        if (TYPE(obj) != obj_type_t::pair) {
            return obj;
        }

        cl = cons(ctx, obj, ctx->call_list);
        ctx->call_list = cl;

        kar = CAR(obj);
        arg = CDR(obj);
        if (TYPE(kar) == obj_type_t::symbol)
                fn = get(ctx, kar);
        else    fn = kar;

        switch (TYPE(fn)) {
            case obj_type_t::prim:
                switch (PRIM(fn)) {
                    case prim_type_t::eval:
                        return eval(ctx, EVAL_ARG());

                    case prim_type_t::set:
                        va = check_type(ctx,
                                        next_arg(ctx, &arg),
                                        obj_type_t::symbol);
                        if (!set(ctx, va, EVAL_ARG())) {
                            return error(ctx,
                                         "set! undefined variable: {}",
                                         printable_t{ctx, va});
                        }
                        return ctx->nil;

                    case prim_type_t::if_: {
                        auto flag     = IS_TRUE(EVAL(CAR(arg)));
                        auto true_br  = CADR(arg);
                        auto false_br = CADDR(arg);
                        return eval(ctx, flag ? true_br : false_br);
                    }

                    case prim_type_t::lambda: {
                        obj_t* params = CAR(arg);
                        obj_t* body   = CDR(arg);
                        auto* type    = vm::mem_area::pop<obj_t*>(*ctx->data_stack);
                        b8 is_macro{};
                        if (type) {
                            kar      = vm::mem_area::pop<obj_t*>(*ctx->data_stack);
                            is_macro = equal(ctx, type, SYM("define-macro"));
                        }
                        return make_proc(ctx,
                                         kar,
                                         params,
                                         body,
                                         is_macro);
                    }

                    case prim_type_t::error:
                        return make_error(ctx, arg, cl);

                    case prim_type_t::while_: {
                        va = next_arg(ctx, &arg);
                        auto gs = save_gc(ctx);
                        while (true) {
                            vb = eval(ctx, va);
                            if (IS_SCM_ERROR(vb))
                                return vb;
                            if (IS_FALSE(vb))
                                break;
                            auto body = arg;
                            while (!IS_NIL(body)) {
                                restore_gc(ctx, gs);
                                push_gc(ctx, body);
                                push_gc(ctx, top_env(ctx));
                                res = eval(ctx, CAR(body));
                                if (IS_SCM_ERROR(res))
                                    return res;
                                body = CDR(body);
                            }
                            restore_gc(ctx, gs);
                        }
                        return res;
                    }

                    case prim_type_t::define: {
                        va = check_type(ctx,
                                        next_arg(ctx, &arg),
                                        obj_type_t::symbol);
                        auto ds_save = ctx->data_stack->size;
                        vm::mem_area::push(*ctx->data_stack, va);
                        vm::mem_area::push(*ctx->data_stack, kar);
                        vb = EVAL_ARG();
                        define(ctx, va, vb);
                        vm::mem_area::shrink_to_size(*ctx->data_stack, ds_save);
                        return vb;
                    }

                    case prim_type_t::define_macro: {
                        va = check_type(ctx,
                                        next_arg(ctx, &arg),
                                        obj_type_t::symbol);
                        auto ds_save = ctx->data_stack->size;
                        vm::mem_area::push(*ctx->data_stack, va);
                        vm::mem_area::push(*ctx->data_stack, kar);
                        vb = EVAL_ARG();
                        check_type(ctx, vb, obj_type_t::proc);
                        define(ctx, va, vb);
                        vm::mem_area::shrink_to_size(*ctx->data_stack, ds_save);
                        return vb;
                    }

                    case prim_type_t::quote:
                        return next_arg(ctx, &arg);

                    case prim_type_t::unquote:
                        return error(ctx,
                                     "unquote is not valid in this context.");

                    case prim_type_t::quasiquote:
                        return eval(ctx, quasiquote(ctx, next_arg(ctx, &arg)));

                    case prim_type_t::unquote_splicing:
                        return error(ctx,
                                     "unquote-splicing is not valid in this context.");

                    case prim_type_t::and_:
                        while (!IS_NIL(arg)) {
                            va = EVAL_ARG();
                            if (IS_FALSE(va))
                                break;
                            res = va;
                        }
                        return res;

                    case prim_type_t::or_:
                        while (!IS_NIL(arg)) {
                            va = EVAL_ARG();
                            if (!IS_FALSE(va)) {
                                res = va;
                                break;
                            }
                        }
                        return res;

                    case prim_type_t::begin: {
                        auto save = save_gc(ctx);
                        while (!IS_NIL(arg)) {
                            restore_gc(ctx, save);
                            push_gc(ctx, arg);
                            push_gc(ctx, top_env(ctx));
                            res = eval(ctx, CAR(arg));
                            if (IS_SCM_ERROR(res))
                                return res;
                            arg = CDR(arg);
                        }
                        return res;
                    }

                    case prim_type_t::cons:
                        va  = EVAL_ARG();
                        return cons(ctx, va, EVAL_ARG());

                    case prim_type_t::car:
                        va = EVAL_ARG();
                        return car(ctx, va);

                    case prim_type_t::cdr:
                        va = EVAL_ARG();
                        return cdr(ctx, va);

                    case prim_type_t::setcar:
                        va = check_type(ctx, EVAL_ARG(), obj_type_t::pair);
                        SET_CAR(va, EVAL_ARG());
                        return ctx->nil;

                    case prim_type_t::setcdr:
                        va = check_type(ctx, EVAL_ARG(), obj_type_t::pair);
                        SET_CDR(va, EVAL_ARG());
                        return ctx->nil;

                    case prim_type_t::list:
                        return eval_list(ctx, arg);

                    case prim_type_t::not_:
                        va = EVAL_ARG();
                        return make_bool(ctx, IS_FALSE(va));

                    case prim_type_t::is:
                        va  = EVAL_ARG();
                        return make_bool(ctx, equal(ctx, va, EVAL_ARG()));

                    case prim_type_t::atom:
                        return make_bool(ctx, TYPE(EVAL_ARG()) != obj_type_t::pair);

                    case prim_type_t::gt:
                        va = EVAL_ARG();
                        vb = EVAL_ARG();
                        return make_bool(ctx, compare(ctx, va, vb) > 0);

                    case prim_type_t::gte:
                        va = EVAL_ARG();
                        vb = EVAL_ARG();
                        return make_bool(ctx, compare(ctx, va, vb) >= 0);

                    case prim_type_t::lt:
                        va = EVAL_ARG();
                        vb = EVAL_ARG();
                        return make_bool(ctx, compare(ctx, va, vb) < 0);

                    case prim_type_t::lte:
                        va = EVAL_ARG();
                        vb = EVAL_ARG();
                        return make_bool(ctx, compare(ctx, va, vb) <= 0);

                    case prim_type_t::add: {
                        flonum_t x = to_flonum(EVAL_ARG());
                        while (!IS_NIL(arg))
                            x += to_flonum(EVAL_ARG());
                        return make_flonum(ctx, x);
                    }

                    case prim_type_t::sub: {
                        flonum_t x = to_flonum(EVAL_ARG());
                        while (!IS_NIL(arg))
                            x -= to_flonum(EVAL_ARG());
                        return make_flonum(ctx, x);
                    }

                    case prim_type_t::mul: {
                        flonum_t x = to_flonum(EVAL_ARG());
                        while (!IS_NIL(arg))
                            x *= to_flonum(EVAL_ARG());
                        return make_flonum(ctx, x);
                    }

                    case prim_type_t::div: {
                        flonum_t x = to_flonum(EVAL_ARG());
                        while (!IS_NIL(arg))
                            x /= to_flonum(EVAL_ARG());
                        return make_flonum(ctx, x);
                    }

                    case prim_type_t::mod: {
                        fixnum_t x = to_fixnum(EVAL_ARG());
                        while (!IS_NIL(arg))
                            x %= to_fixnum(EVAL_ARG());
                        return make_fixnum(ctx, x);
                    }

                    default:
                        break;
                }
                break;

            case obj_type_t::ffi:
                return ffi_apply(ctx, fn, arg);

            case obj_type_t::cfunc:
                return cfunc_apply(ctx, fn, arg);

            case obj_type_t::proc: {
                auto proc = PROC(fn);
                if (!proc->is_macro) {
                    arg = eval_list(ctx, arg);
                    if (IS_SCM_ERROR(arg))
                        return arg;
                }
                push_env(ctx, make_environment(ctx, top_env(ctx)));
                args_to_env(ctx, proc->params, arg);
                auto body = proc->body;
                auto save = save_gc(ctx);
                while (!IS_NIL(body)) {
                    restore_gc(ctx, save);
                    push_gc(ctx, body);
                    push_gc(ctx, top_env(ctx));
                    res = eval(ctx, CAR(body));
                    if (IS_SCM_ERROR(res))
                        return res;
                    body = CDR(body);
                }
                if (proc->is_macro) {
                    *obj = *res;
                    res = eval(ctx, obj);
                }
                finalize_environment(ctx, pop_env(ctx));
                return res;
            }

            default: {
                return error(ctx,
                             "tried to call non-callable value '{}'",
                             s_type_names[u32(TYPE(fn))]);
            }
        }

        return res;
    }

    b8 set(ctx_t* ctx, obj_t* sym, obj_t* v) {
        auto env = top_env(ctx);
        const auto str_id = FIXNUM(sym);
        const auto name = *string::interned::get_slice(str_id);
        auto chain = ctx->handlers.chain;
        while (chain) {
            if (chain->set_enabled && chain->set) {
                if (chain->set(ctx, name, v, env))
                    return true;
            }
            chain = chain->next;
        }
        while (!IS_NIL(env)) {
            check_type(ctx, env, obj_type_t::environment);
            auto e = ENV(env);
            if (hashtab::set(e->bindings, str_id, v))
                return true;
            env = e->parent;
        }
        return false;
    }

    obj_t* eval_list(ctx_t* ctx, obj_t* lst) {
        obj_t* head = ctx->nil;
        obj_t* tail = head;
        while (!IS_NIL(lst)) {
            auto va = EVAL(next_arg(ctx, &lst));
            if (IS_SCM_ERROR(va))
                return va;
            auto r = cons(ctx, va, ctx->nil);
            if (IS_NIL(tail)) {
                head = r;
                tail = head;
            } else {
                SET_CDR(tail, r);
                tail = r;
            }
        }
        return head;
    }

    static u0 format_environment(ctx_t* ctx,
                                 obj_t* env,
                                 fmt_ctx_t& fmt_ctx,
                                 u32 indent) {
        auto e = ENV(env);
        if (indent > 0) {
            fmt::format_to(fmt_ctx.out(),
                           "{:<{}}(",
                           " ",
                           indent);
        }
        b8 first = true;
        printable_t prt{ctx, .obj = {}, .quote = true};
        prt.pretty.indent  = 4;
        prt.pretty.margin  = 37;
        prt.pretty.enabled = true;
        for (const auto& pair : e->bindings) {
            prt.obj = pair.value;
            auto s = string::interned::get_slice(pair.key);
            if (!s)
                continue;
            if (first) {
                fmt::format_to(fmt_ctx.out(),
                               "({:<32} . {})\n",
                               *s,
                               prt);
                first = false;
            } else {
                fmt::format_to(fmt_ctx.out(),
                               "{:<{}}({:<32} . {})\n",
                               " ",
                               indent,
                               *s,
                               prt);
            }
        }
        if (IS_NIL(e->parent)) {
            fmt::format_to(fmt_ctx.out(),
                           "{:<{}}(parent . nil)",
                           " ",
                           indent);
        } else {
            fmt::format_to(fmt_ctx.out(),
                           "{:<{}}(parent . [environment {}])",
                           " ",
                           indent,
                           (u0*) e->parent);
        }
        fmt::format_to(fmt_ctx.out(), ")\n");
    }

    obj_t* quasiquote(ctx_t* ctx, obj_t* obj) {
        if (TYPE(obj) != obj_type_t::pair)
            return CONS(SYM("quote"), CONS1(obj));
        auto a = CAR(obj);
        if (equal(ctx, a, SYM("unquote-splicing"))) {
            return error(ctx,
                         "unquote-splicing not valid in this context");
        }
        if (equal(ctx, a, SYM("unquote")))
            return CADR(obj);
        if (TYPE(a) == obj_type_t::pair) {
            auto aa = CAAR(obj);
            if (equal(ctx, aa, SYM("unquote-splicing"))) {
                obj_t* t1[] = {SYM("append"),
                               CADR(a),
                               quasiquote(ctx, CDR(obj))};
                return make_list(ctx, t1, 3);
            }
        }
        obj_t* t1[] = {SYM("cons"),
                       quasiquote(ctx, a),
                       quasiquote(ctx, CDR(obj))};
        return make_list(ctx, t1, 3);
    }

    u0 define(ctx_t* ctx, obj_t* sym, obj_t* v) {
        auto env = top_env(ctx);
        const auto str_id = FIXNUM(sym);
        const auto name = *string::interned::get_slice(str_id);
        auto chain = ctx->handlers.chain;
        while (chain) {
            if (chain->define_enabled && chain->define) {
                if (chain->define(ctx, name, v, env))
                    return;
            }
            chain = chain->next;
        }
        auto e = ENV(env);
        if (!hashtab::set(e->bindings, str_id, v))
            hashtab::insert(e->bindings, str_id, v);
    }

    u0 finalize_environment(ctx_t* ctx, obj_t* env) {
        check_type(ctx, env, obj_type_t::environment);
        auto e = ENV(env);
        e->free = true;
    }

    u0 print(fmt_buf_t& buf, ctx_t* ctx, obj_t* obj) {
        format::format_to(buf, "{}\n", printable_t{ctx, obj});
    }

    u0 write(fmt_buf_t& buf, ctx_t* ctx, obj_t* obj) {
        format::format_to(buf, "{}", printable_t{ctx, obj});
    }

    str_t to_string(ctx_t* ctx, obj_t* obj, b8 quote) {
        switch (TYPE(obj)) {
            case obj_type_t::nil:
                return str_t("nil"_ss);
            case obj_type_t::pair:
                return format::format("{}",
                                      printable_t{ctx, obj, quote});
            case obj_type_t::prim: {
                auto name = s_prim_names[u32(PRIM(obj))];
                return str_t(name);
            }
            case obj_type_t::fixnum:
                return format::format("{}", FIXNUM(obj));
            case obj_type_t::flonum:
                return format::format("{.7g}", FLONUM(obj));
            case obj_type_t::string: {
                const auto slice = *string::interned::get_slice(STRING_ID(obj));
                if (quote) {
                    return format::format("\"{}\"", slice);
                } else {
                    return format::format("{}", slice);
                }
            }
            case obj_type_t::symbol:
            case obj_type_t::keyword:
                return str_t(*string::interned::get_slice(STRING_ID(obj)));
            case obj_type_t::boolean:
                return str_t(IS_TRUE(obj) ? "#t"_ss : "#f"_ss);
            default: {
                auto name = s_type_names[u32(TYPE(obj))];
                return str_t(name);
            }
        }
    }

    obj_t* make_environment(ctx_t* ctx, obj_t* parent) {
        assert(parent && "parent env cannot be null!");
        env_t* env{};
        for (u32 i = 0; i < ctx->environments.size; ++i) {
            auto e = &ctx->environments[i];
            if (e->free) {
                hashtab::reset(e->bindings);
                env = e;
                break;
            }
        }
        if (!env) {
            env = &array::append(ctx->environments);
            hashtab::init(env->bindings, ctx->alloc);
            array::append(ctx->native_ptrs, env);
            env->native_ptr_idx = ctx->native_ptrs.size;
        }
        env->free    = false;
        env->parent  = parent ? parent : ctx->nil;
        env->protect = true;
        obj_t* obj = make_object(ctx);
        env->self = obj;
        SET_TYPE(obj, obj_type_t::environment);
        SET_FIXNUM(obj, env->native_ptr_idx);
        return obj;
    }

    u0 args_to_env(ctx_t* ctx, obj_t* prm, obj_t* arg) {
        while (!IS_NIL(prm)) {
            if (TYPE(prm) != obj_type_t::pair) {
                define(ctx, prm, arg);
                break;
            }
            auto k = CAR(prm);
            auto v = CAR(arg);
            define(ctx, k, v);
            prm = CDR(prm);
            arg = CDR(arg);
        }
    }

    u0 set_error_handler(ctx_t* ctx, error_func_t func) {
        ctx->handlers.error = func;
    }

    obj_t* ffi_apply(ctx_t* ctx, obj_t* fn, obj_t* args) {
        auto proto = PROTO(fn);
        obj_t* err = ctx->nil;
        param_alias_t ret{};
        u8            sig_buf[32];
        u32           p;
        u32           i;
        u32           len{};
        ffi::status_t status{};

        ffi_t ffi{};
        ffi::init(ffi, 1024, ctx->alloc);

        keyword_table_t keywords{};
        hashtab::init(keywords, ctx->alloc);

        rest_array_t rest{};
        array::init(rest, ctx->alloc);

        err = make_ffi_signature(ctx, args, rest, sig_buf, len, keywords);
        if (IS_SCM_ERROR(err))
            return err;

        auto ol = ffi::proto::match_signature(proto, sig_buf, len);
        if (!ol) {
            err = error(ctx,
                        "[ffi] no matching overload for function: {}",
                        proto->name);
            goto _error;
        }
        for (p = 0, i = 0; i < rest.size; ++p, ++i) {
            const auto param = ol->params[p];
            auto       arg   = rest[i];
            if (p >= ol->params.size) {
                err = error(ctx,
                            "[ffi] too many arguments: {}@{}",
                            ol->name,
                            proto->name);
                goto _error;
            }
            if (p >= ol->req_count) {
                if (param->has_dft) {
                    arg = hashtab::find(keywords, param->name);
                    if (!arg) {
                        ffi::push(ffi, param->value);
                        --i;
                        continue;
                    }
                } else if (param->is_rest) {
                    break;
                } else {
                    err = error(ctx,
                                "[ffi] too many arguments: {}@{}",
                                ol->name,
                                proto->name);
                    goto _error;
                }
            }
            switch (TYPE(arg)) {
                case obj_type_t::nil:
                    switch (ffi_type_t(param->value.type.user)) {
                        case ffi_type_t::list:
                        case ffi_type_t::object:
                        case ffi_type_t::context:
                            ffi::push(ffi, ctx->nil);
                            break;
                        default:
                            ffi::push(ffi, (u0*) nullptr);
                            break;
                    }
                    break;
                case obj_type_t::ptr:
                    switch (ffi_type_t(param->value.type.user)) {
                        case ffi_type_t::object:
                            ffi::push(ffi, arg);
                            break;
                        case ffi_type_t::context:
                            ffi::push(ffi, ctx);
                            break;
                        default:
                            ffi::push(ffi, NATIVE_PTR(arg));
                            break;
                    }
                    break;
                case obj_type_t::pair:
                    switch (ffi_type_t(param->value.type.user)) {
                        case ffi_type_t::list:
                        case ffi_type_t::object:
                            ffi::push(ffi, arg);
                            break;
                        default: {
                            err = error(ctx,
                                        "[ffi] invalid pair argument");
                            goto _error;
                        }
                    }
                    break;
                case obj_type_t::fixnum:
                    switch (param->value.type.cls) {
                        case param_cls_t::int_:
                            ffi::push(ffi, FIXNUM(arg));
                            break;
                        case param_cls_t::float_:
                            ffi::push(ffi, flonum_t(FIXNUM(arg)));
                            break;
                        default: {
                            err = error(ctx,
                                        "[ffi] invalid integer conversion");
                            goto _error;
                        }
                    }
                    break;
                case obj_type_t::flonum:
                    switch (param->value.type.cls) {
                        case param_cls_t::int_:
                            ffi::push(ffi, fixnum_t(FLONUM(arg)));
                            break;
                        case param_cls_t::float_:
                            ffi::push(ffi, FLONUM(arg));
                            break;
                        default: {
                            err = error(ctx,
                                        "[ffi] invalid float conversion");
                            goto _error;
                        }
                    }
                    break;
                case obj_type_t::symbol:
                case obj_type_t::string:
                case obj_type_t::keyword: {
                    auto s = string::interned::get_slice(STRING_ID(arg));
                    if (!s) {
                        err = error(ctx,
                                    "[ffi] invalid interned string id: {}",
                                    STRING_ID(arg));
                        goto _error;
                    }
                    ffi::push(ffi, s);
                    break;
                }
                case obj_type_t::boolean:
                    ffi::push(ffi, arg == ctx->true_ ? true : false);
                    break;
                default: {
                    switch (ffi_type_t(param->value.type.user)) {
                        case ffi_type_t::context:
                            ffi::push(ffi, ctx);
                            break;
                        default:
                            err = error(ctx,
                                        "[ffi] unsupported scm object type");
                            goto _error;
                    }
                }
            }
        }

        if (ol->has_dft) {
            const auto remaining_slots = ol->has_rest ? ol->params.size - 1 :
                                         ol->params.size;
            for (u32 j = p; j < remaining_slots; ++j)
                ffi::push(ffi, ol->params[j]->value);
        }

        if (ol->has_rest) {
            array::erase(rest, 0, i);
            ffi::push(ffi, &rest);
        }

        status = ffi::call(ffi, ol, ret);
        if (!OK(status)) {
           err = error(ctx, "[ffi] call failed: {}", ol->name);
           goto _error;
        }

        ffi::free(ffi);
        array::free(rest);
        hashtab::free(keywords);

        switch (ol->ret_type.cls) {
            case param_cls_t::ptr:
                return !ret.p ? ctx->nil : (obj_t*) ret.p;
            case param_cls_t::int_:
                if (ffi_type_t(ol->ret_type.user) == ffi_type_t::boolean) {
                    return make_bool(ctx, ret.b);
                } else {
                    return make_fixnum(ctx, ret.dw);
                }
            case param_cls_t::void_:
                return ctx->nil;
            case param_cls_t::float_:
                return make_flonum(ctx, ret.fdw);
            default:
                return error(ctx, "[ffi] unsupported return type");
        }

    _error:
        ffi::free(ffi);
        array::free(rest);
        hashtab::free(keywords);
        return err;
    }

    obj_t* make_list(ctx_t* ctx, obj_t** objs, u32 size) {
        obj_t* res = ctx->nil;
        while (size--)
            res = cons(ctx, objs[size], res);
        return res;
    }

    static num_type_t get_numtype(const s8* buf, s32 len) {
        const s8* p = buf;
        s8         ch   = *p;
        num_type_t type = ((ch == '+' && len > 1)
                           || (ch == '-' && len > 1)
                           || isdigit(ch)) ? num_type_t::fixnum : num_type_t::none;
        if (type == num_type_t::none)
            return type;
        while (--len > 0) {
            ch = *(++p);
            if (ch == '.' || ch == 'e' || ch == 'E' || ch == '-') {
                type = num_type_t::flonum;
            } else if (ch < 48 || ch > 57) {
                type = num_type_t::none;
                break;
            }
        }
        return type;
    }

    obj_t* make_native_func(ctx_t* ctx, native_func_t fn) {
        obj_t* obj = make_object(ctx);
        array::append(ctx->native_ptrs, (u0*) fn);
        SET_TYPE(obj, obj_type_t::cfunc);
        SET_FIXNUM(obj, ctx->native_ptrs.size);
        return obj;
    }

    static obj_t* read_expr(ctx_t* ctx, buf_crsr_t& crsr) {
        obj_t* v;
        s8 chr{};

        next:
        chr = CRSR_READ(crsr);

        while (chr && isspace(chr)) {
            if (chr == '\n')
                CRSR_NEWLINE(crsr);
            CRSR_NEXT(crsr);
            chr = CRSR_READ(crsr);
        }

        switch (chr) {
            case '\0':
                return nullptr;

            case ';':
                do {
                    CRSR_NEXT(crsr);
                    chr = CRSR_READ(crsr);
                } while (chr && chr != '\n');
                CRSR_NEXT(crsr); CRSR_NEWLINE(crsr);
                goto next;

            case ']': {
                CRSR_NEXT(crsr);
                auto tos = stack::pop(ctx->cl_stack);
                if (tos == ctx->rparen) {
                    return error(ctx,
                                 "expected closing paren but found closing bracket");
                }
                return ctx->rbrac;
            }

            case ')': {
                CRSR_NEXT(crsr);
                auto tos = stack::pop(ctx->cl_stack);
                if (tos == ctx->rbrac) {
                    return error(ctx,
                                 "expected closing bracket but found closing paren");
                }
                return ctx->rparen;
            }

            case '[':
                stack::push(ctx->cl_stack, ctx->rbrac);
                goto _skip;

            case '(':
                stack::push(ctx->cl_stack, ctx->rparen);
            _skip: {
                CRSR_NEXT(crsr);
                obj_t* head = ctx->nil;
                obj_t* tail = head;
                auto gc = save_gc(ctx);
                push_gc(ctx, head);
                while (true) {
                    v = read_expr(ctx, crsr);
                    if (v == ctx->rparen || v == ctx->rbrac)
                        break;
                    if (v == nullptr)
                        return error(ctx, "unclosed list");
                    if (v == ctx->dot) {
                        SET_CDR(tail, read(ctx, crsr));
                    } else {
                        v = cons(ctx, v, ctx->nil);
                        if (IS_NIL(tail)) {
                            head = v;
                            tail = head;
                        } else {
                            SET_CDR(tail, v);
                            tail = v;
                        }
                    }
                    restore_gc(ctx, gc);
                    push_gc(ctx, head);
                }
                return head;
            }

            case '`': {
                CRSR_NEXT(crsr);
                v = read(ctx, crsr);
                if (!v)
                    return error(ctx, "stray '`'");
                return cons(ctx,
                            make_symbol(ctx, "quasiquote", 10),
                            cons(ctx, v, ctx->nil));
            }

            case ',': {
                CRSR_NEXT(crsr);
                obj_t* sym;
                if (CRSR_READ(crsr) == '@') {
                    CRSR_NEXT(crsr);
                    v = read(ctx, crsr);
                    if (!v)
                        return error(ctx, "stray ',@'");
                    sym = make_symbol(ctx, "unquote-splicing", 16);
                } else {
                    v = read(ctx, crsr);
                    if (!v)
                        return error(ctx, "stray ','");
                    sym = make_symbol(ctx, "unquote", 7);
                }
                return cons(ctx, sym, cons(ctx, v, ctx->nil));
            }

            case '\'': {
                CRSR_NEXT(crsr);
                v = read(ctx, crsr);
                if (!v)
                    return error(ctx, "stray '''");
                return cons(ctx,
                            make_symbol(ctx, "quote", 5),
                            cons(ctx, v, ctx->nil));
            }

            case '"': {
                CRSR_NEXT(crsr);
                u32 s = CRSR_POS(crsr);
                u32 e = CRSR_END(crsr);
                u32 p = s;
                while ((chr = CRSR_READ(crsr) != '"')) {
                    if (p == e)
                        return error(ctx, "string too long");
                    if (chr == '\0')
                        return error(ctx, "unclosed string");
                    if (chr == '\\') {
                        CRSR_NEXT(crsr); chr = CRSR_READ(crsr);
                        if (strchr("nrt", chr))
                            chr = strchr("n\nr\rt\t", chr)[1];
                    }
                    CRSR_NEXT(crsr); p = CRSR_POS(crsr);
                }
                CRSR_NEXT(crsr);
                return make_string(ctx, (const s8*) crsr.buf->data + s, p - s);
            }

            default: {
                u32 s = CRSR_POS(crsr);
                u32 e = CRSR_END(crsr);
                u32 p = s;
                do {
                    if (p == e)
                        return error(ctx, "symbol too long");
                    CRSR_NEXT(crsr); chr = CRSR_READ(crsr);
                    p = CRSR_POS(crsr);
                } while (chr && !strchr(s_delims, chr));
                const s8*  start = (const s8*) (crsr.buf->data + s);
                const auto len = p - s;
                s8* end = (s8*) (start + len);
                if (p != s) {
                    switch (get_numtype(start, len)) {
                        case num_type_t::none:
                            if (strncmp(start, "#:", 2) == 0) {
                                return make_keyword(ctx, start + 2, len - 2);
                            } else {
                                return make_symbol(ctx, start, len);
                            }
                        case num_type_t::fixnum:
                            return make_fixnum(ctx, strtol(start, &end, 10));
                        case num_type_t::flonum:
                            return make_flonum(ctx, strtod(start, &end));
                    }
                } else {
                    return error(ctx, "expected flonum, fixnum, keyword, or symbol");
                }
            }
        }

        return ctx->nil;
    }

    obj_t* cfunc_apply(ctx_t* ctx, obj_t* fn, obj_t* args) {
        return CFUNC(fn)(ctx, eval_list(ctx, args));
    }

    obj_t* make_string(ctx_t* ctx, const s8* str, s32 len) {
        const auto rc = string::interned::fold_for_result(str, len);
        if (!OK(rc.status)) {
            return error(ctx,
                         "make_string: unable to intern string: {}", str);
        }
        auto obj = hashtab::find(ctx->strtab, rc.id);
        if (obj)
            return obj;
        obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::string);
        SET_FIXNUM(obj, rc.id);
        hashtab::insert(ctx->strtab, rc.id, obj);
        return obj;
    }

    obj_t* make_symbol(ctx_t* ctx, const s8* name, s32 len) {
        const auto rc = string::interned::fold_for_result(name, len);
        if (!OK(rc.status)) {
            return error(ctx,
                         "make_symbol: unable to intern string: {}", name);
        }
        auto obj = hashtab::find(ctx->symtab, rc.id);
        if (obj)
            return obj;
        obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::symbol);
        SET_FIXNUM(obj, rc.id);
        hashtab::insert(ctx->symtab, rc.id, obj);
        return obj;
    }

    obj_t* make_keyword(ctx_t* ctx, const s8* name, s32 len) {
        const auto rc = string::interned::fold_for_result(name, len);
        if (!OK(rc.status)) {
            return error(ctx,
                         "make_keyword: unable to intern string: {}", name);
        }
        auto obj = hashtab::find(ctx->symtab, rc.id);
        if (obj)
            return obj;
        obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::keyword);
        SET_FIXNUM(obj, rc.id);
        hashtab::insert(ctx->symtab, rc.id, obj);
        return obj;
    }

    obj_t* check_type(ctx_t* ctx, obj_t* obj, obj_type_t type) {
        if (TYPE(obj) != type) {
            return error(ctx,
                         "expected {}, got {}",
                         s_type_names[u32(type)],
                         s_type_names[u32(TYPE(obj))]);
        }
        return obj;
    }

    u0 set_next_handler(ctx_t* ctx, chained_handler_t* handler) {
        if (!ctx->handlers.chain) {
            ctx->handlers.chain = handler;
        } else {
            handler->next = ctx->handlers.chain;
            ctx->handlers.chain = handler;
        }
    }

    obj_t* make_error(ctx_t* ctx, obj_t* args, obj_t* call_stack) {
        obj_t* obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::error);
        SET_CDR(obj, cons(ctx, args, cons(ctx, call_stack, ctx->nil)));
        return obj;
    }

    obj_t* raise_error(ctx_t* ctx, fmt_str_t fmt_msg, fmt_args_t args) {
        if (IS_SCM_ERROR(ctx->err))
            return ctx->err;

        obj_t* cl = ctx->call_list;
        str_t str{};
        str::init(str, ctx->alloc);
        {
            str_buf_t buf(&str);
            format::format_to(buf, "error: ");
            fmt::vformat_to(buf, fmt_msg, args);
            if (cl && !IS_NIL(cl)) {
                format::format_to(buf, "\n");
                for (; !IS_NIL(cl); cl = CDR(cl)) {
                    format::format_to(buf,
                                      "=> {}\n",
                                      printable_t{ctx, CAR(cl)});
                }
            }
        }

        auto irritants = CONS1(make_string(ctx, str));
        auto obj = make_error(ctx, irritants, ctx->call_list);
        ctx->call_list = ctx->nil;
        if (ctx->handlers.error)
            ctx->handlers.error(ctx, obj);
        ctx->err = obj;
        return obj;
    }

    static print_rule_t s_print_rules[] = {
        {"begin"_ss,        false,  true},
        {"if"_ss,           true,   true},
        {"while"_ss,        true,   true},
        {"lambda"_ss,       true,   true},
        {str::slice_t{}},
    };

    static print_rule_t* find_print_rule(ctx_t* ctx, obj_t* obj) {
        if (TYPE(obj) == obj_type_t::prim
        &&  PRIM(obj) == prim_type_t::begin) {
            return &s_print_rules[0];
        }
        if (TYPE(obj) != obj_type_t::symbol)
            return nullptr;
        const auto& s = *string::interned::get_slice(STRING_ID(obj));
        for (u32 i = 0; s_print_rules[i].symbol.length > 0; ++i) {
            if (s_print_rules[i].symbol == s)
                return &s_print_rules[i];
        }
        return nullptr;
    }

    u0 format_object(const printable_t& printable, fmt_ctx_t& fmt_ctx) {
        auto o   = fmt_ctx.out();
        auto qt  = printable.quote;
        auto ctx = printable.ctx;
        auto obj = printable.obj;
        auto& pretty = printable.pretty;
        auto indent  = pretty.indent;

        switch (TYPE(obj)) {
            case obj_type_t::nil:
                fmt::format_to(o, "nil");
                break;

            case obj_type_t::pair: {
                obj_t* p = obj;
                if (pretty.enabled) {
                    printable_t prt{ctx, ctx->nil, qt};
                    prt.pretty.margin  = pretty.margin;
                    prt.pretty.enabled = true;

                    auto& c = get_container(o);
                    s32 skip_count = -1;
                    auto print_rule = find_print_rule(ctx, CAR(p));
                    if (print_rule) {
                        if (print_rule->skip_first_arg)
                            skip_count = 1;
                        if (print_rule->increase_indent) {
                            fmt::format_to(o,
                                           "\n{:<{}}",
                                           " ",
                                           pretty.margin + indent);
                            indent += 4;
                        }
                    }

                    auto pos = c.size();
                    prt.pretty.indent = indent;
                    fmt::format_to(o, "(");
                    for (;;) {
                        auto a = CAR(p);
                        prt.obj = a;
                        fmt::format_to(o, "{}", prt);
                        p = CDR(p);
                        if (TYPE(p) != obj_type_t::pair)
                            break;
                        if (skip_count > 0) {
                            --skip_count;
                            fmt::format_to(o, " ");
                        } else if (skip_count == 0) {
                            skip_count = -1;
                            fmt::format_to(o,
                                           "\n{:<{}}",
                                           " ",
                                           pretty.margin + indent);
                        } else {
                            const auto width = c.size() - pos;
                            if (width > 30) {
                                fmt::format_to(o,
                                               "\n{:<{}}",
                                               " ",
                                               pretty.margin + indent);
                                pos = c.size();
                            } else {
                                fmt::format_to(o, " ");
                            }
                        }
                    }
                    if (!IS_NIL(p)) {
                        fmt::format_to(o,
                                       " . {}",
                                       printable_t{ctx, p, qt});
                    }
                    fmt::format_to(o, ")");
                } else {
                    fmt::format_to(o, "(");
                    for (;;) {
                        fmt::format_to(o,
                                       "{}",
                                       printable_t{ctx, CAR(p), qt});
                        p = CDR(p);
                        if (TYPE(p) != obj_type_t::pair)
                            break;
                        fmt::format_to(o, " ");
                    }
                    if (!IS_NIL(p))
                        fmt::format_to(o,
                                       " . {}",
                                       printable_t{ctx, p, qt});
                    fmt::format_to(o, ")");
                }
                break;
            }

            case obj_type_t::prim:
                fmt::format_to(o, "{}", s_prim_names[u32(PRIM(obj))]);
                break;

            case obj_type_t::proc:
            case obj_type_t::lambda: {
                auto proc = PROC(obj);
                if (pretty.enabled) {
                    printable_t prt{};
                    prt.ctx   = ctx;
                    prt.obj   = proc->body;
                    prt.quote = qt;
                    prt.pretty.indent  = 4;
                    prt.pretty.enabled = true;
                    prt.pretty.margin  = pretty.margin;
                    fmt::format_to(o,
                                   "(lambda {} {})",
                                   printable_t{ctx, proc->params, qt},
                                   prt);
                } else {
                    fmt::format_to(o,
                                   "(lambda {} {})",
                                   printable_t{ctx, proc->params, qt},
                                   printable_t{ctx, proc->body, qt});
                }
                break;
            }

            case obj_type_t::fixnum:
                fmt::format_to(o, "{}", to_fixnum(obj));
                break;

            case obj_type_t::flonum:
                fmt::format_to(o, "{:.7g}", to_flonum(obj));
                break;

            case obj_type_t::error: {
                auto err_args   = error_args(ctx, obj);
                auto args       = CAR(err_args);
                fmt::format_to(o,
                               "{}",
                               printable_t{ctx, CAR(args), false});
                break;
            }

            case obj_type_t::keyword:
                fmt::format_to(o, "#:");
            case obj_type_t::symbol:
                fmt::format_to(o,
                               "{}",
                               *string::interned::get_slice(STRING_ID(obj)));
                break;

            case obj_type_t::string:
                if (printable.quote) {
                    fmt::format_to(o,
                                   "\"{}\"",
                                   *string::interned::get_slice(STRING_ID(obj)));
                } else {
                    fmt::format_to(o,
                                   "{}",
                                   *string::interned::get_slice(STRING_ID(obj)));
                }
                break;

            case obj_type_t::boolean:
                fmt::format_to(o, "{}", IS_TRUE(obj) ? "#t" : "#f");
                break;

            case obj_type_t::environment:
                format_environment(ctx, obj, fmt_ctx, 0);
                break;

            default:
                fmt::format_to(o, "[{:<8} {}]", type_name(obj), (u0*) obj);
                break;
        }
    }
}
