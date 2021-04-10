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

#include <catch.hpp>
#include <basecode/binfmt/io.h>
#include <basecode/binfmt/ar.h>
#include <basecode/core/stopwatch.h>

using namespace basecode;

TEST_CASE("basecode::binfmt ar read test") {
    using namespace binfmt;

    stopwatch_t timer{};
    stopwatch::start(timer);

#ifdef _WIN32
    auto lib_path = R"(C:\temp\test\msvc\basecode-binfmt.lib)"_path;
//    auto lib_path = R"(C:\temp\test\clang\libbasecode-binfmt.a)"_path;
#else
    auto lib_path = R"(/home/jeff/temp/libbasecode-binfmt.a)"_path;
#endif

    ar::ar_t ar{};
    ar::init(ar);
    defer({
        ar::free(ar);
        path::free(lib_path);
    });

    REQUIRE(OK(ar::read(ar, lib_path)));

    stopwatch::stop(timer);

    str_t s{};
    str::init(s);
    {
        str_buf_t buf(&s);
        for (const auto& member : ar.members) {
            format::format_to(buf, "file . . . . . . . {}\n", member.name);
            format::format_to(buf, "header offset  . . {}\n", member.offset.header);
            format::format_to(buf, "data offset  . . . {}\n", member.offset.data);
            format::format_to(buf, "size . . . . . . . ");
            format::unitized_byte_size(buf, member.content.length);
            format::format_to(buf, "\ndate . . . . . . . {:%Y-%m-%d %H:%M:%S}\n", fmt::localtime(member.date));
            format::format_to(buf, "user id  . . . . . {}\n", member.uid);
            format::format_to(buf, "group id . . . . . {}\n", member.gid);
            format::format_to(buf, "mode . . . . . . . {}\n\n", member.mode);
            format::format_hex_dump(buf, member.content.data, 128, false);
            format::format_to(buf, "\n----\n\n");
        }

        for (const auto& pair : ar.symbol_map) {
            format::format_to(buf, "symbol . . . . . . . {}\n", pair.key);
            format::format_to(buf, "bitmap offset  . . . {}\n", pair.value);
            format::format_to(buf, "found in members:\n");
            for (u32 i = 0; i < ar.members.size; ++i) {
                if (bitset::read(ar.symbol_module_bitmap, pair.value + i)) {
                    const auto& member = ar.members[i];
                    format::format_to(buf, "  {:>04}: {}\n", i, member.name);
                }
            }
            format::format_to(buf, "\n");
        }
    }
    format::print("{}", s);

    stopwatch::print_elapsed("binfmt ar read time"_ss, 40, timer);
}

