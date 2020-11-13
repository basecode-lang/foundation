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

#define FILE_POS()          CRSR_POS(file.crsr)
#define FILE_PTR()          CRSR_PTR(file.crsr)
#define FILE_POP_POS()      SAFE_SCOPE(buf::cursor::pop(file.crsr);)
#define FILE_PUSH_POS()     SAFE_SCOPE(buf::cursor::push(file.crsr);)
#define FILE_SEEK(p)        SAFE_SCOPE(                 \
    if (!OK(buf::cursor::seek(file.crsr, (p))))         \
        return status_t::read_error;)
#define FILE_SEEK_FWD(p)    SAFE_SCOPE(                 \
    if (!OK(buf::cursor::seek_fwd(file.crsr, (p))))     \
        return status_t::read_error;)
#define FILE_SEEK_RWD(p)    SAFE_SCOPE(                 \
    if (!OK(buf::cursor::seek_rwd(file.crsr, (p))))     \
        return status_t::read_error;)
#define FILE_WRITE(t, v)    SAFE_SCOPE(                 \
    static_assert(std::is_same_v<decltype(v), t>);      \
    if (!OK(buf::cursor::write(file.crsr, (v))))        \
        return status_t::write_error;)
#define FILE_READ(t, v)     SAFE_SCOPE(                 \
    static_assert(std::is_same_v<decltype(v), t>);      \
    if (!OK(buf::cursor::read(file.crsr, (v))))         \
        return status_t::read_error;)
#define FILE_READ_STR(d, l)     SAFE_SCOPE(             \
    if (!OK(buf::cursor::read_str(file.crsr, (d), (l))))\
        return status_t::read_error;)
#define FILE_WRITE_STR(v)   SAFE_SCOPE(                 \
    if (!OK(buf::cursor::write_str(file.crsr, (v))))    \
        return status_t::write_error;)
#define FILE_WRITE_CSTR(v)  SAFE_SCOPE(                 \
    if (!OK(buf::cursor::write_cstr(file.crsr, (v))))   \
        return status_t::write_error;)
#define FILE_WRITE_PAD(l)   SAFE_SCOPE(                 \
    if (!OK(buf::cursor::zero_fill(file.crsr, (l))))    \
        return status_t::write_error;)
#define FILE_WRITE0(T)      SAFE_SCOPE(                 \
    T zero{};                                           \
    if (!OK(buf::cursor::write(file.crsr, zero)))       \
        return status_t::write_error;)

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

    struct name_flags_t final {
        u32                 code:       1;
        u32                 init:       1;
        u32                 exec:       1;
        u32                 write:      1;
        u32                 pad:        28;
    };

    struct name_map_t final {
        str::slice_t            name;
        name_flags_t            flags;
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

        status_t save(file_t& file);

        status_t map_existing(file_t& file);

        status_t init(file_t& file, alloc_t* alloc);

        status_t map_new(file_t& file, usize file_size);

        status_t unmap(file_t& file, b8 sync_flush = false);
    }

    namespace name_map {
        u0 add(name_list_t& names,
               section::type_t type,
               name_flags_t flags,
               str::slice_t name);

        u0 free(name_list_t& names);

        u0 init(name_list_t& names, alloc_t* alloc);

        const name_map_t* find(const name_list_t& names,
                               section::type_t type,
                               name_flags_t flags);
    }

    namespace session {
        u0 free(session_t& s);

        file_t* add_file(session_t& s,
                         const s8* path,
                         type_t bin_type,
                         file_type_t file_type,
                         s32 path_len = -1);

        file_t* add_file(session_t& s,
                         const path_t& path,
                         type_t bin_type,
                         file_type_t file_type);

        file_t* add_file(session_t& s,
                         module_t* module,
                         const s8* path,
                         machine::type_t machine,
                         type_t bin_type,
                         file_type_t output_type,
                         s32 path_len = -1);

        file_t* add_file(session_t& s,
                         module_t* module,
                         const path_t& path,
                         machine::type_t machine,
                         type_t bin_type,
                         file_type_t output_type);

        file_t* add_file(session_t& s,
                        const String_Concept auto& path,
                        type_t bin_type,
                        file_type_t file_type) {
            return add_file(s, (const s8*) path.data, bin_type, file_type, path.length);
        }

        file_t* add_file(session_t& s,
                         module_t* module,
                         const String_Concept auto& path,
                         machine::type_t machine,
                         type_t bin_type,
                         file_type_t output_type) {
            return add_file(s,
                            module,
                            (const s8*) path.data,
                            machine,
                            bin_type,
                            output_type,
                            path.length);
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
