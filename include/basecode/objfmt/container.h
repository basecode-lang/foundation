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
#include <basecode/objfmt/types.h>

namespace basecode::objfmt::container {
    enum class type_t : u8 {
        pe,
        elf,
        coff,
        macho
    };
    constexpr u32 max_type_count = 4;

    enum class output_type_t : u8 {
        obj,
        lib,
        exe,
        dll
    };

    struct session_t final {
        const file_t*           file;
        buf_t                   buf;
        buf_crsr_t              crsr;
        struct {
            u32                 gui:        1;
            u32                 console:    1;
            u32                 pad:        30;
        }                       flags;
        struct {
            version_t           linker;
            version_t           min_os;
        }                       versions;
        type_t                  type;
        output_type_t           output_type;
    };

    namespace session {
        u0 free(session_t& s);

        u0 write_pad(session_t& s);

        status_t init(session_t& s);

        status_t save(session_t& s);

        u0 seek(session_t& s, u32 offset);

        u0 write_u8(session_t& s, u8 value);

        u0 write_u16(session_t& s, u16 value);

        u0 write_s16(session_t& s, s16 value);

        u0 write_u32(session_t& s, u32 value);

        u0 write_u64(session_t& s, u64 value);

        u0 write_pad8(session_t& s, str::slice_t slice);

        u0 write_cstr(session_t& s, str::slice_t slice);

        u0 write_pad16(session_t& s, str::slice_t slice);

        u0 write_str(session_t& s, const String_Concept auto& str) {
            buf::cursor::write_str(s.crsr, str);
        }
    }

    using fini_callback_t       = u0 (*)();
    using init_callback_t       = status_t (*)(alloc_t*);
    using read_callback_t       = status_t (*)(session_t&);
    using write_callback_t      = status_t (*)(session_t&);

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
