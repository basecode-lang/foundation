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
        auto ctx  = c.ctx;
        auto kind = &c.kind.prim;
        auto reg  = vm::reg_alloc::reserve(ctx->emitter.gp, 2);
        auto oc   = c;
        oc.obj    = CAR(kind->args);
        oc.target = reg[0];
        scm::compile(oc);
        oc.obj    = CADR(kind->args);
        oc.target = reg[1];
        scm::compile(oc);

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
        s32  offs = 0;
        auto ac   = c;
        ac.target = reg[1];
        while (!IS_NIL(args)) {
            ac.obj = CAR(args);
            scm::compile(ac);
            vm::basic_block::offs(*c.bb,
                                  op::store,
                                  offs,
                                  reg[1],
                                  reg[0],
                                  true);
            args = CDR(args);
            offs += 8;
        }
        return vm::bytecode::arith_op(*c.bb,
                                      op_code,
                                      reg[0],
                                      c.target,
                                      size);
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
                vm::bytecode::get(*c.bb, (u32) OBJ_IDX(c.obj), c.target);
                vm::basic_block::comment(*c.bb,
                                         format::format("symbol: {}",
                                                        to_string(c.ctx, c.obj)));
                break;

            case obj_type_t::pair:
                return proc::apply(c);

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
        c.type      = context_kind_t::none;
        c.target    = target;
        c.top_level = top_level;
        return c;
    }

    bb_t& proc::apply(const context_t& c) {
        auto ctx = c.ctx;

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
                auto proc = (proc_t*) NATIVE_PTR(CDR(form));
                vm::bytecode::save_protected(*c.bb);
                vm::basic_block::reg2(*c.bb, op::env, rf::ep, c.target);
                vm::basic_block::reg2(*c.bb, op::push, c.target, rf::ep);
                {
                    auto reg = vm::reg_alloc::reserve(c.ctx->emitter.gp, 2);
                    auto keys = proc->params;
                    auto vals = args;
                    auto vc   = c;
                    vc.target = reg[1];
                    while (!IS_NIL(keys)) {
                        if (TYPE(keys) != obj_type_t::pair) {
                            vm::bytecode::const_(*c.bb, OBJ_IDX(keys), reg[0]);
                            vc.obj = vals;
                            scm::compile(vc);
                            vm::bytecode::set(*c.bb, reg[0], reg[1]);
                            break;
                        } else {
                            vm::bytecode::const_(*c.bb, OBJ_IDX(CAR(keys)), reg[0]);
                            vc.obj = CAR(vals);
                            scm::compile(vc);
                            vm::bytecode::set(*c.bb, reg[0], reg[1]);
                            keys = CDR(keys);
                            vals = CDR(vals);
                        }
                    }
                }
                vm::basic_block::imm1(*c.bb,
                                      op::blr,
                                      vm::emitter::imm(c.entry_point));
                vm::basic_block::comment(*c.bb,
                                         format::format("call: {}",
                                                        printable_t{c.ctx, sym, true}));
                vm::basic_block::reg2(*c.bb, op::pop, rf::ep, c.target);
                vm::basic_block::comment(*c.bb, "drop apply env"_ss);
                vm::bytecode::restore_protected(*c.bb);
                break;
            }

            case obj_type_t::prim: {
                auto pc = c;
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

    bb_t& proc::compile(const context_t& c) {
        auto ctx = c.ctx;
        auto& proc_bb = vm::basic_block::make_succ(*c.bb);
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
        auto bc   = c;
        bc.bb          = &vm::bytecode::enter(proc_bb, 0);
        bc.type        = context_kind_t::none;
        bc.kind        = {};
        bc.target      = reg[0];
        bc.entry_point = &proc_bb;
        while (!IS_NIL(body)) {
            bc.obj = CAR(body);
            bc.bb  = &scm::compile(bc);
            body = CDR(body);
        }
        return vm::bytecode::leave(*bc.bb);
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
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp);
                auto key = CAR(kind->args);
                u32  idx = OBJ_IDX(key);
                auto vc  = c;
                vc.obj       = CADR(kind->args);
                vc.label     = vm::emitter::make_label(
                    ctx->emitter,
                    *string::interned::get_slice(STRING_ID(OBJ_AT(idx))));
                auto& value_bb = scm::compile(vc);
                if (c.top_level) {
                    auto value = OBJ_AT(G(c.target));
                    set(ctx, (obj_t*) OBJ_AT(idx), value, c.env);
                    if (TYPE(value) == obj_type_t::func
                    ||  TYPE(value) == obj_type_t::macro) {
                        auto proc = (proc_t*) NATIVE_PTR(CDR(value));
                        auto pc   = c;
                        pc.type             = context_kind_t::proc;
                        pc.label            = vc.label;
                        pc.top_level        = false;
                        pc.kind.proc.body   = proc->body;
                        pc.kind.proc.params = proc->params;
                        return proc::compile(pc);
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
                if (!c.top_level)
                    vm::bytecode::const_(*c.bb, idx, c.target);
                break;
            }

            case prim_type_t::if_: {
                auto ic  = c;
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp);

                auto& exit_bb = vm::emitter::make_basic_block(ctx->emitter);
                vm::basic_block::none(exit_bb, op::nop);

                auto& false_bb = vm::emitter::make_basic_block(ctx->emitter);

                ic.obj    = CAR(kind->args);
                ic.target = reg[0];
                scm::compile(ic);
                vm::basic_block::reg1(*c.bb, op::truep, reg[0]);
                vm::basic_block::imm1(*c.bb, op::bne, vm::emitter::imm(&false_bb));

                auto& true_bb = vm::emitter::make_basic_block(ctx->emitter);
                vm::basic_block::succ(*c.bb, true_bb);
                ic.bb     = &true_bb;
                ic.obj    = CADR(kind->args);
                ic.target = c.target;
                auto& true2_bb = scm::compile(ic);
                vm::basic_block::imm1(true2_bb, op::bra, vm::emitter::imm(&exit_bb));
                vm::basic_block::succ(true2_bb, false_bb);

                ic.bb     = &false_bb;
                ic.obj    = CADDR(kind->args);
                ic.target = c.target;
                auto& false2_bb = scm::compile(ic);
                vm::basic_block::succ(false2_bb, exit_bb);

                return exit_bb;
            }

            case prim_type_t::do_: {
                auto args = kind->args;
                if (IS_NIL(args))
                    break;
                auto dc = c;
                while (!IS_NIL(args)) {
                    dc.obj = CAR(args);
                    dc.bb  = &scm::compile(dc);
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
                auto nc  = c;
                nc.obj = CAR(kind->args);
                scm::compile(nc);
                vm::bytecode::lnot(*c.bb, c.target);
                break;
            }

            case prim_type_t::car: {
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp);
                auto cc  = c;
                cc.obj    = CAR(kind->args);
                cc.target = reg[0];
                scm::compile(cc);
                vm::bytecode::car(*c.bb, reg[0], c.target);
                break;
            }

            case prim_type_t::cdr: {
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp);
                auto cc  = c;
                cc.obj    = CAR(kind->args);
                cc.target = reg[0];
                scm::compile(cc);
                vm::bytecode::cdr(*c.bb, reg[0], c.target);
                break;
            }

            case prim_type_t::cons: {
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp, 2);
                auto cc  = c;
                cc.obj    = CAR(kind->args);
                cc.target = reg[0];
                scm::compile(cc);
                cc.obj    = CADR(kind->args);
                cc.target = reg[1];
                scm::compile(cc);
                vm::bytecode::cons(*c.bb, reg[0], reg[1], c.target);
                break;
            }

            case prim_type_t::atom: {
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp);
                auto ac  = c;
                ac.obj    = CAR(kind->args);
                ac.target = reg[0];
                scm::compile(ac);
                vm::basic_block::reg2(*c.bb, op::atomp, reg[0], c.target);
                break;
            }

            case prim_type_t::eval: {
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp);
                auto ec  = c;
                ec.obj    = CAR(kind->args);
                ec.target = reg[0];
                scm::compile(ec);
                vm::basic_block::reg2(*c.bb, op::eval, reg[0], c.target);
                break;
            }

            case prim_type_t::list: {
                auto args = kind->args;
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp, 2);
                u32 size = length(ctx, args);
                vm::bytecode::alloc_stack(*c.bb, size, reg[0]);
                s32  offs = 0;
                auto lc   = c;
                lc.target = reg[1];
                while (!IS_NIL(args)) {
                    lc.obj = CAR(args);
                    lc.bb  = &scm::compile(lc);
                    vm::basic_block::offs(*c.bb,
                                          op::store,
                                          offs,
                                          reg[1],
                                          reg[0],
                                          true);
                    args = CDR(args);
                    offs += 8;
                }
                return vm::bytecode::list(*lc.bb,
                                          c.target,
                                          reg[0],
                                          c.target,
                                          size);
            }

            case prim_type_t::while_: {
                break;
            }

            case prim_type_t::setcar: {
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp);
                auto sc  = c;
                sc.obj    = CADR(kind->args);
                sc.target = reg[0];
                scm::compile(sc);
                sc.obj    = CAR(kind->args);
                sc.target = c.target;
                scm::compile(sc);
                vm::bytecode::setcar(*c.bb, reg[0], c.target);
                break;
            }

            case prim_type_t::setcdr: {
                auto reg = vm::reg_alloc::reserve(ctx->emitter.gp);
                auto sc  = c;
                sc.obj    = CADR(kind->args);
                sc.target = reg[0];
                scm::compile(sc);
                sc.obj    = CAR(kind->args);
                sc.target = c.target;
                scm::compile(sc);
                vm::bytecode::setcdr(*c.bb, reg[0], c.target);
                break;
            }

            case prim_type_t::error:
                vm::bytecode::error(*c.bb, OBJ_IDX(kind->args), c.target);
                vm::basic_block::comment(*c.bb,
                                         format::format("literal: {}",
                                                        to_string(c.ctx, kind->args)),
                                         c.bb->entries.size - 1);
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
