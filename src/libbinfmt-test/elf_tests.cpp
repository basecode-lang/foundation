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
#include <basecode/binfmt/io.h>
#include <basecode/core/defer.h>
#include <basecode/binfmt/binfmt.h>
#include <basecode/core/stopwatch.h>
#include "test.h"

using namespace basecode;

TEST_CASE("basecode::binfmt ELF test") {
    using namespace binfmt;

    stopwatch_t timer{};
    stopwatch::start(timer);

    module_t mod{};
    REQUIRE(OK(module::init(mod, 20)));
    defer(module::free(mod));

    /* .rdata section */ {
        auto rdata_rc = module::make_section(mod,
                                             section::type_t::data,
                                             {
                                               .flags = {
                                                   .data = true,
                                                   .init = true,
                                                   .read = true,
                                               }
                                           });
        auto rdata = module::get_section(mod, rdata_rc.id);
        section::data(mod, rdata_rc.id, s_rot13_table, sizeof(s_rot13_table));
        REQUIRE(OK(rdata_rc.status));
        REQUIRE(rdata);
    }

    io::session_t s{};
    io::session::init(s);
    defer(io::session::free(s));

    auto rot13_exe_path = "rot1e.exe"_path;
    defer(path::free(rot13_exe_path));
    auto rot13_exe_file = io::session::add_file(s,
                                                &mod,
                                                rot13_exe_path,
                                                machine::type_t::x86_64,
                                                io::type_t::elf,
                                                io::output_type_t::exe);
    rot13_exe_file->versions.linker.major = 6;
    rot13_exe_file->versions.linker.minor = 0;
    rot13_exe_file->versions.min_os.major = 4;
    rot13_exe_file->versions.min_os.minor = 0;
    rot13_exe_file->flags.console = true;

    REQUIRE(OK(io::write(s)));

    stopwatch::stop(timer);
    stopwatch::print_elapsed("binfmt write ELF executable time"_ss, 40, timer);
}
