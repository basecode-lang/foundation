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

#include <basecode/binfmt/io.h>

namespace basecode::binfmt::io {
    struct elf_t;
    struct elf_dyn_t;
    struct elf_sym_t;
    struct elf_rel_t;
    struct elf_rela_t;
    struct elf_note_t;
    struct elf_opts_t;
    struct elf_header_t;
    struct elf_strtab_t;

    using elf_str_list_t        = array_t<str::slice_t>;
    using elf_note_list_t       = array_t<elf_note_t>;
    using elf_header_list_t     = array_t<elf_header_t>;

    enum class elf_header_type_t : u8 {
        none,
        section,
        segment
    };

    struct elf_sym_t final {
        u64                     value;
        u64                     size;
        u32                     name_index;
        u16                     index;
        u8                      info;
        u8                      other;
    };

    struct elf_rel_t final {
        u64                     offset;
        u64                     info;
    };

    struct elf_rela_t final {
        u64                     offset;
        u64                     info;
        s64                     addend;
    };

    struct elf_note_t final {
        u64                     type;
        str::slice_t            name;
        str::slice_t            descriptor;
    };

    struct elf_dyn_t final {
        s64                     tag;
        u64                     value;
    };

    struct elf_strtab_t final {
        elf_str_list_t          strings;
        u32                     size;
    };

    struct elf_header_t final {
        const section_t*        section;
        union {
            struct {
                struct {
                    u64         offset;
                    u64         size;
                }               file;
                struct {
                    u64         virt;
                    u64         phys;
                    u64         size;
                }               addr;
                u64             align;
                u32             type;
                u32             flags;
            }                   segment;
            struct {
                struct {
                    u64         base;
                    u64         align;
                }               addr;
                u64             size;
                u64             flags;
                u64             offset;
                u64             entry_size;
                str::slice_t    slice;
                u32             link;
                u32             info;
                u32             type;
                u32             name_index;
            }                   section;
        }                       subclass;
        u32                     number;
        elf_header_type_t       type;
    };

    struct elf_opts_t final {
        alloc_t*                alloc;
        file_t*                 file;
        u64                     entry_point;
    };

    struct elf_t final {
        alloc_t*                alloc;
        elf_header_list_t       headers;
        elf_strtab_t            strings;
        elf_strtab_t            section_names;
        u64                     entry_point;
        struct {
            u64                 offset;
        }                       program;
        struct {
            u64                 offset;
        }                       section;
        u32                     num_section;
        u32                     num_segment;
        u32                     proc_flags;
        u16                     machine;
        u16                     str_ndx;
    };

    namespace elf {
        [[maybe_unused]] constexpr u32 class_64         = 2;
        [[maybe_unused]] constexpr u32 data_2lsb        = 1;
        [[maybe_unused]] constexpr u32 data_2msb        = 2;
        [[maybe_unused]] constexpr u32 os_abi_linux     = 3;
        [[maybe_unused]] constexpr u32 version_current  = 1;

        namespace note {
            [[maybe_unused]] constexpr u32 pr_status    = 1;
            [[maybe_unused]] constexpr u32 pr_fp_reg    = 2;
            [[maybe_unused]] constexpr u32 pr_rs_info   = 3;
            [[maybe_unused]] constexpr u32 task_struct  = 4;
            [[maybe_unused]] constexpr u32 gnu_prop     = 5;
            [[maybe_unused]] constexpr u32 auxv         = 6;

            [[maybe_unused]] constexpr u32 file         = 0x46494c45;
            [[maybe_unused]] constexpr u32 sig_info     = 0x53494749;
            [[maybe_unused]] constexpr u32 prx_fp_reg   = 0x46e62b7f;

            [[maybe_unused]] constexpr u32 i386_tls     = 0x200;
        }

        namespace machine {
            [[maybe_unused]] constexpr u16 x86_64       = 62;
            [[maybe_unused]] constexpr u16 aarch64      = 183;
        }

        namespace file {
            enum class type_t : u8 {
                none,
                rel,
                exec,
                dyn,
                core
            };

            [[maybe_unused]] constexpr u16 header_size  = 64;
        }

        namespace dynamic {
            enum class type_t : u8 {
                null_,
                needed,
                plt_rel_size,
                plt_got,
                hash,
                strtab,
                symtab,
                rela,
                rela_size,
                rela_ent,
                strsz,
                sym_ent,
                init,
                fini,
                soname,
                rpath,
                symbolic,
                rel,
                rel_size,
                rel_ent,
                plt_rel,
                debug,
                text_rel,
                jmp_rel,
//                bind_now,
//                init_array,
//                fini_array,
//                init_array_size,
//                fini_array_size,
                encoding
            };
        }

        namespace section {
            enum class type_t : u8 {
                null_,
                progbits,
                symtab,
                strtab,
                rela,
                hash,
                dynamic,
                note,
                nobits,
                rel,
                shlib,
                dynsym,
            };

            [[maybe_unused]] constexpr u16 header_size          = 64;

            namespace flags {
                [[maybe_unused]] constexpr u32 write            = 0x1;
                [[maybe_unused]] constexpr u32 alloc            = 0x2;
                [[maybe_unused]] constexpr u32 exec_instr       = 0x4;
                [[maybe_unused]] constexpr u32 mask_proc        = 0xf000000;
                [[maybe_unused]] constexpr u32 ro_after_init    = 0x0020000;
                [[maybe_unused]] constexpr u32 rela_live_patch  = 0x0010000;
            }

            namespace info {
            }

            namespace link {
            }

            namespace indexes {
                [[maybe_unused]] constexpr u32 undef            = 0;
                [[maybe_unused]] constexpr u32 live_abs         = 0xfff1;
                [[maybe_unused]] constexpr u32 live_patch       = 0xff20;
                [[maybe_unused]] constexpr u32 live_common      = 0xfff2;
            }
        }

        namespace segment {
            enum class type_t : u32 {
                null_,
                load,
                dynamic,
                interp,
                note,
                shlib,
                pgm_hdr,
            };

            [[maybe_unused]] constexpr u16 header_size          = 56;

            namespace flags {
                [[maybe_unused]] constexpr u32 read             = 0x4;
                [[maybe_unused]] constexpr u32 exec             = 0x1;
                [[maybe_unused]] constexpr u32 write            = 0x2;
            }
        }

        namespace strtab {
            u0 free(elf_strtab_t& strtab);

            u0 write(elf_strtab_t& strtab, file_t& file);

            u0 init(elf_strtab_t& strtab, alloc_t* alloc);

            u32 add_str(elf_strtab_t& strtab, str::slice_t str);
        }

        namespace symbol_table {
            enum class type_t : u8 {
                notype,
                object,
                func,
                section,
                file,
            };

            enum class scope_t : u8 {
                local,
                global,
                weak
            };
        }

        system_t* system();

        u0 free(elf_t& elf);

        u64 hash(const u8* name);

        u0 write_file_header(file_t& file, elf_t& elf);

        u0 write_symbol_hash(file_t& file, elf_t& elf);

        u0 write_pad_section(file_t& file, elf_t& elf);

        u0 write_symbol_table(file_t& file, elf_t& elf);

        u0 write_string_table(file_t& file, elf_t& elf);

        status_t write_sections(file_t& file, elf_t& elf);

        status_t write_segments(file_t& file, elf_t& elf);

        status_t init(elf_t& elf, const elf_opts_t& opts);

        u0 write_sym(file_t& file, elf_t& elf, const elf_sym_t& sym);

        u0 write_dyn(file_t& file, elf_t& elf, const elf_dyn_t& dyn);

        u0 write_rel(file_t& file, elf_t& elf, const elf_rel_t& rel);

        u0 write_rela(file_t& file, elf_t& elf, const elf_rela_t& rela);

        u0 write_note(file_t& file, elf_t& elf, const elf_note_t& note);

        status_t write_header(file_t& s, elf_t& elf, const elf_header_t& hdr);

        status_t get_section_name(const binfmt::section_t* section, str::slice_t& name);
    }
}

/* 64-bit ELF base types. */
//    typedef __u64	Elf64_Addr;
//    typedef __u16	Elf64_Half;
//    typedef __s16	Elf64_SHalf;
//    typedef __u64	Elf64_Off;
//    typedef __s32	Elf64_Sword;
//    typedef __u32	Elf64_Word;
//    typedef __u64	Elf64_Xword;
//    typedef __s64	Elf64_Sxword;

//    typedef struct elf64_hdr {
//        unsigned char	e_ident[EI_NIDENT];	/* ELF "magic number" */
//        Elf64_Half e_type;
//        Elf64_Half e_machine;
//        Elf64_Word e_version;
//        Elf64_Addr e_entry;		/* Entry point virtual address */
//        Elf64_Off e_phoff;		/* Program header table file offset */
//        Elf64_Off e_shoff;		/* Section header table file offset */
//        Elf64_Word e_flags;
//        Elf64_Half e_ehsize;
//        Elf64_Half e_phentsize;
//        Elf64_Half e_phnum;
//        Elf64_Half e_shentsize;
//        Elf64_Half e_shnum;
//        Elf64_Half e_shstrndx;
//    } Elf64_Ehdr;

//    typedef struct elf64_phdr {
//        Elf64_Word p_type;
//        Elf64_Word p_flags;
//        Elf64_Off p_offset;		/* Segment file offset */
//        Elf64_Addr p_vaddr;		/* Segment virtual address */
//        Elf64_Addr p_paddr;		/* Segment physical address */
//        Elf64_Xword p_filesz;		/* Segment size in file */
//        Elf64_Xword p_memsz;		/* Segment size in memory */
//        Elf64_Xword p_align;		/* Segment alignment, file & memory */
//    } Elf64_Phdr;

//    typedef struct elf64_shdr {
//        Elf64_Word sh_name;		/* Section name, index in string tbl */
//        Elf64_Word sh_type;		/* Type of section */
//        Elf64_Xword sh_flags;		/* Miscellaneous section attributes */
//        Elf64_Addr sh_addr;		/* Section virtual addr at execution */
//        Elf64_Off sh_offset;		/* Section file offset */
//        Elf64_Xword sh_size;		/* Size of section in bytes */
//        Elf64_Word sh_link;		/* Index of another section */
//        Elf64_Word sh_info;		/* Additional section information */
//        Elf64_Xword sh_addralign;	/* Section alignment */
//        Elf64_Xword sh_entsize;	/* Entry size if section holds table */
//    } Elf64_Shdr;

