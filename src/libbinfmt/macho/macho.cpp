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

#include <basecode/binfmt/macho.h>

namespace basecode::binfmt::io::macho {
    u0 free(macho_t& macho) {
        UNUSED(macho);
    }

    status_t read(macho_t& macho, file_t& file) {
        UNUSED(macho);
        UNUSED(file);
        return status_t::ok;
    }

    status_t write(macho_t& macho, file_t& file) {
        UNUSED(macho);
        UNUSED(file);
        return status_t::ok;
    }

    status_t get_section_name(const module_t* module,
                              const binfmt::section_t* section,
                              str::slice_t& name) {
        return status_t::ok;
    }

    status_t init(macho_t& macho, const opts_t& opts) {
        macho.alloc = opts.alloc;
        macho.opts  = &opts;
        return status_t::ok;
    }
}
