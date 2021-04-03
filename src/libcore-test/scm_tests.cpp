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
#include <basecode/core/scm/scm.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::scm bytecode emitter", "[scm]") {
    const auto heap_size = 64 * 1024;
    auto alloc = memory::system::default_alloc();

    auto ctx = (scm::ctx_t*) memory::alloc(alloc, heap_size);
    scm::init(ctx, heap_size, alloc);

    scm::obj_t* obj{};

    const auto source = R"(
(= length (fn (ls)
    (if (not ls)
        0
        (if (not (atom ls))
            (+ 1 (length (cdr ls)))
            (error "invalid argument to length")))))
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
    auto expr = scm::read(ctx, crsr);
    if (!expr) {
        obj = scm::nil(ctx);
    } else {
        format::print("expr: {}\n", scm::printable_t{ctx, expr, true});
        obj = scm::eval2(ctx, expr);
        format::print("obj:  {}\n", scm::printable_t{ctx, obj, true});
    }
}
