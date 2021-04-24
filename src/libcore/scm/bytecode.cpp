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

#include <basecode/core/string.h>
#include <basecode/core/scm/bytecode.h>
#include <basecode/core/scm/compiler.h>

namespace basecode::scm::bytecode {
    namespace rf = basecode::scm::vm::register_file;
    namespace op = basecode::scm::vm::instruction::type;

    bb_t& leave(bb_t& bb) {
        auto& entry_block = emitter::make_basic_block(*bb.emit,
                                                      "proc_epilogue"_ss,
                                                      &bb);
        auto& exit_block = emitter::make_basic_block(*bb.emit,
                                                     "proc_exit"_ss,
                                                     &entry_block);
        basic_block::encode(bb)
            .imm1()
                .op(op::b)
                .value(&entry_block)
                .build();
        basic_block::encode(entry_block)
            .comment("*"_ss)
            .comment("**"_ss)
            .comment("*** proc epilogue"_ss)
            .reg2()
                .op(op::move)
                .src(rf::fp)
                .dst(rf::sp)
                .build()
            .reg2()
                .op(op::pop)
                .src(rf::sp)
                .dst(rf::lr)
                .build()
            .reg1()
                .op(op::ret)
                .dst(rf::lr)
                .build();
        return exit_block;
    }

    compile_result_t qt(compiler_t& comp,
                        const context_t& c,
                        obj_t* args) {
        auto ctx = c.ctx;
        auto res = c.target ? c.target :
                   emitter::virtual_var::get(comp.emit, "res"_ss);
        basic_block::encode(c.bb)
            .comment(format::format(
                "literal: {}",
                scm::to_string(c.ctx, c.obj)))
            .imm2()
                .op(op::move)
                .src(u32(OBJ_IDX(CAR(args))))
                .dst(&res)
                .build()
            .imm2()
                .op(op::trap)
                .src(trap_id_t(vm::trap::quote))
                .dst(&res)
                .build();
        return {c.bb, res};
    }

    compile_result_t qq(compiler_t& comp,
                        const context_t& c,
                        obj_t* args) {
        auto ctx = c.ctx;
        auto res = c.target ? c.target :
                   emitter::virtual_var::get(comp.emit, "res"_ss);
        basic_block::encode(c.bb)
            .comment(format::format(
                "literal: {}",
                scm::to_string(c.ctx, c.obj)))
            .imm2()
                .op(op::move)
                .src(u32(OBJ_IDX(CAR(args))))
                .dst(&res)
                .build()
            .imm2()
                .op(op::trap)
                .src(trap_id_t(vm::trap::quasiquote))
                .dst(&res)
                .build();
        return {c.bb, res};
    }

    compile_result_t uq(compiler_t& comp,
                        const context_t& c,
                        obj_t* args) {
        UNUSED(comp);
        UNUSED(c);
        UNUSED(args);
        return {.status = status_t::unquote_invalid};
    }

    compile_result_t car(compiler_t& comp,
                         const context_t& c,
                         obj_t* args) {
        auto ctx = c.ctx;
        auto res = c.target ? c.target :
                   emitter::virtual_var::get(comp.emit, "res"_ss);
        auto cc  = c;
        cc.obj = CAR(args);
        auto comp_res = compiler::compile_expr(comp, cc);
        basic_block::encode(comp_res.bb)
            .reg2()
                .op(op::move)
                .src(&comp_res.var)
                .dst(&res)
                .build()
            .imm2()
                .op(op::trap)
                .src(trap_id_t(vm::trap::car))
                .dst(&res)
                .build();
        return {comp_res.bb, res};
    }

    compile_result_t cdr(compiler_t& comp,
                         const context_t& c,
                         obj_t* args) {
        auto ctx = c.ctx;
        auto res = c.target ? c.target :
                   emitter::virtual_var::get(comp.emit, "res"_ss);
        auto cc  = c;
        cc.obj = CAR(args);
        auto comp_res = compiler::compile_expr(comp, cc);
        basic_block::encode(comp_res.bb)
            .reg2()
                .op(op::move)
                .src(&comp_res.var)
                .dst(&res)
                .build()
            .reg2()
                .op(op::trap)
                .src(trap_id_t(vm::trap::cdr))
                .dst(&res)
                .build();
        return {comp_res.bb, res};
    }

    compile_result_t or_(compiler_t& comp,
                         const context_t& c,
                         obj_t* args) {
        auto ctx = c.ctx;
        auto& exit_bb = emitter::make_basic_block(comp.emit, "or_exit"_ss, c.bb);
        auto res = c.target ? c.target :
                   emitter::virtual_var::get(comp.emit, "res"_ss);
        auto oc  = c;
        while (!IS_NIL(args)) {
            oc.obj = CAR(args);
            auto comp_res = compiler::compile_expr(comp, oc);
            oc.bb = comp_res.bb;
            basic_block::encode(oc.bb)
//                .reg2()
//                    .op(op::truep)
//                    .src(&comp_res.var)
//                    .dst(&tmp)
//                    .build()
                .imm1()
                    .op(op::beq)
                    .value(&exit_bb)
                    .build();
            args = CDR(args);
        }
        basic_block::encode(oc.bb)
            .imm1()
                .op(op::b)
                .value(&exit_bb)
                .build();
        return {&exit_bb, res};
    }

    compile_result_t uqs(compiler_t& comp,
                         const context_t& c,
                         obj_t* args) {
        UNUSED(comp);
        UNUSED(c);
        UNUSED(args);
        return {.status = status_t::unquote_splicing_invalid};
    }

    compile_result_t set(compiler_t& comp,
                         const context_t& c,
                         obj_t* args) {
        auto ctx = c.ctx;
        auto key  = CAR(args);
        u32  idx  = OBJ_IDX(key);
        auto name = *string::interned::get_slice(STRING_ID(OBJ_AT(idx)));
        auto res = c.target ? c.target :
                   emitter::virtual_var::get(comp.emit, name);
        auto vc = c;
        vc.obj = CADR(args);
        auto comp_res = compiler::compile_expr(comp, vc);
        basic_block::encode(comp_res.bb)
            .comment(format::format(
                "symbol: {}",
                printable_t{c.ctx, key, true}))
            .offs()
                .op(op::store)
                .offset(8)
                .src(&comp_res.var)
                .dst(rf::ep)
                .mode(true)
                .build();
        return {comp_res.bb, res};
    }

    compile_result_t if_(compiler_t& comp,
                         const context_t& c,
                         obj_t* args) {
        auto ctx = c.ctx;

        auto& true_bb  = emitter::make_basic_block(comp.emit, "if_true"_ss, c.bb);
        auto& false_bb = emitter::make_basic_block(comp.emit, "if_false"_ss, &true_bb);
        auto& exit_bb  = emitter::make_basic_block(comp.emit, "if_exit"_ss, &false_bb);

        auto res = c.target ? c.target :
                   emitter::virtual_var::get(comp.emit, "res"_ss);

        auto predicate = CAR(args);
        auto ic = c;
        ic.obj = predicate;
        auto pred_res = compiler::compile_expr(comp, ic);
        UNUSED(pred_res);
        basic_block::encode(c.bb)
            .next(true_bb)
            .imm1()
                .op(op::bne)
                .value(&false_bb)
                .build();

        auto consequent = CADR(args);
        ic.bb     = &true_bb;
        ic.obj    = consequent;
        ic.target = res;
        auto true_res = compiler::compile_expr(comp, ic);
        basic_block::encode(true_res.bb)
            .next(false_bb)
            .imm1()
                .op(op::b)
                .value(&exit_bb)
                .build();

        auto alternate = CADDR(args);
        if (!IS_NIL(alternate)) {
            ic.bb     = &false_bb;
            ic.obj    = alternate;
            ic.target = res;
            auto false_res = compiler::compile_expr(comp, ic);
            basic_block::encode(false_res.bb)
                .next(exit_bb)
                .imm1()
                    .op(op::b)
                    .value(&exit_bb)
                    .build();
        }

        // XXX: FIXME
        //      i'm returning the var from the true branch for
        //      now but ultimately this will have to be a phi-node or...
        //      something else; not sure.
        return {&exit_bb, true_res.var};
    }

    // ----------------
    // RETURN VALUE     <-- SP upon entry
    // ----------------
    // LINK REGISTER    <-- SP after saving LR
    // ----------------
    // FP <- SP
    bb_t& enter(bb_t& bb, u32 locals) {
        auto& entry_block = emitter::make_basic_block(*bb.emit,
                                                      "proc_prologue"_ss,
                                                      &bb);
        auto& exit_block  = emitter::make_basic_block(*bb.emit,
                                                      "proc_begin"_ss,
                                                      &entry_block);
        basic_block::encode(bb)
            .imm1()
                .op(op::b)
                .value(&entry_block)
                .build();
        basic_block::encode(entry_block)
            .comment("*** proc prologue"_ss)
            .comment("**"_ss)
            .comment("*"_ss)
            .reg2()
                .op(op::push)
                .src(rf::lr)
                .dst(rf::sp)
                .build()
            .reg2()
                .op(op::move)
                .src(rf::sp)
                .dst(rf::fp)
                .build()
            .imm1()
                .op(op::b)
                .value(&exit_block)
                .build();
        if (locals > 0) {
            basic_block::encode(exit_block)
                .imm2()
                    .src(locals)
                    .dst(rf::sp)
                    .op(op::sub)
                    .build();
        }
        return exit_block;
    }

    compile_result_t ffi(compiler_t& comp,
                         const context_t& c,
                         obj_t* sym,
                         obj_t* form,
                         obj_t* args) {
        todo(*c.bb, "XXX: ffi call"_ss);
        return {c.bb, {}};
    }

    u0 free_stack(bb_t& bb, u32 words) {
        basic_block::encode(bb)
            .imm2()
                .op(op::add)
                .src(words * 8)
                .dst(rf::sp)
                .build();
    }

    compile_result_t cons(compiler_t& comp,
                          const context_t& c,
                          obj_t* args) {
        auto ctx = c.ctx;
        auto res = c.target ? c.target :
                   emitter::virtual_var::get(comp.emit, "res"_ss);
        auto cc  = c;
        cc.obj = CAR(args);
        auto lhs_res = compiler::compile_expr(comp, cc);
        cc.bb  = lhs_res.bb;
        cc.obj = CADR(args);
        auto rhs_res = compiler::compile_expr(comp, cc);
        basic_block::encode(rhs_res.bb)
            .reg3()
                .op(op::trap)
                .a(&lhs_res.var)
                .b(&rhs_res.var)
                .c(&res)
                .build();
        return {rhs_res.bb, res};
    }

    compile_result_t atom(compiler_t& comp,
                          const context_t& c,
                          obj_t* args) {
        auto ctx = c.ctx;
        auto res = c.target ? c.target :
                   emitter::virtual_var::get(comp.emit, "res"_ss);
        auto ac  = c;
        ac.obj = CAR(args);
        auto comp_res = compiler::compile_expr(comp, ac);
        basic_block::encode(comp_res.bb)
            .reg2()
                .op(op::move)
                .src(&comp_res.var)
                .dst(&res)
                .build()
            .imm2()
                .op(op::trap)
                .src(trap_id_t(vm::trap::is_atom))
                .dst(&res)
                .build();
        return {comp_res.bb, res};
    }

    compile_result_t eval(compiler_t& comp,
                          const context_t& c,
                          obj_t* args) {
        auto ctx = c.ctx;
        auto res = c.target ? c.target :
                   emitter::virtual_var::get(comp.emit, "res"_ss);
        auto ec  = c;
        ec.obj = CAR(args);
        auto comp_res = compiler::compile_expr(comp, ec);
//        basic_block::encode(comp_res.bb)
//            .reg2()
//                .op(op::eval)
//                .src(&comp_res.var)
//                .dst(&res)
//                .build();
        return {comp_res.bb, res};
    }

    compile_result_t list(compiler_t& comp,
                          const context_t& c,
                          obj_t* args) {
        auto ctx       = c.ctx;
        u32  size      = length(ctx, args);
        auto base_addr = emitter::virtual_var::get(comp.emit, "base"_ss);
        alloc_stack(*c.bb, size, &base_addr);
        s32  offs = 0;
        auto lc   = c;
        while (!IS_NIL(args)) {
            lc.obj   = CAR(args);
            auto comp_res = compiler::compile_expr(comp, lc);
            lc.bb = comp_res.bb;
            basic_block::encode(comp_res.bb)
                .offs()
                    .op(op::store)
                    .offset(offs)
                    .src(&comp_res.var)
                    .dst(&base_addr)
                    .mode(true)
                    .build();
            args = CDR(args);
            offs += 8;
        }
        auto res = c.target ? c.target :
                   emitter::virtual_var::get(comp.emit, "res"_ss);
        auto& list_bb = emitter::make_basic_block(comp.emit, "make_list"_ss, lc.bb);
        basic_block::encode(list_bb)
            .imm2()
                .op(op::trap)
                .src(trap_id_t(vm::trap::list))
                .dst(&base_addr)
                .build();
        free_stack(list_bb, size);
        return {&list_bb, res};
    }

    compile_result_t not_(compiler_t& comp,
                          const context_t& c,
                          obj_t* args) {
        auto ctx = c.ctx;
        auto nc = c;
        nc.obj = CAR(args);
        auto res = c.target ? c.target :
                   emitter::virtual_var::get(comp.emit, "res"_ss);
        auto comp_res = compiler::compile_expr(comp, nc);
        basic_block::encode(comp_res.bb)
            .imm2()
                .op(op::trap)
                .src(trap_id_t(vm::trap::not_true))
                .dst(&comp_res.var)
                .build();
        return {comp_res.bb, res};
    }

    compile_result_t and_(compiler_t& comp,
                          const context_t& c,
                          obj_t* args) {
        auto ctx = c.ctx;
        auto& exit_bb = emitter::make_basic_block(comp.emit, "and_exit"_ss, c.bb);
        auto res = c.target ? c.target :
                   emitter::virtual_var::get(comp.emit, "res"_ss);
        auto oc  = c;
        while (!IS_NIL(args)) {
            oc.obj        = CAR(args);
            auto comp_res = compiler::compile_expr(comp, oc);
            oc.bb = comp_res.bb;
            basic_block::encode(oc.bb)
                .imm2()
                    .op(op::trap)
                    .src(trap_id_t(vm::trap::is_true))
                    .dst(&res)
                    .build()
                .imm1()
                    .op(op::bne)
                    .value(&exit_bb)
                    .build();
            args = CDR(args);
        }
        return {&exit_bb, res};
    }

    u0 todo(bb_t& bb, str::slice_t msg) {
        basic_block::encode(bb)
            .comment(msg)
            .none()
                .op(op::nop)
                .build();
    }

    compile_result_t begin(compiler_t& comp,
                           const context_t& c,
                           obj_t* args) {
        auto ctx = c.ctx;
        auto dc = c;
        compile_result_t comp_res;
        while (!IS_NIL(args)) {
            dc.obj = CAR(args);
            comp_res = compiler::compile_expr(comp, dc);
            dc.bb     = comp_res.bb;
            dc.target = comp_res.var;
            args = CDR(args);
        }
        return {dc.bb, comp_res.var};
    }

    compile_result_t error(compiler_t& comp,
                           const context_t& c,
                           obj_t* args) {
        auto res = emitter::virtual_var::get(comp.emit, "res"_ss);
        basic_block::encode(c.bb)
            .comment(format::format(
                "literal: {}",
                scm::to_string(c.ctx, c.obj)))
            .offs()
                .op(op::load)
                .offset(8)
                .src(rf::lp)
                .dst(&res)
                .build()
            .imm2()
                .op(op::trap)
                .src(trap_id_t(vm::trap::error))
                .dst(&res)
                .build();
        return {c.bb, res};
    }

    compile_result_t apply(compiler_t& comp,
                           const context_t& c,
                           obj_t* sym,
                           obj_t* form,
                           obj_t* args) {
        auto ctx  = c.ctx;
        auto proc = PROC(form);
        auto base_addr = emitter::virtual_var::get(comp.emit, "base"_ss);
        auto& apply_bb = emitter::make_basic_block(comp.emit, "apply"_ss, c.bb);

        alloc_stack(apply_bb, 1, &base_addr);
        auto keys = proc->params;
        auto vals = args;
        auto vc = c;
        vc.bb = &apply_bb;
        while (!IS_NIL(keys)) {
            auto key  = TYPE(keys) == obj_type_t::pair ? CAR(keys) : keys;
            auto name = *string::interned::get_slice(STRING_ID(key));
            if (TYPE(keys) != obj_type_t::pair) {
                vc.obj = vals;
                auto comp_res = compiler::compile_expr(comp, vc);
                vc.bb = comp_res.bb;
                basic_block::encode(vc.bb)
                    .comment(format::format("symbol: {}", name))
                    .offs()
                        .op(op::store)
                        .offset(8)
                        .src(&comp_res.var)
                        .dst(rf::ep)
                        .mode(true)
                        .build();
                break;
            } else {
                vc.obj = CAR(vals);
                auto comp_res = compiler::compile_expr(comp, vc);
                vc.bb = comp_res.bb;
                basic_block::encode(vc.bb)
                    .comment(format::format("symbol: {}", name))
                    .offs()
                        .op(op::store)
                        .offset(8)
                        .src(&comp_res.var)
                        .dst(rf::ep)
                        .mode(true)
                        .build();
                keys = CDR(keys);
                vals = CDR(vals);
            }
        }

        basic_block::encode(vc.bb)
            .comment(format::format(
                "call: {}",
                printable_t{c.ctx, sym, true}))
            .imm1()
                .op(op::blr)
                .value(proc->addr.bb)
                .build();

        auto res = c.target ? c.target :
                   emitter::virtual_var::get(comp.emit, "res"_ss);
        auto& cleanup_bb = emitter::make_basic_block(comp.emit,
                                                     "apply_cleanup"_ss,
                                                     vc.bb);
        basic_block::encode(cleanup_bb)
            .comment("load return value from proc call"_ss)
            .offs()
                .op(op::load)
                .src(&base_addr)
                .dst(&res)
                .offset(0)
                .build();
        free_stack(cleanup_bb, 1);

        return {&cleanup_bb, res};
    }

    compile_result_t lambda(compiler_t& comp,
                            const context_t& c,
                            obj_t* form,
                            obj_t* args) {
        auto ctx  = c.ctx;
        b8 is_mac = scm::equal(ctx, c.sym, SYM("define-macro"));
        auto params = CAR(args);
        auto body   = CDR(args);
        auto proc = make_proc(c.ctx, c.sym, params, body, is_mac);
        auto idx  = OBJ_IDX(proc);
        auto res  = c.target ? c.target :
                    emitter::virtual_var::get(comp.emit, "res"_ss);
        if (!c.top_level) {
            basic_block::encode(c.bb)
                .comment(format::format("literal: {}",
                                        scm::to_string(c.ctx, c.obj)))
                .imm2()
                    .op(op::move)
                    .src(u32(idx))
                    .dst(&res)
                    .build();
            return {c.bb, res};
        } else {
            return {c.bb, .obj = proc};
        }
    }

    compile_result_t while_(compiler_t& comp,
                            const context_t& c,
                            obj_t* args) {
        UNUSED(comp);
        UNUSED(args);
        return {c.bb, nullptr};
    }

    compile_result_t define(compiler_t& comp,
                            const context_t& c,
                            obj_t* args) {
        auto ctx = c.ctx;
        auto key  = CAR(args);
        u32  idx  = OBJ_IDX(key);
        auto name = *string::interned::get_slice(STRING_ID(OBJ_AT(idx)));
        auto res  = emitter::virtual_var::declare(comp.emit, name);
        auto vc = c;
        vc.sym      = key;
        vc.obj      = CADR(args);
        vc.target   = res;
        vc.is_macro = false;
        auto comp_res = compiler::compile_expr(comp, vc);
        if (c.top_level && comp_res.obj) {
            scm::define(ctx, key, comp_res.obj);
            if (TYPE(comp_res.obj) == obj_type_t::proc) {
                auto proc = PROC(comp_res.obj);
                if (!proc->is_compiled) {
                    auto pc = c;
                    pc.top_level = false;
                    proc->sym    = key;
                    return comp_proc(comp, pc, proc);
                }
            }
        }
        basic_block::encode(comp_res.bb)
            .comment(format::format(
                "define symbol: {}",
                printable_t{c.ctx, key, true}))
            .none()
                .op(op::nop)
               .build();
        return {comp_res.bb, res};
    }

    compile_result_t cmp_op(compiler_t& comp,
                            const context_t& c,
                            prim_type_t type,
                            obj_t* args) {
        auto ctx  = c.ctx;
        auto oc = c;
        oc.obj = CAR(args);
        auto lhs_res = compiler::compile_expr(comp, oc);
        oc.bb  = lhs_res.bb;
        oc.obj = CADR(args);
        auto rhs_res = compiler::compile_expr(comp, oc);
        auto res     = emitter::virtual_var::get(comp.emit, "res"_ss);

        switch (type) {
            case prim_type_t::is:
                basic_block::encode(rhs_res.bb)
                    .comment("prim: is"_ss)
                    .reg2_imm()
                        .op(op::trap)
                        .src(&lhs_res.var)
                        .dst(&rhs_res.var)
                        .value(trap_id_t(vm::trap::compare))
                        .build()
                    .reg1()
                        .op(op::seq)
                        .dst(&res)
                        .build();
                break;

            case prim_type_t::lt:
                basic_block::encode(rhs_res.bb)
                    .comment("prim: lt"_ss)
                    .reg2_imm()
                        .op(op::trap)
                        .src(&lhs_res.var)
                        .dst(&rhs_res.var)
                        .value(trap_id_t(vm::trap::compare))
                        .build()
                    .reg1()
                        .op(op::sl)
                        .dst(&res)
                        .build();
                break;

            case prim_type_t::gt:
                basic_block::encode(rhs_res.bb)
                    .comment("prim: gt"_ss)
                    .reg2_imm()
                        .op(op::trap)
                        .src(&lhs_res.var)
                        .dst(&rhs_res.var)
                        .value(trap_id_t(vm::trap::compare))
                        .build()
                    .reg1()
                        .op(op::sg)
                        .dst(&res)
                        .build();
                break;

            case prim_type_t::lte:
                basic_block::encode(rhs_res.bb)
                    .comment("prim: lte"_ss)
                    .reg2_imm()
                        .op(op::trap)
                        .src(&lhs_res.var)
                        .dst(&rhs_res.var)
                        .value(trap_id_t(vm::trap::compare))
                        .build()
                    .reg1()
                        .op(op::sle)
                        .dst(&res)
                        .build();
                break;

            case prim_type_t::gte:
                basic_block::encode(rhs_res.bb)
                    .comment("prim: gte"_ss)
                    .reg2_imm()
                        .op(op::trap)
                        .src(&lhs_res.var)
                        .dst(&rhs_res.var)
                        .value(trap_id_t(vm::trap::compare))
                        .build()
                    .reg1()
                        .op(op::sge)
                        .dst(&res)
                        .build();
                break;

            default:
                return {.status = status_t::unknown_primitive};

        }

        return {c.bb, res};
    }

    compile_result_t inline_(compiler_t& comp,
                             const context_t& c,
                             obj_t* sym,
                             obj_t* form,
                             obj_t* args) {
        UNUSED(sym);
        UNUSED(form);
        UNUSED(args);
        todo(*c.bb, "XXX: macro call"_ss);
        return {c.bb, 0};
    }

    compile_result_t set_car(compiler_t& comp,
                             const context_t& c,
                             obj_t* args) {
        auto ctx = c.ctx;
        auto sc  = c;
        sc.obj = CADR(args);
        auto rhs_res = compiler::compile_expr(comp, sc);
        sc.bb  = rhs_res.bb;
        sc.obj = CAR(args);
        auto lhs_res = compiler::compile_expr(comp, sc);
        basic_block::encode(lhs_res.bb)
            .reg2_imm()
                .op(op::trap)
                .src(&lhs_res.var)
                .dst(&rhs_res.var)
                .value(trap_id_t(vm::trap::set_car))
                .build();
        return {lhs_res.bb, {}};
    }

    compile_result_t set_cdr(compiler_t& comp,
                             const context_t& c,
                             obj_t* args) {
        auto ctx = c.ctx;
        auto sc  = c;
        sc.obj = CADR(args);
        auto rhs_res = compiler::compile_expr(comp, sc);
        sc.bb  = rhs_res.bb;
        sc.obj = CAR(args);
        auto lhs_res = compiler::compile_expr(comp, sc);
        basic_block::encode(lhs_res.bb)
            .reg2_imm()
                .op(op::trap)
                .src(&lhs_res.var)
                .dst(&rhs_res.var)
                .value(trap_id_t(vm::trap::set_cdr))
                .build();
        return {lhs_res.bb, {}};
    }

    compile_result_t arith_op(compiler_t& comp,
                              const context_t& c,
                              op_code_t op_code,
                              obj_t* args) {
        auto ctx  = c.ctx;
        u32  size = length(ctx, args);
        auto res  = c.target ? c.target :
                    emitter::virtual_var::get(comp.emit, "res"_ss);
        auto base_addr = emitter::virtual_var::get(comp.emit, "base"_ss);
        alloc_stack(*c.bb, size, &base_addr);
        s32  offs = 0;
        auto ac   = c;
        while (!IS_NIL(args)) {
            ac.obj = CAR(args);
            auto comp_res = compiler::compile_expr(comp, ac);
            ac.bb = comp_res.bb;
            basic_block::encode(ac.bb)
                .offs()
                    .op(op::store)
                    .offset(offs)
                    .src(&comp_res.var)
                    .dst(&base_addr)
                    .mode(true)
                    .build();
            args = CDR(args);
            offs += 8;
        }
        basic_block::encode(*ac.bb)
            .reg2_imm()
                .op(op_code)
                .src(&base_addr)
                .dst(&res)
                .value(size * 8)
                .build();
        free_stack(*ac.bb, size);
        return {ac.bb, res};
    }

    compile_result_t prim(compiler_t& comp,
                          const context_t& c,
                          obj_t* sym,
                          obj_t* form,
                          obj_t* args,
                          prim_type_t type) {
        switch (type) {
            case prim_type_t::is:
            case prim_type_t::gt:
            case prim_type_t::lt:
            case prim_type_t::lte:
            case prim_type_t::gte:              return cmp_op(comp, c, type, args);
            case prim_type_t::add:              return arith_op(comp, c, op::add, args);
            case prim_type_t::sub:              return arith_op(comp, c, op::sub, args);
            case prim_type_t::mul:              return arith_op(comp, c, op::mul, args);
            case prim_type_t::div:              return arith_op(comp, c, op::div, args);
            case prim_type_t::mod:              return arith_op(comp, c, op::mod, args);
            case prim_type_t::set:              return set(comp, c, args);
            case prim_type_t::if_:              return if_(comp, c, args);
            case prim_type_t::or_:              return or_(comp, c, args);
            case prim_type_t::and_:             return and_(comp, c, args);
            case prim_type_t::not_:             return not_(comp, c, args);
            case prim_type_t::car:              return car(comp, c, args);
            case prim_type_t::cdr:              return cdr(comp, c, args);
            case prim_type_t::cons:             return cons(comp, c, args);
            case prim_type_t::atom:             return atom(comp, c, args);
            case prim_type_t::eval:             return eval(comp, c, args);
            case prim_type_t::list:             return list(comp, c, args);
            case prim_type_t::quote:            return qt(comp, c, args);
            case prim_type_t::begin:            return begin(comp, c, args);
            case prim_type_t::error:            return error(comp, c, args);
            case prim_type_t::lambda:           return lambda(comp,
                                                              c,
                                                              form,
                                                              args);
            case prim_type_t::define:           return define(comp, c, args);
            case prim_type_t::while_:           return while_(comp, c, args);
            case prim_type_t::setcar:           return set_car(comp, c, args);
            case prim_type_t::setcdr:           return set_cdr(comp, c, args);
            case prim_type_t::unquote:          return uq(comp, c, args);
            case prim_type_t::quasiquote:       return qq(comp, c, args);
            case prim_type_t::define_macro:     return define_macro(comp, c, args);
            case prim_type_t::unquote_splicing: return uqs(comp, c, args);
            default:                            return {c.bb, 0};
        }
    }

    compile_result_t call_back(compiler_t& comp,
                               const context_t& c,
                               obj_t* sym,
                               obj_t* form,
                               obj_t* args) {
        UNUSED(sym);
        UNUSED(form);
        UNUSED(args);
        todo(*c.bb, "XXX: cfunc call"_ss);
        return {c.bb, 0};
    }

    //
    compile_result_t comp_proc(compiler_t& comp,
                               const context_t& c,
                               proc_t* proc) {
        auto ctx = c.ctx;

        auto params = proc->params;
        while (!IS_NIL(params)) {
            auto name = *string::interned::get_slice(STRING_ID(CAR(params)));
            emitter::virtual_var::declare(comp.emit, name);
            params = CDR(params);
        }

        auto name = *string::interned::get_slice(STRING_ID(proc->sym));
        auto& proc_bb = emitter::make_basic_block(comp.emit, name, c.bb);
        proc->addr.bb = &proc_bb;

        printable_t params_prt{c.ctx, proc->params, true};
        printable_t body_prt{c.ctx, proc->body, true};
        body_prt.pretty.indent  = 0;
        body_prt.pretty.margin  = 2;
        body_prt.pretty.enabled = true;
        basic_block::encode(proc_bb)
            .note("----------------------------------------------------------------"_ss)
            .note(format::format("procedure: {}", name))
            .note(format::format("(lambda {}{:<{}}{})",
                                 params_prt,
                                 " ",
                                 6,
                                 body_prt))
            .note("----------------------------------------------------------------"_ss);

        auto res = c.target ? c.target :
                   emitter::virtual_var::get(comp.emit, "res"_ss);
        auto bc = c;
        bc.bb     = &enter(proc_bb, 0);
        bc.obj    = proc->body;
        bc.target = res;
        auto comp_res = compiler::compile_expr(comp, bc);

        basic_block::encode(comp_res.bb)
            .offs()
                .op(op::store)
                .src(&comp_res.var)
                .dst(rf::fp)
                .offset(-16)
                .mode(true)
                .build();

        proc->is_compiled = true;
        return {&leave(*comp_res.bb), comp_res.var};
    }

    compile_result_t define_macro(compiler_t& comp,
                                  const context_t& c,
                                  obj_t* args) {
        auto ctx = c.ctx;
        auto key  = CAR(args);
        u32  idx  = OBJ_IDX(key);
        auto name = *string::interned::get_slice(STRING_ID(OBJ_AT(idx)));
        auto res  = emitter::virtual_var::declare(comp.emit, name);
        auto vc = c;
        vc.sym      = key;
        vc.obj      = CADR(args);
        vc.target   = c.target;
        vc.is_macro = true;
        auto comp_res = compiler::compile_expr(comp, vc);
        if (c.top_level && comp_res.obj) {
            scm::check_type(ctx, comp_res.obj, obj_type_t::proc);
            scm::define(ctx, key, comp_res.obj);
            auto proc = PROC(comp_res.obj);
            if (!proc->is_compiled) {
                auto pc = c;
                pc.top_level = false;
                proc->sym    = key;
                return comp_proc(comp, pc, proc);
            }
        }
        basic_block::encode(comp_res.bb)
            .comment(format::format(
                "symbol: {}",
                printable_t{c.ctx, key, true}))
            .offs()
                .op(op::store)
                .offset(8)
                .src(&comp_res.var)
                .dst(rf::ep)
                .mode(true)
                .build()
            .reg2()
                .op(op::move)
                .src(&comp_res.var)
                .dst(&res)
                .build();
        return {comp_res.bb, res};
    }

    u0 alloc_stack(bb_t& bb, u32 words, var_version_t** base_addr) {
        basic_block::encode(bb)
            .imm2()
                .op(op::sub)
                .src(words * 8)
                .dst(rf::sp)
                .build()
            .offs()
                .op(op::lea)
                .offset(-(words * 8))
                .dst(base_addr)
                .src(rf::sp)
                .build();
    }

    compile_result_t lookup(compiler_t& comp, const context_t& c) {
        auto sym = scm::to_string(c.ctx, c.obj);
        auto res = emitter::virtual_var::get(comp.emit, sym);
        BC_ASSERT_MSG(res, "virtual variable not found; declare it");
        basic_block::encode(c.bb)
            .comment(format::format("symbol: {}", sym))
            .offs()
                .op(op::load)
                .offset(8)
                .src(rf::ep)
                .dst(&res)
                .build();
        return {c.bb, res};
    }

    compile_result_t self_eval(compiler_t& comp, const context_t& c) {
        auto lit = c.target ? c.target :
                   emitter::virtual_var::get(comp.emit, "lit"_ss);
        basic_block::encode(c.bb)
            .comment(format::format(
                "literal: {}",
                scm::to_string(c.ctx, c.obj)))
            .offs()
                .op(op::load)
                .offset(8)
                .src(rf::lp)
                .dst(&lit)
                .build();
        return {c.bb, lit};
    }
}
