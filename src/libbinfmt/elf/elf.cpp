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
    namespace file {
        static str::slice_t s_class_names[] = {
            "None"_ss,
            "ELF32"_ss,
            "ELF64"_ss,
        };

        static str::slice_t s_os_abi_names[] = {
            "UNIX - System V"_ss,
            "HPUX"_ss,
            "NETBSD"_ss,
            "GNU"_ss,
            "LINUX/GNU"_ss,
            "SOLARIS"_ss,
            "AIX"_ss,
            "IRIX"_ss,
            "FREEBSD"_ss,
            "TRU64"_ss,
            "MODESTO"_ss,
            "OPENBSD"_ss,
            "ARM_EABI"_ss,
            "ARM"_ss,
            "STANDALONE"_ss,
        };

        static str::slice_t s_file_type_names[] = {
            "None"_ss,
            "REL (Relocatable file)"_ss,
            "EXEC"_ss,
            "DYN"_ss,
            "CORE"_ss
        };

        static str::slice_t s_version_names[] = {
            "None"_ss,
            "Current"_ss,
        };

        static str::slice_t s_endianess_names[] = {
            "None"_ss,
            "Little endian"_ss,
            "Big endian"_ss
        };

        str::slice_t class_name(u8 cls) {
            return s_class_names[cls];
        }

        str::slice_t os_abi_name(u8 os_abi) {
            return s_os_abi_names[os_abi];
        }

        str::slice_t version_name(u8 version) {
            return s_version_names[version];
        }

        str::slice_t file_type_name(u16 type) {
            return s_file_type_names[type];
        }

        str::slice_t endianess_name(u8 endianess) {
            return s_endianess_names[endianess];
        }
    }

    namespace machine {
        static str::slice_t s_machine_names[] = {
            "None"_ss,
            "M32"_ss,
            "SPARC"_ss,
            "Intel 80386"_ss,
            "Motorola 68000"_ss,
            "Motorola 88000"_ss,
            "Intel 80860"_ss,
            "MIPS R3000 BE"_ss,
            "IBM System/370"_ss,
            "MIPS R330 LE"_ss,
            "????"_ss,
            "????"_ss,
            "????"_ss,
            "????"_ss,
            "PARISC/HPPA"_ss,
            "Fujitsu VPP500"_ss,
            "SPARC32"_ss,
            "Intel 80960"_ss,
            "PowerPC"_ss,
            "PowerPC 64-bit"_ss,
            "IBM S390"_ss,
            "????"_ss,
            "????"_ss,
            "????"_ss,
            "????"_ss,
            "????"_ss,
            "????"_ss,
            "????"_ss,
            "????"_ss,
            "????"_ss,
            "????"_ss,
            "????"_ss,
            "????"_ss,
            "????"_ss,
            "????"_ss,
            "????"_ss,
            "NEC V800"_ss,
            "Fujitsu FR20"_ss,
            "TRW RH-32"_ss,
            "RCE"_ss,
            "ARM"_ss,
            "Digital Alpha"_ss,
            "Hitachi SH"_ss,
            "SPARC v9 64-bit"_ss,
            "Siemens Tricore"_ss,
            "Argonaut RISC Core"_ss,
            "Hitachi H8/300"_ss,
            "Hitachi H8/300H"_ss,
            "Hitachi H8S"_ss,
            "Hitachi H8/500"_ss,
            "Intel Merced"_ss,
            "Stanford MIPS-X"_ss,
            "Motorola Coldfire"_ss,
            "Motorola M68HC12"_ss,
            "Fujitsu MMA"_ss,
            "Siemens PCP"_ss,
            "Sony nCPU embedded RISC"_ss,
            "Denso NDR1 microprocessor"_ss,
            "Motorola Star*Core"_ss,
            "Toyota ME16"_ss,
            "STM ST100"_ss,
            "Tinyj embedded"_ss,
            "AMD x86-64 architecture"_ss,
            "Sony DSP"_ss,
            "????"_ss,
            "????"_ss,
            "Siemens FX66"_ss,
            "STM ST9+ 8/16"_ss,
            "STM ST7 8-bit"_ss,
            "Motorola MC68HC16"_ss,
            "Motorola MC68HC11"_ss,
            "Motorola MC68HC08"_ss,
            "Motorola MC68HC05"_ss,
            "SGI SVx"_ss,
            "STM ST19 8-bit"_ss,
            "Digital VAX"_ss,
            "Axis Comm. 32-bit"_ss,
            "Infineon Tech. 32-bit"_ss,
            "Element 14 64-bit DSP"_ss,
            "LSI Logic 16-bit DSP"_ss,
            "Donald Knuth's educational 64-bit"_ss,
            "Harvard University machine-independent object files"_ss,
            "SiTera Prism"_ss,
            "Atmel AVR 8-bit"_ss,
            "Fujitsu FR30"_ss,
            "Mitsubishi D10V"_ss,
            "Mitsubishi D30V"_ss,
            "NEC v850"_ss,
            "Mitsubishi M32R"_ss,
            "Matsushita MN10300"_ss,
            "Matsushita MN10200"_ss,
            "picoJava"_ss,
            "OpenRISC 32-bit"_ss,
            "ARC Cores Tangent-A5"_ss,
            "Tensilica Xtensa Architecture"_ss,
        };

        str::slice_t name(u16 type) {
            switch (type) {
                case 183:
                    return "ARM AArch64"_ss;
                case 188:
                    return "Tilera TILEPro"_ss;
                case 191:
                    return "Tilera TILE-Gx"_ss;
                case 243:
                    return "RISC-V"_ss;
                default:
                    break;
            }
            return s_machine_names[type];
        }
    }

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
        u0 init(strtab_t& strtab) {
            strtab.type = strtab_type_t::no_copy;
        }

        u0 free(strtab_t& strtab) {
            if (strtab.type == strtab_type_t::owned)
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
            strtab.type = strtab_type_t::owned;
            if (strtab.type == strtab_type_t::owned)
                str_array::init(strtab.strings, alloc);
        }

        u32 add_str(strtab_t& strtab, str::slice_t str) {
            const auto offset = strtab.strings.buf.size + 1;
            str_array::append(strtab.strings, str);
            return offset;
        }

        const s8* get(const strtab_t& strtab, u32 offset) {
            switch (strtab.type) {
                case strtab_type_t::no_copy:
                    return strtab.buf + offset;
                case strtab_type_t::owned:
                    return (const s8*) strtab.strings.buf.data + offset;
                default:
                    break;
            }
            return nullptr;
        }

        status_t write(const strtab_t& strtab, file_t& file) {
            FILE_WRITE0(u8);
            const auto slice = slice::make(strtab.strings.buf.data,
                                           strtab.strings.buf.size);
            FILE_WRITE_STR(slice);
            return status_t::ok;
        }
    }

    namespace symtab {
        u0 init(symtab_t& symtab) {
            symtab.type = symtab_type_t::no_copy;
        }

        u0 free(symtab_t& symtab) {
            if (symtab.type == symtab_type_t::owned) {
                hash::free(symtab.owned.hash);
                array::free(symtab.owned.symbols);
            }
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

        u32 size(const symtab_t& symtab) {
            if (symtab.type == symtab_type_t::no_copy)
                return symtab.no_copy.size;
            else
                return symtab.owned.symbols.size;
        }

        header_t& make_section(elf_t& elf,
                               str::slice_t name,
                               const symtab_t* symtab,
                               u32 first_global_idx) {
            const auto& ssc = symtab->owned;
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
            sc.size       = ssc.symbols.size * elf::symtab::entity_size;

            auto& block = array::append(elf.blocks);
            block.type        = block_type_t::symtab;
            block.offset      = elf.data.rel_offset;
            block.data.symtab = symtab;

            elf.data.rel_offset = align(elf.data.rel_offset + sc.size, sizeof(u64));

            return hdr;
        }

        u0 rehash(symtab_t& symtab, u32 size) {
            if (symtab.type == symtab_type_t::owned)
                hash::rehash(symtab.owned.hash, size);
        }

        sym_t* get(const symtab_t& symtab, u32 idx) {
            if (symtab.type != symtab_type_t::no_copy)
                return nullptr;
            if (idx < symtab.no_copy.size)
                return &symtab.no_copy.buf[idx];
            return nullptr;
        }

        status_t write(const symtab_t& symtab, file_t& file) {
            for (const auto& sym : symtab.owned.symbols) {
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
            auto& ssc = symtab.owned;

            u32 name_index;
            u32 bucket_idx{};

            if (symbol) {
                const auto intern_rc = string::interned::get(symbol->name);
                if (!OK(intern_rc.status))
                    return status_t::symbol_not_found;
                name_index = elf::strtab::add_str(*ssc.strtab, intern_rc.slice);
                const auto name_hash  = hash_name(intern_rc.slice);
                bucket_idx = name_hash % ssc.hash.buckets.size;
            }

            auto& sym = array::append(ssc.symbols);
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

            u32 sym_idx = ssc.hash.buckets[bucket_idx];
            if (sym_idx == 0) {
                sym_idx = ssc.symbols.size - 1;
                ssc.hash.buckets[bucket_idx]                   = sym_idx;
                ssc.hash.chains[symtab.owned.symbols.size - 1] = sym_idx;
            } else {
                while (ssc.hash.chains[sym_idx] != 0)
                    ++sym_idx;
                ssc.hash.chains[sym_idx] = ssc.symbols.size - 1;
            }

            return status_t::ok;
        }

        u0 init(symtab_t& symtab, strtab_t* strtab, alloc_t* alloc) {
            symtab.type = symtab_type_t::owned;
            auto& ssc = symtab.owned;
            ssc.strtab = strtab;
            hash::init(ssc.hash, alloc);
            array::init(ssc.symbols, alloc);
        }

        status_t find_sym(symtab_t& symtab, str::slice_t name, sym_t** sym) {
            auto& ssc = symtab.owned;

            *sym = nullptr;

            const auto name_hash = hash_name(name);
            const auto bucket_idx = name_hash % ssc.hash.buckets.size;

            auto chain_idx = ssc.hash.buckets[bucket_idx];
            while (chain_idx != 0 && chain_idx < ssc.hash.chains.size) {
                auto temp_sym = &ssc.symbols[ssc.hash.chains[chain_idx]];
                auto str_offset = (const s8*) ssc.strtab->strings.buf.data + temp_sym->name_index;
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

    namespace section {
        namespace type {
            static str::slice_t s_type_names[] = {
                "NULL"_ss,
                "PROGBITS"_ss,
                "SYMTAB"_ss,
                "STRTAB"_ss,
                "RELA"_ss,
                "HASH"_ss,
                "DYNAMIC"_ss,
                "NOTE"_ss,
                "NOBITS"_ss,
                "REL"_ss,
                "SHLIB"_ss,
                "DYNSYM"_ss,
                "????"_ss,
                "????"_ss,
                "INIT_ARRAY"_ss,
                "FINI_ARRAY"_ss,
                "PREINIT_ARRAY"_ss,
                "GROUP"_ss,
                "SYMTAB_SHNDX"_ss,
            };

            str::slice_t name(u32 type) {
                switch (type) {
                    case x86_64_unwind:     return "X86_64_UNWIND"_ss;
                    case gnu_eh_frame:      return "GNU_EH_FRAME"_ss;
                    case gnu_stack:         return "GNU_STACK"_ss;
                    case gnu_rel_ro:        return "GNU_RELRO"_ss;
                    case gnu_attributes:    return "GNU_ATTRIBUTES"_ss;
                    case gnu_hash:          return "GNU_HASH"_ss;
                    case gnu_lib_list:      return "GNU_LIBLIST"_ss;
                    case checksum:          return "CHECKSUM"_ss;
                    case gnu_ver_def:       return "GNU_VERDEF"_ss;
                    case gnu_ver_need:      return "GNU_VERNEED"_ss;
                    case gnu_ver_sym:       return "GNU_VERSYM"_ss;
                    case null ... symtab_shndx:
                        return s_type_names[type];
                    default: {
                        if (type >= low_os && type <= high_os) {
                            return string::interned::fold(format::format("LOOS+0x{:x}", type & 0x0fffffffU));
                        } else if (type >= low_proc && type <= high_proc) {
                            return string::interned::fold(format::format("LOPROC+0x{:x}", type & 0x0fffffffU));
                        } else if (type >= low_user && type <= high_user) {
                            return string::interned::fold(format::format("LOUSER+0x{:x}", type & 0x0fffffffU));
                        }
                        break;
                    }
                }
                return "UNKNOWN"_ss;
            }
        }

        namespace flags {
            static const s8 s_flag_chars[] = {
                'W',
                'A',
                'X',
                'M',
                'S',
                'I',
                'L',
                'O',
                'G',
                'T',
                'C',
                'o',
                'E'
            };

            static const s8* s_flag_names[] = {
                "WRITE",
                "ALLOC",
                "EXEC_INSTR",
                "MERGE",
                "STRINGS",
                "INFO",
                "ORDER",
                "OS_NON_CONFORM",
                "GROUP",
                "TLS",
                "COMPRESSED",
                "ORDERED",
                "EXCLUDE",
            };

            static u32 s_flags[] = {
                write, alloc, exec_instr, merge, strings, info_link, link_order,
                os_non_conform, group, tls, compressed, ordered, exclude
            };

            const s8* name(u32 flag) {
                u32 idx{};
                for (u32 mask : s_flags) {
                    if ((flag & mask) == mask)
                        return s_flag_names[idx];
                    ++idx;
                }
                return "UNKNOWN";
            }

            u0 chars(u32 flags, s8* chars) {
                u32 ci{};
                u32 fi{};
                for (u32 mask : s_flags) {
                    if ((flags & mask) == mask)
                        chars[ci++] = s_flag_chars[fi];
                    ++fi;
                }
                chars[ci] = '\0';
            }

            u0 names(u32 flags, const s8** names) {
                u32 idx{};
                for (u32 mask : s_flags) {
                    names[idx++] = (flags & mask) == mask ? s_flag_names[idx] : nullptr;
                }
            }
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
        elf.magic[0]                       = 0x7f;
        elf.magic[1]                       = 'E';
        elf.magic[2]                       = 'L';
        elf.magic[3]                       = 'F';
        elf.magic[file::magic_class]       = opts.clazz;
        elf.magic[file::magic_data]        = opts.endianess;
        elf.magic[file::magic_version]     = opts.version;
        elf.magic[file::magic_os_abi]      = opts.os_abi;
        elf.magic[file::magic_abi_version] = opts.abi_version;

        array::init(elf.blocks, elf.alloc);
        array::init(elf.headers, elf.alloc);

        if (opts.access_mode == access_mode_t::read) {
            strtab::init(elf.strings);
            symtab::init(elf.symbols);
        } else {
            strtab::init(elf.strings, elf.alloc);
            strtab::init(elf.section_names,  elf.alloc);
            symtab::init(elf.symbols, &elf.strings, elf.alloc);
        }

        if (file.file_type == file_type_t::obj) {
            elf.file_type = elf::file::type::rel;
        } else {
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
                case file_type_t::exe:
                    elf.file_type = elf::file::type::exec;
                    break;
                case file_type_t::dll:
                    elf.file_type = elf::file::type::dyn;
                    break;
                default:
                    break;
            }
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

    status_t read_sections(file_t& file, elf_t& elf) {
        if (elf.segment.count > 0) {
            FILE_SEEK(elf.segment.offset);
            for (u32 i = 0; i < elf.segment.count; ++i) {
                auto& hdr = array::append(elf.headers);
                hdr.type   = header_type_t::segment;
                hdr.number = i;

                auto& sc = hdr.subclass.segment;
                FILE_READ(u32, sc.type);
                FILE_READ(u32, sc.flags);
                FILE_READ(u64, sc.file.offset);
                FILE_READ(u64, sc.addr.virt);
                FILE_READ(u64, sc.addr.phys);
                FILE_READ(u64, sc.file.size);
                FILE_READ(u64, sc.addr.size);
                FILE_READ(u64, sc.align);
            }
        }

        if (elf.section.count > 0) {
            FILE_SEEK(elf.section.offset);
            for (u32 i = 0; i < elf.section.count; ++i) {
                auto& hdr = array::append(elf.headers);
                hdr.type   = header_type_t::section;
                hdr.number = i;

                auto& sc = hdr.subclass.section;
                FILE_READ(u32, sc.name_index);
                FILE_READ(u32, sc.type);
                FILE_READ(u64, sc.flags);
                FILE_READ(u64, sc.addr.base);
                FILE_READ(u64, sc.offset);
                FILE_READ(u64, sc.size);
                FILE_READ(u32, sc.link);
                FILE_READ(u32, sc.info);
                FILE_READ(u64, sc.addr.align);
                FILE_READ(u64, sc.entry_size);

                if (elf.str_ndx > 0 && hdr.number == elf.str_ndx) {
                    elf.strings.buf = (const s8*) file.buf.data + sc.offset;
                } else if (sc.type == section::type::symtab) {
                    elf.symbols.no_copy.buf  = (sym_t*) (file.buf.data + sc.offset);
                    elf.symbols.no_copy.size = sc.size / sc.entry_size;
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

    status_t read_file_header(file_t& file, elf_t& elf) {
        u32 version{};
        FILE_READ(u8[16], elf.magic);
        FILE_READ(u16, elf.file_type);
        FILE_READ(u16, elf.machine);
        FILE_READ(u32, version);
        FILE_READ(u64, elf.entry_point);
        FILE_READ(u64, elf.segment.offset);
        FILE_READ(u64, elf.section.offset);
        FILE_READ(u32, elf.proc_flags);
        FILE_SEEK_FWD(sizeof(u32));
        FILE_READ(u16, elf.segment.count);
        FILE_SEEK_FWD(sizeof(u16));
        FILE_READ(u16, elf.section.count);
        FILE_READ(u16, elf.str_ndx);
        return status_t::ok;
    }

    status_t write_file_header(file_t& file, elf_t& elf) {
        for (u8 magic : elf.magic)
            FILE_WRITE(u8, magic);
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
