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

#include <basecode/objfmt/container.h>

namespace basecode::objfmt::container {
    struct rva_t final {
        u32                     base;
        u32                     size;
    };

    struct raw_t final {
        u32                     offset;
        u32                     size;
    };

    struct section_hdr_t final {
        const section_t*        section;
        u32                     rva;
        u32                     size;
        u32                     number;
        u32                     offset;
    };

    struct coff_t final {
        section_hdr_t*          hdrs;
        u32                     num_hdrs;
        u32                     rva;
        u32                     offset;
        struct {
            u32                 opt_hdr;
        }                       size;
        struct {
            u32                 dll;
            u32                 image;
        }                       flags;
        struct {
            u32                 file;
            u32                 section;
        }                       align;
        rva_t                   relocs;
        raw_t                   symbol_table;
        raw_t                   string_table;
    };

    namespace coff {
        [[maybe_unused]] constexpr u32 header_size                      = 0x14;

        namespace section {
            [[maybe_unused]] constexpr u32 header_size                  = 0x28;
            [[maybe_unused]] constexpr u32 code                         = 0x00000020;
            [[maybe_unused]] constexpr u32 init_data                    = 0x00000040;
            [[maybe_unused]] constexpr u32 uninit_data                  = 0x00000080;
            [[maybe_unused]] constexpr u32 align_1                      = 0x00100000;
            [[maybe_unused]] constexpr u32 align_2                      = 0x00200000;
            [[maybe_unused]] constexpr u32 align_4                      = 0x00300000;
            [[maybe_unused]] constexpr u32 align_8                      = 0x00400000;
            [[maybe_unused]] constexpr u32 align_16                     = 0x00500000; // default
            [[maybe_unused]] constexpr u32 align_32                     = 0x00600000;
            [[maybe_unused]] constexpr u32 align_64                     = 0x00700000;
            [[maybe_unused]] constexpr u32 align_128                    = 0x00800000;
            [[maybe_unused]] constexpr u32 align_256                    = 0x00900000;
            [[maybe_unused]] constexpr u32 align_512                    = 0x00a00000;
            [[maybe_unused]] constexpr u32 align_1024                   = 0x00b00000;
            [[maybe_unused]] constexpr u32 align_2048                   = 0x00c00000;
            [[maybe_unused]] constexpr u32 align_4096                   = 0x00d00000;
            [[maybe_unused]] constexpr u32 align_8192                   = 0x00e00000;
            [[maybe_unused]] constexpr u32 link_reloc_overflow          = 0x01000000;
            [[maybe_unused]] constexpr u32 memory_can_discard           = 0x02000000;
            [[maybe_unused]] constexpr u32 memory_not_cached            = 0x04000000;
            [[maybe_unused]] constexpr u32 memory_not_paged             = 0x08000000;
            [[maybe_unused]] constexpr u32 memory_shared                = 0x10000000;
            [[maybe_unused]] constexpr u32 memory_execute               = 0x20000000;
            [[maybe_unused]] constexpr u32 memory_read                  = 0x40000000;
            [[maybe_unused]] constexpr u32 memory_write                 = 0x80000000;
        }

        namespace flags {
            [[maybe_unused]] constexpr u32 relocs_stripped              = 0x0001;
            [[maybe_unused]] constexpr u32 executable_type              = 0x0002;
            [[maybe_unused]] constexpr u32 line_nums_stripped           = 0x0004;
            [[maybe_unused]] constexpr u32 local_syms_stripped          = 0x0008;
            [[maybe_unused]] constexpr u32 aggressive_ws_trim           = 0x0010;
            [[maybe_unused]] constexpr u32 large_address_aware          = 0x0020;
            [[maybe_unused]] constexpr u32 reserved                     = 0x0040;
            [[maybe_unused]] constexpr u32 bytes_reversed_lo            = 0x0080;
            [[maybe_unused]] constexpr u32 machine_32bit                = 0x0100;
            [[maybe_unused]] constexpr u32 debug_stripped               = 0x0200;
            [[maybe_unused]] constexpr u32 removable_run_from_swap      = 0x0400;
            [[maybe_unused]] constexpr u32 net_run_from_swap            = 0x0800;
            [[maybe_unused]] constexpr u32 system_type                  = 0x1000;
            [[maybe_unused]] constexpr u32 dll_type                     = 0x2000;
            [[maybe_unused]] constexpr u32 up_system_only               = 0x4000;
            [[maybe_unused]] constexpr u32 bytes_reversed_hi            = 0x8000;
        }

        system_t* system();

        u0 free(coff_t& coff);

        u0 write_header(session_t& s, coff_t& coff);

        u0 write_section_headers(session_t& s, coff_t& coff);

        str::slice_t get_section_name(objfmt::section::type_t type);

        status_t init(coff_t& coff, section_hdr_t* hdrs, u32 num_hdrs, alloc_t* alloc);
    }
}
