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

    namespace strtab {
        const s8* get(const elf_t& elf, u32 offset) {
            if (elf.file_header->strtab_ndx == 0)
                return nullptr;
            const auto& hdr = elf.sections[elf.file_header->strtab_ndx];
            return ((s8*) elf.file_header) + hdr.offset + offset;
        }
    }

    namespace symtab {
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

        sym_t* get(const elf_t& elf, u32 sect_num, u32 sym_idx) {
            if (sect_num > elf.file_header->sect_hdr_count)
                return nullptr;
            const auto& hdr = elf.sections[sect_num];
            if (sym_idx < (hdr.size / hdr.entity_size)) {
                return (sym_t*) (((u8*) elf.file_header) + hdr.offset + (sizeof(sym_t) * sym_idx));
            }
            return nullptr;
        }
    }

    namespace hashtab {
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
    }

    status_t init(elf_t& elf, const opts_t& opts) {
        auto& file = *opts.file;

        auto buf = FILE_PTR();
        auto fh = (file_header_t*) (buf);
        elf.alloc       = opts.alloc;
        elf.file_header = fh;

        fh->magic[0]                       = 0x7f;
        fh->magic[1]                       = 'E';
        fh->magic[2]                       = 'L';
        fh->magic[3]                       = 'F';
        fh->magic[file::magic_class]       = opts.clazz;
        fh->magic[file::magic_data]        = opts.endianess;
        fh->magic[file::magic_version]     = opts.version;
        fh->magic[file::magic_os_abi]      = opts.os_abi;
        fh->magic[file::magic_abi_version] = opts.abi_version;
        fh->flags          = opts.flags;
        fh->header_size    = file::header_size;
        fh->entry_point    = opts.entry_point == 0 ? 0x00400000 : opts.entry_point;
        if (opts.num_segments > 0) {
            fh->pgm_hdr_size   = segment::header_size;
            fh->pgm_hdr_count  = opts.num_segments;
            fh->pgm_hdr_offset = fh->header_size;
            elf.segments       = (pgm_header_t*) (buf + elf.file_header->pgm_hdr_offset);
        }
        if (opts.num_sections > 0) {
            fh->sect_hdr_size   = section::header_size;
            fh->sect_hdr_count  = opts.num_sections;
            fh->sect_hdr_offset =
                std::max<u64>(fh->pgm_hdr_offset + (fh->pgm_hdr_count * fh->pgm_hdr_size), fh->header_size);
            elf.sections = (sect_header_t*) (buf + elf.file_header->sect_hdr_offset);
        }
        if (opts.strtab_size > 0) {
            fh->strtab_ndx         = 1;
            elf.strtab             = &elf.sections[1];
            elf.strtab->flags      = {};
            elf.strtab->addr       = {};
            elf.strtab->addr_align = 1;
            elf.strtab->size       = opts.strtab_size;
            elf.strtab->type       = section::type::strtab;
        }
        if (opts.symtab_size > 0) {
            elf.symtab              = &elf.sections[fh->sect_hdr_count - 1];
            elf.symtab->link        = fh->strtab_ndx;
            elf.symtab->info        = 0;
            elf.symtab->size        = opts.symtab_size;
            elf.symtab->type        = section::type::strtab;
            elf.symtab->addr        = {};
            elf.symtab->addr_align  = 8;
            elf.symtab->entity_size = symtab::entity_size;
        }

        return status_t::ok;
    }

    status_t read(elf_t& elf, file_t& file) {
        return status_t::ok;
    }

    status_t write(elf_t& elf, file_t& file) {
        using machine_type_t = binfmt::machine::type_t;
        using section_type_t = binfmt::section::type_t;

        auto fh = elf.file_header;
        auto module = file.module;
        auto msc = &module->subclass.object;

        switch (file.file_type) {
            case file_type_t::obj:
                fh->type = file::type::rel;
                break;
            case file_type_t::exe:
                fh->type = file::type::exec;
                break;
            case file_type_t::dll:
                fh->type = file::type::dyn;
                break;
            default:
                return status_t::invalid_file_type;
        }

        switch (file.machine) {
            case machine_type_t::unknown:
                return status_t::invalid_machine_type;
            case machine_type_t::x86_64:
                fh->machine = elf::machine::x86_64;
                break;
            case machine_type_t::aarch64:
                fh->machine = elf::machine::aarch64;
                break;
        }

        u64 virt_addr       {fh->entry_point};
        u64 data_offset     {fh->sect_hdr_offset + (fh->sect_hdr_size * fh->sect_hdr_count)};
        u32 sect_idx        {1};

        if (elf.strtab) {
            elf.strtab->offset = data_offset;
            ++sect_idx;
            data_offset += elf.strtab->size;
        }

        const u32 start_sect_idx  {sect_idx};

        for (const auto& section : msc->sections) {
            auto& hdr = elf.sections[sect_idx];
            hdr.addr       = virt_addr;
            hdr.offset     = data_offset;
            hdr.addr_align = 8;
            switch (section.type) {
                case section_type_t::data:
                case section_type_t::custom: {
                    hdr.flags |= section::flags::alloc;
                    if (!section.flags.init) {
                        hdr.type = section::type::nobits;
                        hdr.size = section.subclass.size;
                    } else {
                        hdr.type = section::type::progbits;
                        hdr.size = section.subclass.data.length;
                        data_offset += hdr.size;
                    }
                    if (section.flags.write)
                        hdr.flags |= section::flags::write;
                    virt_addr = align(virt_addr + hdr.size, hdr.addr_align);
                    break;
                }
                case section_type_t::code: {
                    hdr.type = section::type::progbits;
                    hdr.size = section.subclass.data.length;
                    hdr.flags |= section::flags::alloc | section::flags::exec_instr;
                    if (section.flags.write)
                        hdr.flags |= section::flags::write;
                    data_offset += hdr.size;
                    virt_addr = align(virt_addr + hdr.size, hdr.addr_align);
                    break;
                }
                case section_type_t::reloc: {
                    hdr.type        = section::type::rela;
                    hdr.size        = section.subclass.relocs.size * relocs::entity_size;
                    hdr.entity_size = relocs::entity_size;
                    if (section.flags.group)
                        hdr.flags |= section::flags::group;
                    hdr.link = fh->sect_hdr_count - 1;
                    hdr.info = section.link + start_sect_idx;
                    data_offset += hdr.size;
                    break;
                }
                case section_type_t::group: {
                    hdr.type        = section::type::group;
                    hdr.size        = (section.subclass.relocs.size + 1) * group::entity_size;
                    hdr.link        = fh->sect_hdr_count - 1;
                    hdr.info        = section.link + start_sect_idx;
                    hdr.entity_size = group::entity_size;
                    data_offset += hdr.size;
                    break;
                }
                default:
                    break;
            }
        }

        return status_t::ok;
    }
}
