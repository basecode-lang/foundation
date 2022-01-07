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
// Copyright (C) 2017-2021 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <basecode/core/rpn.h>
#include <basecode/core/str.h>
#include <basecode/core/array.h>

#define SCOPES_TOP()            (t_scopes[t_scope_slot])
#define SCOPES_POP()            (t_scopes[t_scope_slot++])
#define SCOPES_PUSH(s)          (t_scopes[--t_scope_slot] = (s))
#define SCOPES_EMPTY()          (t_scope_slot == max_stack_depth - 1)

#define OPERATORS_TOP()         (t_operators[t_operator_slot])
#define OPERATORS_POP()         (t_operators[t_operator_slot++])
#define OPERATORS_PUSH(t)       (t_operators[--t_operator_slot] = (t))
#define OPERATORS_EMPTY()       (t_operator_slot == max_stack_depth - 1)

namespace basecode {
    namespace token::cls {
        static str::slice_t s_names[] = {
            "none"_ss,
            "operator_"_ss,
            "param_end"_ss,
            "scope_end"_ss,
            "param_begin"_ss,
            "scope_begin"_ss,
            "call_operator"_ss,
            "call_terminator"_ss,
            "stmt_terminator"_ss,
            "operator_consumer"_ss,
        };

        str::slice_t name(token_type_t type) {
            return s_names[TOKEN_CLS(type)];
        }
    }

    namespace rpn {
        constexpr    u32                max_stack_depth = 64;
        thread_local u32                t_scope_slot    = max_stack_depth - 1;
        thread_local u32                t_operator_slot = max_stack_depth - 1;
        thread_local postfix_expr_t*    t_scopes[max_stack_depth];
        thread_local const token_t *    t_operators[max_stack_depth];

        namespace expr {
            u0 free(postfix_expr_t& expr) {
                array::free(expr.tokens);
            }

            u0 init(postfix_expr_t& expr, alloc_t* alloc) {
                expr.arg_count  = {};
                expr.stmt_count = {};
                array::init(expr.tokens, alloc);
            }
        }

        u0 free(postfix_t& postfix) {
            array::free(postfix.exprs);
        }

        status_t init(postfix_t& postfix,
                      token_cache_t* tokens,
                      alloc_t* alloc) {
            if (!postfix.operator_precedences)
                return status_t::invalid_operator_precedence_array;
            postfix.alloc  = alloc;
            postfix.tokens = tokens;
            array::init(postfix.exprs, postfix.alloc);
            array::reserve(postfix.exprs, token::cache::size(*postfix.tokens));
            return status_t::ok;
        }

        b8 to_postfix(postfix_t& postfix) {
            operator_precedence_t* prec;
            const token_t        * token;
            const token_t        * top_op;
            postfix_expr_t       * expr;

            token::cache::seek_first(*postfix.tokens);

            while (token::cache::has_more(*postfix.tokens)) {
                token = &token::cache::current(*postfix.tokens);
                prec  = &postfix.operator_precedences[token->type];

                switch (TOKEN_CLS(token->type)) {
                    case token::cls::operator_:
                        while (!OPERATORS_EMPTY()) {
                            top_op = OPERATORS_TOP();
                            const auto& top_prec = postfix.operator_precedences[top_op->type];
                            const b8 is_lower_prec =    (prec->left > 0  && prec->left  <=  top_prec.left)
                                                     || (prec->right > 0 && prec->right <   top_prec.right);
                            if (is_lower_prec) {
                                array::append(expr->tokens, OPERATORS_POP()->id);
                                continue;
                            }
                            break;
                        }
                        OPERATORS_PUSH(token);
                        break;
                    case token::cls::operator_consumer:
                        while (!OPERATORS_EMPTY()) {
                            top_op = OPERATORS_TOP();
                            auto cls = TOKEN_CLS(top_op->type);
                            if (cls == token::cls::param_begin)
                                break;
                            array::append(expr->tokens, top_op->id);
                            OPERATORS_POP();
                        }
                        expr->arg_count++;
                        break;
                    case token::cls::scope_end: {
                        array::append(expr->tokens, token->id);
                        expr = SCOPES_POP();
                        expr->stmt_count++;
                        expr = add_expr(postfix);
                        break;
                    }
                    case token::cls::param_end:
                        while (!OPERATORS_EMPTY()) {
                            top_op = OPERATORS_POP();
                            if (TOKEN_CLS(top_op->type) == token::cls::param_begin)
                                break;
                            array::append(expr->tokens, top_op->id);
                        }
                        top_op = OPERATORS_TOP();
                        if (top_op && TOKEN_CLS(top_op->type) == token::cls::call_operator) {
//                            top_op->arg_count = expr->arg_count + 1;
                            array::append(expr->tokens, OPERATORS_POP()->id);
                        }
                        break;
                    case token::cls::param_begin:
                        top_op = &token::cache::peek(*postfix.tokens, 1);
                        expr->arg_count = top_op
                                          && TOKEN_CLS(top_op->type) == token::cls::param_end ?
                                             -1 : 0;
                        OPERATORS_PUSH(token);
                        break;
                    case token::cls::scope_begin:
                        expr = add_expr(postfix);
                        array::append(expr->tokens, token->id);
                        SCOPES_PUSH(expr);
                        break;
                    case token::cls::call_terminator:
                        SCOPES_TOP()->stmt_count++;
                        break;
                    case token::cls::stmt_terminator:
                        while (!OPERATORS_EMPTY())
                            array::append(expr->tokens, OPERATORS_POP()->id);
                        array::append(expr->tokens, token->id);
                        expr = SCOPES_TOP();
                        expr->stmt_count++;
                        expr = add_expr(postfix);
                        break;
                    default:
                        array::append(expr->tokens, token->id);
                        break;
                }

                token::cache::move_next(*postfix.tokens);
            }

            return true;
        }

        postfix_expr_t* add_expr(postfix_t& postfix) {
            auto expr = &array::append(postfix.exprs);
            expr::init(*expr, postfix.alloc);
            return expr;
        }
    }
}
