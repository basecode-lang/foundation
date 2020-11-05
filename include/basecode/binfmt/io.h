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

#include <basecode/core/buf.h>
#include <basecode/binfmt/types.h>

namespace basecode::binfmt::io {
    struct file_t;
    struct session_t;
    struct name_map_t;

    using name_list_t           = array_t<name_map_t>;
    using file_list_t           = array_t<file_t>;

    enum class type_t : u8 {
        ar,
        pe,
        elf,
        coff,
        macho
    };
    constexpr u32 max_type_count = 5;

    enum class file_type_t : u8 {
        none,
        obj,
        exe,
        dll
    };

    struct name_map_t final {
        str::slice_t            name;
        section::flags_t        flags;
        section::type_t         type;
    };

    struct file_t final {
        session_t*              session;
        module_t*               module;
        path_t                  path;
        buf_t                   buf;
        buf_crsr_t              crsr;
        machine::type_t         machine;
        struct {
            u32                 gui:        1;
            u32                 save:       1;
            u32                 console:    1;
            u32                 pad:        30;
        }                       flags;
        struct {
            u64                 base_addr;
            u64                 heap_reserve;
            u64                 stack_reserve;
        }                       opts;
        struct {
            version_t           linker;
            version_t           min_os;
        }                       versions;
        type_t                  bin_type;
        file_type_t             file_type;
    };

    struct session_t final {
        alloc_t*                alloc;
        file_list_t             files;
    };

    namespace file {
        u0 free(file_t& file);

        u0 write_pad(file_t& file);

        status_t save(file_t& file);

        u0 seek(file_t& file, u32 offset);

        u0 write_u8(file_t& file, u8 value);

        u0 write_u16(file_t& file, u16 value);

        u0 write_s16(file_t& file, s16 value);

        u0 write_u32(file_t& file, u32 value);

        u0 write_u64(file_t& file, u64 value);

        u0 write_s64(file_t& file, s64 value);

        status_t init(file_t& file, alloc_t* alloc);

        u0 write_pad8(file_t& file, str::slice_t slice);

        u0 write_cstr(file_t& file, str::slice_t slice);

        u0 write_pad16(file_t& file, str::slice_t slice);

        u0 write_str(file_t& file, const String_Concept auto& str) {
            buf::cursor::write_str(file.crsr, str);
        }
    }

    namespace name_map {
        u0 add(name_list_t& names,
               section::type_t type,
               section::flags_t flags,
               str::slice_t name);

        u0 free(name_list_t& names);

        u0 init(name_list_t& names, alloc_t* alloc);

        const name_map_t* find(const name_list_t& names,
                               section::type_t type,
                               section::flags_t flags);
    }

    namespace session {
        u0 free(session_t& s);

        file_t* add_file(session_t& s,
                         module_t* module,
                         const s8* path,
                         machine::type_t machine,
                         type_t bin_type,
                         file_type_t output_type,
                         b8 save_to_disk = true,
                         s32 path_len = -1);

        file_t* add_file(session_t& s,
                         module_t* module,
                         const path_t& path,
                         machine::type_t machine,
                         type_t bin_type,
                         file_type_t output_type,
                         b8 save_to_disk = true);

        file_t* add_file(session_t& s,
                         module_t* module,
                         const String_Concept auto& path,
                         machine::type_t machine,
                         type_t bin_type,
                         file_type_t output_type,
                         b8 save_to_disk = true) {
            return add_file(s,
                            module,
                            (const s8*) path.data,
                            machine,
                            bin_type,
                            output_type,
                            path.length,
                            save_to_disk);
        }

        status_t init(session_t& s, alloc_t* alloc = context::top()->alloc);
    }

    using fini_callback_t       = u0 (*)();
    using init_callback_t       = status_t (*)(alloc_t*);
    using read_callback_t       = status_t (*)(file_t&);
    using write_callback_t      = status_t (*)(file_t&);

    struct system_t final {
        init_callback_t         init;
        fini_callback_t         fini;
        read_callback_t         read;
        write_callback_t        write;
        type_t                  type;
    };

    u0 fini();

    status_t read(session_t& s);

    status_t write(session_t& s);

    status_t init(alloc_t* alloc);
}
