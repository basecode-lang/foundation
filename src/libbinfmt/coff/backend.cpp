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

#include <basecode/binfmt/coff.h>

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
            if (file.file_type != file_type_t::obj)
                return status_t::invalid_input_type;

            if (!OK(buf::map(file.buf, file.path)))
                return status_t::read_error;
            defer(buf::unmap(file.buf));

            coff_t coff{};
            auto status = coff::init(coff, file, g_coff_sys.alloc);
            if (!OK(status))
                return status;
            defer(coff::free(coff));

            status = coff::read_header(file, coff);
            if (!OK(status))
                return status_t::read_error;

            status = coff::read_section_headers(file, coff);
            if (!OK(status))
                return status_t::read_error;

            format::print("                                                         c i u l s e r w\n");
            format::print("                                                         o n n i h x e r\n");
            format::print("                                                         d i i n a e a i\n");
            format::print("                                                         e t n k e c d t\n");
            format::print("                                                             i   e     e\n");
            format::print("No Name             RVA      Size     Data Off Data Size     t          \n");
            for (u32 i = 0; i < coff.headers.size; ++i) {
                auto& hdr = coff.headers[i];
                format::print("{:02} ", hdr.number);
                if (hdr.symbol && hdr.symbol->inlined) {
                    format::print("{:<16} ", hdr.symbol->name.slice);
                } else {
                    format::print("{:16} ", "<STRTAB>");
                }
                format::print("{:08x} {:08x} {:08x} {:08x}  ",
                              hdr.rva.base,
                              hdr.rva.size,
                              hdr.file.offset,
                              hdr.file.size);
                format::print("{} ", (hdr.flags & section::code) == section::code ? "X" : " ");
                format::print("{} ", (hdr.flags & section::init_data) == section::init_data ? "X" : " ");
                format::print("{} ", (hdr.flags & section::uninit_data) == section::uninit_data ? "X" : " ");
                if ((hdr.flags & section::link_info) == section::link_info) {
                    format::print("I ");
                } else if ((hdr.flags & section::link_comdat) == section::link_comdat) {
                    format::print("C ");
                } else {
                    format::print("  ");
                }
                format::print("{} ", (hdr.flags & section::memory_shared) == section::memory_shared ? "X" : " ");
                format::print("{} ", (hdr.flags & section::memory_execute) == section::memory_execute ? "X" : " ");
                format::print("{} ", (hdr.flags & section::memory_read) == section::memory_read ? "X" : " ");
                format::print("{} ", (hdr.flags & section::memory_write) == section::memory_write ? "X" : " ");
                format::print("\n");
            }

            return status_t::ok;
        }

        static status_t write(file_t& file) {
            UNUSED(file);
            return status_t::write_error;
        }

        static status_t init(alloc_t* alloc) {
            using type_t = basecode::binfmt::section::type_t;

            g_coff_sys.alloc = alloc;

            name_map::init(g_coff_sys.section_names, g_coff_sys.alloc);

            name_map::add(g_coff_sys.section_names,
                          type_t::tls,
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
                          type_t::code,
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
                          type_t::data,
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
                          type_t::data,
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
                          type_t::debug,
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
                          type_t::data,
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
                          type_t::import,
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
                          type_t::export_,
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
                          type_t::resource,
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
                          type_t::exception,
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
