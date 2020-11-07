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
#include <basecode/binfmt/binfmt.h>

using namespace basecode;

TEST_CASE("basecode::binfmt read COFF obj file") {
    using namespace binfmt;

    io::session_t s{};
    io::session::init(s);
    defer(io::session::free(s));

    auto backend_obj_path = R"(C:\temp\test\msvc\backend.cpp.obj)"_path;
    defer(path::free(backend_obj_path));

    auto backend_obj = io::session::add_file(s,
                                             backend_obj_path,
                                             io::type_t::coff,
                                             io::file_type_t::obj);
    REQUIRE(backend_obj);
    REQUIRE(OK(io::read(s)));
}
