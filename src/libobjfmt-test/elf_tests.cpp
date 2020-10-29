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
#include <basecode/core/defer.h>
#include <basecode/objfmt/objfmt.h>
#include <basecode/core/stopwatch.h>
#include <basecode/objfmt/container.h>
#include "test.h"

using namespace basecode;

TEST_CASE("basecode::objfmt ELF test") {
    using namespace objfmt;

    stopwatch_t timer{};
    stopwatch::start(timer);

    file_t rot13_pgm{};
    REQUIRE(OK(file::init(rot13_pgm)));
    defer(file::free(rot13_pgm));
    path::set(rot13_pgm.path, "rot13");
    rot13_pgm.machine = machine::type_t::x86_64;

    container::session_t s{};
    container::session::init(s);
    defer(container::session::free(s));
    s.file                  = &rot13_pgm;
    s.type                  = container::type_t::elf;
    s.output_type           = container::output_type_t::exe;
    s.versions.linker.major = 6;
    s.versions.linker.minor = 0;
    s.versions.min_os.major = 4;
    s.versions.min_os.minor = 0;
    s.flags.console         = true;
    REQUIRE(OK(container::write(s)));

    stopwatch::stop(timer);
    stopwatch::print_elapsed("objfmt write ELF executable time"_ss, 40, timer);
}
