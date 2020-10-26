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
#include <basecode/objfmt/types.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

static const u8 s_rot13_table[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
    55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 'N', 'O', 'P', 'Q', 'R', 'S',
    'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
    'J', 'K', 'L', 'M', 91, 92, 93, 94, 95, 96, 'n', 'o', 'p', 'q', 'r', 's',
    't', 'u', 'v', 'w', 'x', 'y', 'z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150,
    151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166,
    167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182,
    183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198,
    199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214,
    215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230,
    231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246,
    247, 248, 249, 250, 251, 252, 253, 254, 255
};

TEST_CASE("basecode::objfmt rot13 to PE/COFF exe") {
    obj_file_t rot13_pgm{};
    REQUIRE(OK(objfmt::obj_file::init(rot13_pgm)));
    path::set(rot13_pgm.path, "rot13.exe");
    defer(objfmt::obj_file::free(rot13_pgm));

    section_t text{};
    REQUIRE(OK(objfmt::section::init(text, section_type_t::data, ".text"_ss)));
    text.flags.code = true;
    text.flags.read = true;
    text.flags.exec = true;

    section_t rdata{};
    REQUIRE(OK(objfmt::section::init(rdata, section_type_t::data, ".rdata"_ss)));
    objfmt::section::data(rdata, s_rot13_table, sizeof(s_rot13_table));
    rdata.flags.data = true;
    rdata.flags.read = true;

    section_t idata{};
    REQUIRE(OK(objfmt::section::init(idata, section_type_t::import, ".idata"_ss)));
    idata.flags.data  = true;
    idata.flags.read  = true;
    idata.flags.write = true;
    import_t* kernel32{};
    REQUIRE(OK(objfmt::section::import(idata, &kernel32, "kernel32.dll"_ss)));
    REQUIRE(kernel32);

    section_t bss{};
    REQUIRE(OK(objfmt::section::init(bss, section_type_t::uninit, ".bss"_ss)));
    objfmt::section::reserve(bss, 4096);
    bss.flags.data   = true;
    bss.flags.read   = true;
    bss.flags.write  = true;

    REQUIRE(OK(objfmt::container::write(objfmt::container::type_t::pe, rot13_pgm)));
}
