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
        u0 free(hash_t& hash) {
            array::free(hash.buckets);
            array::free(hash.chains);
        }

        header_t& make_section(elf_t& elf,
                               str::slice_t name,
                               const hash_t* hash) {
            auto& hdr = array::append(elf.headers);
            hdr.section = {};
            hdr.number  = ++elf.section.count;
            hdr.type    = header_type_t::section;

            auto& sc = hdr.subclass.section;
            sc.type       = elf::section::type::hash;
            sc.link       = hash->link;
            sc.offset     = elf.data.rel_offset;
            sc.entry_size = elf::hash::entity_size;
            sc.addr.align = sizeof(u64);
            sc.name_index = elf::strtab::add_str(elf.section_names,
                                                 string::interned::fold(name));
            sc.size       = (2 + hash->buckets.size + hash->chains.size) * elf::hash::entity_size;

            auto& block = array::append(elf.blocks);
            block.type      = block_type_t::hash;
            block.offset    = elf.data.rel_offset;
            block.data.hash = hash;

            elf.data.rel_offset = align(elf.data.rel_offset + sc.size, sizeof(u64));

            return hdr;
        }

        u0 rehash(hash_t& hash, u32 size) {
            array::resize(hash.chains, size);
            array::resize(hash.buckets, size * 3);
        }

        u0 init(hash_t& hash, alloc_t* alloc) {
            array::init(hash.buckets, alloc);
            array::init(hash.chains, alloc);
        }

        status_t write(const hash_t& hash, file_t& file) {
            FILE_WRITE(u32, hash.buckets.size);
            FILE_WRITE(u32, hash.chains.size);
            for (auto sym_idx : hash.buckets)
                FILE_WRITE(u32, sym_idx);
            for (auto sym_idx : hash.chains)
                FILE_WRITE(u32, sym_idx);
            return status_t::ok;
        }
    }

    namespace strtab {
        u0 free(strtab_t& strtab) {
            str_array::free(strtab.strings);
        }

        header_t& make_section(elf_t& elf,
                               str::slice_t name,
                               const strtab_t* strtab) {
            auto& hdr = array::append(elf.headers);
            hdr.section = {};
            hdr.number  = ++elf.section.count;
            hdr.type    = header_type_t::section;

            auto& sc = hdr.subclass.section;
            sc.flags      = elf::section::flags::strings;
            sc.type       = elf::section::type::strtab;
            sc.offset     = elf.data.rel_offset;
            sc.entry_size = 0;
            sc.addr.align = 1;
            sc.name_index = elf::strtab::add_str(elf.section_names,
                                                 string::interned::fold(name));
            sc.size       = strtab->strings.buf.size + 1;

            auto& block = array::append(elf.blocks);
            block.type        = block_type_t::strtab;
            block.offset      = elf.data.rel_offset;
            block.data.strtab = strtab;

            elf.data.rel_offset = align(elf.data.rel_offset + sc.size, sizeof(u64));

            return hdr;
        }

        u0 init(strtab_t& strtab, alloc_t* alloc) {
            str_array::init(strtab.strings, alloc);
        }

        status_t write(const strtab_t& strtab, file_t& file) {
            FILE_WRITE0(u8);
            const auto slice = slice::make(strtab.strings.buf.data,
                                           strtab.strings.buf.size);
            FILE_WRITE_STR(slice);
            return status_t::ok;
        }

        u32 add_str(strtab_t& strtab, str::slice_t str) {
            const auto offset = strtab.strings.buf.size + 1;
            str_array::append(strtab.strings, str);
            return offset;
        }
    }

    namespace symtab {
        u0 free(symtab_t& symtab) {
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

        header_t& make_section(elf_t& elf,
                               str::slice_t name,
                               const symtab_t* symtab,
                               u32 first_global_idx) {
            auto& hdr = array::append(elf.headers);
            hdr.section = {};
            hdr.number  = ++elf.section.count;
            hdr.type    = header_type_t::section;

            auto& sc = hdr.subclass.section;
            sc.type       = elf::section::type::symtab;
            sc.link       = symtab->link;
            sc.info       = first_global_idx;
            sc.offset     = elf.data.rel_offset;
            sc.entry_size = elf::symtab::entity_size;
            sc.addr.align = sizeof(u64);
            sc.name_index = elf::strtab::add_str(elf.section_names,
                                                 string::interned::fold(name));
            sc.size       = symtab->symbols.size * elf::symtab::entity_size;

            auto& block = array::append(elf.blocks);
            block.type        = block_type_t::symtab;
            block.offset      = elf.data.rel_offset;
            block.data.symtab = symtab;

            elf.data.rel_offset = align(elf.data.rel_offset + sc.size, sizeof(u64));

            return hdr;
        }

        u0 rehash(symtab_t& symtab, u32 size) {
            hash::rehash(symtab.hash, size);
        }

        status_t write(const symtab_t& symtab, file_t& file) {
            for (const auto& sym : symtab.symbols) {
                FILE_WRITE(u32, sym.name_index);
                FILE_WRITE(u8, sym.info);
                FILE_WRITE(u8, sym.other);
                FILE_WRITE(u16, sym.index);
                FILE_WRITE(u64, sym.value);
                FILE_WRITE(u64, sym.size);
            }
            return status_t::ok;
        }

        status_t add_sym(symtab_t& symtab, const symbol_t* symbol) {
            u32 name_index;
            u32 bucket_idx{};

            if (symbol) {
                const auto intern_rc = string::interned::get(symbol->name);
                if (!OK(intern_rc.status))
                    return status_t::symbol_not_found;
                name_index = elf::strtab::add_str(*symtab.strtab, intern_rc.slice);
                const auto name_hash  = hash_name(intern_rc.slice);
                bucket_idx = name_hash % symtab.hash.buckets.size;
            }

            auto& sym = array::append(symtab.symbols);
            if (symbol) {
                u32 scope{};
                u32 type{};
                u32 vis{};

                switch (symbol->type) {
                    case symbol::type_t::none:
                        type = elf::symtab::type::notype;
                        break;
                    case symbol::type_t::file:
                        type = elf::symtab::type::file;
                        break;
                    case symbol::type_t::object:
                        type = elf::symtab::type::object;
                        break;
                    case symbol::type_t::function:
                        type = elf::symtab::type::func;
                        break;
                }

                switch (symbol->scope) {
                    case symbol::scope_t::none:
                    case symbol::scope_t::local:
                        scope = elf::symtab::scope::local;
                        break;
                    case symbol::scope_t::global:
                        scope = elf::symtab::scope::global;
                        break;
                    case symbol::scope_t::weak:
                        scope = elf::symtab::scope::weak;
                        break;
                }

                switch (symbol->visibility) {
                    case symbol::visibility_t::default_:
                        vis = elf::symtab::visibility::default_;
                        break;
                    case symbol::visibility_t::internal_:
                        vis = elf::symtab::visibility::internal;
                        break;
                    case symbol::visibility_t::hidden:
                        vis = elf::symtab::visibility::hidden;
                        break;
                    case symbol::visibility_t::protected_:
                        vis = elf::symtab::visibility::protected_;
                        break;
                }

                sym.name_index = name_index;
                sym.value      = symbol->value;
                sym.size       = symbol->size;
                sym.info       = ELF64_ST_INFO(scope, type);
                sym.other      = ELF64_ST_VISIBILITY(vis);
                sym.index      = symbol->section;
            }

            u32 sym_idx = symtab.hash.buckets[bucket_idx];
            if (sym_idx == 0) {
                sym_idx = symtab.symbols.size - 1;
                symtab.hash.buckets[bucket_idx]             = sym_idx;
                symtab.hash.chains[symtab.symbols.size - 1] = sym_idx;
            } else {
                while (symtab.hash.chains[sym_idx] != 0)
                    ++sym_idx;
                symtab.hash.chains[sym_idx] = symtab.symbols.size - 1;
            }

            return status_t::ok;
        }

        u0 init(symtab_t& symtab, strtab_t* strtab, alloc_t* alloc) {
            symtab.strtab = strtab;
            hash::init(symtab.hash, alloc);
            array::init(symtab.symbols, alloc);
        }

        status_t find_sym(symtab_t& symtab, str::slice_t name, sym_t** sym) {
            *sym = nullptr;

            const auto name_hash = hash_name(name);
            const auto bucket_idx = name_hash % symtab.hash.buckets.size;

            auto chain_idx = symtab.hash.buckets[bucket_idx];
            while (chain_idx != 0 && chain_idx < symtab.hash.chains.size) {
                auto temp_sym = &symtab.symbols[symtab.hash.chains[chain_idx]];
                auto str_offset = (const s8*) symtab.strtab->strings.buf.data + temp_sym->name_index;
                auto cmp = strncmp(str_offset, (const s8*) name.data, name.length);
                if (cmp == 0) {
                    *sym = temp_sym;
                    return status_t::ok;
                }
                ++chain_idx;
            }

            return status_t::symbol_not_found;
        }
    }

    u0 free(elf_t& elf) {
        array::free(elf.blocks);
        array::free(elf.headers);
        symtab::free(elf.symbols);
        strtab::free(elf.strings);
        strtab::free(elf.section_names);
    }

    status_t init(elf_t& elf, const opts_t& opts) {
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
        ZERO_MEM(&null_section, header_t);
        null_section.type = header_type_t::section;

        switch (file.machine) {
            case type_t::unknown:
                return status_t::invalid_machine_type;
            case type_t::x86_64:
                elf.machine = elf::machine::x86_64;
                break;
            case type_t::aarch64:
                elf.machine = elf::machine::aarch64;
                break;
        }

        switch (file.file_type) {
            case file_type_t::none:
            case file_type_t::obj:
                elf.file_type = elf::file::type::rel;
                break;
            case file_type_t::exe:
                elf.file_type = elf::file::type::exec;
                break;
            case file_type_t::dll:
                elf.file_type = elf::file::type::dyn;
                break;
        }

        return status_t::ok;
    }

    status_t write_blocks(file_t& file, elf_t& elf) {
        for (const auto& block : elf.blocks) {
            FILE_SEEK(block.offset);
            switch (block.type) {
                case block_type_t::slice:
                    FILE_WRITE_STR(*block.data.slice);
                    break;
                case block_type_t::strtab: {
                    auto status = strtab::write(*block.data.strtab, file);
                    if (!OK(status))
                        return status;
                    break;
                }
                case block_type_t::hash: {
                    auto status = hash::write(*block.data.hash, file);
                    if (!OK(status))
                        return status;
                    break;
                }
                case block_type_t::symtab: {
                    auto status = symtab::write(*block.data.symtab, file);
                    if (!OK(status))
                        return status;
                    break;
                }
            }
        }
        return status_t::ok;
    }

    status_t write_segments(file_t& file, elf_t& elf) {
        for (const auto& hdr : elf.headers) {
            if (hdr.type != header_type_t::segment)
                continue;
            auto status = write_header(file, elf, hdr);
            if (!OK(status))
                return status;
        }
        return status_t::ok;
    }

    status_t write_sections(file_t& file, elf_t& elf) {
        for (const auto& hdr : elf.headers) {
            if (hdr.type != header_type_t::section)
                continue;
            auto status = write_header(file, elf, hdr);
            if (!OK(status))
                return status;
        }
        return status_t::ok;
    }

    status_t write_file_header(file_t& file, elf_t& elf) {
        FILE_WRITE(u8, u8(0x7f));
        FILE_WRITE(u8, u8('E'));
        FILE_WRITE(u8, u8('L'));
        FILE_WRITE(u8, u8('F'));
        FILE_WRITE(u8, u8(class_64));
        FILE_WRITE(u8, u8(data_2lsb));
        FILE_WRITE(u8, u8(version_current));
        FILE_WRITE(u8, u8(os_abi_linux));
        FILE_WRITE0(u64);
        FILE_WRITE(u16, elf.file_type);
        FILE_WRITE(u16, elf.machine);
        FILE_WRITE(const u32, version_current);
        FILE_WRITE(u64, elf.entry_point);
        FILE_WRITE(u64, elf.segment.offset);
        FILE_WRITE(u64, elf.section.offset);
        FILE_WRITE(u32, elf.proc_flags);
        FILE_WRITE(const u16, elf::file::header_size);
        FILE_WRITE(u16, u16(elf.segment.count > 0 ? elf::segment::header_size : 0));
        FILE_WRITE(u16, u16(elf.segment.count));
        FILE_WRITE(u16, u16(elf.section.count > 0 ? elf::section::header_size : 0));
        FILE_WRITE(u16, u16(elf.section.count));
        FILE_WRITE(u16, elf.str_ndx);
        return status_t::ok;
    }

    status_t write_dyn(file_t& file, elf_t& elf, const dyn_t& dyn) {
        FILE_WRITE(s64, dyn.tag);
        FILE_WRITE(u64, dyn.value);
        return status_t::ok;
    }

    status_t write_rel(file_t& file, elf_t& elf, const rel_t& rel) {
        FILE_WRITE(u64, rel.offset);
        FILE_WRITE(u64, rel.info);
        return status_t::ok;
    }

    status_t write_note(file_t& file, elf_t& elf, const note_t& note) {
        const auto name_aligned_len = align(note.name.length, sizeof(u64));
        const auto desc_aligned_len = align(note.descriptor.length, sizeof(u64));
        FILE_WRITE(u64, u64(note.name.length));
        FILE_WRITE(u64, u64(note.descriptor.length));
        FILE_WRITE_STR(note.name);
        FILE_WRITE0(u8);
        FILE_WRITE_PAD(std::min<s32>(0, (name_aligned_len - note.name.length) + 1));
        FILE_WRITE_STR(note.descriptor);
        FILE_WRITE_PAD(std::min<s32>(0, (desc_aligned_len - note.descriptor.length) + 1));
        FILE_WRITE0(u8);
        FILE_WRITE(u64, note.type);
        return status_t::ok;
    }

    status_t write_rela(file_t& file, elf_t& elf, const rela_t& rela) {
        FILE_WRITE(u64, rela.offset);
        FILE_WRITE(u64, rela.info);
        FILE_WRITE(s64, rela.addend);
        return status_t::ok;
    }

    status_t write_header(file_t& file, elf_t& elf, const header_t& hdr) {
        switch (hdr.type) {
            case header_type_t::section: {
                auto& sc = hdr.subclass.section;
                FILE_WRITE(u32, sc.name_index);
                FILE_WRITE(u32, sc.type);
                FILE_WRITE(u64, sc.flags);
                FILE_WRITE(u64, sc.addr.base);
                FILE_WRITE(u64, sc.offset);
                FILE_WRITE(u64, sc.size);
                FILE_WRITE(u32, sc.link);
                FILE_WRITE(u32, sc.info);
                FILE_WRITE(u64, sc.addr.align);
                FILE_WRITE(u64, sc.entry_size);
                break;
            }
            case header_type_t::segment: {
                auto& sc = hdr.subclass.segment;
                FILE_WRITE(u32, sc.type);
                FILE_WRITE(u32, sc.flags);
                FILE_WRITE(u64, sc.file.offset);
                FILE_WRITE(u64, sc.addr.virt);
                FILE_WRITE(u64, sc.addr.phys);
                FILE_WRITE(u64, sc.file.size);
                FILE_WRITE(u64, sc.addr.size);
                FILE_WRITE(u64, sc.align);
                break;
            }
            default:
                return status_t::invalid_section_type;
        }
        return status_t::ok;
    }
}
