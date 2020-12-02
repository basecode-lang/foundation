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

#include <basecode/core/scm.h>
#include <basecode/core/error.h>
#include <basecode/core/string.h>

#define OBJ_AT(idx)             (&ctx->objects[(idx)])
#define OBJ_IDX(x)              ((x) - ctx->objects)
#define CAR(x)                  (&ctx->objects[(x)->pair.car_idx])
#define SET_CAR(x, o)           ((x)->pair.car_idx = OBJ_IDX(o))
#define CDR(x)                  (&ctx->objects[(x)->pair.cdr_idx])
#define SET_CDR(x, o)           ((x)->pair.cdr_idx = OBJ_IDX(o))
#define IS_NIL(x)               ((x) == ctx->nil)
#define TYPE(x)                 (obj_type_t((x)->hdr.type))
#define IS_GC_MARKED(x)         ((x)->hdr.gc_mark)
#define SET_GC_MARK(x, v)       ((x)->hdr.gc_mark = (v))
#define SET_TYPE(x, t)          ((x)->hdr.type = u8((t)))
#define INTEGER(x)              (u32((x)->number.value))
#define NUMBER(x)               (numeric_alias_t{.dw = ((x)->number.value)}.fdw)
#define SET_NUMBER(x, v)        ((x)->number.value = numeric_alias_t{.fdw = (v)}.dw)
#define SET_INTEGER(x, v)       ((x)->number.value = u32(v))
#define PRIM(x)                 (prim_type_t((x)->prim.code))
#define SET_PRIM(x, p)          ((x)->prim.code = u32(p))
#define NATIVE_PTR(x)           (ctx->native_ptrs[(x)->number.value])
#define CLOSERS_SIZE            (256U)
#define SCRATCH_SIZE            (1024U)
#define GC_STACK_SIZE           (1024U)
#define NATIVE_PTR_SIZE         (256U)
#define EVAL_ARG()              eval(ctx, next_arg(ctx, &arg), env, nullptr)
#define ARITH_INT_OP(op)        SAFE_SCOPE(                                                     \
                                    u32 x = NUMBER(EVAL_ARG());                                 \
                                    while (!IS_NIL(arg))                                        \
                                        x = x op u32(NUMBER(EVAL_ARG()));                       \
                                    res = make_number(ctx, x);                                  \
                                )
#define ARITH_NUM_OP(op)        SAFE_SCOPE(                                                     \
                                    number_t x = NUMBER(EVAL_ARG());                            \
                                    while (!IS_NIL(arg))                                        \
                                        x = x op NUMBER(EVAL_ARG());                            \
                                    res = make_number(ctx, x);                                  \
                                )
#define NUM_CMP_OP(op)          SAFE_SCOPE(                                                     \
                                    va = check_type(ctx, EVAL_ARG(), obj_type_t::number);       \
                                    vb = check_type(ctx, EVAL_ARG(), obj_type_t::number);       \
                                    res = make_bool(ctx, NUMBER(va) op NUMBER(vb));             \
                                )

namespace basecode::scm {
    enum class prim_type_t : u8 {
        let,
        set,
        if_,
        fn,
        mac,
        while_,
        quote,
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
        max
    };

    static const s8* s_prim_names[] = {
        "let",
        "=",
        "if",
        "fn",
        "mac",
        "while",
        "quote",
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
        "number",
        "symbol",
        "string",
        "func",
        "macro",
        "prim",
        "cfunc",
        "ptr"
    };

    static const s8* s_delims   = " \n\t\r()[];";

    struct obj_t {
        union {
            struct {
                u64             type:       5;
                u64             gc_mark:    1;
                u64             pad:        58;
            }                   hdr;
            struct {
                u64             type:       5;
                u64             gc_mark:    1;
                u64             code:       32;
                u64             pad:        26;
            }                   prim;
            struct {
                u64             type:       5;
                u64             gc_mark:    1;
                u64             car_idx:    29;
                u64             cdr_idx:    29;
            }                   pair;
            struct {
                u64             type:       5;
                u64             gc_mark:    1;
                u64             value:      32;
                u64             pad:        26;
            }                   number;
            u64                 full;
        };
    };
    static_assert(sizeof(obj_t) == 8, "obj_t is no longer 8 bytes!");

    struct ctx_t {
        handlers_t              handlers;
        u0*                     native_ptrs[NATIVE_PTR_SIZE];
        obj_t*                  gc_stack[GC_STACK_SIZE];
        obj_t*                  closers[CLOSERS_SIZE];
        obj_t*                  objects;
        obj_t*                  sym_list;
        obj_t*                  call_list;
        obj_t*                  free_list;
        obj_t*                  nil;
        obj_t*                  dot;
        obj_t*                  true_;
        obj_t*                  rbrac;
        obj_t*                  rparen;
        s8                      scratch[SCRATCH_SIZE];
        u32                     closer_idx;
        u32                     object_used;
        u32                     gc_stack_idx;
        u32                     object_count;
        u32                     native_ptr_idx;
        s8                      next_chr;
    };

    struct Char_Ptr_Int {
        s8*                     p;
        u32                     n;
    };

    static obj_t* eval(ctx_t* ctx,
                       obj_t* obj,
                       obj_t* env,
                       obj_t** new_env);

    static obj_t* args_to_env(ctx_t* ctx, obj_t* prm, obj_t* arg, obj_t* env);

    static b8 equal(obj_t* a, obj_t* b) {
        if (a == b) {
            return true;
        }
        if (TYPE(a) != TYPE(b)) {
            return false;
        }
        if (TYPE(a) == obj_type_t::number) {
            const auto e = std::numeric_limits<number_t>::epsilon();
            auto d = NUMBER(b) - NUMBER(a);
            return d <= e;
        } else if (TYPE(a) == obj_type_t::string) {
            return INTEGER(a) == INTEGER(b);
        }
        return false;
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

    static b8 str_eq(obj_t* obj, u32 str_id) {
        auto obj_rc = string::interned::get(INTEGER(obj));
        if (!OK(obj_rc.status))
            return false;
        return obj_rc.id == str_id;
    }

    static s8 read_fp(ctx_t* ctx, u0* udata) {
        s32 chr;
        UNUSED(ctx);
        return (chr = fgetc((FILE*) udata)) == EOF ? '\0' : s8(chr);
    }

    static u0 write_fp(ctx_t* ctx, u0* udata, s8 chr) {
        UNUSED(ctx);
        fputc(chr, (FILE*) udata);
    }

    static u0 write_buf(ctx_t* ctx, u0* udata, s8 chr) {
        auto x = (Char_Ptr_Int*) udata;
        UNUSED(ctx);
        if (x->n) {
            *x->p++ = chr;
            x->n--;
        }
    }

    static obj_t* do_list(ctx_t* ctx, obj_t* lst, obj_t* env) {
        obj_t* res = ctx->nil;
        u32 save = save_gc(ctx);
        while (!IS_NIL(lst)) {
            restore_gc(ctx, save);
            push_gc(ctx, lst);
            push_gc(ctx, env);
            res = eval(ctx, next_arg(ctx, &lst), env, &env);
        }
        return res;
    }

    static obj_t* read_(ctx_t* ctx, read_func_t fn, u0* udata) {
        obj_t* v;

        s8 chr = ctx->next_chr ? ctx->next_chr : fn(ctx, udata);
        ctx->next_chr = '\0';

        while (chr && strchr(" \n\t\r", chr)) {
            chr = fn(ctx, udata);
        }

        switch (chr) {
            case '\0':
                return nullptr;

            case ';':
                while (chr && chr != '\n') {
                    chr = fn(ctx, udata);
                }
                return read_(ctx, fn, udata);

            case ']':
                if (ctx->closers[ctx->closer_idx++] == ctx->rparen)
                    error(ctx, "expected closing paren but found closing bracket");
                return ctx->rbrac;

            case ')':
                if (ctx->closers[ctx->closer_idx++] == ctx->rbrac)
                    error(ctx, "expected closing bracket but found closing paren");
                return ctx->rparen;

            case '[':
                ctx->closers[--ctx->closer_idx] = ctx->rbrac;
                goto _skip;

            case '(':
                ctx->closers[--ctx->closer_idx] = ctx->rparen;
            _skip: {
                obj_t* head = ctx->nil;
                obj_t* tail = head;
                auto gc = save_gc(ctx);
                /* to cause error on too-deep nesting */
                push_gc(ctx, head);
                while (true) {
                    v = read_(ctx, fn, udata);
                    if (v == ctx->rparen || v == ctx->rbrac)
                        break;
                    if (v == nullptr) {
                        error(ctx, "unclosed list");
                    }
                    if (v == ctx->dot) {
                        /* dotted pair */
                        SET_CDR(tail, read(ctx, fn, udata));
                    } else {
                        /* proper pair */
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

            case '\'': {
                v = read(ctx, fn, udata);
                if (!v) {
                    error(ctx, "stray '''");
                }
                return cons(ctx,
                            make_symbol(ctx, "quote", 5),
                            cons(ctx, v, ctx->nil));
            }

            case '"': {
                s8* p = ctx->scratch;
                const s8* e = p + (SCRATCH_SIZE - 1);
                chr = fn(ctx, udata);
                while (chr != '"') {
                    if (p == e) {
                        error(ctx, "string too long");
                    }
                    if (chr == '\0') {
                        error(ctx, "unclosed string");
                    }
                    if (chr == '\\') {
                        chr = fn(ctx, udata);
                        if (strchr("nrt", chr)) {
                            chr = strchr("n\nr\rt\t", chr)[1];
                        }
                    }
                    *p++ = chr;
                    chr = fn(ctx, udata);
                }
                return make_string(ctx, ctx->scratch, p - ctx->scratch);
            }

            default: {
                s8* p = ctx->scratch;
                const s8* e = p + (SCRATCH_SIZE - 1);
                do {
                    if (p == e)
                        error(ctx, "symbol too long");
                    *p++ = chr;
                    chr = fn(ctx, udata);
                } while (chr && !strchr(s_delims, chr));
                ctx->next_chr = chr;
                *p = '\0';
                const auto len = p - ctx->scratch;
                auto n = strtod(ctx->scratch, &p);
                if (p != ctx->scratch && strchr(s_delims, *p)) {
                    return make_number(ctx, n);
                } else if (strncmp(ctx->scratch, "#:", 2) == 0) {
                    return make_keyword(ctx, ctx->scratch + 2, len - 2);
                } else {
                    return make_symbol(ctx, ctx->scratch, len);
                }
            }
        }
    }

    static obj_t* eval_list(ctx_t* ctx, obj_t* lst, obj_t* env) {
        obj_t* head = ctx->nil;
        obj_t* tail = head;
        while (!IS_NIL(lst)) {
            auto r = cons(ctx,
                         eval(ctx, next_arg(ctx, &lst), env, nullptr),
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

    static obj_t* check_type(ctx_t* ctx, obj_t* obj, obj_type_t type) {
        s8 buf[64];
        if (TYPE(obj) != type) {
            sprintf(buf,
                    "expected %s, got %s",
                    s_type_names[u32(type)],
                    s_type_names[u32(TYPE(obj))]);
            error(ctx, buf);
        }
        return obj;
    }

    static obj_t* eval(ctx_t* ctx, obj_t* obj, obj_t* env, obj_t** new_env) {
        obj_t* fn;
        obj_t* arg;
        obj_t* res;
        obj_t* va;
        obj_t* vb;
        u32   n;
        u32   gc;

        if (TYPE(obj) == obj_type_t::symbol)
            return CDR(get(ctx, obj, env));

        if (TYPE(obj) != obj_type_t::pair)
            return obj;

        auto cl = cons(ctx, obj, ctx->call_list);
        ctx->call_list = cl;

        gc  = save_gc(ctx);
        fn  = eval(ctx, CAR(obj), env, nullptr);
        arg = CDR(obj);
        res = ctx->nil;

        switch (TYPE(fn)) {
            case obj_type_t::prim:
                switch (PRIM(fn)) {
                    case prim_type_t::let:
                        va = check_type(ctx, next_arg(ctx, &arg), obj_type_t::symbol);
                        if (new_env) {
                            *new_env = cons(ctx, cons(ctx, va, EVAL_ARG()), env);
                        }
                        break;

                    case prim_type_t::set:
                        va = check_type(ctx, next_arg(ctx, &arg), obj_type_t::symbol);
                        SET_CDR(get(ctx, va, env), EVAL_ARG());
                        break;

                    case prim_type_t::if_:
                        while (!IS_NIL(arg)) {
                            va = EVAL_ARG();
                            if (!IS_NIL(va)) {
                                res = IS_NIL(arg) ? va : EVAL_ARG();
                                break;
                            }
                            if (IS_NIL(arg)) { break; }
                            arg = CDR(arg);
                        }
                        break;

                    case prim_type_t::fn:
                    case prim_type_t::mac:
                        va = cons(ctx, env, arg);
                        next_arg(ctx, &arg);
                        res = make_object(ctx);
                        SET_TYPE(res, PRIM(fn) == prim_type_t::fn ? obj_type_t::func : obj_type_t::macro);
                        SET_CDR(res, va);
                        break;

                    case prim_type_t::while_:
                        va = next_arg(ctx, &arg);
                        n  = save_gc(ctx);
                        while (!IS_NIL(eval(ctx, va, env, nullptr))) {
                            do_list(ctx, arg, env);
                            restore_gc(ctx, n);
                        }
                        break;

                    case prim_type_t::quote:
                        res = next_arg(ctx, &arg);
                        break;

                    case prim_type_t::and_:
                        while (!IS_NIL(arg) && !IS_NIL(res = EVAL_ARG()));
                        break;

                    case prim_type_t::or_:
                        while (!IS_NIL(arg) && IS_NIL(res = EVAL_ARG()));
                        break;

                    case prim_type_t::do_:
                        res = do_list(ctx, arg, env);
                        break;

                    case prim_type_t::cons:
                        va  = EVAL_ARG();
                        res = cons(ctx, va, EVAL_ARG());
                        break;

                    case prim_type_t::car:
                        res = car(ctx, EVAL_ARG());
                        break;

                    case prim_type_t::cdr:
                        res = cdr(ctx, EVAL_ARG());
                        break;

                    case prim_type_t::setcar:
                        va = check_type(ctx, EVAL_ARG(), obj_type_t::pair);
                        SET_CAR(va, EVAL_ARG());
                        break;

                    case prim_type_t::setcdr:
                        va = check_type(ctx, EVAL_ARG(), obj_type_t::pair);
                        SET_CDR(va, EVAL_ARG());
                        break;

                    case prim_type_t::list:
                        res = eval_list(ctx, arg, env);
                        break;

                    case prim_type_t::not_:
                        res = make_bool(ctx, IS_NIL(EVAL_ARG()));
                        break;

                    case prim_type_t::is:
                        va  = EVAL_ARG();
                        res = make_bool(ctx, equal(va, EVAL_ARG()));
                        break;

                    case prim_type_t::atom:
                        res = make_bool(ctx, type(ctx, EVAL_ARG()) != obj_type_t::pair);
                        break;

                    case prim_type_t::print:
                        while (!IS_NIL(arg)) {
                            write_fp(ctx, EVAL_ARG(), stdout);
                            if (!IS_NIL(arg)) {
                                printf(" ");
                            }
                        }
                        printf("\n");
                        break;

                    case prim_type_t::gt:
                        NUM_CMP_OP(>);
                        break;

                    case prim_type_t::gte:
                        NUM_CMP_OP(>=);
                        break;

                    case prim_type_t::lt:
                        NUM_CMP_OP(<);
                        break;

                    case prim_type_t::lte:
                        NUM_CMP_OP(<=);
                        break;

                    case prim_type_t::add:
                        ARITH_NUM_OP(+);
                        break;

                    case prim_type_t::sub:
                        ARITH_NUM_OP(-);
                        break;

                    case prim_type_t::mul:
                        ARITH_NUM_OP(*);
                        break;

                    case prim_type_t::div:
                        ARITH_NUM_OP(/);
                        break;

                    case prim_type_t::mod:
                        ARITH_INT_OP(%);
                        break;

                    default:
                        break;
                }
                break;

            case obj_type_t::cfunc: {
                auto func = (native_func_t) NATIVE_PTR(fn);
                res = func(ctx, eval_list(ctx, arg, env));
                break;
            }

            case obj_type_t::func:
                arg  = eval_list(ctx, arg, env);
                va   = CDR(fn); /* (env params ...) */
                vb   = CDR(va); /* (params ...) */
                res  = do_list(ctx, CDR(vb), args_to_env(ctx, CAR(vb), arg, CAR(va)));
                break;

            case obj_type_t::macro:
                va = CDR(fn); /* (env params ...) */
                vb = CDR(va); /* (params ...) */
                /* replace caller object with code generated by macro and re-eval */
                *obj = *do_list(ctx, CDR(vb), args_to_env(ctx, CAR(vb), arg, CAR(va)));
                restore_gc(ctx, gc);
                ctx->call_list = CDR(cl);
                return eval(ctx, obj, env, nullptr);

            default:
                error(ctx, "tried to call non-callable value");
        }

        restore_gc(ctx, gc);
        push_gc(ctx, res);
        ctx->call_list = CDR(cl);
        return res;
    }

    static u32 write_str(ctx_t* ctx, write_func_t fn, u0* udata, const s8* s) {
        u32 len{};
        while (*s) {
            fn(ctx, udata, *s++);
            ++len;
        }
        return len;
    }

    static obj_t* args_to_env(ctx_t* ctx, obj_t* prm, obj_t* arg, obj_t* env) {
        while (!IS_NIL(prm)) {
            if (TYPE(prm) != obj_type_t::pair) {
                env = cons(ctx, cons(ctx, prm, arg), env);
                break;
            }
            env = cons(ctx, cons(ctx, CAR(prm), car(ctx, arg)), env);
            prm = CDR(prm);
            arg = cdr(ctx, arg);
        }
        return env;
    }

    u0 free(ctx_t* ctx) {
        ctx->gc_stack_idx = 0;
        ctx->sym_list     = ctx->nil;
        collect_garbage(ctx);
    }

    obj_t* nil(ctx_t* ctx) {
        return ctx->nil;
    }

    u32 save_gc(ctx_t* ctx) {
        return ctx->gc_stack_idx;
    }

    obj_t* pop_gc(ctx_t* ctx) {
        return ctx->gc_stack_idx > 0 ? ctx->gc_stack[ctx->gc_stack_idx--] : nullptr;
    }

    u0 collect_garbage(ctx_t* ctx) {
        for (u32 i = 0; i < ctx->gc_stack_idx; i++)
            mark(ctx, ctx->gc_stack[i]);
        mark(ctx, ctx->sym_list);
        for (u32 i = 0; i < ctx->object_count; i++) {
            obj_t* obj = &ctx->objects[i];
            if (TYPE(obj) == obj_type_t::free)
                continue;
            if (!IS_GC_MARKED(obj)) {
                if (TYPE(obj) == obj_type_t::ptr
                &&  ctx->handlers.gc) {
                    ctx->handlers.gc(ctx, obj);
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

    ctx_t* init(u0* ptr, u32 size) {
        // init context struct
        auto ctx = (ctx_t*) ptr;
        std::memset(ctx, 0, sizeof(ctx_t));
        ptr = (u8*) ptr + sizeof(ctx_t);
        size -= sizeof(ctx_t);

        // init objects memory region
        ctx->objects      = (obj_t*) ptr;
        ctx->object_used  = 0;
        ctx->object_count = size / sizeof(obj_t);

        // the nil object is a special case that
        // we manually allocate from the heap
        ctx->nil       = &ctx->objects[ctx->object_used++];
        ctx->nil->full = 0;
        SET_TYPE(ctx->nil, obj_type_t::nil);

        // init lists
        ctx->sym_list  = ctx->nil;
        ctx->call_list = ctx->nil;
        ctx->free_list = ctx->nil;

        // populate freelist
        for (u32 i = ctx->object_used; i < ctx->object_count; i++) {
            obj_t* obj = &ctx->objects[i];
            SET_TYPE(obj, obj_type_t::free);
            SET_CDR(obj, ctx->free_list);
            ctx->free_list = obj;
        }

        // init objects
        auto save = save_gc(ctx);
        ctx->rbrac  = make_object(ctx);
        ctx->rparen = make_object(ctx);
        ctx->dot    = make_symbol(ctx, ".", 1);
        ctx->true_  = make_symbol(ctx, "#t", 2);
        set(ctx, ctx->dot, ctx->dot);
        set(ctx, ctx->true_, ctx->true_);
        set(ctx, make_symbol(ctx, "#f", 2), ctx->nil);
        set(ctx, make_symbol(ctx, "nil", 3), ctx->nil);
        restore_gc(ctx, save);

        // register built in primitives
        for (u32 i = 0; i < u32(prim_type_t::max); i++) {
            obj_t* v = make_object(ctx);
            SET_TYPE(v, obj_type_t::prim);
            SET_PRIM(v, i);
            set(ctx, make_symbol(ctx, s_prim_names[i]), v);
            restore_gc(ctx, save);
        }

        ctx->native_ptr_idx = 0;
        ctx->closer_idx     = CLOSERS_SIZE - 1;

        return ctx;
    }

    u0 mark(ctx_t* ctx, obj_t* obj) {
        obj_t* car;
    begin:
        if (IS_GC_MARKED(obj))
            return;

        car = CAR(obj);
        SET_GC_MARK(obj, true);

        switch (TYPE(obj)) {
            case obj_type_t::string:
                break;

            case obj_type_t::pair:
                mark(ctx, car);
                /* fall through */
            case obj_type_t::func:
            case obj_type_t::macro:
            case obj_type_t::symbol:
            case obj_type_t::keyword:
                obj = CDR(obj);
                goto begin;

            case obj_type_t::ptr:
                if (ctx->handlers.mark) {
                    ctx->handlers.mark(ctx, obj);
                }
                break;

            default:
                break;
        }
    }

    handlers_t* handlers(ctx_t* ctx) {
        return &ctx->handlers;
    }

    b8 is_nil(ctx_t* ctx, obj_t* obj) {
        UNUSED(ctx);
        return IS_NIL(obj);
    }

    u0 restore_gc(ctx_t* ctx, u32 idx) {
        ctx->gc_stack_idx = idx;
    }

    b8 is_true(ctx_t* ctx, obj_t* obj) {
        UNUSED(ctx);
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
            case obj_type_t::string: {
                auto str_rc = string::interned::get(INTEGER(obj));
                if (!OK(str_rc.status))
                    error(ctx, "unable to find interned string");
                return str_rc.slice.length;
            }
            default:
                break;
        }
        return 0;
    }

    u0 push_gc(ctx_t* ctx, obj_t* obj) {
        if (ctx->gc_stack_idx == GC_STACK_SIZE) {
            error(ctx, "gc stack overflow");
        }
        ctx->gc_stack[ctx->gc_stack_idx++] = obj;
    }

    obj_t* car(ctx_t* ctx, obj_t* obj) {
        if (IS_NIL(obj)) {
            return obj;
        }
        return CAR(check_type(ctx, obj, obj_type_t::pair));
    }

    obj_t* cdr(ctx_t* ctx, obj_t* obj) {
        if (IS_NIL(obj)) {
            return obj;
        }
        return CDR(check_type(ctx, obj, obj_type_t::pair));
    }

    obj_t* eval(ctx_t* ctx, obj_t* obj) {
        return eval(ctx, obj, ctx->nil, nullptr);
    }

    u0 error(ctx_t* ctx, const s8* msg) {
        obj_t* cl = ctx->call_list;
        ctx->call_list = ctx->nil;
        if (ctx->handlers.error) {
            ctx->handlers.error(ctx, msg, cl);
        }
        fprintf(stderr, "error: %s\n", msg);
        for (; !IS_NIL(cl); cl = CDR(cl)) {
            to_string(ctx, CAR(cl), ctx->scratch, sizeof(ctx->scratch));
            fprintf(stderr, "=> %s\n", ctx->scratch);
        }
        exit(EXIT_FAILURE);
    }

    obj_t* read_fp(ctx_t* ctx, FILE* fp) {
        return read(ctx, read_fp, fp);
    }

    u32 to_integer(ctx_t* ctx, obj_t* obj) {
        return u32(NUMBER(check_type(ctx, obj, obj_type_t::number)));
    }

    obj_t* make_bool(ctx_t* ctx, b8 value) {
        UNUSED(ctx);
        return value ? ctx->true_ : ctx->nil;
    }

    obj_type_t type(ctx_t* ctx, obj_t* obj) {
        UNUSED(ctx);
        return TYPE(obj);
    }

    u0* to_user_ptr(ctx_t* ctx, obj_t* obj) {
        return ctx->native_ptrs[INTEGER(check_type(ctx, obj, obj_type_t::ptr))];
    }

    u0 set(ctx_t* ctx, obj_t* sym, obj_t* v) {
        UNUSED(ctx);
        sym = get(ctx, sym, ctx->nil);
        SET_CDR(sym, v);
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
        SET_TYPE(obj, obj_type_t::ptr);
        SET_INTEGER(obj, ctx->native_ptr_idx);
        NATIVE_PTR(obj) = ptr;
        ++ctx->native_ptr_idx;
        return obj;
    }

    obj_t* make_number(ctx_t* ctx, number_t n) {
        obj_t* obj  = make_object(ctx);
        SET_TYPE(obj, obj_type_t::number);
        SET_NUMBER(obj, n);
        return obj;
    }

    number_t to_number(ctx_t* ctx, obj_t* obj) {
        return NUMBER(check_type(ctx, obj, obj_type_t::number));
    }

    u32 write_fp(ctx_t* ctx, obj_t* obj, FILE* fp) {
        return write(ctx, obj, write_fp, fp, 0);
    }

    obj_t* get(ctx_t* ctx, obj_t* sym, obj_t* env) {
        UNUSED(ctx);
        for (; !IS_NIL(env); env = CDR(env)) {
            obj_t* x = CAR(env);
            if (CAR(x) == sym) {
                return x;
            }
        }
        return CDR(sym);
    }

    obj_t* next_arg_no_chk(ctx_t* ctx, obj_t** arg) {
        obj_t* a = *arg;
        if (TYPE(a) != obj_type_t::pair)
            return ctx->nil;
        *arg = CDR(a);
        return CAR(a);
    }

    obj_t* cons(ctx_t* ctx, obj_t* car, obj_t* cdr) {
        obj_t* obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::pair);
        SET_CAR(obj, car);
        SET_CDR(obj, cdr);
        return obj;
    }

    obj_t* read(ctx_t* ctx, read_func_t fn, u0* udata) {
        obj_t* obj = read_(ctx, fn, udata);
        if (obj == ctx->rparen) {
            error(ctx, "stray ')'");
        }
        if (obj == ctx->rbrac) {
            error(ctx, "stray ']'");
        }
        return obj;
    }

    obj_t* make_list(ctx_t* ctx, obj_t** objs, u32 size) {
        obj_t* res = ctx->nil;
        while (size--)
            res = cons(ctx, objs[size], res);
        return res;
    }

    obj_t* make_native_func(ctx_t* ctx, native_func_t fn) {
        obj_t* obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::cfunc);
        SET_INTEGER(obj, ctx->native_ptr_idx);
        NATIVE_PTR(obj) = (u0*) fn;
        ++ctx->native_ptr_idx;
        return obj;
    }

    obj_t* make_string(ctx_t* ctx, const s8* str, s32 len) {
        auto intern_rc = string::interned::fold_for_result(str, len);
        if (!OK(intern_rc.status))
            error(ctx, "make_string unable to intern string");
        obj_t* obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::string);
        SET_INTEGER(obj, intern_rc.id);
        return obj;
    }

    obj_t* find_symbol(ctx_t* ctx, const s8* name, s32 len) {
        const auto name_rc = string::interned::fold_for_result(name, len);
        if (!OK(name_rc.status))
            error(ctx, "find_symbol unable to intern string");
        for (obj_t* obj = ctx->sym_list; !IS_NIL(obj); obj = CDR(obj)) {
            auto str = CAR(CDR(CAR(obj)));
            if (str_eq(str, name_rc.id))
                return CAR(obj);
        }
        return ctx->nil;
    }

    obj_t* make_symbol(ctx_t* ctx, const s8* name, s32 len) {
        obj_t* obj = find_symbol(ctx, name, len);
        if (!IS_NIL(obj))
            return obj;
        obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::symbol);
        SET_CDR(obj, cons(ctx, make_string(ctx, name, len), ctx->nil));
        ctx->sym_list = cons(ctx, obj, ctx->sym_list);
        return obj;
    }

    obj_t* make_keyword(ctx_t* ctx, const s8* name, s32 len) {
        obj_t* obj = find_symbol(ctx, name, len);
        if (!IS_NIL(obj))
            return obj;
        obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::keyword);
        SET_CDR(obj, cons(ctx, make_string(ctx, name, len), ctx->nil));
        ctx->sym_list = cons(ctx, obj, ctx->sym_list);
        return obj;
    }

    u32 to_string(ctx_t* ctx, obj_t* obj, s8* dst, u32 size) {
        Char_Ptr_Int x{};
        x.p = dst;
        x.n = size - 1;
        write(ctx, obj, write_buf, &x, 0);
        *x.p = '\0';
        return size - x.n - 1;
    }

    u32 write(ctx_t* ctx, obj_t* obj, write_func_t fn, u0* udata, u32 qt) {
        s8 buf[32];

        switch (TYPE(obj)) {
            case obj_type_t::nil:
                return write_str(ctx, fn, udata, "nil");

            case obj_type_t::number:
                sprintf(buf, "%.7g", NUMBER(obj));
                return write_str(ctx, fn, udata, buf);

            case obj_type_t::pair: {
                u32 len = 2;
                fn(ctx, udata, '(');
                for (;;) {
                    len += write(ctx, CAR(obj), fn, udata, 1);
                    obj = CDR(obj);
                    if (TYPE(obj) != obj_type_t::pair) {
                        break;
                    }
                    fn(ctx, udata, ' ');
                }
                if (!IS_NIL(obj)) {
                    len += write_str(ctx, fn, udata, " . ");
                    len += write(ctx, obj, fn, udata, 1);
                }
                fn(ctx, udata, ')');
                return len;
            }

            case obj_type_t::keyword:
                write_str(ctx, fn, udata, "#:");
            case obj_type_t::symbol:
                return write(ctx, CAR(CDR(obj)), fn, udata, 0) + 2;

            case obj_type_t::string: {
                if (qt) {
                    fn(ctx, udata, '"');
                }
                auto intern_rc = string::interned::get(INTEGER(obj));
                if (!OK(intern_rc.status)) {
                    error(ctx, "unable to find interned string");
                }
                auto len = write_str(ctx, fn, udata, (s8*) intern_rc.slice.data);
                if (qt) {
                    fn(ctx, udata, '"');
                }
                return len + 2;
            }

            case obj_type_t::prim:
                if (obj == ctx->true_) {
                    write_str(ctx, fn, udata, "#t");
                    break;
                }
                // N.B. fallthrough intentional

            default:
                sprintf(buf,
                        "[%s %p]",
                        s_type_names[u32(TYPE(obj))],
                        (u0*) obj);
                return write_str(ctx, fn, udata, buf);
        }

        return 0;
    }
}
