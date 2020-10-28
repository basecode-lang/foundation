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

#include <basecode/objfmt/types.h>

namespace basecode::objfmt::container {
    enum class type_t : u8 {
        pe,
        elf,
        coff,
        macho
    };
    constexpr u32 max_type_count = 4;

    enum class output_type_t : u8 {
        obj,
        lib,
        exe,
        dll
    };

    struct context_t final {
        const file_t*           file;
        struct {
            u32                 gui:        1;
            u32                 console:    1;
            u32                 pad:        30;
        }                       flags;
        struct {
            version_t           linker;
            version_t           min_os;
        }                       versions;
        type_t                  type;
        output_type_t           output_type;
    };

    using fini_callback_t       = u0 (*)();
    using init_callback_t       = status_t (*)(alloc_t*);
    using read_callback_t       = status_t (*)(const context_t&);
    using write_callback_t      = status_t (*)(const context_t&);

    struct system_t final {
        init_callback_t         init;
        fini_callback_t         fini;
        read_callback_t         read;
        write_callback_t        write;
        type_t                  type;
    };

    u0 fini();

    status_t init(alloc_t* alloc);

    status_t read(const context_t& ctx);

    status_t write(const context_t& ctx);
}
