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
        for (auto& file : s.input_files) {
            const auto idx = u32(file.bin_type);
            if (idx > max_type_count - 1)
                return status_t::invalid_container_type;
            if (file.file_type == file_type_t::lib)
                ar::reset(s.ar);
            auto status =  s_systems[idx]->read(file);
            if (!OK(status))
                return status;
        }
        return status_t::ok;
    }

    status_t write(session_t& s) {
        b8 is_archive = s.output_files.size == 1
                        && s.output_files[0].file_type == file_type_t::lib;
        if (is_archive)
            ar::reset(s.ar);
        for (auto& file : s.input_files) {
            const auto idx = u32(file.bin_type);
            if (idx > max_type_count - 1)
                return status_t::invalid_container_type;
            auto status = s_systems[idx]->write(file);
            if (!OK(status))
                return status;
            if (!is_archive) {
                status = file::save(file);
                if (!OK(status))
                    return status;
            }
        }
        if (is_archive)
            return ar::write(s.ar, s.output_files[0].path);
        return status_t::ok;
    }

    namespace name_map {
        u0 free(name_list_t& names) {
            array::free(names);
        }

        u0 add(name_list_t& names,
               section::type_t type,
               section::flags_t flags,
               str::slice_t name) {
            auto entry = &array::append(names);
            entry->type      = type;
            entry->flags     = flags;
            entry->flags.pad = {};
            entry->name      = string::interned::fold(name);
        }

        u0 init(name_list_t& names, alloc_t* alloc) {
            array::init(names, alloc);
        }

        const name_map_t* find(const name_list_t& names,
                               section::type_t type,
                               section::flags_t flags) {
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
            buf::free(file.buf);
            path::free(file.path);
        }

        u0 write_pad(file_t& file) {
            if ((file.crsr.pos % 2) == 0) return;
            file.buf.data[file.crsr.pos++] = 0;
        }

        status_t save(file_t& file) {
            auto status = buf::save(file.buf, file.path);
            if (!OK(status))
                return status_t::write_error;
            return status_t::ok;
        }

        u0 seek(file_t& file, u32 offset) {
            file.crsr.pos = offset;
        }

        u0 write_u8(file_t& file, u8 value) {
            buf::write(file.buf, file.crsr.pos, (const u8*) &value, sizeof(u8));
            file.crsr.pos += sizeof(u8);
        }

        u0 write_u16(file_t& file, u16 value) {
            buf::write(file.buf, file.crsr.pos, (const u8*) &value, sizeof(u16));
            file.crsr.pos += sizeof(u16);
        }

        u0 write_s16(file_t& file, s16 value) {
            buf::write(file.buf, file.crsr.pos, (const u8*) &value, sizeof(s16));
            file.crsr.pos += sizeof(s16);
        }

        u0 write_u32(file_t& file, u32 value) {
            buf::write(file.buf, file.crsr.pos, (const u8*) &value, sizeof(u32));
            file.crsr.pos += sizeof(u32);
        }

        u0 write_u64(file_t& file, u64 value) {
            buf::write(file.buf, file.crsr.pos, (const u8*) &value, sizeof(u64));
            file.crsr.pos += sizeof(u64);
        }

        u0 write_s64(file_t& file, s64 value) {
            buf::write(file.buf, file.crsr.pos, (const u8*) &value, sizeof(s64));
            file.crsr.pos += sizeof(s64);
        }

        status_t init(file_t& file, alloc_t* alloc) {
            path::init(file.path, alloc);
            buf::init(file.buf, alloc);
            buf::reserve(file.buf, 64 * 1024);
            buf::cursor::init(file.crsr, file.buf);
            return status_t::ok;
        }

        u0 write_cstr(file_t& file, str::slice_t slice) {
            buf::write(file.buf, file.crsr.pos, slice.data, slice.length + 1);
            file.crsr.pos += slice.length + 1;
        }

        u0 write_pad8(file_t& file, str::slice_t slice) {
            buf::zero_fill(file.buf, file.crsr.pos, 8);
            buf::write(file.buf,
                       file.crsr.pos,
                       slice.data,
                       std::min<u32>(slice.length, 8));
            file.crsr.pos += 8;
        }

        u0 write_pad16(file_t& file, str::slice_t slice) {
            buf::zero_fill(file.buf, file.crsr.pos, 16);
            buf::write(file.buf,
                       file.crsr.pos,
                       slice.data,
                       std::min<u32>(slice.length, 16));
            file.crsr.pos += 16;
        }
    }

    namespace session {
        u0 free(session_t& s) {
            for (auto& file : s.input_files)
                file::free(file);
            for (auto& file : s.output_files)
                file::free(file);
            array::free(s.input_files);
            array::free(s.output_files);
            ar::free(s.ar);
        }

        file_t* add_input_file(session_t& s,
                               const module_t* module,
                               const path_t& path,
                               machine::type_t machine,
                               type_t bin_type,
                               file_type_t output_type) {
            return add_input_file(s,
                                  module,
                                  path::c_str(path),
                                  machine,
                                  bin_type,
                                  output_type,
                                  path.str.length);
        }

        file_t* add_input_file(session_t& s,
                               const module_t* module,
                               const s8* path,
                               machine::type_t machine,
                               type_t bin_type,
                               file_type_t output_type,
                               s32 path_len) {
            auto file = &array::append(s.input_files);
            file::init(*file, s.alloc);
            path::set(file->path, path, path_len);
            file->session   = &s;
            file->module    = module;
            file->machine   = machine;
            file->bin_type  = bin_type;
            file->file_type = output_type;
            return file;
        }

        status_t init(session_t& s, alloc_t* alloc) {
            s.alloc = alloc;
            ar::init(s.ar, s.alloc);
            array::init(s.input_files, s.alloc);
            array::init(s.output_files, s.alloc);
            return status_t::ok;
        }
    }
}
