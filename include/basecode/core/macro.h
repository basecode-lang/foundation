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

#pragma once

#include <basecode/core/types.h>
#include <basecode/core/array.h>

namespace basecode {
    struct macro_t final {
        u32                     id;
    };

    //  .macro(quux, n: int, f: bool, lhs: ast, rhs: ast) {
    //      .local(i: int)
    //      .loop(0 .. n : i) {
    //          .before-all(f)  { "Let's get ready to RUMBLE!!!\n" }
    //          .each           { lhs && " is " && rhs             }
    //          .even           { "------------------\n"           }
    //          .odd            { "==================\n"           }
    //      }
    //  }
    //
    //  Hello, ${first_name}!
    //
    //  .label(left) This special macro quux, does something .label(right) funny:
    //  ${quux(3, true, left, right)}.
    //
    namespace macro {
        namespace system {
            u0 fini();

            status_t init(alloc_t* alloc = context::top()->alloc.main);
        }
    }
}
