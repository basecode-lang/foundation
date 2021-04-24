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

#include <basecode/core/bits.h>
#include <basecode/binfmt/ar.h>

namespace basecode::binfmt::archive {
    namespace internal {
        static u0 fini();

        static status_t read(file_t& file);

        static status_t write(file_t& file);

        static status_t init(alloc_t* alloc);

        io_system_t             g_archive_backend {
            .init                   = init,
            .fini                   = fini,
            .read                   = read,
            .write                  = write,
            .type                   = type_t::ar
        };

        struct archive_system_t final {
            alloc_t*                alloc;
        };

        archive_system_t            g_archive_sys{};

        static u0 fini() {
        }

        static status_t read(file_t& file) {
            return status_t::ok;
        }

        static status_t write(file_t& file) {
            return status_t::ok;
        }

        static status_t init(alloc_t* alloc) {
            g_archive_sys.alloc = alloc;
            return status_t::ok;
        }
    }

    io_system_t* system() {
        return &internal::g_archive_backend;
    }
}

