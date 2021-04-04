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

//    format::print("Address            Offset             Size    Reg Value              Top\n");
//    format::print("-----------------------------------------------------------------------------\n");
//    for (const auto& entry : vm.memory_map.entries) {
//        if (!entry.valid)
//            continue;
//        format::print("0x{:016X} 0x{:016X} 0x{:05X}",
//                      entry.addr,
//                      entry.offs,
//                      entry.size);
//        format::print(" {:<2}  0x{:016X} {}\n",
//                      scm::register_file::name(entry.reg),
//                      G(entry.reg),
//                      entry.top ? "  X" : "");
//    }
//    format::print("\n");
//

#include <basecode/core/scm/scm.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/scm/types.h>
#include <basecode/core/scm/compiler.h>

namespace basecode::scm {
    static const s8* s_prim_names[] = {
        "eval",
        "let",
        "=",
        "if",
        "fn",
        "mac",
        "while",
        "error",
        "quote",
        "unquote",
        "quasiquote",
        "unquote-splicing",
        "and",
        "or",
        "do",
        "cons",
        "car",
        "cdr",
        "setcar",
        "setcdr",
        "list",
        "not",
        "is",
        "atom",
        "print",
        ">",
        ">=",
        "<",
        "<=",
        "+",
        "-",
        "*",
        "/",
        "mod"
    };

    static const s8* s_type_names[] = {
        "pair",
        "free",
        "nil",
        "fixnum",
        "flonum",
        "symbol",
        "string",
        "func",
        "macro",
        "prim",
        "cfunc",
        "ptr",
        "boolean",
        "keyword",
        "ffi",
        "error",
        "environment",
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

    static b8 equal(ctx_t* ctx, obj_t* a, obj_t* b);

    static numtype_t get_numtype(const s8* buf, s32 len);

    static obj_t* read_expr(ctx_t* ctx, buf_crsr_t& crsr);

    [[maybe_unused]] static obj_t* reverse(ctx_t* ctx, obj_t* obj);

    static obj_t* check_type(ctx_t* ctx, obj_t* obj, obj_type_t type);

    static u0 args_to_env(ctx_t* ctx, obj_t* prm, obj_t* arg, obj_t* env);

    [[maybe_unused]] static u0 finalize_environment(ctx_t* ctx, obj_t* env);

    static u32 make_ffi_signature(ctx_t* ctx, obj_t* args, obj_t* env, u8* buf);

    static u0 format_environment(ctx_t* ctx, obj_t* env, fmt_ctx_t& fmt_ctx, u32 indent);

    u0 free(ctx_t* ctx) {
        collect_garbage(ctx);
        array::free(ctx->native_ptrs);
        stack::free(ctx->cl_stack);
        stack::free(ctx->gc_stack);
        hashtab::free(ctx->strtab);
        hashtab::free(ctx->symtab);
        for (auto& env : ctx->environments)
            hashtab::free(env.bindings);
        array::free(ctx->environments);
        array::free(ctx->procedures);
        ffi::free(ctx->ffi);
        vm::emitter::free(ctx->emitter);
        vm::free(ctx->vm);
    }

    obj_t* nil(ctx_t* ctx) {
        return ctx->nil;
    }

    u32 save_gc(ctx_t* ctx) {
        return ctx->gc_stack.size;
    }

    obj_t* pop_gc(ctx_t* ctx) {
        return stack::pop(ctx->gc_stack);
    }

    obj_type_t type(obj_t* obj) {
        return TYPE(obj);
    }

    u0 collect_garbage(ctx_t* ctx) {
//        format::print("before: ctx->object_used = {}\n", ctx->object_used);
        for (const auto& pair : ctx->strtab)
            mark(ctx, pair.value);
        for (const auto& pair : ctx->symtab)
            mark(ctx, pair.value);
        for (auto& env : ctx->environments) {
            if (!env.gc_protect)
                continue;
            for (const auto& pair : env.bindings)
                mark(ctx, pair.value);
        }
        for (u32 i = 0; i < ctx->gc_stack.size; i++)
            mark(ctx, ctx->gc_stack[i]);
        for (u32 i = 0; i < ctx->object_count; i++) {
            obj_t* obj = &ctx->objects[i];
            if (TYPE(obj) == obj_type_t::free)
                continue;
            if (!IS_GC_MARKED(obj)) {
                switch (TYPE(obj)) {
                    case obj_type_t::ptr:
                        if (ctx->handlers.gc)
                            ctx->handlers.gc(ctx, obj);
                        break;

                    case obj_type_t::environment: {
                        auto e = (env_t*) NATIVE_PTR(CDR(obj));
                        e->parent = ctx->nil;
                        hashtab::free(e->bindings);
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
//        format::print("after : ctx->object_used = {}\n", ctx->object_used);
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
                mark(ctx, car);         /* fall through */
            case obj_type_t::func:
            case obj_type_t::macro:
                obj = CDR(obj);
                goto begin;

            case obj_type_t::ptr:
                if (ctx->handlers.mark)
                    ctx->handlers.mark(ctx, obj);
                break;

            default:
                break;
        }
    }

    handlers_t* handlers(ctx_t* ctx) {
        return &ctx->handlers;
    }

    b8 is_nil(ctx_t* ctx, obj_t* obj) {
        return IS_NIL(obj);
    }

    u0 restore_gc(ctx_t* ctx, u32 idx) {
        stack::truncate(ctx->gc_stack, idx);
    }

    b8 is_true(ctx_t* ctx, obj_t* obj) {
        return ctx->true_ == obj;
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
                if (!OK(rc.status))
                    error(ctx, "unable to find interned string: {}", STRING_ID(obj));
                return rc.slice.length;
            }
            default:
                break;
        }
        return 0;
    }

    u0 push_gc(ctx_t* ctx, obj_t* obj) {
        if (obj < ctx->objects || obj >= ctx->objects + ctx->object_count)
            error(ctx, "fatal: obj address outside of heap range! {}", (u0*) obj);
        stack::push(ctx->gc_stack, obj);
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
        return get(ctx, sym, ctx->env);
    }

    obj_t* eval(ctx_t* ctx, obj_t* obj) {
        return eval(ctx, obj, ctx->env);
    }

    obj_t* eval2(ctx_t* ctx, obj_t* obj) {
        auto& bb = vm::emitter::make_basic_block(ctx->emitter);
        auto reg = vm::reg_alloc::reserve(ctx->emitter.gp);
        TIME_BLOCK(
            "compile expr"_ss,
            auto tc = make_context(bb, ctx, obj, ctx->env, reg[0], true);
            auto& comp_bb = scm::compile(tc);
            UNUSED(comp_bb);
            vm::basic_block::imm1(
                bb,
                instruction::type::exit,
                vm::emitter::imm(1, imm_type_t::boolean));
            );
        str_t str{};
        str::init(str, ctx->alloc);
        {
            str_buf_t buf{&str};
            vm::emitter::disassemble(ctx->emitter, bb, buf);
        }
        format::print("{}\n", str);
        return ctx->nil;
    }

    static obj_t* make_object(ctx_t* ctx) {
        obj_t* obj;
        if (IS_NIL(ctx->free_list)) {
            collect_garbage(ctx);
            if (IS_NIL(ctx->free_list))
                error(ctx, "out of memory");
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

    u0* to_user_ptr(ctx_t* ctx, obj_t* obj) {
        return ctx->native_ptrs[FIXNUM(check_type(ctx, obj, obj_type_t::ptr))];
    }

    str_t to_string(ctx_t* ctx, obj_t* obj) {
        switch (TYPE(obj)) {
            case obj_type_t::nil:
                return str_t("nil"_ss);
            case obj_type_t::pair:
                return format::format("{}",
                                      printable_t{ctx, obj, true});
            case obj_type_t::prim: {
                auto name = s_prim_names[u32(PRIM(obj))];
                return str_t(name);
            }
            case obj_type_t::fixnum:
                return format::format("{}", FIXNUM(obj));
            case obj_type_t::flonum:
                return format::format("{.7g}", FLONUM(obj));
            case obj_type_t::symbol:
            case obj_type_t::string:
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

    obj_t* make_bool(ctx_t* ctx, obj_t* obj) {
        return IS_TRUE(obj) ? ctx->true_ : ctx->false_;
    }

    u0 set(ctx_t* ctx, obj_t* sym, obj_t* v) {
        set(ctx, sym, v, ctx->env);
    }

    obj_t* next_arg(ctx_t* ctx, obj_t** arg) {
        obj_t* a = *arg;
        if (TYPE(a) != obj_type_t::pair) {
            if (IS_NIL(a))
                error(ctx, "too few arguments");
            error(ctx, "dotted pair in argument list");
        }
        *arg = CDR(a);
        return CAR(a);
    }

    obj_t* make_user_ptr(ctx_t* ctx, u0* ptr) {
        obj_t* obj = make_object(ctx);
        array::append(ctx->native_ptrs, ptr);
        SET_TYPE(obj, obj_type_t::ptr);
        SET_FIXNUM(obj, ctx->native_ptrs.size - 1);
        return obj;
    }

    obj_t* error_args(ctx_t* ctx, obj_t* obj) {
        return CDR(check_type(ctx, obj, obj_type_t::error));
    }

    obj_t* read(ctx_t* ctx, buf_crsr_t& crsr) {
        obj_t* obj = read_expr(ctx, crsr);
        if (obj == ctx->rparen) {
            error(ctx, "stray ')'");
        }
        if (obj == ctx->rbrac) {
            error(ctx, "stray ']'");
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
        SET_FIXNUM(obj, ctx->native_ptrs.size - 1);
        return obj;
    }

    u0 print(FILE* file, ctx_t* ctx, obj_t* obj) {
        format::print(file, "{}\n", printable_t{ctx, obj});
    }

    u0 write(FILE* file, ctx_t* ctx, obj_t* obj) {
        format::print(file, "{}", printable_t{ctx, obj});
    }

    ctx_t* init(u0* ptr, u32 size, alloc_t* alloc) {
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
        vm::init(vm, ctx->alloc, size);
        vm::memory_map(vm, scm::memory_area_t::code, scm::register_file::lp, 256 * 1024);
        vm::memory_map(vm, scm::memory_area_t::heap, scm::register_file::hp, 64 * 1024);
        vm::memory_map(vm, scm::memory_area_t::env_stack, scm::register_file::ep, 4 * 1024, true);
        vm::memory_map(vm, scm::memory_area_t::data_stack, scm::register_file::dp, 4 * 1024, true);
        vm::reset(vm);
        vm::emitter::init(ctx->emitter, &vm, LP, ctx->alloc);
        array::init(ctx->native_ptrs, ctx->alloc);
        array::reserve(ctx->native_ptrs, 256);
        stack::init(ctx->gc_stack, ctx->alloc);
        stack::reserve(ctx->gc_stack, 1024);
        stack::init(ctx->cl_stack, ctx->alloc);
        stack::reserve(ctx->cl_stack, 1024);
        hashtab::init(ctx->strtab, ctx->alloc);
        hashtab::init(ctx->symtab, ctx->alloc);
        array::init(ctx->environments, ctx->alloc);
        array::init(ctx->procedures, ctx->alloc);

        // init objects
        ctx->nil       = &ctx->objects[ctx->object_used++];
        ctx->nil->full = 0;
        SET_TYPE(ctx->nil, obj_type_t::nil);

        // init lists
        ctx->call_list = ctx->nil;
        ctx->free_list = ctx->nil;

        // populate freelist
        for (s32 i = ctx->object_count - 1; i >= ctx->object_used; i--) {
            obj_t* obj = &ctx->objects[i];
            SET_TYPE(obj, obj_type_t::free);
            SET_CDR(obj, ctx->free_list);
            ctx->free_list = obj;
        }

        ctx->env       = make_environment(ctx, ctx->nil);

        /* true */ {
            ctx->true_ = make_object(ctx);
            SET_TYPE(ctx->true_, obj_type_t::boolean);
            SET_FIXNUM(ctx->true_, 1);
            set(ctx, make_symbol(ctx, "#t", 2), ctx->true_);
        }

        /* false */ {
            ctx->false_ = make_object(ctx);
            SET_TYPE(ctx->false_, obj_type_t::boolean);
            SET_FIXNUM(ctx->false_, 0);
            set(ctx, make_symbol(ctx, "#f", 2), ctx->false_);
        }

        ctx->rbrac  = SYM("]");
        ctx->rparen = SYM(")");
        ctx->dot    = SYM(".");
        set(ctx, ctx->dot, ctx->dot);
        set(ctx, ctx->rbrac, ctx->rbrac);
        set(ctx, SYM("nil"), ctx->nil);
        set(ctx, ctx->rparen, ctx->rparen);

        // register built in primitives
        for (u32 i = 0; i < u32(prim_type_t::max); i++) {
            obj_t* v = make_object(ctx);
            SET_TYPE(v, obj_type_t::prim);
            SET_PRIM(v, i);
            set(ctx, make_symbol(ctx, s_prim_names[i]), v);
        }

        ffi::init(ctx->ffi);

        return ctx;
    }

    static obj_t* reverse(ctx_t* ctx, obj_t* obj) {
        obj_t* res;
        for (res = ctx->nil; !IS_NIL(obj); obj = CDR(obj))
            res = cons(ctx, CAR(obj), res);
        return res;
    }

    obj_t* get(ctx_t* ctx, obj_t* sym, obj_t* env) {
        check_type(ctx, env, obj_type_t::environment);
        auto e     = (env_t*) NATIVE_PTR(CDR(env));
        auto value = hashtab::find(e->bindings, FIXNUM(sym));
        if (value)
            return value;
        if (IS_NIL(e->parent))
            return ctx->nil;
        return get(ctx, sym, e->parent);
    }

    obj_t* next_arg_no_chk(ctx_t* ctx, obj_t** arg) {
        obj_t* a = *arg;
        if (TYPE(a) != obj_type_t::pair)
            return ctx->nil;
        *arg = CDR(a);
        return CAR(a);
    }

    static b8 equal(ctx_t* ctx, obj_t* a, obj_t* b) {
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
            case obj_type_t::nil:
            case obj_type_t::free:      return true;
            case obj_type_t::pair:
            case obj_type_t::func:
            case obj_type_t::macro: {
                while (!IS_NIL(a)) {
                    if (IS_NIL(b))  return false;
                    if (!equal(ctx, CAR(a), CAR(b))
                    ||  !equal(ctx, CDR(a), CDR(b))) {
                        return false;
                    }
                    next_arg(ctx, &a);
                    next_arg(ctx, &b);
                }
                return true;
            }
            case obj_type_t::prim:      return PRIM(a) == PRIM(b);
            case obj_type_t::ptr:
            case obj_type_t::cfunc:     return NATIVE_PTR(a) == NATIVE_PTR(b);
            default:                    break;
        }
        return false;
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

        return -1;
    }

    obj_t* eval(ctx_t* ctx, obj_t* obj, obj_t* env) {
        obj_t* fn   {};
        obj_t* kar  {};
        obj_t* arg  {};
        obj_t* res  {};
        obj_t* va   {};
        obj_t* vb   {};
        obj_t* cl   {};
        u8          buf[16]{};
        u32    n    {};
        u32    gc   {save_gc(ctx)};

        while (!res) {
            if (TYPE(obj) == obj_type_t::symbol) {
                res = get(ctx, obj, env);
                break;
            }

            if (TYPE(obj) != obj_type_t::pair) {
                res = obj;
                break;
            }

            cl = cons(ctx, obj, ctx->call_list);
            ctx->call_list = cl;

            kar = CAR(obj);
            arg = CDR(obj);
            if (TYPE(kar) == obj_type_t::symbol)
                    fn = get(ctx, kar, env);
            else    fn = kar;

            switch (TYPE(fn)) {
                case obj_type_t::prim:
                    switch (PRIM(fn)) {
                        case prim_type_t::eval:
                            obj = EVAL_ARG();
                            break;

                        case prim_type_t::let:
                        case prim_type_t::set:
                            va = check_type(ctx, next_arg(ctx, &arg), obj_type_t::symbol);
                            set(ctx, va, EVAL_ARG(), env);
                            res = ctx->nil;
                            break;

                        case prim_type_t::if_:
                            obj = IS_TRUE(EVAL(CAR(arg))) ? CADR(arg) : CADDR(arg);
                            break;

                        case prim_type_t::fn:
                        case prim_type_t::mac:
                            res = make_proc(ctx, CAR(arg), CADR(arg), env, PRIM(fn) == prim_type_t::mac);
                            break;

                        case prim_type_t::error:
                            res = make_error(ctx, arg, cl);
                            break;

                        case prim_type_t::while_:
                            va = next_arg(ctx, &arg);
                            n  = save_gc(ctx);
                            while (true) {
                                vb = EVAL(va);
                                if (IS_NIL(vb) || IS_FALSE(vb))
                                    break;
                                vb = arg;
                                for (; !IS_NIL(vb); vb = CDR(vb)) {
                                    restore_gc(ctx, n);
                                    push_gc(ctx, vb);
                                    push_gc(ctx, env);
                                    eval(ctx, CAR(vb), env);
                                }
                                restore_gc(ctx, n);
                            }
                            res = ctx->nil;
                            break;

                        case prim_type_t::quote:
                            res = next_arg(ctx, &arg);
                            break;

                        case prim_type_t::unquote:
                            error(ctx, "unquote is not valid in this context.");
                            break;

                        case prim_type_t::quasiquote:
                            obj = quasiquote(ctx, next_arg(ctx, &arg), env);
                            break;

                        case prim_type_t::unquote_splicing:
                            error(ctx, "unquote-splicing is not valid in this context.");
                            break;

                        case prim_type_t::and_:
                            obj = ctx->nil;
                            for (; !IS_NIL(arg) && !IS_NIL(va = EVAL_ARG()); obj = va);
                            break;

                        case prim_type_t::or_:
                            while (!IS_NIL(arg) && IS_NIL(obj = EVAL_ARG()));
                            break;

                        case prim_type_t::do_: {
                            u32 save = save_gc(ctx);
                            for (; !IS_NIL(CDR(arg)); arg = CDR(arg)) {
                                restore_gc(ctx, save);
                                push_gc(ctx, arg);
                                push_gc(ctx, env);
                                eval(ctx, CAR(arg), env);
                            }
                            obj = CAR(arg);
                            break;
                        }

                        case prim_type_t::cons:
                            va  = EVAL_ARG();
                            res = cons(ctx, va, EVAL_ARG());
                            break;

                        case prim_type_t::car:
                            va = EVAL_ARG();
                            res = car(ctx, va);
                            break;

                        case prim_type_t::cdr:
                            va = EVAL_ARG();
                            res = cdr(ctx, va);
                            break;

                        case prim_type_t::setcar:
                            va = check_type(ctx, EVAL_ARG(), obj_type_t::pair);
                            SET_CAR(va, EVAL_ARG());
                            res = ctx->nil;
                            break;

                        case prim_type_t::setcdr:
                            va = check_type(ctx, EVAL_ARG(), obj_type_t::pair);
                            SET_CDR(va, EVAL_ARG());
                            res = ctx->nil;
                            break;

                        case prim_type_t::list:
                            res = eval_list(ctx, arg, env);
                            break;

                        case prim_type_t::not_:
                            va = EVAL_ARG();
                            res = make_bool(ctx, IS_FALSE(va));
                            break;

                        case prim_type_t::is:
                            va  = EVAL_ARG();
                            res = make_bool(ctx, equal(ctx, va, EVAL_ARG()));
                            break;

                        case prim_type_t::atom:
                            res = make_bool(ctx, TYPE(EVAL_ARG()) != obj_type_t::pair);
                            break;

                        case prim_type_t::print:
                            while (!IS_NIL(arg)) {
                                format::print("{}", printable_t{ctx, EVAL_ARG()});
                                if (!IS_NIL(arg))
                                    format::print(" ");
                            }
                            format::print("\n");
                            res = ctx->nil;
                            break;

                        case prim_type_t::gt:
                            va  = EVAL_ARG();
                            vb  = EVAL_ARG();
                            res = make_bool(ctx, to_flonum(va) > to_flonum(vb));
                            break;

                        case prim_type_t::gte:
                            va  = EVAL_ARG();
                            vb  = EVAL_ARG();
                            res = make_bool(ctx, to_flonum(va) >= to_flonum(vb));
                            break;

                        case prim_type_t::lt:
                            va  = EVAL_ARG();
                            vb  = EVAL_ARG();
                            res = make_bool(ctx, to_flonum(va) < to_flonum(vb));
                            break;

                        case prim_type_t::lte:
                            va  = EVAL_ARG();
                            vb  = EVAL_ARG();
                            res = make_bool(ctx, to_flonum(va) <= to_flonum(vb));
                            break;

                        case prim_type_t::add: {
                            flonum_t x = to_flonum(EVAL_ARG());
                            while (!IS_NIL(arg))
                                x += to_flonum(EVAL_ARG());
                            res = make_flonum(ctx, x);
                            break;
                        }

                        case prim_type_t::sub: {
                            flonum_t x = to_flonum(EVAL_ARG());
                            while (!IS_NIL(arg))
                                x -= to_flonum(EVAL_ARG());
                            res = make_flonum(ctx, x);
                            break;
                        }

                        case prim_type_t::mul: {
                            flonum_t x = to_flonum(EVAL_ARG());
                            while (!IS_NIL(arg))
                                x *= to_flonum(EVAL_ARG());
                            res = make_flonum(ctx, x);
                            break;
                        }

                        case prim_type_t::div: {
                            flonum_t x = to_flonum(EVAL_ARG());
                            while (!IS_NIL(arg))
                                x /= to_flonum(EVAL_ARG());
                            res = make_flonum(ctx, x);
                            break;
                        }

                        case prim_type_t::mod: {
                            fixnum_t x = to_fixnum(EVAL_ARG());
                            while (!IS_NIL(arg))
                                x %= to_fixnum(EVAL_ARG());
                            res = make_fixnum(ctx, x);
                            break;
                        }

                        default:
                            break;
                    }
                    break;

                case obj_type_t::ffi: {
                    auto proto = (proto_t*) NATIVE_PTR(fn);
                    auto ol = ffi::proto::match_signature(proto,
                                                          buf,
                                                          make_ffi_signature(ctx, arg, env, buf));
                    if (!ol)
                        error(ctx, "ffi: no matching overload for function: {}", proto->name);
                    n = 0;
                    ffi::reset(ctx->ffi);
                    while (!IS_NIL(arg)) {
                        if (n >= ol->params.size)
                            error(ctx, "ffi: too many arguments: {}@{}", ol->name, proto->name);
                        auto& param = ol->params[n];
                        va          = EVAL(CAR(arg));
                        switch (TYPE(va)) {
                            case obj_type_t::nil:
                                ffi::push(ctx->ffi, (u0*) nullptr);
                                break;
                            case obj_type_t::ptr:
                                switch (ffi_type_t(param->value.type.user)) {
                                    case ffi_type_t::object:
                                        ffi::push(ctx->ffi, va);
                                        break;
                                    case ffi_type_t::context:
                                        ffi::push(ctx->ffi, ctx);
                                        break;
                                    default:
                                        ffi::push(ctx->ffi, NATIVE_PTR(va));
                                        break;
                                }
                                break;
                            case obj_type_t::pair:
                                switch (ffi_type_t(param->value.type.user)) {
                                    case ffi_type_t::list:
                                        ffi::push(ctx->ffi, arg);
                                        break;
                                    default:
                                        error(ctx, "ffi: invalid pair argument");
                                        break;
                                }
                                break;
                            case obj_type_t::fixnum:
                                switch (param->value.type.cls) {
                                    case param_cls_t::int_:
                                        ffi::push(ctx->ffi, FIXNUM(va));
                                        break;
                                    case param_cls_t::float_:
                                        ffi::push(ctx->ffi, flonum_t(FIXNUM(va)));
                                        break;
                                    default:
                                        error(ctx, "ffi: invalid float conversion");
                                }
                                break;
                            case obj_type_t::flonum:
                                switch (param->value.type.cls) {
                                    case param_cls_t::int_:
                                        ffi::push(ctx->ffi, fixnum_t(FLONUM(va)));
                                        break;
                                    case param_cls_t::float_:
                                        ffi::push(ctx->ffi, FLONUM(va));
                                        break;
                                    default:
                                        error(ctx, "ffi: invalid float conversion");
                                }
                                break;
                            case obj_type_t::symbol:
                            case obj_type_t::string:
                            case obj_type_t::keyword: {
                                auto s = string::interned::get_slice(STRING_ID(va));
                                if (!s)
                                    error(ctx, "ffi: invalid interned string id: {}", STRING_ID(va));
                                ffi::push(ctx->ffi, s);
                                break;
                            }
                            case obj_type_t::boolean:
                                ffi::push(ctx->ffi, va == ctx->true_ ? true : false);
                                break;
                            default: {
                                switch (ffi_type_t(param->value.type.user)) {
                                    case ffi_type_t::context:
                                        ffi::push(ctx->ffi, ctx);
                                        break;
                                    default:
                                        break;
                                }
                                error(ctx, "ffi: unsupported scm object type");
                            }
                        }
                        arg = CDR(arg);
                        ++n;
                    }
                    param_alias_t ret{};
                    auto status = ffi::call(ctx->ffi, ol, ret);
                    if (!OK(status))
                        error(ctx, "ffi: call failed: {}", ol->name);
                    switch (ol->ret_type.cls) {
                        case param_cls_t::ptr:
                            res = !ret.p ? ctx->nil : (obj_t*) ret.p;
                            break;
                        case param_cls_t::int_:
                            if (ffi_type_t(ol->ret_type.user) == ffi_type_t::boolean) {
                                res = make_bool(ctx, ret.b);
                            } else {
                                res = make_fixnum(ctx, ret.dw);
                            }
                            break;
                        case param_cls_t::void_:
                            res = ctx->nil;
                            break;
                        case param_cls_t::float_:
                            res = make_flonum(ctx, ret.fdw);
                            break;
                        default:
                            error(ctx, "ffi: unsupported return type");
                    }
                    break;
                }

                case obj_type_t::cfunc:
                    res = ((native_func_t) NATIVE_PTR(fn))(ctx, eval_list(ctx, arg, env));
                    break;

                case obj_type_t::func: {
                    auto proc = (proc_t*) NATIVE_PTR(CDR(fn));
                    arg = eval_list(ctx, arg, env);
                    args_to_env(ctx, proc->params, arg, proc->env_obj);
                    arg = proc->params;
                    u32 save = save_gc(ctx);
                    for (; !IS_NIL(CDR(arg)); arg = CDR(arg)) {
                        restore_gc(ctx, save);
                        push_gc(ctx, arg);
                        eval(ctx, CAR(arg), proc->env_obj);
                    }
                    obj = CAR(arg);
                    res = eval(ctx, CAR(arg), proc->env_obj);
                    break;
                }

                case obj_type_t::macro: {
                    auto proc = (proc_t*) NATIVE_PTR(CDR(fn));
                    obj_t* expr{};
                    args_to_env(ctx, proc->params, arg, proc->env_obj);
                    arg = proc->params;
                    u32 save = save_gc(ctx);
                    while (!IS_NIL(arg)) {
                        restore_gc(ctx, save);
                        push_gc(ctx, arg);
                        expr = eval(ctx, next_arg(ctx, &arg), proc->env_obj);
                    }
                    *obj = *expr;
                    ctx->call_list = CDR(cl);
                    res = eval(ctx, obj, env);
                    break;
                }

                default:
                    error(ctx, "tried to call non-callable value");
            }

            restore_gc(ctx, gc);
        }

        restore_gc(ctx, gc);
        push_gc(ctx, res);
        if (cl)
            ctx->call_list = CDR(cl);

        return res;
    }

    u0 print(fmt_buf_t& buf, ctx_t* ctx, obj_t* obj) {
        format::format_to(buf, "{}\n", printable_t{ctx, obj});
    }

    u0 write(fmt_buf_t& buf, ctx_t* ctx, obj_t* obj) {
        format::format_to(buf, "{}", printable_t{ctx, obj});
    }

    obj_t* make_environment(ctx_t* ctx, obj_t* parent) {
        if (parent && !IS_NIL(parent))
            check_type(ctx, parent, obj_type_t::environment);
        obj_t* obj = make_object(ctx);
        auto env = &array::append(ctx->environments);
        env->parent     = parent && !IS_NIL(parent) ? parent : ctx->nil;
        env->gc_protect = true;
        hashtab::init(env->bindings, ctx->alloc);
        SET_TYPE(obj, obj_type_t::environment);
        SET_CDR(obj, make_user_ptr(ctx, env));
        return obj;
    }

    obj_t* make_list(ctx_t* ctx, obj_t** objs, u32 size) {
        obj_t* res = ctx->nil;
        while (size--)
            res = cons(ctx, objs[size], res);
        return res;
    }

    static numtype_t get_numtype(const s8* buf, s32 len) {
        const s8* p = buf;
        s8 ch = *p;
        numtype_t type = ((ch == '+' && len > 1)
                          || (ch == '-' && len > 1)
                          || isdigit(ch)) ? numtype_t::fixnum : numtype_t::none;
        if (type == numtype_t::none)
            return type;
        while (--len > 0) {
            ch = *(++p);
            if (ch == '.' || ch == 'e' || ch == 'E' || ch == '-') {
                type = numtype_t::flonum;
            } else if (ch < 48 || ch > 57) {
                type = numtype_t::none;
                break;
            }
        }
        return type;
    }

    u0 set(ctx_t* ctx, obj_t* sym, obj_t* v, obj_t* env) {
        check_type(ctx, env, obj_type_t::environment);
        auto e = (env_t*) NATIVE_PTR(CDR(env));
        hashtab::set(e->bindings, FIXNUM(sym), v);
    }

    obj_t* eval_list(ctx_t* ctx, obj_t* lst, obj_t* env) {
        obj_t* head = ctx->nil;
        obj_t* tail = head;
        while (!IS_NIL(lst)) {
            auto r = cons(ctx,
                          EVAL(next_arg(ctx, &lst)),
                          ctx->nil);
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

    obj_t* make_native_func(ctx_t* ctx, native_func_t fn) {
        obj_t* obj = make_object(ctx);
        array::append(ctx->native_ptrs, (u0*) fn);
        SET_TYPE(obj, obj_type_t::cfunc);
        SET_FIXNUM(obj, ctx->native_ptrs.size - 1);
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
                if (tos == ctx->rparen)
                    error(ctx, "expected closing paren but found closing bracket");
                return ctx->rbrac;
            }

            case ')': {
                CRSR_NEXT(crsr);
                auto tos = stack::pop(ctx->cl_stack);
                if (tos == ctx->rbrac)
                    error(ctx, "expected closing bracket but found closing paren");
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
                        error(ctx, "unclosed list");
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
                    error(ctx, "stray '`'");
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
                        error(ctx, "stray ',@'");
                    sym = make_symbol(ctx, "unquote-splicing", 16);
                } else {
                    v = read(ctx, crsr);
                    if (!v)
                        error(ctx, "stray ','");
                    sym = make_symbol(ctx, "unquote", 7);
                }
                return cons(ctx, sym, cons(ctx, v, ctx->nil));
            }

            case '\'': {
                CRSR_NEXT(crsr);
                v = read(ctx, crsr);
                if (!v)
                    error(ctx, "stray '''");
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
                        error(ctx, "string too long");
                    if (chr == '\0')
                        error(ctx, "unclosed string");
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
                        error(ctx, "symbol too long");
                    CRSR_NEXT(crsr); chr = CRSR_READ(crsr);
                    p = CRSR_POS(crsr);
                } while (chr && !strchr(s_delims, chr));
                const s8*  start = (const s8*) (crsr.buf->data + s);
                const auto len = p - s;
                s8* end = (s8*) (start + len);
                if (p != s) {
                    switch (get_numtype(start, len)) {
                        case numtype_t::none:
                            if (strncmp(start, "#:", 2) == 0) {
                                return make_keyword(ctx, start + 2, len - 2);
                            } else {
                                return make_symbol(ctx, start, len);
                            }
                        case numtype_t::fixnum:
                            return make_fixnum(ctx, strtol(start, &end, 10));
                        case numtype_t::flonum:
                            return make_flonum(ctx, strtod(start, &end));
                    }
                } else {
                    error(ctx, "expected flonum, fixnum, keyword, or symbol");
                }
            }
        }

        return ctx->nil;
    }

    obj_t* quasiquote(ctx_t* ctx, obj_t* obj, obj_t* env) {
        if (TYPE(obj) != obj_type_t::pair)
            return CONS(SYM("quote"), CONS1(obj));
        auto a = CAR(obj);
        if (equal(ctx, a, SYM("unquote-splicing")))
            error(ctx, "unquote-splicing not valid in this context");
        if (equal(ctx, a, SYM("unquote")))
            return CADR(obj);
        if (TYPE(a) == obj_type_t::pair) {
            auto aa = CAAR(obj);
            if (equal(ctx, aa, SYM("unquote-splicing"))) {
                obj_t* t1[] = {SYM("append"), CADR(a), quasiquote(ctx, CDR(obj), env)};
                return make_list(ctx, t1, 3);
            }
        }
        obj_t* t1[] = {SYM("cons"), quasiquote(ctx, a, env), quasiquote(ctx, CDR(obj), env)};
        return make_list(ctx, t1, 3);
    }

    static u0 finalize_environment(ctx_t* ctx, obj_t* env) {
        check_type(ctx, env, obj_type_t::environment);
        auto e = (env_t*) NATIVE_PTR(CDR(env));
        e->gc_protect = false;
    }

    obj_t* make_symbol(ctx_t* ctx, const s8* name, s32 len) {
        const auto rc = string::interned::fold_for_result(name, len);
        if (!OK(rc.status))
            error(ctx, "make_symbol: unable to intern string: {}", name);
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
        if (!OK(rc.status))
            error(ctx, "make_keyword: unable to intern string: {}", name);
        auto obj = hashtab::find(ctx->symtab, rc.id);
        if (obj)
            return obj;
        obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::keyword);
        SET_FIXNUM(obj, rc.id);
        hashtab::insert(ctx->symtab, rc.id, obj);
        return obj;
    }

    u0 format_error(ctx_t* ctx, fmt_str_t fmt_msg, fmt_args_t args) {
        obj_t* cl = ctx->call_list;
        if (cl && ctx->handlers.error)
            ctx->handlers.error(ctx, fmt_msg.data(), cl);
        format::print(stderr, "error: ");
        format::vprint(ctx->alloc, stderr, fmt_msg, args);
        if (cl) {
            format::print(stderr, "\n");
            for (; !IS_NIL(cl); cl = CDR(cl))
                format::print(stderr, "=> {}\n", printable_t{ctx, CAR(cl)});
        }
        ctx->call_list = ctx->nil;
        exit(EXIT_FAILURE);
    }

    obj_t* make_error(ctx_t* ctx, obj_t* args, obj_t* call_stack) {
        obj_t* obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::error);
        SET_CDR(obj, cons(ctx, args, cons(ctx, call_stack, ctx->nil)));
        return obj;
    }

    obj_t* make_string(ctx_t* ctx, const s8* str, s32 len, u32 id) {
        const auto rc = string::interned::fold_for_result(str, len);
        if (!OK(rc.status))
            error(ctx, "make_string: unable to intern string: {}", str);
        auto obj = hashtab::find(ctx->strtab, rc.id);
        if (obj)
            return obj;
        obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::string);
        SET_FIXNUM(obj, rc.id);
        hashtab::insert(ctx->strtab, rc.id, obj);
        return obj;
    }

    static obj_t* check_type(ctx_t* ctx, obj_t* obj, obj_type_t type) {
        if (TYPE(obj) != type) {
            error(ctx,
                  "expected {}, got {}",
                  s_type_names[u32(type)],
                  s_type_names[u32(TYPE(obj))]);
        }
        return obj;
    }

    u0 format_object(const printable_t& printable, fmt_ctx_t& fmt_ctx) {
        auto o = fmt_ctx.out();
        auto ctx = printable.ctx;
        auto obj = printable.obj;

        switch (TYPE(obj)) {
            case obj_type_t::nil:
                fmt::format_to(o, "nil");
                break;

            case obj_type_t::pair:
            case obj_type_t::func:
            case obj_type_t::macro: {
                obj_t* p = obj;
                fmt::format_to(o, "(");
                for (;;) {
                    fmt::format_to(o, "{}", printable_t{ctx, CAR(p), true});
                    p = CDR(p);
                    if (TYPE(p) != obj_type_t::pair)
                        break;
                    fmt::format_to(o, " ");
                }
                if (!IS_NIL(p))
                    fmt::format_to(o, " . {}", printable_t{ctx, p, true});
                fmt::format_to(o, ")");
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
                auto args       = car(ctx, err_args);
                auto call_stack = car(ctx, CDR(err_args));
                fmt::format_to(o, "error: ");
                for (; !IS_NIL(args); args = CDR(args)) {
                    auto expr = eval(ctx, CAR(args));
                    fmt::format_to(o, "{} ", printable_t{ctx, expr});
                }
                fmt::format_to(o, "\n");
                for (; !IS_NIL(call_stack); call_stack = CDR(call_stack))
                    fmt::format_to(o, "=> {}\n", printable_t{ctx, CAR(call_stack)});
                break;
            }

            case obj_type_t::keyword:
                fmt::format_to(o, "#:");
            case obj_type_t::symbol:
                fmt::format_to(o, "{}", to_string(ctx, obj));
                break;

            case obj_type_t::string:
                if (printable.quote)
                    fmt::format_to(o, "\"{}\"", to_string(ctx, obj));
                else
                    fmt::format_to(o, "{}", to_string(ctx, obj));
                break;

            case obj_type_t::boolean:
                fmt::format_to(o, "{}", to_string(ctx, obj));
                break;

            case obj_type_t::environment:
                format_environment(ctx, obj, fmt_ctx, printable.indent);
                break;

            default:
                fmt::format_to(o, "[{} {}]", type_name(obj), (u0*) obj);
                break;
        }
    }

    static u0 args_to_env(ctx_t* ctx, obj_t* prm, obj_t* arg, obj_t* env) {
        while (!IS_NIL(prm)) {
            if (TYPE(prm) != obj_type_t::pair) {
                set(ctx, prm, arg, env);
                break;
            }
            auto k = CAR(prm);
            auto v = CAR(arg);
            set(ctx, k, v, env);
            prm = CDR(prm);
            arg = CDR(arg);
        }
    }

    static u32 make_ffi_signature(ctx_t* ctx, obj_t* args, obj_t* env, u8* buf) {
        u32 len{};
        if (IS_NIL(args)) {
            buf[len++] = u8(param_cls_t::void_);
            return len;
        }
        while (!IS_NIL(args)) {
            const auto type = TYPE(EVAL(CAR(args)));
            const auto& mapping = s_types[u32(type)];
            buf[len++] = mapping.type;
            buf[len++] = mapping.size;
            args = CDR(args);
        }
        return len;
    }

    obj_t* make_proc(ctx_t* ctx, obj_t* params, obj_t* body, obj_t* env, b8 macro) {
        auto new_env = make_environment(ctx, env);
        auto lst     = params;
        while (!IS_NIL(lst)) {
            if (TYPE(lst) != obj_type_t::pair) {
                set(ctx, lst, ctx->false_, new_env);
                break;
            }
            set(ctx, CAR(lst), ctx->false_, new_env);
            lst = CDR(lst);
        }
        auto e = (env_t*) NATIVE_PTR(CDR(new_env));
        auto proc = &array::append(ctx->procedures);
        proc->env          = e;
        proc->addr         = {};
        proc->body         = body;
        proc->params       = params;
        proc->is_tco       = false;
        proc->env_obj      = new_env;
        proc->is_macro     = macro;
        proc->is_assembled = false;
        auto obj = make_object(ctx);
        SET_TYPE(obj, macro ? obj_type_t::macro : obj_type_t::func);
        SET_CDR(obj, make_user_ptr(ctx, proc));
        return obj;
    }

    static u0 format_environment(ctx_t* ctx, obj_t* env, fmt_ctx_t& fmt_ctx, u32 indent) {
        auto e = (env_t*) NATIVE_PTR(CDR(env));
        fmt::format_to(fmt_ctx.out(), "{:<{}}(", " ", indent);
        b8 first = true;
        for (const auto& pair : e->bindings) {
            auto s = string::interned::get_slice(pair.key);
            if (!s)
                continue;
            if (first) {
                fmt::format_to(fmt_ctx.out(),
                               "({:<16} . {})\n",
                               *s,
                               printable_t{ctx, pair.value, true});
                first = false;
            } else {
                fmt::format_to(fmt_ctx.out(),
                               "{:<{}}({:<16} . {})\n",
                               " ",
                               indent,
                               *s,
                               printable_t{ctx, pair.value, true});
            }
        }
        if (IS_NIL(e->parent)) {
            fmt::format_to(fmt_ctx.out(), "{:<{}}(parent . nil)", " ", indent);
        } else {
            fmt::format_to(fmt_ctx.out(),
                           "{:<{}}(parent . [environment {}])",
                           " ",
                           indent,
                           (u0*) e->parent);
//                           printable_t{ctx, e->parent, false, indent + 5});
        }
        fmt::format_to(fmt_ctx.out(), ")\n");
    }
}