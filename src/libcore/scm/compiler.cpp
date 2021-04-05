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

    u0 free(compiler_t& comp) {
        vm::emitter::free(comp.emitter);
    }

    u0 reset(compiler_t& comp) {
        vm::emitter::reset(comp.emitter);
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
        switch (TYPE(c.obj)) {
            case obj_type_t::pair: {
                auto ctx = c.ctx;
                obj_t* sym  = CAR(c.obj);
                obj_t* args = CDR(c.obj);
                obj_t* form = TYPE(sym) == obj_type_t::symbol ? get(ctx, sym, c.env) : sym;

                auto cl = cons(ctx, c.obj, ctx->call_list);
                ctx->call_list = cl;
                defer(ctx->call_list = CDR(cl));

                switch (TYPE(form)) {
                    case obj_type_t::ffi:   return vm::bytecode::ffi(comp, c, sym, form, args);
                    case obj_type_t::prim:  return vm::bytecode::prim(comp, c, form, args, PRIM(form));
                    case obj_type_t::func:  return vm::bytecode::apply(comp, c, sym, form, args);
                    case obj_type_t::macro: return vm::bytecode::inline_(comp, c, sym, form, args);
                    case obj_type_t::cfunc: return vm::bytecode::call_back(comp, c, sym, form, args);
                    default:                error(ctx, "tried to call non-callable value");
                }
                return {c.bb, 0};
            }
            case obj_type_t::symbol:    return vm::bytecode::lookup(comp, c);
            default:                    return vm::bytecode::self_eval(comp, c);
        }
    }

    context_t make_context(bb_t& bb, ctx_t* ctx, obj_t* obj, obj_t* env, b8 top_level) {
        context_t c{};
        c.bb        = &bb;
        c.ctx       = ctx;
        c.obj       = obj;
        c.env       = env;
        c.target    = {};
        c.top_level = top_level;
        return c;
    }
}
