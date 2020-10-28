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

#include <basecode/objfmt/types.h>
#include <basecode/objfmt/pe.h>
#include <basecode/objfmt/elf.h>
#include <basecode/objfmt/macho.h>

namespace basecode::objfmt::container {
    static alloc_t*             s_alloc{};
    static system_t*            s_systems[max_type_count];

    u0 fini() {
        for (auto sys : s_systems)
            sys->fini();
    }

    status_t init(alloc_t* alloc) {
        s_alloc = alloc;

        s_systems[u32(type_t::pe)]    = pe::system();
        s_systems[u32(type_t::elf)]   = elf::system();
        s_systems[u32(type_t::coff)]  = coff::system();
        s_systems[u32(type_t::macho)] = macho::system();

        for (auto sys : s_systems) {
            auto status = sys->init(alloc);
            if (!OK(status))
                return status;
        }

        return status_t::ok;
    }

    status_t read(session_t& s) {
        const auto idx = u32(s.type);
        if (idx > max_type_count - 1)
            return status_t::invalid_container_type;
        return s_systems[idx]->read(s);
    }

    status_t write(session_t& s) {
        const auto idx = u32(s.type);
        if (idx > max_type_count - 1)
            return status_t::invalid_container_type;
        return s_systems[idx]->write(s);
    }

    namespace session {
        u0 free(session_t& s) {
            buf::free(s.buf);
        }

        u0 write_pad(session_t& s) {
            if ((s.crsr.pos % 2) == 0) return;
            buf::cursor::write_u8(s.crsr, 0);
        }

        status_t save(session_t& s) {
            auto status = buf::save(s.buf, s.file->path);
            if (!OK(status))
                return status_t::write_error;
            return status_t::ok;
        }

        status_t init(session_t& s) {
            buf::init(s.buf, s_alloc);
            buf::reserve(s.buf, 64 * 1024);
            buf::cursor::init(s.crsr, s.buf);
            return status_t::ok;
        }

        u0 seek(session_t& s, u32 offset) {
            buf::cursor::seek(s.crsr, offset);
        }

        u0 write_u8(session_t& s, u8 value) {
            buf::cursor::write_u8(s.crsr, value);
        }

        u0 write_u16(session_t& s, u16 value) {
            buf::cursor::write_u16(s.crsr, value);
        }

        u0 write_u32(session_t& s, u32 value) {
            buf::cursor::write_u32(s.crsr, value);
        }

        u0 write_u64(session_t& s, u64 value) {
            buf::cursor::write_u64(s.crsr, value);
        }

        u0 write_pad8(session_t& s, str::slice_t slice) {
            for (u32 i = 0; i < 8; ++i)
                buf::cursor::write_u8(s.crsr, i < slice.length ? slice[i] : 0);
        }

        u0 write_cstr(session_t& s, str::slice_t slice) {
            for (u32 i = 0; i < slice.length; ++i)
                buf::cursor::write_u8(s.crsr, slice[i]);
            buf::cursor::write_u8(s.crsr, 0);
        }

        u0 write_pad16(session_t& s, str::slice_t slice) {
            for (u32 i = 0; i < 16; ++i)
                buf::cursor::write_u8(s.crsr, i < slice.length ? slice[i] : 0);
        }
    }
}
