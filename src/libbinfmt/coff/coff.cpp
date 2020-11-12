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
#include <basecode/binfmt/coff.h>
#include <basecode/core/string.h>
#include <basecode/core/numbers.h>

namespace basecode::binfmt::io::coff {
    namespace reloc {
        namespace type::x86_64 {
            static str::slice_t s_names[] = {
                "IMAGE_REL_AMD64_ABSOLUTE"_ss,
                "IMAGE_REL_AMD64_ADDR64"_ss,
                "IMAGE_REL_AMD64_ADDR32"_ss,
                "IMAGE_REL_AMD64_ADDR32NB"_ss,
                "IMAGE_REL_AMD64_REL32"_ss,
                "IMAGE_REL_AMD64_REL32_1"_ss,
                "IMAGE_REL_AMD64_REL32_2"_ss,
                "IMAGE_REL_AMD64_REL32_3"_ss,
                "IMAGE_REL_AMD64_REL32_4"_ss,
                "IMAGE_REL_AMD64_REL32_5"_ss,
                "IMAGE_REL_AMD64_SECTION"_ss,
                "IMAGE_REL_AMD64_SECREL"_ss,
                "IMAGE_REL_AMD64_SECREL7"_ss,
                "IMAGE_REL_AMD64_TOKEN"_ss,
                "IMAGE_REL_AMD64_SREL32"_ss,
                "IMAGE_REL_AMD64_PAIR"_ss,
                "IMAGE_REL_AMD64_SSPAN32"_ss,
            };

            str::slice_t name(u16 type) {
                return s_names[type];
            }
        }

        namespace type::aarch64 {
            static str::slice_t s_names[] = {
                "IMAGE_REL_ARM64_ABSOLUTE"_ss,
                "IMAGE_REL_ARM64_ADDR32"_ss,
                "IMAGE_REL_ARM64_ADDR32NB"_ss,
                "IMAGE_REL_ARM64_BRANCH26"_ss,
                "IMAGE_REL_ARM64_PAGEBASE_REL21"_ss,
                "IMAGE_REL_ARM64_REL21"_ss,
                "IMAGE_REL_ARM64_PAGEOFFSET_12L"_ss,
                "IMAGE_REL_ARM64_SECREL"_ss,
                "IMAGE_REL_ARM64_SECREL_LOW12A"_ss,
                "IMAGE_REL_ARM64_SECREL_HIGH12A"_ss,
                "IMAGE_REL_ARM64_SECREL_LOW12L"_ss,
                "IMAGE_REL_ARM64_TOKEN"_ss,
                "IMAGE_REL_ARM64_ADDR64"_ss,
                "IMAGE_REL_ARM64_BRANCH19"_ss,
                "IMAGE_REL_ARM64_BRANCH14"_ss,
                "IMAGE_REL_ARM64_REL32"_ss,
            };

            str::slice_t name(u16 type) {
                return s_names[type];
            }
        }

        reloc_t get(const coff_t& coff, const header_t& hdr, u32 idx) {
            reloc_t r{};

            if (idx > hdr.relocs.file.size)
                return r;

            auto p = coff.buf + hdr.relocs.file.offset + (idx * reloc::entry_size);
            std::memcpy(&r.rva, p, sizeof(u32));
            p += sizeof(u32);

            std::memcpy(&r.symtab_idx, p, sizeof(u32));
            p += sizeof(u32);

            std::memcpy(&r.type, p, sizeof(u16));

            return r;
        }
    }

    namespace comdat {
        static str::slice_t s_names[] = {
            "NONE"_ss,
            "IMAGE_COMDAT_SELECT_NODUPLICATES"_ss,
            "IMAGE_COMDAT_SELECT_ANY"_ss,
            "IMAGE_COMDAT_SELECT_SAME_SIZE"_ss,
            "IMAGE_COMDAT_SELECT_EXACT_MATCH"_ss,
            "IMAGE_COMDAT_SELECT_ASSOCIATIVE"_ss,
            "IMAGE_COMDAT_SELECT_LARGEST"_ss,
        };

        str::slice_t name(u32 sel) {
            return s_names[sel];
        }
    }

    namespace unwind {
        status_t get(const coff_t& coff, const header_t& hdr, u32 idx, unwind_t& u) {
            const auto num_unwinds = hdr.file.size / unwind::entry_size;
            if (idx >= num_unwinds)
                return status_t::section_entry_out_of_bounds;

            auto p = coff.buf + hdr.file.offset + (idx * reloc::entry_size);
            std::memcpy(&u.begin_rva, p, sizeof(u32));
            p += sizeof(u32);

            std::memcpy(&u.end_rva, p, sizeof(u32));
            p += sizeof(u32);

            std::memcpy(&u.info, p, sizeof(u32));

            return status_t::ok;
        }
    }

    namespace strtab {
        u0 free(coff_t& coff) {
            array::free(coff.strtab.list);
        }

        u0 init(coff_t& coff, alloc_t* alloc) {
            coff.strtab.file.offset = 0;
            coff.strtab.file.size   = sizeof(u32);
            array::init(coff.strtab.list, alloc);
        }

        u32 add(coff_t& coff, str::slice_t str) {
            auto offset = coff.strtab.file.size;
            array::append(coff.strtab.list, str);
            coff.strtab.file.size += str.length + 1;
            return offset;
        }

        const s8* get(coff_t& coff, u64 offset) {
            return (const s8*) coff.buf + coff.strtab.file.offset + (offset >> u32(32));
        }

    }

    namespace symtab {
        static status_t parse_ar_long_name(coff_t& coff, str::slice_t name, u32& offset) {
            if (name.length >= 2
            && (name[0] == '/' && isdigit(name[1]))) {
                const auto os = slice::make(name.data + 1, 7);
                if (OK(numbers::integer::parse(os, 10, offset))) {
                    return status_t::ok;
                }
            }
            return status_t::not_ar_long_name;
        }

        u0 free(coff_t& coff) {
            array::free(coff.symtab.list);
        }

        sym_t* make_symbol(coff_t& coff) {
            auto sym = &array::append(coff.symtab.list);
            sym->id   = coff.symtab.list.size;
            sym->type = sym_type_t::sym;
            return sym;
        }

        u0 init(coff_t& coff, alloc_t* alloc) {
            array::init(coff.symtab.list, alloc);
        }

        sym_t* find_symbol(coff_t& coff, u64 name) {
            for (auto& sym : coff.symtab.list) {
                if (sym.type == sym_type_t::sym
                &&  sym.subclass.sym.strtab_offset == name) {
                    return &sym;
                }
            }
            return nullptr;
        }

        sym_t* get_aux(coff_t& coff, sym_t* sym, u32 idx) {
            if (sym->type != sym_type_t::sym)
                return nullptr;
            auto sc = &sym->subclass.sym;
            if (!sc->aux.idx || idx > sc->aux.len)
                return nullptr;
            return &coff.symtab.list[sc->aux.idx + idx];
        }

        sym_t* make_symbol(coff_t& coff, str::slice_t name) {
            auto sym = &array::append(coff.symtab.list);
            sym->id   = coff.symtab.list.size;
            sym->type = sym_type_t::sym;
            auto sc = &sym->subclass.sym;
            sc->name = name;
            u32 offset{};
            if (!OK(parse_ar_long_name(coff, sc->name, offset))) {
                if (name.length > 8) {
                    sc->strtab_offset = strtab::add(coff, name);
                    sc->strtab_offset <<= u32(32);
                } else {
                    u64_bytes_t thunk{};
                    std::memcpy(thunk.bytes, name.data, sizeof(u64));
                    sc->strtab_offset = thunk.value;
                }
            } else {
                sc->strtab_offset = offset;
                const s8* p = (const s8*) coff.buf
                              + coff.strtab.file.offset
                              + sizeof(u32)
                              + sc->strtab_offset;
                sc->name = slice::make(p);
                sc->strtab_offset <<= u32(32);
            }
            coff.symtab.file.size += entry_size;
            return sym;
        }

        sym_t* make_symbol(coff_t& coff, u64 name, u32 offset) {
            auto sym = &array::append(coff.symtab.list);
            sym->id   = coff.symtab.list.size;
            sym->type = sym_type_t::sym;
            auto sc = &sym->subclass.sym;
            sc->strtab_offset = name;
            if ((sc->strtab_offset & 0xffffffff) == 0) {
                auto str = strtab::get(coff, sc->strtab_offset);
                sc->name = slice::make(str);
            } else {
                u8* p = coff.buf + offset;
                u8* e = p;
                u32 len{};
                while (len < 8 && *e != 0)
                    ++e, ++len;
                sc->name = slice::make(p, len);
                u32 strtab_offset{};
                if (OK(parse_ar_long_name(coff, sc->name, strtab_offset))) {
                    sc->strtab_offset = strtab_offset;
                    const s8* pp = (const s8*) coff.buf
                                   + coff.strtab.file.offset
                                   + sizeof(u32)
                                   + sc->strtab_offset;
                    sc->name = slice::make(pp);
                    sc->strtab_offset <<= u32(32);
                }
            }
            return sym;
        }

        sym_t* make_aux(coff_t& coff, sym_t* sym, sym_type_t type) {
            auto aux = &array::append(coff.symtab.list);
            if (!sym->subclass.sym.aux.idx) {
                sym->subclass.sym.aux.idx = coff.symtab.list.size - 1;
            }
            ++sym->subclass.sym.aux.len;
            aux->type = type;
            switch (type) {
                case sym_type_t::aux_xf:
                    aux->subclass.aux_xf = {};
                    break;
                case sym_type_t::aux_file:
                    std::memset(aux->subclass.aux_file.bytes,
                                0,
                                sizeof(aux->subclass.aux_file.bytes));
                    break;
                case sym_type_t::aux_section:
                    aux->subclass.aux_section = {};
                    break;
                case sym_type_t::aux_func_def:
                    aux->subclass.aux_func_def = {};
                    break;
                case sym_type_t::aux_token_def:
                    aux->subclass.aux_token_def = {};
                    break;
                case sym_type_t::aux_weak_extern:
                    aux->subclass.aux_weak_extern = {};
                    break;
                default:
                    break;
            }
            coff.symtab.file.size += entry_size;
            return aux;
        }
    }

    namespace section {
        sym_t* get_symbol(coff_t& coff, header_t& hdr) {
            if (!hdr.symbol) return nullptr;
            return &coff.symtab.list[hdr.symbol - 1];
        }
    }

    namespace line_num {
        line_num_t get(const coff_t& coff, const header_t& hdr, u32 idx) {
            line_num_t num{};

            if (idx > hdr.line_nums.file.size)
                return num;

            auto p = coff.buf + hdr.line_nums.file.offset + (idx * line_num::entry_size);
            std::memcpy(&num.rva, p, sizeof(u32));
            p += sizeof(u32);

            std::memcpy(&num.number, p, sizeof(u16));

            return num;
        }
    }

    u0 free(coff_t& coff) {
        array::free(coff.headers);
        strtab::free(coff);
        symtab::free(coff);
    }

    u0 update_symbol_table(coff_t& coff) {
        coff.symtab.file.offset = coff.offset;
        coff.strtab.file.offset = coff.symtab.file.offset + coff.symtab.file.size;
    }

    u0 set_section_flags(file_t& file, header_t& hdr) {
        const auto section = hdr.section;
        const auto& flags = section->flags;

        if (!flags.code) {
            if (flags.init)
                hdr.flags |= section::data_init;
            else
                hdr.flags |= section::data_uninit;
        } else {
            hdr.flags |= section::content_code;
        }
        if (flags.alloc)    hdr.flags |= section::memory_read;
        if (flags.write)    hdr.flags |= section::memory_write;
        if (flags.exec)     hdr.flags |= section::memory_execute;

        if (file.file_type == file_type_t::obj) {
            switch (section->align) {
                case 1:
                    hdr.flags |= section::memory_align_1;
                    break;
                case 2:
                    hdr.flags |= section::memory_align_2;
                    break;
                case 4:
                    hdr.flags |= section::memory_align_4;
                    break;
                case 8:
                    hdr.flags |= section::memory_align_8;
                    break;
                case 32:
                    hdr.flags |= section::memory_align_32;
                    break;
                case 64:
                    hdr.flags |= section::memory_align_64;
                    break;
                case 128:
                    hdr.flags |= section::memory_align_128;
                    break;
                case 256:
                    hdr.flags |= section::memory_align_256;
                    break;
                case 512:
                    hdr.flags |= section::memory_align_512;
                    break;
                case 1024:
                    hdr.flags |= section::memory_align_1024;
                    break;
                case 2048:
                    hdr.flags |= section::memory_align_2048;
                    break;
                case 4096:
                    hdr.flags |= section::memory_align_4096;
                    break;
                case 8192:
                    hdr.flags |= section::memory_align_8192;
                    break;
                default:
                    hdr.flags |= section::memory_align_16;
                    break;
            }
        }
    }

    status_t write_header(file_t& file, coff_t& coff) {
        FILE_WRITE(u16, coff.machine);
        FILE_WRITE(u16, u16(coff.headers.size));
        FILE_WRITE(u32, coff.timestamp);
        FILE_WRITE(u32, coff.symtab.file.offset);
        FILE_WRITE(u32, coff.symtab.file.size / symtab::entry_size);
        FILE_WRITE(u16, coff.size.opt_hdr);
        FILE_WRITE(u16, coff.flags.image);
        return status_t::ok;
    }

    status_t read_header(file_t& file, coff_t& coff) {
        u16 num_headers{};

        FILE_READ(u16, coff.machine);
        FILE_READ(u16, num_headers);
        FILE_READ(u32, coff.timestamp);
        FILE_READ(u32, coff.symtab.file.offset);
        FILE_READ(u32, coff.symtab.num_symbols);
        FILE_READ(u16, coff.size.opt_hdr);
        FILE_READ(u16, coff.flags.image);

        array::resize(coff.headers, num_headers);
        coff.symtab.file.size   = coff.symtab.num_symbols * symtab::entry_size;
        coff.strtab.file.offset = coff.symtab.file.offset + coff.symtab.file.size;

        return status_t::ok;
    }

    status_t write_string_table(file_t& file, coff_t& coff) {
        FILE_WRITE(u32, coff.strtab.file.size);
        for (auto str : coff.strtab.list)
            FILE_WRITE_CSTR(str);
        return status_t::ok;
    }

    status_t write_symbol_table(file_t& file, coff_t& coff) {
        FILE_SEEK(coff.symtab.file.offset);
        for (const auto& sym : coff.symtab.list) {
            switch (sym.type) {
                case sym_type_t::sym: {
                    auto sc = &sym.subclass.sym;
                    FILE_WRITE(u64, sc->strtab_offset);
                    FILE_WRITE(u32, sc->value);
                    FILE_WRITE(s16, sc->section);
                    FILE_WRITE(u16, sc->type);
                    FILE_WRITE(u8, sc->sclass);
                    FILE_WRITE(u8, u8(sc->aux.len));
                    break;
                }
                case sym_type_t::aux_xf: {
                    auto sc = &sym.subclass.aux_xf;
                    FILE_WRITE0(u32);
                    FILE_WRITE(u16, sc->line_num);
                    FILE_WRITE_PAD(6);
                    FILE_WRITE(u32, sc->ptr_next_func);
                    FILE_WRITE0(u16);
                    break;
                }
                case sym_type_t::aux_file: {
                    auto sc = &sym.subclass.aux_file;
                    FILE_WRITE_STR(slice::make(sc->bytes, sizeof(sc->bytes)));
                    break;
                }
                case sym_type_t::aux_section: {
                    auto sc = &sym.subclass.aux_section;
                    FILE_WRITE(u32, sc->len);
                    FILE_WRITE(u16, sc->num_relocs);
                    FILE_WRITE(u16, sc->num_lines);
                    FILE_WRITE(u32, sc->check_sum);
                    FILE_WRITE(u16, sc->sect_num);
                    FILE_WRITE(u8, sc->comdat_sel);
                    FILE_WRITE_PAD(3);
                    break;
                }
                case sym_type_t::aux_func_def: {
                    auto sc = &sym.subclass.aux_func_def;
                    FILE_WRITE(u32, sc->tag_idx);
                    FILE_WRITE(u32, sc->total_size);
                    FILE_WRITE(u32, sc->ptr_line_num);
                    FILE_WRITE(u32, sc->ptr_next_func);
                    FILE_WRITE0(u16);
                    break;
                }
                case sym_type_t::aux_token_def: {
                    auto sc = &sym.subclass.aux_token_def;
                    FILE_WRITE(u8, sc->aux_type);
                    FILE_WRITE0(u8);
                    FILE_WRITE(u32, sc->symtab_idx);
                    FILE_WRITE_PAD(12);
                    break;
                }
                case sym_type_t::aux_weak_extern: {
                    auto sc = &sym.subclass.aux_weak_extern;
                    FILE_WRITE(u32, sc->tag_idx);
                    FILE_WRITE(u32, sc->flags);
                    FILE_WRITE_PAD(10);
                    break;
                }
                default:
                    break;
            }
        }
        return write_string_table(file, coff);
    }

    status_t build_sections(file_t& file, coff_t& coff) {
        coff.size.headers = coff.offset
                            + coff::header_size
                            + (coff.headers.size * coff::section::header_size);
        coff.offset = align(coff.size.headers, coff.align.file);
        coff.rva    = align(coff.size.headers, coff.align.section);
        for (auto& hdr : coff.headers) {
            auto status = build_section(file, coff, hdr);
            if (!OK(status))
                return status;
        }
        update_symbol_table(coff);
        return status_t::ok;
    }

    status_t read_symbol_table(file_t& file, coff_t& coff) {
        FILE_PUSH_POS();
        defer(FILE_POP_POS());

        FILE_SEEK(coff.symtab.file.offset);

        u64 strtab_offset{};
        u32 num_symbols = coff.symtab.num_symbols;
        u32 sym_offset;
        u8  num_aux_recs{};

        while (num_symbols) {
            sym_offset = FILE_POS();

            FILE_READ(u64, strtab_offset);

            auto sym = symtab::make_symbol(coff, strtab_offset, sym_offset);
            auto sc = &sym->subclass.sym;

            FILE_READ(u32, sc->value);
            FILE_READ(s16, sc->section);
            FILE_READ(u16, sc->type);
            FILE_READ(u8, sc->sclass);
            FILE_READ(u8, num_aux_recs);

            for (u32 i = 0; i < num_aux_recs; ++i) {
                switch (sc->sclass) {
                    case symtab::sclass::file: {
                        auto aux = symtab::make_aux(coff, sym, sym_type_t::aux_file);
                        auto asc = &aux->subclass.aux_file;
                        FILE_READ(s8[18], asc->bytes);
                        break;
                    }
                    case symtab::sclass::static_: {
                        if (sc->type == 0) {
                            auto aux = symtab::make_aux(coff, sym, sym_type_t::aux_section);
                            auto asc = &aux->subclass.aux_section;
                            FILE_READ(u32, asc->len);
                            FILE_READ(u16, asc->num_relocs);
                            FILE_READ(u16, asc->num_lines);
                            FILE_READ(u32, asc->check_sum);
                            FILE_READ(u16, asc->sect_num);
                            FILE_READ(u8, asc->comdat_sel);
                            FILE_SEEK_FWD(3);
                        }
                        break;
                    }
                    case symtab::sclass::function: {
                        auto aux = symtab::make_aux(coff, sym, sym_type_t::aux_xf);
                        auto asc = &aux->subclass.aux_xf;
                        FILE_SEEK_FWD(4);
                        FILE_READ(u16, asc->line_num);
                        FILE_SEEK_FWD(6);
                        FILE_READ(u32, asc->ptr_next_func);
                        FILE_SEEK_FWD(2);
                        break;
                    }
                    case symtab::sclass::external_: {
                        if (sc->section == 0) {
                            if (sc->value == 0 && sc->section == 0) {
                                auto aux = symtab::make_aux(coff, sym, sym_type_t::aux_weak_extern);
                                auto asc = &aux->subclass.aux_weak_extern;
                                FILE_READ(u32, asc->tag_idx);
                                FILE_READ(u32, asc->flags);
                                FILE_SEEK_FWD(10);
                            } else if (sc->type == symtab::type::function) {
                                auto aux = symtab::make_aux(coff, sym, sym_type_t::aux_func_def);
                                auto asc = &aux->subclass.aux_func_def;
                                FILE_READ(u32, asc->tag_idx);
                                FILE_READ(u32, asc->total_size);
                                FILE_READ(u32, asc->ptr_next_func);
                                FILE_SEEK_FWD(2);
                            }
                        }
                        break;
                    }
                    case symtab::sclass::clr_token: {
                        auto aux = symtab::make_aux(coff, sym, sym_type_t::aux_token_def);
                        auto asc = &aux->subclass.aux_token_def;
                        FILE_READ(u8, asc->aux_type);
                        FILE_SEEK_FWD(1);
                        FILE_READ(u32, asc->symtab_idx);
                        FILE_SEEK_FWD(12);
                        break;
                    }
                    default: {
                        // XXX: this shouldn't happen!
                        FILE_SEEK_FWD(18);
                        break;
                    }
                }
                --num_symbols;
            }

            --num_symbols;
        }

        return status_t::ok;
    }

    status_t write_sections_data(file_t& file, coff_t& coff) {
        for (auto& hdr : coff.headers) {
            auto status = write_section_data(file, coff, hdr);
            if (!OK(status))
                return status;
        }
        return status_t::ok;
    }

    status_t read_section_headers(file_t& file, coff_t& coff) {
        u32 hdr_offset;
        u16 num_relocs  {};
        u16 num_line_nos{};
        for (u32 i = 0; i < coff.headers.size; ++i) {
            auto& hdr = coff.headers[i];
            hdr.number = i + 1;

            hdr_offset = FILE_POS();

            FILE_READ(u64, hdr.short_name);

            {
                s8* p = (s8*) coff.buf + hdr_offset;
                s8* e = p;
                u32 len{};
                while (len < 8 && *e != '\0')
                    ++e, ++len;
                hdr.name = slice::make(p, len);
                u32 offset{};
                if (OK(symtab::parse_ar_long_name(coff, hdr.name, offset))) {
                    const s8* pp = (const s8*) coff.buf
                                   + coff.strtab.file.offset
                                   + offset;
                    hdr.name = slice::make(pp);
                }
            }

            for (const auto& sym : coff.symtab.list) {
                if (sym.type == sym_type_t::sym
                &&  sym.subclass.sym.name == hdr.name
                &&  sym.subclass.sym.section == hdr.number
                &&  sym.subclass.sym.sclass == symtab::sclass::static_) {
                    hdr.symbol = sym.id;
                    break;
                }
            }

            FILE_READ(u32, hdr.rva.size);
            FILE_READ(u32, hdr.rva.base);
            FILE_READ(u32, hdr.file.size);
            FILE_READ(u32, hdr.file.offset);
            FILE_READ(u32, hdr.relocs.file.offset);
            FILE_READ(u32, hdr.line_nums.file.offset);
            FILE_READ(u16, num_relocs);
            FILE_READ(u16, num_line_nos);
            FILE_READ(u32, hdr.flags);

            hdr.relocs.file.size    = num_relocs;
            hdr.line_nums.file.size = num_line_nos;
        }
        return status_t::ok;
    }

    status_t init(coff_t& coff, file_t& file, alloc_t* alloc) {
        using type_t = binfmt::section::type_t;

        coff.alloc = alloc;
        coff.buf   = file.buf.data;

        array::init(coff.headers, coff.alloc);
        strtab::init(coff, coff.alloc);
        symtab::init(coff, coff.alloc);

        coff.timestamp     = std::time(nullptr);
        coff.align.file    = 0x200;
        coff.align.section = 0x1000;

        switch (file.machine) {
            case binfmt::machine::type_t::unknown:
                coff.machine = 0;
                break;
            case binfmt::machine::type_t::x86_64:
                coff.machine = machine::amd64;
                break;
            case binfmt::machine::type_t::aarch64:
                coff.machine = machine::arm64;
                break;
        }

        if (!file.module)
            return status_t::ok;

        status_t status{};

        const auto module = file.module;
        auto& module_sc = module->subclass.object;

        for (u32 i = 0; i < module_sc.sections.size; ++i) {
            auto& hdr = array::append(coff.headers);
            hdr = {};
            hdr.number  = i + 1;
            hdr.section = &module_sc.sections[i];

            str::slice_t name{};
            if (hdr.section->type == type_t::custom) {
                auto intern_rc = string::interned::get(hdr.section->symbol);
                if (!OK(intern_rc.status))
                    return status_t::symbol_not_found;
                name = intern_rc.slice;
            } else {
                status = coff::get_section_name(hdr.section, name);
                if (!OK(status))
                    return status;
            }

            auto sym = coff::symtab::make_symbol(coff, name);
            {
                auto sc = &sym->subclass.sym;
                sc->section = hdr.number;
                sc->value   = {};
                sc->type    = 0;
                sc->sclass  = symtab::sclass::static_;
                hdr.symbol  = sym->id;
            }

            auto aux = coff::symtab::make_aux(coff, sym, sym_type_t::aux_section);
            {
                auto sc = &aux->subclass.aux_section;
                sc->len        = hdr.rva.size;
                sc->sect_num   = hdr.number;
                sc->check_sum  = 0;
                sc->comdat_sel = 0;
                sc->num_relocs = hdr.relocs.file.size;
                sc->num_lines  = hdr.line_nums.file.size;
            }
        }

        for (const auto& symbol : module->symbols) {
            const auto intern_rc = string::interned::get(symbol.name);
            if (!OK(intern_rc.status))
                return status_t::symbol_not_found;
            auto sym = coff::symtab::make_symbol(coff, intern_rc.slice);
            auto sc = &sym->subclass.sym;
            sc->value   = symbol.value;
            sc->section = symbol.section;
            switch (symbol.type) {
                case symbol::type_t::none:
                case symbol::type_t::object:
                    sc->type = 0;
                    break;
                case symbol::type_t::file:
                    sc->type   = 0;
                    sc->sclass = symtab::sclass::file;
                    break;
                case symbol::type_t::function:
                    sc->type = symtab::type::function;
                    break;
            }
            if (!sc->sclass) {
                switch (symbol.scope) {
                    case symbol::scope_t::none:
                        sc->sclass = symtab::sclass::null_;
                        break;
                    case symbol::scope_t::local:
                        sc->sclass = symtab::sclass::static_;
                        break;
                    case symbol::scope_t::global:
                        sc->sclass = symtab::sclass::external_;
                        break;
                    case symbol::scope_t::weak:
                        sc->sclass = symtab::sclass::weak_external;
                        break;
                }
            }
        }

        return status;
    }

    status_t write_section_headers(file_t& file, coff_t& coff) {
        for (auto& hdr : coff.headers) {
            auto status = write_section_header(file, coff, hdr);
            if (!OK(status))
                return status;
        }
        return status_t::ok;
    }

    status_t build_section(file_t& file, coff_t& coff, header_t& hdr) {
        using type_t = binfmt::section::type_t;

        hdr.file.offset = coff.offset;
        hdr.rva.base    = coff.rva;
        const auto& sc = hdr.section->subclass;
        switch (hdr.section->type) {
            case type_t::code:
                if (!coff.code.base)
                    coff.code.base = hdr.rva.base;
                hdr.rva.size    = sc.data.length;
                hdr.file.size   = align(hdr.rva.size, coff.align.file);
                coff.code.size += hdr.rva.size;
                coff.size.image = align(coff.size.image + hdr.rva.size, coff.align.section);
                coff.offset     = align(coff.offset + hdr.rva.size, coff.align.file);
                coff.rva        = align(coff.rva + hdr.rva.size, coff.align.section);
                break;
            case type_t::data:
            case type_t::custom:
                if (hdr.section->flags.init) {
                    if (!coff.init_data.base)
                        coff.init_data.base = hdr.rva.base;
                    hdr.rva.size  = sc.data.length;
                    hdr.file.size = align(hdr.rva.size, coff.align.file);
                    coff.init_data.size += hdr.rva.size;
                    coff.size.image = align(coff.size.image + hdr.rva.size, coff.align.section);
                    coff.offset     = align(coff.offset + hdr.rva.size, coff.align.file);
                    coff.rva        = align(coff.rva + hdr.rva.size, coff.align.section);
                } else {
                    if (!coff.uninit_data.base)
                        coff.uninit_data.base = hdr.rva.base;
                    hdr.rva.size    = sc.size;
                    hdr.file.offset = 0;
                    hdr.file.size   = 0;
                    coff.uninit_data.size += hdr.rva.size;
                    coff.size.image = align(coff.size.image + hdr.rva.size, coff.align.section);
                    coff.rva        = align(coff.rva + hdr.rva.size, coff.align.section);
                }
                break;
            case type_t::reloc:
            case type_t::group:
            case type_t::import:
            case type_t::export_:
                return status_t::invalid_section_type;
            case type_t::rsrc:
            case type_t::note:
            case type_t::init:
            case type_t::fini:
            case type_t::debug:
            case type_t::unwind:
            case type_t::pre_init:
                return status_t::not_implemented;
        }

        set_section_flags(file, hdr);

        auto sym = section::get_symbol(coff, hdr);
        auto aux_sec = symtab::get_aux(coff, sym, 0);
        if (aux_sec)
            aux_sec->subclass.aux_section.len = hdr.rva.size;

        return status_t::ok;
    }

    status_t write_section_data(file_t& file, coff_t& coff, header_t& hdr) {
        using type_t = binfmt::section::type_t;

        const auto type = hdr.section->type;

        if (type == type_t::data && !hdr.section->flags.init)
            return status_t::ok;

        const auto& sc = hdr.section->subclass;
        FILE_SEEK(hdr.file.offset);

        switch (type) {
            case type_t::code:
            case type_t::data:
            case type_t::custom:
                FILE_WRITE_STR(sc.data);
                break;
            case type_t::import:
            case type_t::export_:
                return status_t::invalid_section_type;
            case type_t::rsrc:
            case type_t::debug:
            case type_t::unwind:
                return status_t::not_implemented;
            default:
                break;
        }

        return status_t::ok;
    }

    status_t write_section_header(file_t& file, coff_t& coff, header_t& hdr) {
        auto sym = section::get_symbol(coff, hdr);
        FILE_WRITE(u64, sym->subclass.sym.strtab_offset);
        FILE_WRITE(u32, hdr.rva.size);
        FILE_WRITE(u32, hdr.rva.base);
        FILE_WRITE(u32, hdr.file.size);
        FILE_WRITE(u32, hdr.file.offset);
        FILE_WRITE(u32, hdr.relocs.file.offset);
        FILE_WRITE(u32, hdr.line_nums.file.offset);
        FILE_WRITE(u16, u16(hdr.relocs.file.size));
        FILE_WRITE(u16, u16(hdr.line_nums.file.size));
        FILE_WRITE(u32, hdr.flags);
        return status_t::ok;
    }
}
