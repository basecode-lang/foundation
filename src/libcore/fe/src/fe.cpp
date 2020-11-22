/*
** Copyright (c) 2020 rxi
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
** IN THE SOFTWARE.
*/

#include "fe.h"
#include <basecode/core/error.h>
#include <basecode/core/string.h>

#define CAR(x)                  ((x)->car.o)
#define CDR(x)                  ((x)->cdr.o)
#define TAG(x)                  ((x)->car.c)
#define IS_NIL(x)               ((x) == &s_nil)
#define TYPE(x)                 (obj_type_t((TAG(x) & 0x1U ? TAG(x) >> 2U : u32(obj_type_t::pair))))
#define SET_TYPE(x, t)          (TAG(x) = u32((t)) << 2U | 1U)
#define NUMBER(x)               ((x)->cdr.n)
#define PRIM(x)                 ((x)->cdr.c)
#define CFUNC(x)                ((x)->cdr.f)
#define GC_MARK_BIT             (0x2U)
#define CLOSERS_SIZE            (256U)
#define SCRATCH_SIZE            (1024U)
#define GC_STACK_SIZE           (1024U)
#define EVAL_ARG()              eval(ctx, next_arg(ctx, &arg), env, nullptr)
#define ARITH_OP(op)            SAFE_SCOPE(                                                     \
                                    number_t x = to_number(ctx, EVAL_ARG());                    \
                                    while (!IS_NIL(arg)) {                                      \
                                        x = x op to_number(ctx, EVAL_ARG());                    \
                                    }                                                           \
                                    res = make_number(ctx, x);                                  \
                                )
#define NUM_CMP_OP(op)          SAFE_SCOPE(                                                     \
                                    va = check_type(ctx, EVAL_ARG(), obj_type_t::number);       \
                                    vb = check_type(ctx, EVAL_ARG(), obj_type_t::number);       \
                                    res = make_bool(ctx, NUMBER(va) op NUMBER(vb));             \
                                )

namespace basecode::fe {
    enum {
        P_LET,
        P_SET,
        P_IF,
        P_FN,
        P_MAC,
        P_WHILE,
        P_QUOTE,
        P_AND,
        P_OR,
        P_DO,
        P_CONS,
        P_CAR,
        P_CDR,
        P_SETCAR,
        P_SETCDR,
        P_LIST,
        P_NOT,
        P_IS,
        P_ATOM,
        P_PRINT,
        P_LT,
        P_LTE,
        P_ADD,
        P_SUB,
        P_MUL,
        P_DIV,
        P_MAX
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
        "<",
        "<=",
        "+",
        "-",
        "*",
        "/"
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

    union value_t {
        obj_t*                  o;
        native_func_t           f;
        number_t                n;
        u8                      c;
    };

    struct obj_t {
        value_t                 car;
        value_t                 cdr;
    };

    struct ctx_t {
        handlers_t              handlers;
        obj_t*                  gc_stack[GC_STACK_SIZE];
        obj_t*                  closers[CLOSERS_SIZE];
        obj_t*                  objects;
        obj_t*                  sym_list;
        obj_t*                  str_list;
        obj_t*                  call_list;
        obj_t*                  free_list;
        s8                      scratch[SCRATCH_SIZE];
        u32                     closer_idx;
        u32                     object_used;
        u32                     gc_stack_idx;
        u32                     object_count;
        s8                      next_chr;
    };

    struct Char_Ptr_Int {
        s8*                     p;
        u32                     n;
    };

    static obj_t*               s_dot;
    static obj_t*               s_true;
    static obj_t                s_nil = {
        {(obj_t*) (u32(obj_type_t::nil) << 2U | 1U)},
        {nullptr}
    };
    static obj_t                s_rbrac;
    static obj_t                s_rparen;

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
            // XXX: this should be comparing the diff with EPSILON
            return NUMBER(a) == NUMBER(b);
        } else if (TYPE(a) == obj_type_t::string) {
            return u32(NUMBER(a)) == u32(NUMBER(b));
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
        auto obj_rc = string::interned::get(NUMBER(obj));
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
        obj_t* res = &s_nil;
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
                if (ctx->closers[ctx->closer_idx++] == &s_rparen)
                    error(ctx, "expected closing paren but found closing bracket");
                return &s_rbrac;

            case ')':
                if (ctx->closers[ctx->closer_idx++] == &s_rbrac)
                    error(ctx, "expected closing bracket but found closing paren");
                return &s_rparen;

            case '[':
                ctx->closers[--ctx->closer_idx] = &s_rbrac;
                goto _skip;

            case '(':
                ctx->closers[--ctx->closer_idx] = &s_rparen;
            _skip: {
                obj_t*  res  = &s_nil;
                obj_t** tail = &res;
                auto gc = save_gc(ctx);
                /* to cause error on too-deep nesting */
                push_gc(ctx, res);
                while (true) {
                    v = read_(ctx, fn, udata);
                    if (v == &s_rparen || v == &s_rbrac)
                        break;
                    if (v == nullptr) {
                        error(ctx, "unclosed list");
                    }
                    if (v == s_dot) {
                        /* dotted pair */
                        *tail = read(ctx, fn, udata);
                    } else {
                        /* proper pair */
                        *tail = cons(ctx, v, &s_nil);
                        tail = &CDR(*tail);
                    }
                    restore_gc(ctx, gc);
                    push_gc(ctx, res);
                }
                return res;
            }

            case '\'': {
                v = read(ctx, fn, udata);
                if (!v) {
                    error(ctx, "stray '''");
                }
                return cons(ctx,
                            make_symbol(ctx, "quote", 5),
                            cons(ctx, v, &s_nil));
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
        obj_t* res   = &s_nil;
        obj_t** tail = &res;
        while (!IS_NIL(lst)) {
            *tail = cons(ctx,
                         eval(ctx, next_arg(ctx, &lst), env, nullptr),
                         &s_nil);
            tail = &CDR(*tail);
        }
        return res;
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
        obj_t cl{};
        u32   n;
        u32   gc;

        if (TYPE(obj) == obj_type_t::symbol) {
            return CDR(get(ctx, obj, env));
        }
        if (TYPE(obj) != obj_type_t::pair) {
            return obj;
        }

        CAR(&cl) = obj, CDR(&cl) = ctx->call_list;
        ctx->call_list = &cl;

        gc  = save_gc(ctx);
        fn  = eval(ctx, CAR(obj), env, nullptr);
        arg = CDR(obj);
        res = &s_nil;

        switch (TYPE(fn)) {
            case obj_type_t::prim:
                switch (PRIM(fn)) {
                    case P_LET:
                        va = check_type(ctx, next_arg(ctx, &arg), obj_type_t::symbol);
                        if (new_env) {
                            *new_env = cons(ctx, cons(ctx, va, EVAL_ARG()), env);
                        }
                        break;

                    case P_SET:
                        va = check_type(ctx, next_arg(ctx, &arg), obj_type_t::symbol);
                        CDR(get(ctx, va, env)) = EVAL_ARG();
                        break;

                    case P_IF:
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

                    case P_FN:
                    case P_MAC:
                        va = cons(ctx, env, arg);
                        next_arg(ctx, &arg);
                        res = make_object(ctx);
                        SET_TYPE(res, PRIM(fn) == P_FN ? obj_type_t::func : obj_type_t::macro);
                        CDR(res) = va;
                        break;

                    case P_WHILE:
                        va = next_arg(ctx, &arg);
                        n  = save_gc(ctx);
                        while (!IS_NIL(eval(ctx, va, env, nullptr))) {
                            do_list(ctx, arg, env);
                            restore_gc(ctx, n);
                        }
                        break;

                    case P_QUOTE:
                        res = next_arg(ctx, &arg);
                        break;

                    case P_AND:
                        while (!IS_NIL(arg) && !IS_NIL(res = EVAL_ARG()));
                        break;

                    case P_OR:
                        while (!IS_NIL(arg) && IS_NIL(res = EVAL_ARG()));
                        break;

                    case P_DO:
                        res = do_list(ctx, arg, env);
                        break;

                    case P_CONS:
                        va  = EVAL_ARG();
                        res = cons(ctx, va, EVAL_ARG());
                        break;

                    case P_CAR:
                        res = car(ctx, EVAL_ARG());
                        break;

                    case P_CDR:
                        res = cdr(ctx, EVAL_ARG());
                        break;

                    case P_SETCAR:
                        va = check_type(ctx, EVAL_ARG(), obj_type_t::pair);
                        CAR(va) = EVAL_ARG();
                        break;

                    case P_SETCDR:
                        va = check_type(ctx, EVAL_ARG(), obj_type_t::pair);
                        CDR(va) = EVAL_ARG();
                        break;

                    case P_LIST:
                        res = eval_list(ctx, arg, env);
                        break;

                    case P_NOT:
                        res = make_bool(ctx, IS_NIL(EVAL_ARG()));
                        break;

                    case P_IS:
                        va  = EVAL_ARG();
                        res = make_bool(ctx, equal(va, EVAL_ARG()));
                        break;

                    case P_ATOM:
                        res = make_bool(ctx, type(ctx, EVAL_ARG()) != obj_type_t::pair);
                        break;

                    case P_PRINT:
                        while (!IS_NIL(arg)) {
                            write_fp(ctx, EVAL_ARG(), stdout);
                            if (!IS_NIL(arg)) {
                                printf(" ");
                            }
                        }
                        printf("\n");
                        break;

                    case P_LT:
                        NUM_CMP_OP(<);
                        break;

                    case P_LTE:
                        NUM_CMP_OP(<=);
                        break;

                    case P_ADD:
                        ARITH_OP(+);
                        break;

                    case P_SUB:
                        ARITH_OP(-);
                        break;

                    case P_MUL:
                        ARITH_OP(*);
                        break;

                    case P_DIV:
                        ARITH_OP(/);
                        break;
                }
                break;

            case obj_type_t::cfunc:
                res = CFUNC(fn)(ctx, eval_list(ctx, arg, env));
                break;

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
                ctx->call_list = CDR(&cl);
                return eval(ctx, obj, env, nullptr);

            default:
                error(ctx, "tried to call non-callable value");
        }

        restore_gc(ctx, gc);
        push_gc(ctx, res);
        ctx->call_list = CDR(&cl);
        return res;
    }

    static u0 write_str(ctx_t* ctx, write_func_t fn, u0* udata, const s8* s) {
        while (*s)
            fn(ctx, udata, *s++);
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

    obj_t* nil() {
        return &s_nil;
    }

    u0 free(ctx_t* ctx) {
        ctx->gc_stack_idx = 0;
        ctx->str_list     = &s_nil;
        ctx->sym_list     = &s_nil;
        collect_garbage(ctx);
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
        mark(ctx, ctx->str_list);
        for (u32 i = 0; i < ctx->object_count; i++) {
            obj_t* obj = &ctx->objects[i];
            if (TYPE(obj) == obj_type_t::free) {
                continue;
            }
            if (~ (u32) TAG(obj) & GC_MARK_BIT) {
                if (TYPE(obj) == obj_type_t::ptr
                &&  ctx->handlers.gc) {
                    ctx->handlers.gc(ctx, obj);
                }
                SET_TYPE(obj, obj_type_t::free);
                CDR(obj) = ctx->free_list;
                ctx->free_list = obj;
                --ctx->object_used;
            } else {
                TAG(obj) &= ~GC_MARK_BIT;
            }
        }
    }

    ctx_t* make(u0* ptr, u32 size) {
        /* init context struct */
        auto ctx = (ctx_t*) ptr;
        std::memset(ctx, 0, sizeof(ctx_t));
        ptr = (u8*) ptr + sizeof(ctx_t);
        size -= sizeof(ctx_t);

        /* init objects memory region */
        ctx->objects      = (obj_t*) ptr;
        ctx->object_used  = 0;
        ctx->object_count = size / sizeof(obj_t);

        /* init lists */
        ctx->sym_list  = &s_nil;
        ctx->str_list  = &s_nil;
        ctx->call_list = &s_nil;
        ctx->free_list = &s_nil;

        /* populate freelist */
        for (u32 i = 0; i < ctx->object_count; i++) {
            obj_t* obj = &ctx->objects[i];
            SET_TYPE(obj, obj_type_t::free);
            CDR(obj) = ctx->free_list;
            ctx->free_list = obj;
        }

        /* init objects */
        auto save = save_gc(ctx);
        s_dot  = make_symbol(ctx, ".", 1);
        s_true = make_symbol(ctx, "#t", 2);
        set(ctx, s_dot, s_dot);
        set(ctx, s_true, s_true);
        set(ctx, make_symbol(ctx, "#f", 2), &s_nil);
        set(ctx, make_symbol(ctx, "nil", 3), &s_nil);
        restore_gc(ctx, save);

        /* register built in primitives */
        for (u32 i = 0; i < P_MAX; i++) {
            obj_t* v = make_object(ctx);
            SET_TYPE(v, obj_type_t::prim);
            PRIM(v) = i;
            set(ctx, make_symbol(ctx, s_prim_names[i]), v);
            restore_gc(ctx, save);
        }

        ctx->closer_idx = CLOSERS_SIZE - 1;

        return ctx;
    }

    u0 mark(ctx_t* ctx, obj_t* obj) {
        obj_t* car;
    begin:
        if (TAG(obj) & GC_MARK_BIT) {
            return;
        }
        /* store car before modifying it with GCMARKBIT */
        car = CAR(obj);
        TAG(obj) |= GC_MARK_BIT;

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
        return s_true == obj;
    }

    u32 length(ctx_t* ctx, obj_t* obj) {
        switch (TYPE(obj)) {
            case obj_type_t::pair: {
                u32 len = 0;
                for (obj_t* pair = obj; !IS_NIL(pair); pair = CDR(pair)) {
//                    auto intern_rc = string::interned::get(NUMBER(CAR(pair)));
//                    format::print("id = {}, slice = {}\n", intern_rc.id, intern_rc.slice);
                    ++len;
                }
                return len;
            }
            case obj_type_t::string: {
                auto str_rc = string::interned::get(NUMBER(obj));
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

    u0* to_ptr(ctx_t* ctx, obj_t* obj) {
        return CDR(check_type(ctx, obj, obj_type_t::ptr));
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
        return eval(ctx, obj, &s_nil, nullptr);
    }

    u0 error(ctx_t* ctx, const s8* msg) {
        obj_t* cl = ctx->call_list;
        ctx->call_list = &s_nil;
        if (ctx->handlers.error) {
            ctx->handlers.error(ctx, msg, cl);
        }
        fprintf(stderr, "error: %s\n", msg);
        for (; !IS_NIL(cl); cl = CDR(cl)) {
            s8 buf[64];
            to_string(ctx, CAR(cl), buf, sizeof(buf));
            fprintf(stderr, "=> %s\n", buf);
        }
        exit(EXIT_FAILURE);
    }

    obj_t* make_ptr(ctx_t* ctx, u0* ptr) {
        obj_t* obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::ptr);
        CDR(obj) = (obj_t*) ptr;
        return obj;
    }

    obj_t* read_fp(ctx_t* ctx, FILE* fp) {
        return read(ctx, read_fp, fp);
    }

    obj_t* make_bool(ctx_t* ctx, b8 value) {
        UNUSED(ctx);
        return value ? s_true : &s_nil;
    }

    obj_type_t type(ctx_t* ctx, obj_t* obj) {
        UNUSED(ctx);
        return TYPE(obj);
    }

    u0 set(ctx_t* ctx, obj_t* sym, obj_t* v) {
        UNUSED(ctx);
        CDR(get(ctx, sym, &s_nil)) = v;
    }

    obj_t* next_arg(ctx_t* ctx, obj_t** arg) {
        obj_t* a = *arg;
        if (TYPE(a) != obj_type_t::pair) {
            if (IS_NIL(a)) {
                error(ctx, "too few arguments");
            }
            error(ctx, "dotted pair in argument list");
        }
        *arg = CDR(a);
        return CAR(a);
    }

    obj_t* make_number(ctx_t* ctx, number_t n) {
        obj_t* obj  = make_object(ctx);
        SET_TYPE(obj, obj_type_t::number);
        NUMBER(obj) = n;
        return obj;
    }

    number_t to_number(ctx_t* ctx, obj_t* obj) {
        return NUMBER(check_type(ctx, obj, obj_type_t::number));
    }

    u0 write_fp(ctx_t* ctx, obj_t* obj, FILE* fp) {
        write(ctx, obj, write_fp, fp, 0);
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

    obj_t* cons(ctx_t* ctx, obj_t* car, obj_t* cdr) {
        obj_t* obj = make_object(ctx);
        CAR(obj)   = car;
        CDR(obj)   = cdr;
        return obj;
    }

    obj_t* read(ctx_t* ctx, read_func_t fn, u0* udata) {
        obj_t* obj = read_(ctx, fn, udata);
        if (obj == &s_rparen) {
            error(ctx, "stray ')'");
        }
        if (obj == &s_rbrac) {
            error(ctx, "stray ']'");
        }
        return obj;
    }

    obj_t* make_list(ctx_t* ctx, obj_t** objs, u32 size) {
        obj_t* res = &s_nil;
        while (size--)
            res = cons(ctx, objs[size], res);
        return res;
    }

    obj_t* make_native_func(ctx_t* ctx, native_func_t fn) {
        obj_t* obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::cfunc);
        CFUNC(obj) = fn;
        return obj;
    }

    obj_t* make_string(ctx_t* ctx, const s8* str, s32 len) {
        auto intern_rc = string::interned::fold_for_result(str, len);
        if (!OK(intern_rc.status))
            error(ctx, "make_string unable to intern string");
//        for (obj_t* obj = ctx->str_list; !IS_NIL(obj); obj = CDR(obj)) {
//            if (str_eq(CAR(obj), intern_rc.id))
//                return CAR(obj);
//        }
        obj_t* obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::string);
        NUMBER(obj) = number_t(intern_rc.id);
//        ctx->str_list = cons(ctx, obj, ctx->str_list);
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
        return &s_nil;
    }

    obj_t* make_symbol(ctx_t* ctx, const s8* name, s32 len) {
        obj_t* obj = find_symbol(ctx, name, len);
        if (!IS_NIL(obj))
            return obj;
        obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::symbol);
        CDR(obj) = cons(ctx, make_string(ctx, name, len), &s_nil);
        ctx->sym_list = cons(ctx, obj, ctx->sym_list);
        return obj;
    }

    obj_t* make_keyword(ctx_t* ctx, const s8* name, s32 len) {
        obj_t* obj = find_symbol(ctx, name, len);
        if (!IS_NIL(obj))
            return obj;
        obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::keyword);
        CDR(obj) = cons(ctx, make_string(ctx, name, len), &s_nil);
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

    u0 write(ctx_t* ctx, obj_t* obj, write_func_t fn, u0* udata, u32 qt) {
        s8 buf[32];

        switch (TYPE(obj)) {
            case obj_type_t::nil:
                write_str(ctx, fn, udata, "nil");
                break;

            case obj_type_t::number:
                sprintf(buf, "%.7g", NUMBER(obj));
                write_str(ctx, fn, udata, buf);
                break;

            case obj_type_t::pair:
                fn(ctx, udata, '(');
                for (;;) {
                    write(ctx, CAR(obj), fn, udata, 1);
                    obj = CDR(obj);
                    if (TYPE(obj) != obj_type_t::pair) {
                        break;
                    }
                    fn(ctx, udata, ' ');
                }
                if (!IS_NIL(obj)) {
                    write_str(ctx, fn, udata, " . ");
                    write(ctx, obj, fn, udata, 1);
                }
                fn(ctx, udata, ')');
                break;

            case obj_type_t::keyword:
                write_str(ctx, fn, udata, "#:");
            case obj_type_t::symbol:
                write(ctx, CAR(CDR(obj)), fn, udata, 0);
                break;

            case obj_type_t::string: {
                if (qt) {
                    fn(ctx, udata, '"');
                }
                auto intern_rc = string::interned::get(NUMBER(obj));
                if (!OK(intern_rc.status)) {
                    error(ctx, "unable to find interned string");
                }
                write_str(ctx, fn, udata, (s8*) intern_rc.slice.data);
                if (qt) {
                    fn(ctx, udata, '"');
                }
                break;
            }

            case obj_type_t::prim:
                if (obj == s_true) {
                    write_str(ctx, fn, udata, "#t");
                    break;
                }
                // N.B. fallthrough intentional

            default:
                sprintf(buf, "[%s %p]", s_type_names[u32(TYPE(obj))], (u0*) obj);
                write_str(ctx, fn, udata, buf);
                break;
        }
    }
}

#ifdef FE_STANDALONE

#include <setjmp.h>

static jmp_buf toplevel;
static char buf[64000];

static void onerror(fe_Context *ctx, const char *msg, fe_Object *cl) {
  unused(ctx), unused(cl);
  fprintf(stderr, "error: %s\n", msg);
  longjmp(toplevel, -1);
}


int main(int argc, char **argv) {
  int gc;
  fe_Object *obj;
  FILE *volatile fp = stdin;
  fe_Context *ctx = fe_open(buf, sizeof(buf));

  /* init input file */
  if (argc > 1) {
    fp = fopen(argv[1], "rb");
    if (!fp) { fe_error(ctx, "could not open input file"); }
  }

  if (fp == stdin) { fe_handlers(ctx)->error = onerror; }
  gc = fe_savegc(ctx);
  setjmp(toplevel);

  /* re(p)l */
  for (;;) {
    fe_restoregc(ctx, gc);
    if (fp == stdin) { printf("> "); }
    if (!(obj = fe_readfp(ctx, fp))) { break; }
    obj = fe_eval(ctx, obj);
    if (fp == stdin) { fe_writefp(ctx, obj, stdout); printf("\n"); }
  }

  return EXIT_SUCCESS;
}

#endif
