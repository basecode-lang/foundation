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

#include <basecode/binfmt/io.h>

namespace basecode::binfmt::io::coff {
    namespace internal {
        struct coff_system_t final {
            alloc_t*                alloc;
            name_list_t             section_names;
        };

        coff_system_t               g_coff_sys{};

        static u0 fini() {
            name_map::free(g_coff_sys.section_names);
        }

        static status_t read(file_t& file) {
            UNUSED(file);
            return status_t::read_error;
        }

        static status_t write(file_t& file) {
            UNUSED(file);
            return status_t::write_error;
        }

        static status_t init(alloc_t* alloc) {
            g_coff_sys.alloc = alloc;

            name_map::init(g_coff_sys.section_names, g_coff_sys.alloc);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::tls,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = true,
                              .alloc = false,
                              .can_free = false
                          },
                          ".tls"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::code,
                          {
                              .code = true,
                              .data = false,
                              .read = true,
                              .exec = true,
                              .write = false,
                              .alloc = false,
                              .can_free = false
                          },
                          ".text"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::data,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = true,
                              .alloc = false,
                              .can_free = false
                          },
                          ".data"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::data,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = false,
                              .alloc = false,
                              .can_free = false
                          },
                          ".rdata"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::debug,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = false,
                              .alloc = false,
                              .can_free = true
                          },
                          ".debug"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::data,
                          {
                              .code = false,
                              .data = true,
                              .init = false,
                              .read = true,
                              .exec = false,
                              .write = true,
                              .alloc = false,
                              .can_free = false
                          },
                          ".bss"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::import,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = true,
                              .alloc = false,
                              .can_free = false
                          },
                          ".idata"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::export_,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = false,
                              .alloc = false,
                              .can_free = false
                          },
                          ".edata"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::resource,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = false,
                              .alloc = false,
                              .can_free = false
                          },
                          ".rsrc"_ss);

            name_map::add(g_coff_sys.section_names,
                          section::type_t::exception,
                          {
                              .code = false,
                              .data = true,
                              .init = true,
                              .read = true,
                              .exec = false,
                              .write = false,
                              .alloc = false,
                              .can_free = false
                          },
                          ".pdata"_ss);

            return status_t::ok;
        }

        system_t                    g_coff_backend {
            .init   = init,
            .fini   = fini,
            .read   = read,
            .write  = write,
            .type   = type_t::coff
        };
    }

    system_t* system() {
        return &internal::g_coff_backend;
    }

    status_t get_section_name(const binfmt::section_t* section, str::slice_t& name) {
        const auto entry = name_map::find(internal::g_coff_sys.section_names, section->type, section->flags);
        if (!entry)
            return status_t::cannot_map_section_name;
        name = entry->name;
        return status_t::ok;
    }
}
