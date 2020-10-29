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
    namespace file {
        enum class type_t : u8 {
            none,
            rel,
            exec,
            dyn,
            core
        };
    }

    namespace dynamic {
        enum class type_t : u8 {
            null_,
            needed,
            plt_rel_size,
            plt_got,
            hash,
            strtab,
            symtab,
            rela,
            rela_size,
            rela_ent,
            strsz,
            sym_ent,
            init,
            fini,
            soname,
            rpath,
            symbolic,
            rel,
            rel_size,
            rel_ent,
            plt_rel,
            debug,
            text_rel,
            encoding
        };
    }

    namespace segment {
        enum class type_t : u32 {
            null_,
            load,
            dynamic,
            interp,
            note,
            shlib,
            pgm_hdr,
            tls,
        };
    }

    namespace section {
        enum class type_t : u8 {
            null_,
            progbits,
            symtab,
            strtab,
            rela,
            hash,
            dynamic,
            note,
            nobits,
            rel,
            shlib,
            dynsym,
        };

        namespace indexes {
            [[maybe_unused]] constexpr u32 undef            = 0;
            [[maybe_unused]] constexpr u32 live_patch       = 0xff20;
            [[maybe_unused]] constexpr u32 live_abs         = 0xfff1;
            [[maybe_unused]] constexpr u32 live_common      = 0xfff2;
        }

        namespace flags {
            [[maybe_unused]] constexpr u32 write            = 0x1;
            [[maybe_unused]] constexpr u32 alloc            = 0x2;
            [[maybe_unused]] constexpr u32 exec_instr       = 0x4;
            [[maybe_unused]] constexpr u32 rela_live_patch  = 0x0010000;
            [[maybe_unused]] constexpr u32 ro_after_init    = 0x0020000;
            [[maybe_unused]] constexpr u32 mask_proc        = 0xf000000;
        }

        namespace program_header {
            enum class permissions_t : u8 {
                read    = 0x4,
                write   = 0x2,
                exec    = 0x1,
            };
        }
    }

    struct section_hdr_t final {
        const section_t*        section;
        struct {
            str::slice_t        slice;
            u32                 ndx;
        }                       name;
        u64                     addr;
        u64                     size;
        u64                     align;
        u64                     flags;
        u64                     offset;
        u64                     entry_size;
        u32                     number;
        u32                     link;
        u32                     info;
        section::type_t         type;
    };

    struct program_hdr_t final {
    };

    struct elf_opts_t final {
        alloc_t*                alloc;
        struct {
            section_hdr_t*      section;
            program_hdr_t*      program;
            u16                 num_section;
            u16                 num_program;
            u16                 size_section;
            u16                 size_program;
        }                       headers;
    };

    struct elf_t final {
        alloc_t*                alloc;
        struct {
            section_hdr_t*      section;
            program_hdr_t*      program;
            u16                 num_section;
            u16                 num_program;
            u16                 size_section;
            u16                 size_program;
        }                       headers;
        section_hdr_t*          hdrs;
        u64                     entry_point;
        u16                     machine;
        u16                     header_size;
        struct {
            u64                 offset;
        }                       program;
        struct {
            u64                 offset;
        }                       section;
        u32                     proc_flags;
        u16                     str_ndx;
    };

    namespace symbol_table {
        enum class type_t : u8 {
            notype,
            object,
            func,
            section,
            file,
            common,
            tls
        };

        enum class scope_t : u8 {
            local,
            global,
            weak
        };
    }

    namespace elf {
        [[maybe_unused]] constexpr u32 class_64        = 2;
        [[maybe_unused]] constexpr u32 data_2lsb       = 1;
        [[maybe_unused]] constexpr u32 data_2msb       = 2;
        [[maybe_unused]] constexpr u32 version_current = 1;
        [[maybe_unused]] constexpr u32 os_abi_linux    = 3;

        system_t* system();

        u0 free(elf_t& elf);

        u0 write_header(session_t& s, elf_t& elf);

        status_t init(elf_t& elf, const elf_opts_t& opts);

        u0 write_section_header(session_t& s, elf_t& elf, section_hdr_t& hdr);

        u0 write_pgm_section_header(session_t& s, elf_t& elf, program_hdr_t& hdr);

        status_t get_section_name(const objfmt::section_t* section, str::slice_t& name);
    }
}
