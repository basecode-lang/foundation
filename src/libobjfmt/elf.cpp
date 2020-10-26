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

#include <basecode/objfmt/elf.h>

namespace basecode::objfmt::container::elf {
    static u0 fini() {
    }

    static status_t init(alloc_t* alloc) {
        UNUSED(alloc);
        return status_t::ok;
    }

    static status_t read(obj_file_t& file) {
        return status_t::read_error;
    }

    static status_t write(obj_file_t& file) {
        return status_t::write_error;
    }

    system_t                    g_elf_sys {
        .init   = init,
        .fini   = fini,
        .read   = read,
        .write  = write,
        .type   = type_t::elf
    };

    system_t* system() {
        return &g_elf_sys;
    }
}
