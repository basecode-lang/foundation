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

#include <basecode/objfmt/container.h>
#include <basecode/objfmt/pe.h>
#include <basecode/objfmt/coff.h>
#include <basecode/objfmt/elf.h>
#include <basecode/objfmt/macho.h>

namespace basecode::objfmt::container {
    static system_t*            s_systems[max_type_count];

    u0 fini() {
        for (auto sys : s_systems)
            sys->fini();
    }

    u0 init(alloc_t* alloc) {
        s_systems[u32(type_t::coff)]  = coff::system();
        s_systems[u32(type_t::pe)]    = pe::system();
        s_systems[u32(type_t::elf)]   = elf::system();
        s_systems[u32(type_t::macho)] = macho::system();

        for (auto sys : s_systems)
            sys->init(alloc);
    }

    system_t* system(type_t type) {
        return s_systems[u32(type)];
    }
}
