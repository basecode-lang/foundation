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
#include <basecode/core/scm/types.h>
#include <basecode/core/scm/bytecode.h>

using namespace basecode;

TEST_CASE("basecode::scm bytecode emitter", "[scm]") {
    const auto heap_size = 64 * 1024;
    auto alloc = memory::system::default_alloc();

    auto ctx = (scm::ctx_t*) memory::alloc(alloc, heap_size);
    auto& vm = ctx->vm;

    scm::init(ctx, heap_size, alloc);
    format::print("Address            Offset             Size    Reg Value               Top\n");
    format::print("-----------------------------------------------------------------------------\n");
    for (const auto& entry : vm.mem_map.entries) {
        if (!entry.valid)
            continue;
        format::print("0x{:016X} 0x{:016X} 0x{:05X}",
                      entry.addr,
                      entry.offs,
                      entry.size);
        format::print(" {:<2}  0x{:016X} {}\n",
                      scm::register_file::name(entry.reg),
                      G(entry.reg),
                      entry.top ? "  X" : "");
    }
    format::print("\n");

    const auto source = R"(
(do
    (= length (fn (ls)
        (if (not ls)
            0
            (if (not (atom ls))
                (+ 1 (length (cdr ls)))
                (error "invalid argument to length")))))

    (let nums (list 1 2 3 4 5 6))
    (length nums))
)"_ss;

    buf_t buf{};
    buf::init(buf);
    REQUIRE(OK(buf::load(buf, source)));
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
        auto obj = scm::eval2(ctx, expr);
        format::print("obj:  {}\n", scm::printable_t{ctx, obj, true});
    }
}
