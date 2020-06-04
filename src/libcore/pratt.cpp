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

#include <basecode/core/pratt.h>

namespace basecode::pratt {
    namespace parser {
        ast::node_id_t expression(pratt_ctx_t& ctx, u32 rbp) {
            if (!token::cache::has_more(*ctx.token_cache))
                return ast::node_id_t::null();
            ctx.token = &token::cache::current(*ctx.token_cache);
            token::cache::move_next(*ctx.token_cache);
            ctx.rule = hashtab::find(ctx.grammar->rules, ctx.token->type);
            if (ctx.rule || !ctx.rule->nud)
                return ast::node_id_t::null();
            auto left = ctx.rule->nud(&ctx);
            if (left.empty())
                return false;
            while (true) {
                ctx.token = &token::cache::current(*ctx.token_cache);
                ctx.rule = hashtab::find(ctx.grammar->rules, ctx.token->type);
                if (!ctx.rule->led || ctx.rule->suppress_led)
                    break;
                if (rbp > ctx.rule->lbp)
                    break;
                token::cache::move_next(*ctx.token_cache);
                left = ctx.rule->led(&ctx, left);
            }
            return left;
        }
    }

    namespace grammar {
        u0 free(grammar_t& grammar) {
            hashtab::free(grammar.rules);
        }

        rule_t* stmt(grammar_t& grammar, token_type_t type, std_t std) {
            auto rule = terminal(grammar, type);
            rule->std = std;
            return rule;
        }

        rule_t* terminal(grammar_t& grammar, token_type_t type, u32 bp) {
            auto rule = hashtab::find(grammar.rules, type);
            if (!rule) {
                rule = hashtab::emplace(grammar.rules, type);
                rule->std           = {};
                rule->nud           = {};
                rule->led           = {};
                rule->lbp           = {};
                rule->postfix       = false;
                rule->suppress_led  = false;
                rule->associativity = associativity::none;
            }
            if (bp > rule->lbp)
                rule->lbp = bp;
            return rule;
        }

        u0 init(grammar_t& grammar, u32* operator_types, alloc_t* alloc) {
            grammar.alloc          = alloc;
            grammar.default_rbp    = 100;
            grammar.operator_types = operator_types;
            hashtab::init(grammar.rules, alloc);
        }

        rule_t* prefix(grammar_t& grammar, token_type_t type, nud_t nud) {
            auto rule = terminal(grammar, type);
            if (nud) {
                rule->nud = [](pratt_ctx_t* ctx) {
                    auto op_type = ctx->grammar->operator_types[ctx->token->type];
                    auto expr_id = parser::expression(*ctx, ctx->grammar->default_rbp);
                    return ast::make_unary_op(*ctx->ast, op_type, expr_id, ctx->token->id);
                };
            } else {
                rule->nud = nud;
            }
            rule->associativity = associativity::none;
            return rule;
        }

        rule_t* infix(grammar_t& grammar, token_type_t type, u32 bp, led_t led) {
            auto rule = terminal(grammar, type, bp);
            if (led) {
                rule->led = [](pratt_ctx_t* ctx, ast::node_id_t left) {
                    auto op_type = ctx->grammar->operator_types[ctx->token->type];
                    auto expr_id = parser::expression(*ctx, ctx->rule->lbp);
                    return ast::make_binary_op(*ctx->ast, op_type, left, expr_id, ctx->token->id);
                };
            } else {
                rule->led = led;
            }
            rule->associativity = associativity::left;
            return rule;
        }

        rule_t* infixr(grammar_t& grammar, token_type_t type, u32 bp, led_t led) {
            auto rule = terminal(grammar, type, bp);
            if (led) {
                rule->led = [](pratt_ctx_t* ctx, ast::node_id_t left) {
                    auto op_type = ctx->grammar->operator_types[ctx->token->type];
                    auto expr_id = parser::expression(*ctx, ctx->rule->lbp - 1);
                    return ast::make_binary_op(*ctx->ast, op_type, left, expr_id, ctx->token->id);
                };
            } else {
                rule->led = led;
            }
            rule->associativity = associativity::right;
            return rule;
        }

        rule_t* postfix(grammar_t& grammar, token_type_t type, u32 bp, led_t led) {
            auto rule = terminal(grammar, type, bp);
            if (led) {
                rule->led = [](pratt_ctx_t* ctx, ast::node_id_t left) {
                    auto op_type = ctx->grammar->operator_types[ctx->token->type];
                    return ast::make_unary_op(*ctx->ast, op_type, left, ctx->token->id);
                };
            } else {
                rule->led = led;
            }
            rule->postfix = true;
            rule->associativity = associativity::left;
            return rule;
        }
    }
}
