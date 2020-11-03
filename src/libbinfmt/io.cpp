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
#include <basecode/core/string.h>

namespace basecode::binfmt::io {
    static alloc_t*             s_alloc{};
    static system_t*            s_systems[max_type_count];

    // N.B. forward declare back-end accessor functions
    namespace pe {
        system_t* system();
    }

    namespace elf {
        system_t* system();
    }

    namespace coff {
        system_t* system();
    }

    namespace macho {
        system_t* system();
    }

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
        for (auto& file : s.files) {
            const auto idx = u32(file.bin_type);
            if (idx > max_type_count - 1)
                return status_t::invalid_container_type;
            auto status =  s_systems[idx]->read(file);
            if (!OK(status))
                return status;
        }
        return status_t::ok;
    }

    status_t write(session_t& s) {
        for (auto& file : s.files) {
            const auto idx = u32(file.bin_type);
            if (idx > max_type_count - 1)
                return status_t::invalid_container_type;
            auto status = s_systems[idx]->write(file);
            if (!OK(status))
                return status;
        }
        return status_t::ok;
    }

    namespace name_map {
        u0 free(name_list_t& names) {
            array::free(names);
        }

        u0 init(name_list_t& names, alloc_t* alloc) {
            array::init(names, alloc);
        }

        u0 add(name_list_t& names, section::type_t type, section::flags_t flags, str::slice_t name) {
            auto entry = &array::append(names);
            entry->type      = type;
            entry->flags     = flags;
            entry->flags.pad = {};
            entry->name      = string::interned::fold(name);
        }

        const name_map_t* find(const name_list_t& names, section::type_t type, section::flags_t flags) {
            for (const auto& entry : names) {
                if (entry.type == type
                &&  std::memcmp(&entry.flags, &flags, sizeof(section::flags_t)) == 0) {
                    return &entry;
                }
            }
            return nullptr;
        }
    }

    namespace file {
        u0 free(file_t& file) {
            path::free(file.path);
        }

        status_t save(file_t& file) {
            auto status = buf::save(file.session->buf, file.path);
            if (!OK(status))
                return status_t::write_error;
            return status_t::ok;
        }

        status_t init(file_t& file, alloc_t* alloc) {
            path::init(file.path, alloc);
            return status_t::ok;
        }
    }

    namespace session {
        u0 free(session_t& s) {
            for (auto& file : s.files)
                file::free(file);
            array::free(s.files);
            buf::free(s.buf);
        }

        u0 write_pad(session_t& s) {
            if ((s.crsr.pos % 2) == 0) return;
            s.buf.data[s.crsr.pos++] = 0;
        }

        file_t* add_file(session_t& s,
                         const module_t* module,
                         const path_t& path,
                         machine::type_t machine,
                         type_t bin_type,
                         output_type_t output_type) {
            return add_file(s, module, path::c_str(path), machine, bin_type, output_type, path.str.length);
        }

        file_t* add_file(session_t& s,
                         const module_t* module,
                         const s8* path,
                         machine::type_t machine,
                         type_t bin_type,
                         output_type_t output_type,
                         s32 path_len) {
            auto file = &array::append(s.files);
            file::init(*file, s.alloc);
            path::set(file->path, path, path_len);
            file->session     = &s;
            file->module      = module;
            file->machine     = machine;
            file->bin_type    = bin_type;
            file->output_type = output_type;
            return file;
        }

        u0 seek(session_t& s, u32 offset) {
            s.crsr.pos = offset;
        }

        u0 write_u8(session_t& s, u8 value) {
            buf::write(s.buf, s.crsr.pos, (const u8*) &value, sizeof(u8));
            s.crsr.pos += sizeof(u8);
        }

        u0 write_u16(session_t& s, u16 value) {
            buf::write(s.buf, s.crsr.pos, (const u8*) &value, sizeof(u16));
            s.crsr.pos += sizeof(u16);
        }

        u0 write_s16(session_t& s, s16 value) {
            buf::write(s.buf, s.crsr.pos, (const u8*) &value, sizeof(s16));
            s.crsr.pos += sizeof(s16);
        }

        u0 write_u32(session_t& s, u32 value) {
            buf::write(s.buf, s.crsr.pos, (const u8*) &value, sizeof(u32));
            s.crsr.pos += sizeof(u32);
        }

        u0 write_u64(session_t& s, u64 value) {
            buf::write(s.buf, s.crsr.pos, (const u8*) &value, sizeof(u64));
            s.crsr.pos += sizeof(u64);
        }

        u0 write_s64(session_t& s, s64 value) {
            buf::write(s.buf, s.crsr.pos, (const u8*) &value, sizeof(s64));
            s.crsr.pos += sizeof(s64);
        }

        status_t init(session_t& s, alloc_t* alloc) {
            s.alloc = alloc;
            array::init(s.files, s.alloc);
            buf::init(s.buf, s.alloc);
            buf::reserve(s.buf, 64 * 1024);
            buf::cursor::init(s.crsr, s.buf);
            return status_t::ok;
        }

        u0 write_pad8(session_t& s, str::slice_t slice) {
            buf::zero_fill(s.buf, s.crsr.pos, 8);
            buf::write(s.buf,
                       s.crsr.pos,
                       slice.data,
                       std::min<u32>(slice.length, 8));
            s.crsr.pos += 8;
        }

        u0 write_cstr(session_t& s, str::slice_t slice) {
            buf::write(s.buf, s.crsr.pos, slice.data, slice.length + 1);
            s.crsr.pos += slice.length + 1;
        }

        u0 write_pad16(session_t& s, str::slice_t slice) {
            buf::zero_fill(s.buf, s.crsr.pos, 16);
            buf::write(s.buf,
                       s.crsr.pos,
                       slice.data,
                       std::min<u32>(slice.length, 16));
            s.crsr.pos += 16;
        }
    }
}
