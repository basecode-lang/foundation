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

#include <basecode/core/scm/scm.h>
#include <basecode/core/scm/types.h>
#include <basecode/core/scm/compiler.h>

namespace basecode::scm {
    namespace op = instruction::type;
    namespace rf = register_file;

    static bb_t& compile_cmp_op(const context_t& c, prim_type_t type) {
        auto ctx = c.ctx;
        auto kind = &c.kind.prim;
        auto reg  = vm::reg_alloc::reserve(ctx->emitter.gp, 2);
        scm::compile(make_context(*c.bb, ctx,  CAR(kind->args), c.env, reg[0]));
        scm::compile(make_context(*c.bb, ctx, CADR(kind->args), c.env, reg[1]));

        switch (type) {
            case prim_type_t::is:
                vm::bytecode::is(*c.bb, reg[0], reg[1], c.target);
                break;

            case prim_type_t::lt:
                vm::bytecode::lt(*c.bb, reg[0], reg[1], c.target);
                break;

            case prim_type_t::gt:
                vm::bytecode::gt(*c.bb, reg[0], reg[1], c.target);
                break;

            case prim_type_t::lte:
                vm::bytecode::lte(*c.bb, reg[0], reg[1], c.target);
                break;

            case prim_type_t::gte:
                vm::bytecode::gte(*c.bb, reg[0], reg[1], c.target);
                break;

            default:
                error(ctx, "unknown compare prim");
        }

        return *c.bb;
    }

    static bb_t& compile_arith_op(const context_t& c, op_code_t op_code) {
        auto ctx = c.ctx;
        auto kind = &c.kind.prim;

        auto args = kind->args;
        auto reg = vm::reg_alloc::reserve(ctx->emitter.gp, 2);
        vm::reg_alloc::protect(ctx->emitter.gp, reg[0], true);
        u32 size = length(ctx, args);
        vm::bytecode::alloc_stack(*c.bb, size, reg[0]);
        s32 offs = 0;
        while (!IS_NIL(args)) {
            scm::compile(make_context(*c.bb, ctx, CAR(args), c.env, reg[1]));
            vm::basic_block::offs(*c.bb, op::store, offs, reg[1], reg[0], true);
            args = CDR(args);
            offs += 8;
        }
        return vm::bytecode::arith_op(*c.bb, op_code, reg[0], c.target, size);
    }

    bb_t& compile(const context_t& c) {
        auto ctx = c.ctx;

        auto type = TYPE(c.obj);
        switch (type) {
            case obj_type_t::nil:
            case obj_type_t::fixnum:
            case obj_type_t::flonum:
            case obj_type_t::string:
            case obj_type_t::boolean:
                vm::bytecode::const_(*c.bb, OBJ_IDX(c.obj), c.target);
                vm::basic_block::comment(*c.bb,
                                         format::format("literal: {}",
                                                        to_string(c.ctx, c.obj)));
                break;

            case obj_type_t::symbol:
                vm::bytecode::get(*c.bb, OBJ_IDX(c.obj), c.target);
                vm::basic_block::comment(*c.bb,
                                         format::format("symbol: {}",
                                                        to_string(c.ctx, c.obj)));
                break;

            case obj_type_t::pair: {
                obj_t* sym  = CAR(c.obj);
                obj_t* args = CDR(c.obj);
                obj_t* form = TYPE(sym) == obj_type_t::symbol ? get(ctx, sym, c.env) : sym;

                bb_t* new_bb{};
                auto cl = cons(ctx, c.obj, ctx->call_list);
                ctx->call_list = cl;

                switch (TYPE(form)) {
                    case obj_type_t::ffi: {
                        vm::basic_block::none(*c.bb, op::nop);
                        vm::basic_block::comment(*c.bb, "XXX: ffi call"_ss);
                        break;
                    }

                    case obj_type_t::func: {
                        for (reg_t r = rf::r0; r < rf::r15; ++r) {
                            if (vm::reg_alloc::is_protected(c.bb->emitter->gp, r))
                                vm::basic_block::reg1(*c.bb, op::push, r);
                        }
                        vm::basic_block::none(*c.bb, op::nop);
                        vm::basic_block::comment(*c.bb,
                                                 format::format("call: {}",
                                                                printable_t{c.ctx, sym, true}));
                        for (reg_t r = rf::r15; r >= rf::r0; --r) {
                            if (vm::reg_alloc::is_protected(c.bb->emitter->gp, r))
                                vm::basic_block::reg1(*c.bb, op::pop, r);
                        }
                        break;
                    }

                    case obj_type_t::prim: {
                        auto pc = make_context(*c.bb, c.ctx, c.obj, c.env, c.target, c.top_level);
                        pc.label          = c.label;
                        pc.kind.prim.form = form;
                        pc.kind.prim.args = args;
                        new_bb = &prim::compile(pc);
                        break;
                    }

                    case obj_type_t::macro: {
                        vm::basic_block::none(*c.bb, op::nop);
                        vm::basic_block::comment(*c.bb, "XXX: macro call"_ss);
                        break;
                    }

                    case obj_type_t::cfunc: {
                        vm::basic_block::none(*c.bb, op::nop);
                        vm::basic_block::comment(*c.bb, "XXX: cfunc call"_ss);
                        break;
                    }

                    default:
                        error(ctx, "tried to call non-callable value");
                }

                ctx->call_list = CDR(cl);
                return *new_bb;
            }

            default:
                break;
        }

        return *c.bb;
    }

    context_t make_context(bb_t& bb,
                           ctx_t* ctx,
                           obj_t* obj,
                           obj_t* env,
                           reg_t target,
                           b8 top_level) {
        context_t c{};
        c.bb        = &bb;
        c.ctx       = ctx;
        c.obj       = obj;
        c.env       = env;
        c.target    = target;
        c.top_level = top_level;
        return c;
    }

    bb_t& proc::compile(const context_t& c, u32& bb_id) {
        auto ctx = c.ctx;
        auto& proc_bb = vm::basic_block::make_succ(*c.bb);
        bb_id = proc_bb.id;
        str_t note_str;
        if (c.label) {
            vm::basic_block::apply_label(proc_bb, c.label);
            note_str = format::format(
                "proc: ({} (fn {} {}))",
                c.ctx->emitter.strings[c.label - 1],
                printable_t{c.ctx, c.kind.proc.params, true},
                printable_t{c.ctx, c.kind.proc.body, true});
        } else {
            note_str = format::format(
                "proc: (_ (fn {} {}))",
                printable_t{c.ctx, c.kind.proc.params, true},
                printable_t{c.ctx, c.kind.proc.body, true});
        }
        vm::basic_block::note(proc_bb, note_str);
        auto reg  = vm::reg_alloc::reserve(c.ctx->emitter.gp);
        auto body = c.kind.proc.body;
        auto& body_bb = vm::bytecode::enter(proc_bb, 0);
        auto body_ctx = make_context(body_bb,
                                     c.ctx,
                                     ctx->nil,
                                     c.kind.proc.new_env,
                                     reg[0]);
        while (!IS_NIL(body)) {
            body_ctx.obj = CAR(body);
            auto& next_bb = scm::compile(body_ctx);
            body_ctx.bb = &next_bb;
            body = CDR(body);
        }
        return vm::bytecode::leave(*body_ctx.bb);
    }

    bb_t& prim::compile(const context_t& c) {
        auto ctx = c.ctx;
        auto& vm = ctx->vm;
        auto kind = &c.kind.prim;
        auto type = PRIM(c.kind.prim.form);

        switch (type) {
            case prim_type_t::is:
            case prim_type_t::gt:
            case prim_type_t::lt:
            case prim_type_t::lte:
            case prim_type_t::gte:  return compile_cmp_op(c, type);
            case prim_type_t::add:  return compile_arith_op(c, op::ladd);
            case prim_type_t::sub:  return compile_arith_op(c, op::lsub);
            case prim_type_t::mul:  return compile_arith_op(c, op::lmul);
            case prim_type_t::div:  return compile_arith_op(c, op::ldiv);
            case prim_type_t::mod:  return compile_arith_op(c, op::lmod);

            case prim_type_t::let:
            case prim_type_t::set: {
                auto reg      = vm::reg_alloc::reserve(ctx->emitter.gp);
                auto key      = CAR(kind->args);
                u32  idx      = OBJ_IDX(key);
                auto vc = make_context(*c.bb, ctx, CADR(kind->args), c.env, c.target);
                vc.label = vm::emitter::make_label(
                    ctx->emitter,
                    *string::interned::get_slice(STRING_ID(OBJ_AT(idx))));
                auto& value_bb = scm::compile(vc);
                if (c.top_level) {
                    auto value = OBJ_AT(G(c.target));
                    set(ctx, (obj_t*) OBJ_AT(idx), value, c.env);
                    if (TYPE(value) == obj_type_t::func
                    ||  TYPE(value) == obj_type_t::macro) {
                        auto proc = (proc_t*) NATIVE_PTR(CDR(value));
                        auto pc = make_context(*c.bb, c.ctx, c.obj, c.env);
                        pc.label             = vc.label;
                        pc.kind.proc.body    = proc->body;
                        pc.kind.proc.params  = proc->params;
                        pc.kind.proc.new_env = proc->env_obj;
                        u32 bb_id;
                        auto& next_bb = proc::compile(pc, bb_id);
                        proc->addr = bb_id;
                        return next_bb;
                    }
                } else {
                    vm::bytecode::const_(*c.bb, idx, reg[0]);
                    vm::bytecode::set(*c.bb, reg[0], c.target);
                    vm::basic_block::comment(*c.bb,
                                             format::format("symbol: {}",
                                                            printable_t{c.ctx, key, true}));
                }
                return value_bb;
            }

            case prim_type_t::fn:
            case prim_type_t::mac: {
                b8   is_mac = PRIM(kind->form) == prim_type_t::mac;
                auto proc   = make_proc(ctx,
                                        CAR(kind->args),
                                        CDR(kind->args),
                                        c.env,
                                        is_mac);
                auto idx    = OBJ_IDX(proc);
                G(c.target) = idx;
                vm::bytecode::const_(*c.bb, idx, c.target);
                break;
            }

            case prim_type_t::if_: {
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp);

                auto& exit_bb = vm::emitter::make_basic_block(ctx->emitter);
                vm::basic_block::none(exit_bb, op::nop);

                auto& false_bb = vm::emitter::make_basic_block(ctx->emitter);

                scm::compile(make_context(*c.bb, ctx, CAR(kind->args), c.env, reg[0]));
                vm::basic_block::reg1(*c.bb, op::truep, reg[0]);
                vm::basic_block::imm1(*c.bb, op::bne, vm::emitter::imm(&false_bb));

                auto& true_bb = vm::emitter::make_basic_block(ctx->emitter);
                vm::basic_block::succ(*c.bb, true_bb);
                auto& true2_bb = scm::compile(make_context(true_bb, ctx, CADR(kind->args), c.env, c.target));
                vm::basic_block::imm1(true2_bb, op::bra, vm::emitter::imm(&exit_bb));
                vm::basic_block::succ(true2_bb, false_bb);

                auto& false2_bb = scm::compile(make_context(false_bb, ctx, CADDR(kind->args), c.env, c.target));
                vm::basic_block::succ(false2_bb, exit_bb);

                return exit_bb;
            }

            case prim_type_t::do_: {
                auto args = kind->args;
                if (IS_NIL(args))
                    break;
                auto dc = make_context(*c.bb, ctx, ctx->nil, c.env, c.target);
                while (!IS_NIL(args)) {
                    dc.obj = CAR(args);
                    auto& next_bb = scm::compile(dc);
                    dc.bb = &next_bb;
                    args = CDR(args);
                }
                return *dc.bb;
            }

            case prim_type_t::or_:
                break;

            case prim_type_t::and_:
                break;

            case prim_type_t::not_: {
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp);
                scm::compile(make_context(*c.bb, ctx, CAR(kind->args), c.env, reg[0]));
                vm::bytecode::not_(*c.bb, reg[0], c.target);
                break;
            }

            case prim_type_t::car: {
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp);
                scm::compile(make_context(*c.bb, ctx, CAR(kind->args), c.env, reg[0]));
                vm::bytecode::car(*c.bb, reg[0], c.target);
                break;
            }

            case prim_type_t::cdr: {
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp);
                scm::compile(make_context(*c.bb, ctx, CAR(kind->args), c.env, reg[0]));
                vm::bytecode::cdr(*c.bb, reg[0], c.target);
                break;
            }

            case prim_type_t::cons: {
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp, 2);
                scm::compile(make_context(*c.bb, ctx, CAR(kind->args), c.env, reg[0]));
                scm::compile(make_context(*c.bb, ctx, CADR(kind->args),c.env, reg[1]));
                vm::bytecode::cons(*c.bb, reg[0], reg[1], c.target);
                break;
            }

            case prim_type_t::atom: {
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp, 2);
                scm::compile(make_context(*c.bb, ctx, CAR(kind->args), c.env, reg[0]));
                vm::basic_block::reg2(*c.bb, op::atomp, reg[0], reg[1]);
                break;
            }

            case prim_type_t::eval: {
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp);
                scm::compile(make_context(*c.bb, ctx, CAR(kind->args), c.env, reg[0]));
                vm::basic_block::reg2(*c.bb, op::eval, reg[0], c.target);
                break;
            }

            case prim_type_t::list: {
                auto args = kind->args;
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp, 2);
                u32 size = length(ctx, args);
                vm::bytecode::alloc_stack(*c.bb, size, reg[0]);
                s32 offs = 0;
                while (!IS_NIL(args)) {
                    scm::compile(make_context(*c.bb, ctx, CAR(args), c.env, reg[1]));
                    vm::basic_block::offs(*c.bb, op::store, offs, reg[1], reg[0], true);
                    args = CDR(args);
                    offs += 8;
                }
                return vm::bytecode::list(*c.bb, c.target, reg[0], c.target, size);
            }

            case prim_type_t::while_: {
                break;
            }

            case prim_type_t::setcar: {
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp);
                scm::compile(make_context(*c.bb, ctx, CAR(kind->args), c.env, reg[0]));
                scm::compile(make_context(*c.bb, ctx, CADR(kind->args), c.env, c.target));
                vm::bytecode::setcar(*c.bb, reg[0], c.target);
                break;
            }

            case prim_type_t::setcdr: {
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp);
                scm::compile(make_context(*c.bb, ctx,  CAR(kind->args), c.env,reg[0]));
                scm::compile(make_context(*c.bb, ctx, CADR(kind->args), c.env, c.target));
                vm::bytecode::setcdr(*c.bb, reg[0], c.target);
                break;
            }

            case prim_type_t::error:
                vm::bytecode::error(*c.bb, OBJ_IDX(kind->args), c.target);
                break;

            case prim_type_t::print:
                break;

            case prim_type_t::quote:
                vm::bytecode::qt(*c.bb, OBJ_IDX(CAR(kind->args)), c.target);
                break;

            case prim_type_t::unquote:
                error(ctx, "unquote is not valid in this context.");
                break;

            case prim_type_t::quasiquote:
                vm::bytecode::qq(*c.bb, OBJ_IDX(CAR(kind->args)), c.target);
                break;

            case prim_type_t::unquote_splicing:
                error(ctx, "unquote-splicing is not valid in this context.");
                break;

            default:
                break;
        }

        return *c.bb;
    }
}
