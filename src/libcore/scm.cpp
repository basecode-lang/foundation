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
#include <basecode/core/string.h>

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
#define FLONUM(x)               (numeric_alias_t{.dw = (x)->number.value}.fdw)
#define STRING_ID(x)            (FIXNUM((x)))
#define SET_FIXNUM(x, v)        ((x)->number.value = (v))
#define SET_FLONUM(x, v)        ((x)->number.value = numeric_alias_t{.fdw =(v)}.dw)
#define PRIM(x)                 (prim_type_t((x)->prim.code))
#define SET_PRIM(x, p)          ((x)->prim.code = fixnum_t((p)))
#define NATIVE_PTR(x)           (ctx->native_ptrs[FIXNUM((x))])
#define CLOSERS_SIZE            (256U)
#define SCRATCH_SIZE            (1024U)
#define GC_STACK_SIZE           (1024U)
#define NATIVE_PTR_SIZE         (256U)
#define EVAL(o)                 eval(ctx, (o), env, nullptr)
#define EVAL_ARG()              eval(ctx, next_arg(ctx, &arg), env, nullptr)
#define SYM(o)                  make_symbol(ctx, (o))
#define CONS1(a)                cons(ctx, (a), ctx->nil)
#define CONS(a, d)              cons(ctx, (a), (d))
#define PRINT(h, o)             SAFE_SCOPE(                                                     \
                                    fprintf(stdout, (h));                                       \
                                    write_fp(ctx, (o), stdout);                                 \
                                    fprintf(stdout, "\n");                                      \
                                    fflush(stdout);                                             \
                                )

namespace basecode::scm {
    enum class prim_type_t : u8 {
        eval,
        let,
        set,
        if_,
        fn,
        mac,
        while_,
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

    static const s8* s_prim_names[] = {
        "eval",
        "let",
        "=",
        "if",
        "fn",
        "mac",
        "while",
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
    };

    enum class numtype_t : u8 {
        none,
        fixnum,
        flonum,
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
        obj_t*                  false_;
        obj_t*                  rbrac;
        obj_t*                  rparen;
        s8                      scratch[SCRATCH_SIZE];
        ffi_t                   ffi;
        u32                     closer_idx;
        u32                     object_used;
        u32                     gc_stack_idx;
        u32                     object_count;
        u32                     native_ptr_idx;
        s8                      next_chr;
    } __attribute__((aligned(128)));

    struct char_ptr_int_t final {
        s8*                     p;
        u32                     n;
    } __attribute__((aligned(16)));

    struct ffi_type_map_t final {
        u8                      type;
        u8                      size;
    } __attribute__((aligned(2)));

    static ffi_type_map_t s_types[] = {
        [u32(obj_type_t::pair)]     = ffi_type_map_t{.type = 'P', .size = 'q'},
        [u32(obj_type_t::nil)]      = ffi_type_map_t{.type = 'P', .size = 'q'},
        [u32(obj_type_t::fixnum)]   = ffi_type_map_t{.type = 'I', .size = 'd'},
        [u32(obj_type_t::flonum)]   = ffi_type_map_t{.type = 'F', .size = 'd'},
        [u32(obj_type_t::symbol)]   = ffi_type_map_t{.type = 'S', .size = 'q'},
        [u32(obj_type_t::string)]   = ffi_type_map_t{.type = 'S', .size = 'q'},
        [u32(obj_type_t::boolean)]  = ffi_type_map_t{.type = 'B', .size = 'b'},
    };

    static obj_t* eval(ctx_t* ctx,
                       obj_t* obj,
                       obj_t* env,
                       obj_t** new_env);

    static obj_t* args_to_env(ctx_t* ctx, obj_t* prm, obj_t* arg, obj_t* env);

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

    static s8 read_fp(ctx_t* ctx, u0* udata) {
        s32 chr;
        UNUSED(ctx);
        return (chr = fgetc((FILE*) udata)) == EOF ? '\0' : s8(chr);
    }

    static numtype_t get_numtype(const s8* buf, u32 len) {
        const s8* p = buf;
        s8 ch = *p;
        numtype_t type = ((ch == '+' && len > 1)
                    || (ch == '-' && len > 1)
                    || isdigit(ch)) ? numtype_t::fixnum : numtype_t::none;
        if (type == numtype_t::none)
            return type;
        ch = *(++p);
        while (ch) {
            if (ch == '.' || ch == 'e' || ch == 'E' || ch == '-') {
                type = numtype_t::flonum;
            } else if (ch < 48 || ch > 57) {
                type = numtype_t::none;
                break;
            }
            ch = *(++p);
        }
        return type;
    }

    static b8 equal(ctx_t* ctx, obj_t* a, obj_t* b) {
        if (a == b)
            return true;
        if ((TYPE(a) == obj_type_t::fixnum || TYPE(a) == obj_type_t::flonum)
        &&  (TYPE(b) == obj_type_t::fixnum || TYPE(b) == obj_type_t::flonum)) {
            switch (TYPE(a)) {
                case obj_type_t::fixnum: {
                    auto fa = to_fixnum(ctx, a);
                    auto fb = to_fixnum(ctx, b);
                    return fa == fb;
                }
                case obj_type_t::flonum: {
                    const auto e = std::numeric_limits<flonum_t>::epsilon();
                    auto x = fabs(to_flonum(ctx, a));
                    auto y = fabs(to_flonum(ctx, b));
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
            case obj_type_t::cfunc:
            case obj_type_t::string:    return STRING_ID(a) == STRING_ID(b);
            default:                    break;
        }
        return false;
    }

    static u0 write_fp(ctx_t* ctx, u0* udata, s8 chr) {
        UNUSED(ctx);
        fputc(chr, (FILE*) udata);
    }

    static u0 write_buf(ctx_t* ctx, u0* udata, s8 chr) {
        auto x = (char_ptr_int_t*) udata;
        UNUSED(ctx);
        if (x->n) {
            *x->p++ = chr;
            x->n--;
        }
    }

    static b8 str_eq(ctx_t* ctx, obj_t* obj, u32 str_id) {
        auto obj_rc = string::interned::get(STRING_ID(obj));
        if (!OK(obj_rc.status))
            return false;
        return obj_rc.id == str_id;
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

            case '`': {
                v = read(ctx, fn, udata);
                if (!v)
                    error(ctx, "stray '`'");
                return cons(ctx,
                            make_symbol(ctx, "quasiquote", 10),
                            cons(ctx, v, ctx->nil));
            }

            case ',': {
                obj_t* sym;
                chr = fn(ctx, udata);
                if (chr == '@') {
                    v = read(ctx, fn, udata);
                    if (!v)
                        error(ctx, "stray ',@'");
                    sym = make_symbol(ctx, "unquote-splicing", 16);
                } else {
                    ctx->next_chr = chr;
                    v = read(ctx, fn, udata);
                    if (!v)
                        error(ctx, "stray ','");
                    sym = make_symbol(ctx, "unquote", 7);
                }
                return cons(ctx, sym, cons(ctx, v, ctx->nil));
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
                if (p != ctx->scratch) {
                    switch (get_numtype(ctx->scratch, p - ctx->scratch)) {
                        case numtype_t::none:
                            if (strncmp(ctx->scratch, "#:", 2) == 0) {
                                return make_keyword(ctx, ctx->scratch + 2, len - 2);
                            } else {
                                return make_symbol(ctx, ctx->scratch, len);
                            }
                        case numtype_t::fixnum:
                            return make_fixnum(ctx, strtol(ctx->scratch, &p, 10));
                        case numtype_t::flonum:
                            return make_flonum(ctx, strtod(ctx->scratch, &p));
                    }
                } else {
                    error(ctx, "expected flonum, fixnum, keyword, or symbol");
                }
            }
        }

        return ctx->nil;
    }

    static obj_t* eval_list(ctx_t* ctx, obj_t* lst, obj_t* env) {
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

    static obj_t* quasiquote(ctx_t* ctx, obj_t* obj, obj_t* env) {
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

    static u32 make_ffi_signature(ctx_t* ctx, obj_t* args, obj_t* env, u8* buf) {
        u32 len{};
        while (!IS_NIL(args)) {
            const auto type = TYPE(EVAL(CAR(args)));
            const auto& mapping = s_types[u32(type)];
            buf[len++] = mapping.type;
            buf[len++] = mapping.size;
            args = CDR(args);
        }
        return len;
    }

    static obj_t* eval(ctx_t* ctx, obj_t* obj, obj_t* env, obj_t** new_env) {
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
                res = CDR(get(ctx, obj, env));
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
                    fn  = CDR(get(ctx, kar, env));
            else    fn = kar;

            switch (TYPE(fn)) {
                case obj_type_t::prim:
                    switch (PRIM(fn)) {
                        case prim_type_t::eval:
                            obj = EVAL_ARG();
                            break;

                        case prim_type_t::let:
                            va = check_type(ctx, next_arg(ctx, &arg), obj_type_t::symbol);
                            if (new_env)
                                *new_env = cons(ctx, cons(ctx, va, EVAL_ARG()), env);
                            res = ctx->nil;
                            break;

                        case prim_type_t::set:
                            va = check_type(ctx, next_arg(ctx, &arg), obj_type_t::symbol);
                            SET_CDR(get(ctx, va, env), EVAL_ARG());
                            res = ctx->nil;
                            break;

                        case prim_type_t::if_:
                            obj = IS_TRUE(EVAL(CAR(arg))) ? CADR(arg) : CADDR(arg);
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
                            while (true) {
                                vb = EVAL(va);
                                if (IS_NIL(vb) || IS_FALSE(vb))
                                    break;
                                vb = arg;
                                for (; !IS_NIL(vb); vb = CDR(vb)) {
                                    restore_gc(ctx, n);
                                    push_gc(ctx, vb);
                                    push_gc(ctx, env);
                                    eval(ctx, CAR(vb), env, &env);
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
                                eval(ctx, CAR(arg), env, &env);
                            }
                            obj = CAR(arg);
                            break;
                        }

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
                            res = ctx->nil;
                            break;

                        case prim_type_t::gt:
                            va  = EVAL_ARG();
                            vb  = EVAL_ARG();
                            res = make_bool(ctx, to_flonum(ctx, va) > to_flonum(ctx, vb));
                            break;

                        case prim_type_t::gte:
                            va  = EVAL_ARG();
                            vb  = EVAL_ARG();
                            res = make_bool(ctx, to_flonum(ctx, va) >= to_flonum(ctx, vb));
                            break;

                        case prim_type_t::lt:
                            va  = EVAL_ARG();
                            vb  = EVAL_ARG();
                            res = make_bool(ctx, to_flonum(ctx, va) < to_flonum(ctx, vb));
                            break;

                        case prim_type_t::lte:
                            va  = EVAL_ARG();
                            vb  = EVAL_ARG();
                            res = make_bool(ctx, to_flonum(ctx, va) <= to_flonum(ctx, vb));
                            break;

                        case prim_type_t::add: {
                            flonum_t x = to_flonum(ctx, EVAL_ARG());
                            while (!IS_NIL(arg))
                                x += to_flonum(ctx, EVAL_ARG());
                            res = make_flonum(ctx, x);
                            break;
                        }

                        case prim_type_t::sub: {
                            flonum_t x = to_flonum(ctx, EVAL_ARG());
                            while (!IS_NIL(arg))
                                x -= to_flonum(ctx, EVAL_ARG());
                            res = make_flonum(ctx, x);
                            break;
                        }

                        case prim_type_t::mul: {
                            flonum_t x = to_flonum(ctx, EVAL_ARG());
                            while (!IS_NIL(arg))
                                x *= to_flonum(ctx, EVAL_ARG());
                            res = make_flonum(ctx, x);
                            break;
                        }

                        case prim_type_t::div: {
                            flonum_t x = to_flonum(ctx, EVAL_ARG());
                            while (!IS_NIL(arg))
                                x /= to_flonum(ctx, EVAL_ARG());
                            res = make_flonum(ctx, x);
                            break;
                        }

                        case prim_type_t::mod: {
                            fixnum_t x = to_fixnum(ctx, EVAL_ARG());
                            while (!IS_NIL(arg))
                                x %= to_fixnum(ctx, EVAL_ARG());
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
                        error(ctx, "ffi: no matching overload for function");
                    n = 0;
                    ffi::reset(ctx->ffi);
                    while (!IS_NIL(arg)) {
                        if (n >= ol->params.size)
                            error(ctx, "ffi: too many arguments");
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
                            case obj_type_t::keyword: {
                                va = CADR(va);
                                auto s = string::interned::get_slice(STRING_ID(va));
                                if (!s)
                                    error(ctx, "ffi: invalid interned string id for keyword");
                                break;
                            }
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
                                va = CADR(va);
                                // N.B. fallthrough intentional
                            case obj_type_t::string: {
                                auto s = string::interned::get_slice(STRING_ID(va));
                                if (!s)
                                    error(ctx, "ffi: invalid interned string id");
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
                        error(ctx, "ffi: call failed");
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
                    arg  = eval_list(ctx, arg, env);
                    va   = CDR(fn); // (env params ...)
                    vb   = CDR(va); // (params ...)
                    env  = args_to_env(ctx, CAR(vb), arg, CAR(va));
                    arg  = CDR(vb);
                    u32 save = save_gc(ctx);
                    for (; !IS_NIL(CDR(arg)); arg = CDR(arg)) {
                        restore_gc(ctx, save);
                        push_gc(ctx, arg);
                        push_gc(ctx, env);
                        eval(ctx, CAR(arg), env, &env);
                    }
                    obj = CAR(arg);
                    break;
                }

                case obj_type_t::macro: {
                    // replace caller object with code generated by macro and re-eval
                    va = CDR(fn); // (env params ...)
                    vb = CDR(va); // (params ...)
                    auto mac_env = args_to_env(ctx, CAR(vb), arg, CAR(va));
                    arg = CDR(vb);
                    u32 save = save_gc(ctx);
                    while (!IS_NIL(arg)) {
                        restore_gc(ctx, save);
                        push_gc(ctx, arg);
                        push_gc(ctx, mac_env);
                        res = eval(ctx, next_arg(ctx, &arg), mac_env, &mac_env);
                    }
                    *obj = *res;
                    res  = {};
                    ctx->call_list = CDR(cl);
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
        ffi::free(ctx->ffi);
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
//        format::print("before: ctx->object_used = {}\n", ctx->object_used);
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
//        format::print("after : ctx->object_used = {}\n", ctx->object_used);
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

        ctx->rbrac  = make_object(ctx);
        ctx->rparen = make_object(ctx);
        ctx->dot    = make_symbol(ctx, ".", 1);
        set(ctx, ctx->dot, ctx->dot);
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

        ffi::init(ctx->ffi);

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
                auto rc = string::interned::get(STRING_ID(obj));
                if (!OK(rc.status))
                    error(ctx, "unable to find interned string");
                return rc.slice.length;
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

    obj_t* make_bool(ctx_t* ctx, b8 value) {
        return value ? ctx->true_ : ctx->false_;
    }

    obj_type_t type(ctx_t* ctx, obj_t* obj) {
        UNUSED(ctx);
        return TYPE(obj);
    }

    u0* to_user_ptr(ctx_t* ctx, obj_t* obj) {
        return ctx->native_ptrs[FIXNUM(check_type(ctx, obj, obj_type_t::ptr))];
    }

    obj_t* make_bool(ctx_t* ctx, obj_t* obj) {
        return IS_TRUE(obj) ? ctx->true_ : ctx->false_;
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
        SET_FIXNUM(obj, ctx->native_ptr_idx);
        NATIVE_PTR(obj) = ptr;
        ++ctx->native_ptr_idx;
        return obj;
    }

    fixnum_t to_fixnum(ctx_t* ctx, obj_t* obj) {
        numeric_alias_t a{.dw = obj->number.value};
        if (TYPE(obj) == obj_type_t::fixnum)
            return a.dw;
        else
            return fixnum_t(a.fdw);
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

    flonum_t to_flonum(ctx_t* ctx, obj_t* obj) {
        numeric_alias_t a{.dw = obj->number.value};
        if (TYPE(obj) == obj_type_t::flonum)
            return a.fdw;
        else
            return flonum_t(a.dw);
    }

    obj_t* make_ffi(ctx_t* ctx, proto_t* proto) {
        obj_t* obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::ffi);
        SET_FIXNUM(obj, ctx->native_ptr_idx);
        NATIVE_PTR(obj) = proto;
        ++ctx->native_ptr_idx;
        return obj;
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
        SET_FIXNUM(obj, ctx->native_ptr_idx);
        NATIVE_PTR(obj) = (u0*) fn;
        ++ctx->native_ptr_idx;
        return obj;
    }

    obj_t* make_string(ctx_t* ctx, const s8* str, s32 len) {
        auto rc = string::interned::fold_for_result(str, len);
        if (!OK(rc.status))
            error(ctx, "make_string unable to intern string");
        obj_t* obj = make_object(ctx);
        SET_TYPE(obj, obj_type_t::string);
        SET_FIXNUM(obj, rc.id);
        return obj;
    }

    obj_t* find_symbol(ctx_t* ctx, const s8* name, s32 len) {
        const auto rc = string::interned::fold_for_result(name, len);
        if (!OK(rc.status))
            error(ctx, "find_symbol unable to intern string");
        for (obj_t* obj = ctx->sym_list; !IS_NIL(obj); obj = CDR(obj)) {
            auto str = CAR(CDR(CAR(obj)));
            if (str_eq(ctx, str, rc.id))
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
        char_ptr_int_t x{};
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

            case obj_type_t::fixnum:
                sprintf(buf, "%d", FIXNUM(obj));
                return write_str(ctx, fn, udata, buf);

            case obj_type_t::flonum:
                sprintf(buf, "%.7g", FLONUM(obj));
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
                auto id = STRING_ID(obj);
                auto intern_rc = string::interned::get(id);
                if (!OK(intern_rc.status)) {
                    error(ctx, "unable to find interned string");
                }
                auto len = write_str(ctx, fn, udata, (s8*) intern_rc.slice.data);
                if (qt) {
                    fn(ctx, udata, '"');
                }
                return len + 2;
            }

            case obj_type_t::boolean:
                return write_str(ctx, fn, udata, obj == ctx->true_ ? "#t" : "#f");

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
