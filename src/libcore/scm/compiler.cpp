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
#include <basecode/core/scm/compiler.h>

namespace basecode::scm::compiler {
    namespace rf = register_file;
    namespace op = instruction::type;

    namespace proc {
        compile_result_t apply(compiler_t& comp, const context_t& c) {
            auto ctx = c.ctx;

            obj_t* sym  = CAR(c.obj);
            obj_t* args = CDR(c.obj);
            obj_t* form = TYPE(sym) == obj_type_t::symbol ? get(ctx, sym, c.env) : sym;

            auto cl = cons(ctx, c.obj, ctx->call_list);
            ctx->call_list = cl;
            defer(ctx->call_list = CDR(cl));

            switch (TYPE(form)) {
                case obj_type_t::ffi: {
                    vm::basic_block::none(*c.bb, op::nop);
                    vm::basic_block::comment(*c.bb, "XXX: ffi call"_ss);
                    return {c.bb, 0};
                }

                case obj_type_t::func: {
                    auto proc = (proc_t*) NATIVE_PTR(CDR(form));
                    vm::bytecode::save_protected(*c.bb, comp);
                    auto target_reg = vm::reg_alloc::reserve_one(comp.emitter.gp);
                    vm::basic_block::reg2(*c.bb, op::env, rf::ep, target_reg);
                    vm::basic_block::reg2(*c.bb, op::push, target_reg, rf::ep);
                    {
                        auto keys = proc->params;
                        auto vals = args;
                        auto vc   = c;
                        vc.target = {};
                        while (!IS_NIL(keys)) {
                            if (TYPE(keys) != obj_type_t::pair) {
                                vc.obj = vals;
                                auto res = compiler::compile(comp, vc);
                                vc.bb  = res.bb;
                                vm::bytecode::set(*vc.bb,
                                                  (u32) OBJ_IDX(keys),
                                                  res.reg);
                                release_result(comp, res);
                                break;
                            } else {
                                vc.obj = CAR(vals);
                                auto res = compiler::compile(comp, vc);
                                vc.bb  = res.bb;
                                vm::bytecode::set(*vc.bb,
                                                  (u32) OBJ_IDX(CAR(keys)),
                                                  res.reg);
                                keys = CDR(keys);
                                vals = CDR(vals);
                                release_result(comp, res);
                            }
                        }
                    }
                    assert(proc->addr.bb);
                    vm::basic_block::imm1(
                        *c.bb,
                        op::blr,
                        proc->is_assembled ?
                        vm::emitter::imm(proc->addr.absolute) :
                        vm::emitter::imm(proc->addr.bb));
                    vm::basic_block::comment(*c.bb,
                                             format::format("call: {}",
                                                            printable_t{c.ctx, sym, true}));
                    vm::basic_block::reg2(*c.bb, op::pop, rf::ep, target_reg);
                    vm::basic_block::comment(*c.bb, "drop apply env"_ss);
                    vm::bytecode::restore_protected(*c.bb, comp);
                    return {c.bb, target_reg, true};
                }

                case obj_type_t::prim: {
                    auto pc = c;
                    pc.kind.prim.form = form;
                    pc.kind.prim.args = args;
                    return prim::compile(comp, pc);
                }

                case obj_type_t::macro: {
                    vm::basic_block::none(*c.bb, op::nop);
                    vm::basic_block::comment(*c.bb, "XXX: macro call"_ss);
                    return {c.bb, 0};
                }

                case obj_type_t::cfunc: {
                    vm::basic_block::none(*c.bb, op::nop);
                    vm::basic_block::comment(*c.bb, "XXX: cfunc call"_ss);
                    return {c.bb, 0};
                }

                default:
                    error(ctx, "tried to call non-callable value");
            }

            return {c.bb, 0};
        }

        compile_result_t compile(compiler_t& comp, const context_t& c) {
            auto ctx  = c.ctx;
            auto kind = c.kind.proc;
            auto& proc_bb = vm::basic_block::make_succ(*c.bb);
            kind->addr.bb = &proc_bb;
            str_t note_str;
            if (c.label) {
                vm::basic_block::apply_label(proc_bb, c.label);
                note_str = format::format(
                    "proc: ({} (fn {} {}))",
                    vm::emitter::get_string(comp.emitter, c.label),
                    printable_t{c.ctx, kind->params, true},
                    printable_t{c.ctx, kind->body, true});
            } else {
                note_str = format::format(
                    "proc: (_ (fn {} {}))",
                    printable_t{c.ctx, kind->params, true},
                    printable_t{c.ctx, kind->body, true});
            }
            vm::basic_block::note(proc_bb, note_str);
            auto body = kind->body;
            auto bc   = c;
            bc.bb     = &vm::bytecode::enter(proc_bb, 0);
            bc.type   = context_kind_t::none;
            bc.kind   = {};
            while (!IS_NIL(body)) {
                bc.obj = CAR(body);
                auto res = compiler::compile(comp, bc);
                bc.bb = res.bb;
                body = CDR(body);
                release_result(comp, res);
            }
            kind->is_compiled = true;
            return {&vm::bytecode::leave(*bc.bb), 0};
        }
    }

    namespace prim {
        static compile_result_t compile_cmp_op(compiler_t& comp,
                                               const context_t& c,
                                               prim_type_t type) {
            auto ctx  = c.ctx;
            auto kind = &c.kind.prim;
            auto oc = c;
            oc.obj = CAR(kind->args);
            auto lhs_res = compiler::compile(comp, oc);
            oc.bb  = lhs_res.bb;
            oc.obj = CADR(kind->args);
            auto rhs_res    = compiler::compile(comp, oc);
            auto target_reg = compiler::reserve_reg(comp, c);

            switch (type) {
                case prim_type_t::is:
                    vm::bytecode::is(*rhs_res.bb,
                                     lhs_res.reg,
                                     rhs_res.reg,
                                     target_reg);
                    break;

                case prim_type_t::lt:
                    vm::bytecode::lt(*rhs_res.bb,
                                     lhs_res.reg,
                                     rhs_res.reg,
                                     target_reg);
                    break;

                case prim_type_t::gt:
                    vm::bytecode::gt(*rhs_res.bb,
                                     lhs_res.reg,
                                     rhs_res.reg,
                                     target_reg);
                    break;

                case prim_type_t::lte:
                    vm::bytecode::lte(*rhs_res.bb,
                                      lhs_res.reg,
                                      rhs_res.reg,
                                      target_reg);
                    break;

                case prim_type_t::gte:
                    vm::bytecode::gte(*rhs_res.bb,
                                      lhs_res.reg,
                                      rhs_res.reg,
                                      target_reg);
                    break;

                default:
                    error(ctx, "unknown compare prim");
            }

            release_result(comp, lhs_res);
            release_result(comp, rhs_res);

            return {c.bb, target_reg, true};
        }

        static compile_result_t compile_arith_op(compiler_t& comp,
                                                 const context_t& c,
                                                 op_code_t op_code) {
            auto ctx = c.ctx;
            auto kind = &c.kind.prim;

            auto args = kind->args;
            auto reg = vm::reg_alloc::reserve(comp.emitter.gp);
            vm::reg_alloc::protect(comp.emitter.gp, reg[0], true);
            u32 size = length(ctx, args);
            vm::bytecode::alloc_stack(*c.bb, size, reg[0]);
            s32  offs = 0;
            auto ac   = c;
            while (!IS_NIL(args)) {
                ac.obj = CAR(args);
                auto res = compiler::compile(comp, ac);
                ac.bb = res.bb;
                vm::basic_block::offs(*res.bb,
                                      op::store,
                                      offs,
                                      res.reg,
                                      reg[0],
                                      true);
                args = CDR(args);
                offs += 8;
                release_result(comp, res);
            }
            auto target_reg = compiler::reserve_reg(comp, ac);
            auto arith_bb   = &vm::bytecode::arith_op(*ac.bb,
                                                      op_code,
                                                      reg[0],
                                                      target_reg,
                                                      size);
            return {arith_bb, target_reg, true};
        }

        compile_result_t compile(compiler_t& comp, const context_t& c) {
            auto ctx = c.ctx;
            auto& vm = ctx->vm;
            auto kind = &c.kind.prim;
            auto type = PRIM(c.kind.prim.form);

            switch (type) {
                case prim_type_t::is:
                case prim_type_t::gt:
                case prim_type_t::lt:
                case prim_type_t::lte:
                case prim_type_t::gte:  return compile_cmp_op(comp, c, type);
                case prim_type_t::add:  return compile_arith_op(comp, c, op::ladd);
                case prim_type_t::sub:  return compile_arith_op(comp, c, op::lsub);
                case prim_type_t::mul:  return compile_arith_op(comp, c, op::lmul);
                case prim_type_t::div:  return compile_arith_op(comp, c, op::ldiv);
                case prim_type_t::mod:  return compile_arith_op(comp, c, op::lmod);

                case prim_type_t::let:
                case prim_type_t::set: {
                    auto key = CAR(kind->args);
                    u32  idx = OBJ_IDX(key);
                    auto vc  = c;
                    vc.obj    = CADR(kind->args);
                    vc.target = &comp.ret_reg;
                    vc.label  = vm::emitter::make_label(
                        comp.emitter,
                        *string::interned::get_slice(STRING_ID(OBJ_AT(idx))));
                    auto res = compiler::compile(comp, vc);
                    if (c.top_level) {
                        auto value = OBJ_AT(G(res.reg));
                        set(ctx, (obj_t*) OBJ_AT(idx), value, c.env);
                        release_result(comp, res);
                        if (TYPE(value) == obj_type_t::func
                            ||  TYPE(value) == obj_type_t::macro) {
                            auto proc = (proc_t*) NATIVE_PTR(CDR(value));
                            if (!proc->is_compiled) {
                                auto pc = c;
                                pc.type      = context_kind_t::proc;
                                pc.label     = vc.label;
                                pc.top_level = false;
                                pc.kind.proc = proc;
                                return proc::compile(comp, pc);
                            }
                        }
                    } else {
                        vm::bytecode::set(*res.bb, idx, res.reg);
                        vm::basic_block::comment(*res.bb,
                                                 format::format("symbol: {}",
                                                                printable_t{c.ctx, key, true}));
                        release_result(comp, res);
                    }
                    return res;
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
                    auto target_reg = compiler::reserve_reg(comp, c);
                    G(target_reg)   = idx;
                    if (!c.top_level)
                        vm::bytecode::const_(*c.bb, idx, target_reg);
                    return {c.bb, target_reg, true};
                }

                case prim_type_t::if_: {
                    auto& true_bb  = vm::basic_block::make_succ(*c.bb);
                    auto& false_bb = vm::emitter::make_basic_block(*c.bb->emitter);
                    auto& exit_bb  = vm::emitter::make_basic_block(*c.bb->emitter);

                    auto ic  = c;
                    auto reg = vm::reg_alloc::reserve(comp.emitter.gp);

                    ic.obj = CAR(kind->args);
                    auto pred_res = compiler::compile(comp, ic);
                    vm::basic_block::reg1(*c.bb, op::truep, pred_res.reg);
                    vm::basic_block::imm1(*c.bb, op::bne, vm::emitter::imm(&false_bb));
                    release_result(comp, pred_res);

                    ic.bb     = &true_bb;
                    ic.obj    = CADR(kind->args);
                    ic.target = &comp.ret_reg;
                    auto true_res = compiler::compile(comp, ic);
                    vm::basic_block::imm1(*true_res.bb, op::br, vm::emitter::imm(&exit_bb));
                    vm::basic_block::succ(*true_res.bb, false_bb);
                    release_result(comp, true_res);

                    ic.bb     = &false_bb;
                    ic.obj    = CADDR(kind->args);
                    ic.target = &comp.ret_reg;
                    auto false_res = compiler::compile(comp, ic);
                    vm::basic_block::succ(*false_res.bb, exit_bb);
                    release_result(comp, false_res);

                    return {&exit_bb, 0};
                }

                case prim_type_t::do_: {
                    auto args = kind->args;
                    if (IS_NIL(args))
                        break;
                    auto dc = c;
                    while (!IS_NIL(args)) {
                        dc.obj   = CAR(args);
                        auto res = compiler::compile(comp, dc);
                        dc.bb = res.bb;
                        args = CDR(args);
                        release_result(comp, res);
                    }
                    return {dc.bb, 0};
                }

                case prim_type_t::or_: {
                    auto& exit_bb   = vm::emitter::make_basic_block(comp.emitter);
                    auto target_reg = compiler::reserve_reg(comp, c);
                    auto args       = kind->args;
                    auto oc         = c;
                    while (!IS_NIL(args)) {
                        oc.obj = CAR(args);
                        if (IS_NIL(CDR(args)))
                            oc.target = &comp.ret_reg;
                        auto res = compiler::compile(comp, oc);
                        oc.bb = res.bb;
                        vm::basic_block::reg1(*oc.bb, op::truep, res.reg);
                        vm::basic_block::imm1(*oc.bb, op::beq, vm::emitter::imm(&exit_bb));
                        release_result(comp, res);
                        args = CDR(args);
                    }
                    vm::basic_block::succ(*oc.bb, exit_bb);
                    return {&exit_bb, target_reg};
                }

                case prim_type_t::and_: {
                    auto& exit_bb   = vm::emitter::make_basic_block(comp.emitter);
                    auto target_reg = compiler::reserve_reg(comp, c);
                    auto args       = kind->args;
                    auto oc         = c;
                    while (!IS_NIL(args)) {
                        oc.obj = CAR(args);
                        if (IS_NIL(CDR(args)))
                            oc.target = &comp.ret_reg;
                        auto res = compiler::compile(comp, oc);
                        oc.bb = res.bb;
                        vm::basic_block::reg1(*oc.bb, op::truep, res.reg);
                        vm::basic_block::imm1(*oc.bb, op::bne, vm::emitter::imm(&exit_bb));
                        release_result(comp, res);
                        args = CDR(args);
                    }
                    vm::basic_block::succ(*oc.bb, exit_bb);
                    return {&exit_bb, target_reg};
                }

                case prim_type_t::not_: {
                    auto nc = c;
                    nc.obj = CAR(kind->args);
                    auto res = compiler::compile(comp, nc);
                    vm::bytecode::lnot(*res.bb, res.reg);
                    release_result(comp, res);
                    return res;
                }

                case prim_type_t::car: {
                    auto target_reg = compiler::reserve_reg(comp, c);
                    auto cc = c;
                    cc.obj = CAR(kind->args);
                    auto res = compiler::compile(comp, cc);
                    vm::bytecode::car(*res.bb, res.reg, target_reg);
                    release_result(comp, res);
                    return {res.bb, target_reg, true};
                }

                case prim_type_t::cdr: {
                    auto target_reg = compiler::reserve_reg(comp, c);
                    auto cc = c;
                    cc.obj = CAR(kind->args);
                    auto res = compiler::compile(comp, cc);
                    vm::bytecode::cdr(*res.bb, res.reg, target_reg);
                    release_result(comp, res);
                    return {res.bb, target_reg, true};
                }

                case prim_type_t::cons: {
                    auto target_reg = compiler::reserve_reg(comp, c);
                    auto cc = c;
                    cc.obj = CAR(kind->args);
                    auto lhs_res = compiler::compile(comp, cc);
                    cc.bb  = lhs_res.bb;
                    cc.obj = CADR(kind->args);
                    auto rhs_res = compiler::compile(comp, cc);
                    vm::bytecode::cons(*rhs_res.bb, lhs_res.reg, rhs_res.reg, target_reg);
                    release_result(comp, lhs_res);
                    release_result(comp, rhs_res);
                    return {rhs_res.bb, target_reg, true};
                }

                case prim_type_t::atom: {
                    auto target_reg = compiler::reserve_reg(comp, c);
                    auto ac = c;
                    ac.obj = CAR(kind->args);
                    auto res = compiler::compile(comp, ac);
                    vm::basic_block::reg2(*res.bb, op::atomp, res.reg, target_reg);
                    release_result(comp, res);
                    return {res.bb, target_reg, true};
                }

                case prim_type_t::eval: {
                    auto target_reg = compiler::reserve_reg(comp, c);
                    auto ec = c;
                    ec.obj = CAR(kind->args);
                    auto res = compiler::compile(comp, ec);
                    vm::basic_block::reg2(*res.bb, op::eval, res.reg, target_reg);
                    release_result(comp, res);
                    return {res.bb, target_reg, true};
                }

                case prim_type_t::list: {
                    auto reg  = vm::reg_alloc::reserve(comp.emitter.gp);
                    auto args = kind->args;
                    u32  size = length(ctx, args);
                    vm::bytecode::alloc_stack(*c.bb, size, reg[0]);
                    s32  offs = 0;
                    auto lc   = c;
                    while (!IS_NIL(args)) {
                        lc.obj   = CAR(args);
                        auto res = compiler::compile(comp, lc);
                        lc.bb = res.bb;
                        vm::basic_block::offs(*res.bb,
                                              op::store,
                                              offs,
                                              res.reg,
                                              reg[0],
                                              true);
                        args = CDR(args);
                        offs += 8;
                        release_result(comp, res);
                    }
                    auto target_reg = compiler::reserve_reg(comp, lc);
                    auto list_bb = &vm::bytecode::list(*lc.bb,
                                                       reg[0],
                                                       target_reg,
                                                       size);
                    return {list_bb, target_reg, true};
                }

                case prim_type_t::while_: {
                    break;
                }

                case prim_type_t::setcar: {
                    auto sc = c;
                    sc.obj = CADR(kind->args);
                    auto rhs_res = compiler::compile(comp, sc);
                    sc.bb  = rhs_res.bb;
                    sc.obj = CAR(kind->args);
                    auto lhs_res = compiler::compile(comp, sc);
                    vm::bytecode::setcar(*lhs_res.bb, lhs_res.reg, rhs_res.reg);
                    release_result(comp, lhs_res);
                    release_result(comp, rhs_res);
                    return {lhs_res.bb, 0};
                }

                case prim_type_t::setcdr: {
                    auto sc = c;
                    sc.obj = CADR(kind->args);
                    auto rhs_res = compiler::compile(comp, sc);
                    sc.bb  = rhs_res.bb;
                    sc.obj = CAR(kind->args);
                    auto lhs_res = compiler::compile(comp, sc);
                    vm::bytecode::setcdr(*lhs_res.bb, lhs_res.reg, rhs_res.reg);
                    release_result(comp, lhs_res);
                    release_result(comp, rhs_res);
                    return {lhs_res.bb, 0};
                }

                case prim_type_t::error: {
                    auto target_reg = compiler::reserve_reg(comp, c);
                    vm::bytecode::error(*c.bb, OBJ_IDX(kind->args), target_reg);
                    vm::basic_block::comment(*c.bb,
                                             format::format("literal: {}",
                                                            to_string(c.ctx, kind->args)),
                                             c.bb->entries.size - 1);
                    break;
                }

                case prim_type_t::print:
                    break;

                case prim_type_t::quote: {
                    auto target_reg = compiler::reserve_reg(comp, c);
                    vm::bytecode::qt(*c.bb, OBJ_IDX(CAR(kind->args)), target_reg);
                    break;
                }

                case prim_type_t::unquote:
                    error(ctx, "unquote is not valid in this context.");
                    break;

                case prim_type_t::quasiquote: {
                    auto target_reg = compiler::reserve_reg(comp, c);
                    vm::bytecode::qq(*c.bb, OBJ_IDX(CAR(kind->args)), target_reg);
                    break;
                }

                case prim_type_t::unquote_splicing:
                    error(ctx, "unquote-splicing is not valid in this context.");
                    break;

                default:
                    break;
            }

            return {c.bb, 0};
        }
    }

    u0 free(compiler_t& comp) {
        vm::emitter::free(comp.emitter);
    }

    u0 reset(compiler_t& comp) {
        vm::emitter::reset(comp.emitter);
    }

    context_t make_context(bb_t& bb,
                           ctx_t* ctx,
                           obj_t* obj,
                           obj_t* env,
                           b8 top_level) {
        context_t c{};
        c.bb        = &bb;
        c.ctx       = ctx;
        c.obj       = obj;
        c.env       = env;
        c.type      = context_kind_t::none;
        c.target    = {};
        c.top_level = top_level;
        return c;
    }

    u0 release_reg(compiler_t& comp, reg_t reg) {
        vm::reg_alloc::release_one(comp.emitter.gp, reg);
    }

    reg_t reserve_reg(compiler_t& comp, const context_t& c) {
        if (c.target)
            return *c.target;
        return vm::reg_alloc::reserve_one(comp.emitter.gp);
    }

    u0 init(compiler_t& comp, vm_t* vm, u64 addr, alloc_t* alloc) {
        comp.vm = vm;
        vm::emitter::init(comp.emitter, vm, addr, alloc);
    }

    compile_result_t compile(compiler_t& comp, const context_t& c) {
        auto ctx = c.ctx;

        auto type = TYPE(c.obj);
        switch (type) {
            case obj_type_t::nil:
            case obj_type_t::fixnum:
            case obj_type_t::flonum:
            case obj_type_t::string:
            case obj_type_t::boolean: {
                auto target_reg = vm::basic_block::find_bind(*c.bb, c.obj);
                if (!target_reg) {
                    auto reg = compiler::reserve_reg(comp, c);
                    vm::bytecode::const_(*c.bb, OBJ_IDX(c.obj), reg);
                    vm::basic_block::comment(*c.bb,
                                             format::format("literal: {}",
                                                            to_string(c.ctx, c.obj)));
                    vm::basic_block::set_bind(*c.bb, reg, c.obj);
                    return {c.bb, reg, false};
                }
                return {c.bb, *target_reg, false};
            }
            case obj_type_t::symbol: {
                auto target_reg = vm::basic_block::find_bind(*c.bb, c.obj);
                if (!target_reg) {
                    auto reg = compiler::reserve_reg(comp, c);
                    vm::bytecode::get(*c.bb, (u32) OBJ_IDX(c.obj), reg);
                    vm::basic_block::comment(*c.bb,
                                             format::format("symbol: {}",
                                                            to_string(c.ctx, c.obj)));
                    vm::basic_block::set_bind(*c.bb, reg, c.obj);
                    return {c.bb, reg, false};
                }
                return {c.bb, *target_reg, false};
            }
            case obj_type_t::pair:  return proc::apply(comp, c);
            default:                break;
        }

        return {c.bb, 0};
    }
}
