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
#include <basecode/binfmt/binfmt.h>

using namespace basecode;

TEST_CASE("basecode::binfmt COFF read obj file") {
    binfmt::session_t s{};
    binfmt::session::init(s);
    defer(binfmt::session::free(s));

    auto backend_obj_path = R"(C:\temp\test\msvc\backend.cpp.obj)"_path;
    defer(path::free(backend_obj_path));

    auto backend_obj = binfmt::session::add_file(s,
                                                 backend_obj_path,
                                                 binfmt::type_t::coff,
                                                 binfmt::file_type_t::obj);
    REQUIRE(backend_obj);
    REQUIRE(OK(binfmt::session::read(s)));
}
