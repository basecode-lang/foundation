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

#include <catch2/catch.hpp>
#include <basecode/core/buf.h>
#include <basecode/core/string.h>
#include <basecode/core/stopwatch.h>
#include <basecode/core/scm/bytecode.h>

using namespace basecode;

TEST_CASE("basecode::scm bytecode emitter", "[scm]") {
    scm::vm_t vm{};
    scm::vm::init(vm);

    scm::vm::memory_map(vm, scm::memory_area_t::code, scm::register_file::lp, 256 * 1024);
    scm::vm::memory_map(vm, scm::memory_area_t::heap, scm::register_file::hp, 64 * 1024);
    scm::vm::memory_map(vm, scm::memory_area_t::env_stack, scm::register_file::ep, 4 * 1024, true);
    scm::vm::memory_map(vm, scm::memory_area_t::data_stack, scm::register_file::dp, 4 * 1024, true);
    scm::vm::reset(vm);

    format::print("Address            Offset             Size    Reg Value              Top\n");
    format::print("-----------------------------------------------------------------------------\n");
    for (const auto& entry : vm.memory_map.entries) {
        if (!entry.valid)
            continue;
        format::print("0x{:016X} 0x{:016X} 0x{:05X}", entry.addr, entry.offs, entry.size);
        format::print(" {:<2}  0x{:016X} {}\n", scm::register_file::name(entry.reg), G(entry.reg), entry.top ? "  X" : "");
    }
    format::print("\n");

    scm::emitter_t e{};
    scm::vm::emitter::init(e, &vm, G(scm::register_file::lp));
    defer(
        scm::vm::emitter::free(e);
        scm::vm::free(vm)
        );

    namespace op = scm::instruction::type;
    namespace rf = scm::register_file;
    namespace bb = scm::vm::basic_block;
    namespace bc = scm::vm::bytecode;
    namespace emitter = scm::vm::emitter;

    auto& bb0 = emitter::make_basic_block(e, scm::bb_type_t::code);
    auto& main_bb = emitter::make_basic_block(e, scm::bb_type_t::code);
    bb::imm1(main_bb, op::blr, emitter::imm(&bb0));
    bb::imm1(main_bb, op::exit, emitter::imm(1, scm::imm_type_t::boolean));
    bb::succ(main_bb, bb0);

    bb::note(bb0, "********************************************"_ss);
    bb::note(bb0, "proc: length"_ss);
    bb::note(bb0, "********************************************"_ss);
    auto& enter_bb = bc::enter(bb0, 2);
    bc::leave(enter_bb);

    REQUIRE(OK(emitter::assemble(e, main_bb)));
    format::print_hex_dump(vm.heap, LP);
    emitter::disassemble(e, main_bb);

    TIME_BLOCK(
        "vm execution"_ss,
         auto status = scm::vm::step(vm, nullptr);
         if (!OK(status))
            REQUIRE(false););
}
