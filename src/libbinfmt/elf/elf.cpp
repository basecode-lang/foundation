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

#include <basecode/core/bits.h>
#include <basecode/binfmt/elf.h>
#include <basecode/core/string.h>

namespace basecode::binfmt::io::elf {
    namespace hash {
        u0 free(elf_hash_t& hash) {
            array::free(hash.buckets);
        }

        elf_header_t& make_section(elf_t& elf,
                                   str::slice_t name,
                                   const elf_hash_t* hash) {
            auto& hdr = array::append(elf.headers);
            hdr.section = {};
            hdr.number  = ++elf.section.count;
            hdr.type    = elf_header_type_t::section;

            auto& sc = hdr.subclass.section;
            sc.type       = elf::section::type::hash;
            sc.link       = hash->link;
            sc.offset     = elf.data.rel_offset;
            sc.entry_size = elf::hash::entity_size;
            sc.addr.align = sizeof(u64);
            sc.name_index = elf::strtab::add_str(elf.section_names,
                                                 string::interned::fold(name));
            sc.size       = (2 + hash->buckets.size + hash->num_sym) * elf::hash::entity_size;

            auto& block = array::append(elf.blocks);
            block.type      = elf_block_type_t::hash;
            block.offset    = elf.data.rel_offset;
            block.data.hash = hash;

            elf.data.rel_offset = align(elf.data.rel_offset + sc.size, sizeof(u64));

            return hdr;
        }

        u0 resize(elf_hash_t& hash, u32 size) {
            array::resize(hash.buckets, size);
        }

        u0 init(elf_hash_t& hash, alloc_t* alloc) {
            array::init(hash.buckets, alloc);
        }

        u0 write(const elf_hash_t& hash, file_t& file) {
            auto& s     = *file.session;
            session::write_u32(s, hash.buckets.size);
            session::write_u32(s, hash.num_sym);
            for (auto sym_idx : hash.buckets) {
                session::write_u32(s, sym_idx);
            }
            for (u32  i = 0; i < hash.num_sym; ++i) {
                session::write_u32(s, 0);
            }
        }
    }

    namespace strtab {
        u0 free(elf_strtab_t& strtab) {
            array::free(strtab.strings);
        }

        elf_header_t& make_section(elf_t& elf,
                                   str::slice_t name,
                                   const elf_strtab_t* strtab) {
            auto& hdr = array::append(elf.headers);
            hdr.section = {};
            hdr.number  = ++elf.section.count;
            hdr.type    = elf_header_type_t::section;

            auto& sc = hdr.subclass.section;
            sc.flags      = elf::section::flags::strings;
            sc.type       = elf::section::type::strtab;
            sc.offset     = elf.data.rel_offset;
            sc.entry_size = 0;
            sc.addr.align = 1;
            sc.name_index = elf::strtab::add_str(elf.section_names,
                                                 string::interned::fold(name));
            sc.size       = strtab->size;

            auto& block = array::append(elf.blocks);
            block.type        = elf_block_type_t::strtab;
            block.offset      = elf.data.rel_offset;
            block.data.strtab = strtab;

            elf.data.rel_offset = align(elf.data.rel_offset + sc.size, sizeof(u64));

            return hdr;
        }

        u0 init(elf_strtab_t& strtab, alloc_t* alloc) {
            array::init(strtab.strings, alloc);
            strtab.size = 1;
        }

        u0 write(const elf_strtab_t& strtab, file_t& file) {
            auto& s = *file.session;
            session::write_u8(s, 0);
            for (const auto& str : strtab.strings) {
                session::write_cstr(s, str);
            }
        }

        u32 add_str(elf_strtab_t& strtab, str::slice_t str) {
            const auto offset = strtab.size;
            array::append(strtab.strings, str);
            strtab.size += str.length + 1;
            return offset;
        }
    }

    namespace symtab {
        u0 free(elf_symtab_t& symtab) {
            hash::free(symtab.hash);
            array::free(symtab.symbols);
        }

        u64 hash_name(str::slice_t str) {
            u64       h = 0, g;
            for (auto ch : str) {
                h      = (h << u32(4)) + ch;
                if ((g = h & 0xf0000000))
                    h ^= g >> u32(24);
                h &= u32(0x0fffffff);
            }
            return h;
        }

        elf_header_t& make_section(elf_t& elf,
                                   str::slice_t name,
                                   const elf_symtab_t* symtab) {
            auto& hdr = array::append(elf.headers);
            hdr.section = {};
            hdr.number  = ++elf.section.count;
            hdr.type    = elf_header_type_t::section;

            auto& sc = hdr.subclass.section;
            sc.type       = elf::section::type::symtab;
            sc.link       = symtab->link;
            sc.offset     = elf.data.rel_offset;
            sc.entry_size = elf::symtab::entity_size;
            sc.addr.align = sizeof(u64);
            sc.name_index = elf::strtab::add_str(elf.section_names,
                                                 string::interned::fold(name));
            sc.size       = symtab->symbols.size * elf::symtab::entity_size;

            auto& block = array::append(elf.blocks);
            block.type        = elf_block_type_t::symtab;
            block.offset      = elf.data.rel_offset;
            block.data.symtab = symtab;

            elf.data.rel_offset = align(elf.data.rel_offset + sc.size, sizeof(u64));

            return hdr;
        }

        u0 resize(elf_symtab_t& symtab, u32 size) {
            hash::resize(symtab.hash, size);
        }

        u0 write(const elf_symtab_t& symtab, file_t& file) {
            auto           & s = *file.session;
            for (const auto& sym : symtab.symbols) {
                session::write_u32(s, sym.name_index);
                session::write_u8(s, sym.info);
                session::write_u8(s, sym.other);
                session::write_u16(s, sym.index);
                session::write_u64(s, sym.value);
                session::write_u64(s, sym.size);
            }
        }

        status_t add_sym(elf_symtab_t& symtab, const symbol_t* symbol) {
            const auto intern_rc = string::interned::get(symbol->name);
            if (!OK(intern_rc.status))
                return status_t::symbol_not_found;

            auto& sym = array::append(symtab.symbols);
            sym.name_index = elf::strtab::add_str(*symtab.strtab, intern_rc.slice);
            sym.value      = symbol->value;
            sym.size       = symbol->length;
            sym.info       = {};
            sym.other      = {};
            sym.index      = symbol->section;

            const auto name_hash  = hash_name(intern_rc.slice);
            const auto bucket_idx = name_hash % symtab.hash.buckets.size;
            symtab.hash.buckets[bucket_idx] = symtab.symbols.size - 1;
            ++symtab.hash.num_sym;

            return status_t::ok;
        }

        u0 init(elf_symtab_t& symtab, elf_strtab_t* strtab, alloc_t* alloc) {
            symtab.strtab = strtab;
            hash::init(symtab.hash, alloc);
            array::init(symtab.symbols, alloc);
        }
    }

    u0 free(elf_t& elf) {
        array::free(elf.blocks);
        array::free(elf.headers);
        symtab::free(elf.symbols);
        strtab::free(elf.strings);
        strtab::free(elf.section_names);
    }

    u0 write_blocks(file_t& file, elf_t& elf) {
        auto           & s = *file.session;
        for (const auto& block : elf.blocks) {
            session::seek(s, block.offset);
            switch (block.type) {
                case elf_block_type_t::slice:session::write_str(s, *block.data.slice);
                    break;
                case elf_block_type_t::strtab:strtab::write(*block.data.strtab, file);
                    break;
                case elf_block_type_t::hash:hash::write(*block.data.hash, file);
                    break;
                case elf_block_type_t::symtab:symtab::write(*block.data.symtab, file);
                    break;
            }
        }
    }

    u0 write_file_header(file_t& file, elf_t& elf) {
        auto& s = *file.session;
        session::write_u8(s, 0x7f);
        session::write_u8(s, 'E');
        session::write_u8(s, 'L');
        session::write_u8(s, 'F');
        session::write_u8(s, class_64);
        session::write_u8(s, data_2lsb);
        session::write_u8(s, version_current);
        session::write_u8(s, os_abi_linux);
        for (u32 i = 0; i < 8; ++i) {
            session::write_u8(s, 0);
        }
        session::write_u16(s, elf.file_type);
        session::write_u16(s, elf.machine);
        session::write_u32(s, version_current);
        session::write_u64(s, elf.entry_point);
        session::write_u64(s, elf.segment.offset);
        session::write_u64(s, elf.section.offset);
        session::write_u32(s, elf.proc_flags);
        session::write_u16(s, elf::file::header_size);
        session::write_u16(s, elf::segment::header_size);
        session::write_u16(s, elf.segment.count);
        session::write_u16(s, elf::section::header_size);
        session::write_u16(s, elf.section.count);
        session::write_u16(s, elf.str_ndx);
    }

    status_t write_segments(file_t& file, elf_t& elf) {
        for (const auto& hdr : elf.headers) {
            if (hdr.type != elf_header_type_t::segment)
                continue;
            auto status = write_header(file, elf, hdr);
            if (!OK(status))
                return status;
        }
        return status_t::ok;
    }

    status_t write_sections(file_t& file, elf_t& elf) {
        for (const auto& hdr : elf.headers) {
            if (hdr.type != elf_header_type_t::section)
                continue;
            auto status = write_header(file, elf, hdr);
            if (!OK(status))
                return status;
        }
        return status_t::ok;
    }

    status_t init(elf_t& elf, const elf_opts_t& opts) {
        using type_t = binfmt::machine::type_t;

        const auto& file = *opts.file;

        elf.alloc = opts.alloc;
        array::init(elf.blocks, elf.alloc);
        array::init(elf.headers, elf.alloc);
        strtab::init(elf.strings, elf.alloc);
        strtab::init(elf.section_names, elf.alloc);
        symtab::init(elf.symbols, &elf.strings, elf.alloc);

        elf.entry_point = opts.entry_point == 0 ? 0x00400000 : opts.entry_point;
        ++elf.section.count;

        auto& null_section = array::append(elf.headers);
        ZERO_MEM(&null_section, elf_header_t);
        null_section.type = elf_header_type_t::section;

        switch (file.machine) {
            case type_t::unknown:return status_t::invalid_machine_type;
            case type_t::x86_64:elf.machine = elf::machine::x86_64;
                break;
            case type_t::aarch64:elf.machine = elf::machine::aarch64;
                break;
        }

        switch (file.output_type) {
            case output_type_t::obj:elf.file_type = elf::file::type::rel;
                break;
            case output_type_t::lib:elf.file_type = elf::file::type::none;
                break;
            case output_type_t::exe:elf.file_type = elf::file::type::exec;
                break;
            case output_type_t::dll:elf.file_type = elf::file::type::dyn;
                break;
        }

        return status_t::ok;
    }

    u0 write_dyn(file_t& file, elf_t& elf, const elf_dyn_t& dyn) {
        auto& s = *file.session;
        session::write_s64(s, dyn.tag);
        session::write_u64(s, dyn.value);
    }

    u0 write_rel(file_t& file, elf_t& elf, const elf_rel_t& rel) {
        auto& s = *file.session;
        session::write_u64(s, rel.offset);
        session::write_u64(s, rel.info);
    }

    u0 write_note(file_t& file, elf_t& elf, const elf_note_t& note) {
        auto& s = *file.session;
        const auto name_aligned_len = align(note.name.length, sizeof(u64));
        const auto desc_aligned_len = align(note.descriptor.length, sizeof(u64));
        session::write_u64(s, note.name.length);
        session::write_u64(s, note.descriptor.length);
        session::write_str(s, note.name);
        session::write_u8(s, 0);
        for (u32 i = 0; i < std::min<s32>(0, (name_aligned_len - note.name.length) + 1); ++i) {
            session::write_u8(s, 0);
        }
        session::write_str(s, note.descriptor);
        for (u32 i = 0; i < std::min<s32>(0, (desc_aligned_len - note.descriptor.length) + 1); ++i) {
            session::write_u8(s, 0);
        }
        session::write_u8(s, 0);
        session::write_u64(s, note.type);
    }

    u0 write_rela(file_t& file, elf_t& elf, const elf_rela_t& rela) {
        auto& s = *file.session;
        session::write_u64(s, rela.offset);
        session::write_u64(s, rela.info);
        session::write_s64(s, rela.addend);
    }

    status_t write_header(file_t& file, elf_t& elf, const elf_header_t& hdr) {
        auto& s = *file.session;
        switch (hdr.type) {
            case elf_header_type_t::section: {
                auto& sc = hdr.subclass.section;
                session::write_u32(s, sc.name_index);
                session::write_u32(s, sc.type);
                session::write_u64(s, sc.flags);
                session::write_u64(s, sc.addr.base);
                session::write_u64(s, sc.offset);
                session::write_u64(s, sc.size);
                session::write_u32(s, sc.link);
                session::write_u32(s, sc.info);
                session::write_u64(s, sc.addr.align);
                session::write_u64(s, sc.entry_size);
                break;
            }
            case elf_header_type_t::segment: {
                auto& sc = hdr.subclass.segment;
                session::write_u32(s, sc.type);
                session::write_u32(s, sc.flags);
                session::write_u64(s, sc.file.offset);
                session::write_u64(s, sc.addr.virt);
                session::write_u64(s, sc.addr.phys);
                session::write_u64(s, sc.file.size);
                session::write_u64(s, sc.addr.size);
                session::write_u64(s, sc.align);
                break;
            }
            default:return status_t::invalid_section_type;
        }
        return status_t::ok;
    }
}
