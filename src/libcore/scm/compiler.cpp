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

#include <basecode/core/path.h>
#include <basecode/core/scm/scm.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/scm/compiler.h>
#include <basecode/core/scm/bytecode.h>

namespace basecode::scm::compiler {
    u0 free(compiler_t& comp) {
        emitter::free(comp.emit);
    }

    u0 reset(compiler_t& comp) {
        emitter::reset(comp.emit);
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
        c.sym       = ctx->nil;
        c.target    = {};
        c.is_macro  = false;
        c.top_level = top_level;
        return c;
    }

    u0 init(compiler_t& comp, vm_t* vm, alloc_t* alloc) {
        comp.vm = vm;
        emitter::init(comp.emit, vm, alloc);
    }

    compile_result_t compile(compiler_t& comp, ctx_t* ctx, obj_t* obj) {
        compiler::reset(comp);

        auto& bb = emitter::make_basic_block(comp.emit,
                                                 "eval2"_ss,
                                                 {});
        emitter::virtual_var::declare(comp.emit, "_"_ss);
        emitter::virtual_var::declare(comp.emit, "res"_ss);
        emitter::virtual_var::declare(comp.emit, "lit"_ss);
        emitter::virtual_var::declare(comp.emit, "base"_ss);

        TIME_BLOCK(
            "compile expr"_ss,
            basic_block::encode(bb)
                .area(mem_area_type_t::heap)
                .db({2, 3, 7, 15, 31, 63, 127, 255})
                .dw({1024, 2048, 4096, 8192, 16386})
                .dd({(u32) INT_MIN, INT_MAX, UINT_MAX})
                .dq({(u64) LONG_MIN, LONG_MAX, ULONG_MAX})
                .code();
            auto tc = compiler::make_context(bb,
                                             ctx,
                                             obj,
                                             top_env(ctx),
                                             true);
            auto comp_result = compiler::compile_expr(comp, tc);
            basic_block::encode(comp_result.bb)
                .imm1()
                    .op(vm::instruction::type::exit)
                    .value(1)
                    .build();
            auto status = emitter::find_liveliness_intervals(comp.emit);
            if (!OK(status)) {
            }
            status = emitter::allocate_registers(comp.emit);
            if (!OK(status)) {
            }
        );

        emitter::format_liveliness_intervals(comp.emit);

        {
            str_t str{};
            str::init(str, context::top()->alloc.temp);
            str::reserve(str, 32 * 1024);
            {
                str_buf_t buf{&str};
                emitter::disassemble(comp.emit, bb, buf);
            }
            format::print("{}\n", str);
        }

        auto dot_file = "eval2.dot"_path;
        if (!OK(emitter::create_dot(ctx->compiler.emit, dot_file))) {
            format::print("error writing dot file.\n");
        }
        path::free(dot_file);

        return {&bb,
                emitter::virtual_var::get(comp.emit, "res"_ss)};
    }

    compile_result_t compile_expr(compiler_t& comp, const context_t& c) {
        switch (TYPE(c.obj)) {
            case obj_type_t::pair: {
                auto ctx = c.ctx;
                obj_t* sym  = CAR(c.obj);
                obj_t* args = CDR(c.obj);
                obj_t* form = TYPE(sym) == obj_type_t::symbol ?
                              get(ctx, sym) : sym;

                auto cl = cons(ctx, c.obj, ctx->call_list);
                ctx->call_list = cl;
                defer(ctx->call_list = CDR(cl));

                switch (TYPE(form)) {
                    case obj_type_t::ffi:
                        return bytecode::ffi(comp, c, sym, form, args);
                    case obj_type_t::prim:
                        return bytecode::prim(comp,
                                              c,
                                              sym,
                                              form,
                                              args,
                                              PRIM(form));
                    case obj_type_t::proc:
                        return bytecode::apply(comp, c, sym, form, args);
                    case obj_type_t::cfunc:
                        return bytecode::call_back(comp, c, sym, form, args);
                    default:
                        error(ctx, "tried to call non-callable value");
                }
                return {c.bb, 0};
            }
            case obj_type_t::symbol:
                return bytecode::lookup(comp, c);
            default:
                return bytecode::self_eval(comp, c);
        }
    }
}
