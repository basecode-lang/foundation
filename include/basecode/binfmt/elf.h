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

#pragma once

#include <basecode/binfmt/types.h>

namespace basecode::binfmt::elf {
    namespace file {
        str::slice_t class_name(u8 cls);

        str::slice_t os_abi_name(u8 os_abi);

        str::slice_t version_name(u8 version);

        str::slice_t file_type_name(u16 type);

        str::slice_t endianess_name(u8 endianess);
    }

    namespace machine {
        str::slice_t name(u16 type);
    }

    namespace dynamic {
        namespace type {
            str::slice_t name(u32 type);
        }
    }

    namespace section {
        namespace type {
            str::slice_t name(u32 type);
        }

        namespace flags {
            const s8* name(u32 flag);

            u0 chars(u32 flags, s8* chars);

            u0 names(u32 flags, const s8* names[13]);
        }
    }

    namespace symtab {
        u64 hash_name(str::slice_t str);

        elf_sym_t* get(const elf_t& elf, u32 sect_num, u32 sym_idx);
    }

    u0 free(elf_t& elf);

    status_t read(elf_t& elf, file_t& file);

    status_t write(elf_t& elf, file_t& file);

    status_t get_section_name(const module_t* module,
                              const binfmt::section_t* section,
                              str::slice_t& name);

    status_t init(elf_t& elf, const elf_opts_t& opts);

    u0 format_report(str_buf_t& buf, const elf_t& elf);

    u32 section_alignment(const binfmt::section_t* section);

    u32 section_file_size(const binfmt::section_t* section);
}
