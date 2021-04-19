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

#include <catch.hpp>
#include <basecode/core/buf.h>
#include <basecode/core/string.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/scm/bytecode.h>

using namespace basecode;

static u0 assemble_and_execute_block(scm::ctx_t* ctx,
                                     scm::bb_t& bb,
                                     s32 cycles,
                                     b8 c,
                                     b8 i,
                                     b8 z,
                                     b8 n,
                                     b8 v) {
    auto& emit = ctx->compiler.emit;
    auto& vm   = ctx->vm;

    auto code_size = scm::emitter::assembled_size_bytes(emit, bb);
    auto code_buf  = (u64*) memory::alloc(ctx->alloc, code_size, alignof(u64));
    defer(memory::free(ctx->alloc, code_buf));

    auto status = scm::emitter::assemble(emit, bb, code_buf);
    if (!OK(status))
        REQUIRE(false);

    const auto start_addr = u64(code_buf);
    PC = start_addr;
    status = scm::vm::step(vm, ctx, cycles);
    if (!OK(status))
        REQUIRE(false);
    if (PC != start_addr + (sizeof(scm::encoded_inst_t) * cycles))
        REQUIRE(false);
    auto flags = (scm::flag_register_t*) &F;
    if (flags->c != c) REQUIRE(false);
    if (flags->i != i) REQUIRE(false);
    if (flags->z != z) REQUIRE(false);
    if (flags->n != n) REQUIRE(false);
    if (flags->v != v) REQUIRE(false);
}

TEST_CASE("basecode::scm::vm instructions") {
    namespace rf = scm::vm::register_file;
    namespace op = scm::vm::instruction;

    const auto heap_size = 64 * 1024;
    auto alloc = memory::system::default_alloc();

    auto ctx = (scm::ctx_t*) memory::alloc(alloc, heap_size, alignof(u0*));
    scm::init(ctx, heap_size, alloc);
    defer(scm::free(ctx);
        memory::free(alloc, ctx));

    auto& emit = ctx->compiler.emit;
    auto& vm = ctx->vm;

    TIME_BLOCK(
        "execute virtual machine instructions"_ss,
        /* nop */ {
            scm::emitter::reset(emit);
            auto& bb = scm::emitter::make_basic_block(emit,
                                                      "test"_ss,
                                                      nullptr);
            scm::basic_block::encode(bb)
                .none()
                    .op(op::type::nop)
                    .build();
            assemble_and_execute_block(ctx, bb,
                                       1,
                                       false,
                                       false,
                                       false,
                                       false,
                                       false);
        }

        /* move */ {
            scm::emitter::reset(emit);
            auto& bb = scm::emitter::make_basic_block(emit,
                                                      "test"_ss,
                                                      nullptr);
            scm::basic_block::encode(bb)
                .imm2()
                    .op(op::type::move)
                    .src(42)
                    .dst(rf::r0)
                    .build();
            assemble_and_execute_block(ctx,
                                       bb,
                                       1,
                                       false,
                                       false,
                                       false,
                                       false,
                                       false);
            if (R(0) != 42)
                REQUIRE(false);

            auto& bb2 = scm::emitter::make_basic_block(emit,
                                                       "test2"_ss,
                                                       nullptr);
            scm::basic_block::encode(bb2)
                .reg2()
                    .op(op::type::move)
                    .src(rf::r0)
                    .dst(rf::r1)
                    .build();
            assemble_and_execute_block(ctx,
                                       bb2,
                                       1,
                                       false,
                                       false,
                                       false,
                                       false,
                                       false);
            if (R(0) != 42)
                REQUIRE(false);
            if (R(1) != 42)
                REQUIRE(false);

            auto& bb3 = scm::emitter::make_basic_block(emit,
                                                       "test3"_ss,
                                                       nullptr);
            scm::basic_block::encode(bb3)
                .imm2()
                    .op(op::type::move)
                    .src(0)
                    .dst(rf::r2)
                    .build();
            assemble_and_execute_block(ctx,
                                       bb3,
                                       1,
                                       false,
                                       false,
                                       true,
                                       false,
                                       false);
            if (R(2) != 0)
                REQUIRE(false);

            auto& bb4 = scm::emitter::make_basic_block(emit,
                                                       "test4"_ss,
                                                       nullptr);
            scm::basic_block::encode(bb4)
                .imm2()
                    .op(op::type::move)
                    .src(-42)
                    .dst(rf::r2)
                    .build();
            assemble_and_execute_block(ctx,
                                       bb4,
                                       1,
                                       false,
                                       false,
                                       false,
                                       true,
                                       false);
            if (R(2) != u32(-42))
                REQUIRE(false);
        }

        /* add, adds */ {
            scm::emitter::reset(emit);
            auto& bb = scm::emitter::make_basic_block(emit,
                                                      "test"_ss,
                                                      nullptr);
            scm::basic_block::encode(bb)
                .imm2()
                    .op(op::type::add)
                    .src(u32(42))
                    .dst(rf::r0)
                    .build();
            assemble_and_execute_block(ctx,
                                       bb,
                                       1,
                                       false,
                                       false,
                                       false,
                                       false,
                                       false);
            if (R(0) != 84)
                REQUIRE(false);

            auto& bb2 = scm::emitter::make_basic_block(emit,
                                                       "test2"_ss,
                                                       nullptr);
            scm::basic_block::encode(bb2)
                .reg2()
                    .op(op::type::add)
                    .src(rf::r0)
                    .dst(rf::r1)
                    .build();
            assemble_and_execute_block(ctx,
                                       bb2,
                                       1,
                                       false,
                                       false,
                                       false,
                                       false,
                                       false);
            if (R(1) != 126)
                REQUIRE(false);
        }

        /* mul */ {
        }

        /* div */ {
        }

        /* pow */ {
        }

        /* mod */ {
        }

        /* neg */ {
        }

        /* not */ {
        }

        /* shl */ {
        }

        /* shr */ {
        }

        /* or */ {
        }

        /* and */ {
        }

        /* xor */ {
        }

        /* push, pop */ {
        }

        /* load */ {
        }

        /* store */ {
        }

        /* lea */ {
        }

        /* trap */ {
        }

        /* unconditional branches: br, bra */ {
        }

        /* subroutine branch: blr, ret */ {
        }

        /* conditional branches: cmp, beq, bne, bl, ble, bg, bge */ {
        }

        /* conditional set: cmp, seq, sne, sl, sle, sg, sge */ {
        }

        /* fix, flo */ {
        }

        /* cons, car, cdr, setcar, setcdr */ {
        }

        /* const */ {
        }

        /* env, error */ {
        }

        /* collect */ {
        }
    );
}

TEST_CASE("basecode::scm bytecode emitter", "[scm]") {
    const auto heap_size = 64 * 1024;
    auto alloc = memory::system::default_alloc();

    auto ctx = (scm::ctx_t*) memory::alloc(alloc, heap_size, alignof(u0*));
    scm::init(ctx, heap_size, alloc);

    const auto source = R"(
(begin
    (define length (lambda (ls)
        (if (not ls)
            0
            (if (not (atom ls))
                (+ 1 (length (cdr ls)))
                (error "invalid argument to length")))))

    (define nums (list 1 2 3 4 5 6))
    (length nums))
)"_ss;

    buf_t buf{};
    buf::init(buf);
    if (!OK(buf::load(buf, source)))
        REQUIRE(false);
    buf_crsr_t crsr{};
    buf::cursor::init(crsr, buf);
    auto gc = scm::save_gc(ctx);
    defer(
        scm::restore_gc(ctx, gc);
        buf::cursor::free(crsr);
        buf::free(buf);
        scm::free(ctx);
        memory::free(alloc, ctx));
    while (true) {
        auto expr = scm::read(ctx, crsr);
        if (!expr)
            break;
        if (IS_SCM_ERROR(expr)) {
            format::print("{}\n", scm::printable_t{ctx, expr});
            break;
        }
        auto obj = scm::eval2(ctx, expr);
        format::print("obj:  {}\n", scm::printable_t{ctx, obj, true});
    }
}
