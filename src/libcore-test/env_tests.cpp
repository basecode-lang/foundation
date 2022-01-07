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
#include <basecode/core/env.h>
#include <basecode/core/stopwatch.h>
#include "test.h"

using namespace basecode;

TEST_CASE("basecode::env basics") {
#ifdef _WIN32
    TIME_BLOCK("basecode::env basics"_ss,
               auto root_env = env::system::get_root();
               REQUIRE(!root_env->parent);
               REQUIRE(!hashtab::empty(root_env->vartab));

               env_value_t* val{};

               // N.B. cmd.exe & powershell seem to use a proper-cased
               //      version of this key while MinGW capitalizes it.
               val     = env::get(root_env, "SystemRoot"_ss);
               if (!val)
               val = env::get(root_env, "SYSTEMROOT"_ss);
               REQUIRE(val);
               REQUIRE(val->type == env_value_type_t::string);
               REQUIRE(val->kind.str == "C:\\WINDOWS"_ss);

               val = env::get(root_env, "PATHEXT"_ss);
               REQUIRE(val);
               REQUIRE(val->type == env_value_type_t::array);

               const auto& list = val->kind.list;
               REQUIRE(!array::empty(list));
               REQUIRE(list[0] == ".COM"_ss);
               REQUIRE(list[1] == ".EXE"_ss));

    TIME_BLOCK("basecode::env expand macros"_ss,
               auto root_env = env::system::get_root();
//               format_env(root_env);
               auto val = env::set(root_env,
                                   "HOME_TEST"_ss,
                                   "$HOMEDRIVE$HOMEPATH"_ss);
               str_t expanded{};
               str::init(expanded);
               auto status = env::expand(root_env, val, expanded);
               REQUIRE(OK(status));
               REQUIRE(expanded == "C:\\Users\\jeff"_ss));
#else
    // XXX
#endif
}
