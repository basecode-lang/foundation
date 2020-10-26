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

#pragma once

#include <basecode/core/types.h>

namespace basecode::objfmt::container {
    struct obj_file_t;

    enum class status_t : u8 {
        ok,
        init_failure,
        fini_failure,
        read_error,
        write_error,
    };

    using fini_callback_t       = status_t (*)();
    using init_callback_t       = status_t (*)(alloc_t*);
    using read_callback_t       = status_t (*)(obj_file_t&);
    using write_callback_t      = status_t (*)(obj_file_t&);

    enum class type_t : u8 {
        coff,
        pe,
        elf,
        macho
    };
    constexpr u32 max_type_count = 4;

    struct system_t final {
        init_callback_t         init;
        fini_callback_t         fini;
        read_callback_t         read;
        write_callback_t        write;
        type_t                  type;
    };

    u0 fini();

    u0 init(alloc_t* alloc);

    system_t* system(type_t type);
}
