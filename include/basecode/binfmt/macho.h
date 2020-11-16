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

namespace basecode::binfmt::io::macho {
    union lc_str_t final {
        u32                     offset;
#ifndef __LP64__
        s8*                     name;
#endif
    };

    struct fat_header_t final {
        u32                     magic;
        u32                     num_fat_arch;
    };

    struct fat_arch_t final {
        s32                     cpu_type;
        s32                     cpu_sub_type;
        u32                     offset;
        u32                     size;
        u32                     align;
    };

    struct file_header_t final {
        u32                     magic;
        s32                     cpu_type;
        s32                     cpu_sub_type;
        u32                     file_type;
        u32                     num_load_commands;
        u32                     size_of_commands;
        u32                     flags;
        u32                     reserved;
    };

    struct load_cmd_t final {
        u32                     cmd;
        u32                     size;
    };

    struct uuid_cmd_t final {
        load_cmd_t              hdr;
        u8                      uuid[16];
    };

    struct rpath_cmd_t final {
        load_cmd_t              hdr;
        lc_str_t                path;
    };

    struct dylib_cmd_t final {
        load_cmd_t              hdr;
        lc_str_t                name;
        u32                     timestamp;
        u32                     current_version;
        u32                     compatibility_version;
    };

    struct symtab_cmd_t final {
        load_cmd_t              hdr;
        u32                     sym_offs;
        u32                     num_syms;
        u32                     str_offs;
        u32                     str_size;
    };

    struct segment_cmd_t final {
        load_cmd_t              hdr;
        s8                      name[16];
        u64                     virt_addr;
        u64                     virt_size;
        u64                     file_offs;
        u64                     file_size;
        s32                     max_prot;
        s32                     init_prot;
        u32                     num_sects;
        u32                     flags;
    };

    struct sub_library_cmd_t final {
        load_cmd_t              hdr;
        lc_str_t                sub_library;
    };

    struct sub_umbrella_cmd_t final {
        load_cmd_t              hdr;
        lc_str_t                sub_umbrella;
    };

    struct sub_framework_cmd_t final {
        load_cmd_t              hdr;
        lc_str_t                umbrella;
    };

    struct sub_client_cmd_t final {
        load_cmd_t              hdr;
        lc_str_t                client;
    };

    struct prebind_cksum_cmd_t final {
        load_cmd_t              hdr;
        u32                     checksum;
    };

    struct link_edit_cmd_t final {
        load_cmd_t              hdr;
        u32                     data_offs;
        u32                     data_size;
    };

    struct dysymtab_cmd_t final {
        load_cmd_t              hdr;
        u32                     idx_local_sym;
        u32                     num_local_sym;
        u32                     idx_ext_def_sym;
        u32                     num_ext_def_sym;
        u32                     idx_undef_sym;
        u32                     num_undef_sym;
        u32                     toc_tab_offs;
        u32                     num_toc_entries;
        u32                     mod_tab_offs;
        u32                     num_mod_tab_entries;
        u32                     ext_ref_sym_offs;
        u32                     num_ext_ref_syms;
        u32                     indirect_sym_offs;
        u32                     num_indirect_syms;
        u32                     ext_reloc_offs;
        u32                     num_ext_reloc;
        u32                     local_reloc_offs;
        u32                     num_local_reloc;
    };

    struct dylinker_cmd_t final {
        load_cmd_t              hdr;
        lc_str_t                name;
    };

    struct entry_point_cmd_t final {
        load_cmd_t              hdr;
        u64                     file_offs;
        u64                     stack_size;
    };

    struct prebound_dylib_cmd_t final {
        load_cmd_t              hdr;
        lc_str_t                name;
        u32                     num_modules;
        lc_str_t                linked_modules;
    };

    struct encryption_info_cmd_t final {
        load_cmd_t              hdr;
        u32                     crypt_offs;
        u32                     crypt_size;
        u32                     crypt_id;
    };

    struct section_t final {
        s8                      name[16];
        s8                      segment_name[16];
        u64                     addr;
        u64                     size;
        u32                     offset;
        u32                     align;
        u32                     rel_offs;
        u32                     num_relocs;
        u32                     flags;
        u32                     reserved1;
        u32                     reserved2;
        u32                     reserved3;
    };

    struct nlist_t final {
        u32                     str_offs;
        u8                      type;
        u8                      sect_num;
        u16                     desc;
        u64                     value;
    };

    struct opts_t final {
        alloc_t*                alloc;
        file_t*                 file;
        u64                     entry_point;
    };

    struct macho_t final {
        alloc_t*                alloc;
        const opts_t*           opts;
        file_header_t*          file_header;
    };

    namespace fat {
        constexpr u32 magic                         = 0xcafebabe;
        constexpr u32 cigam                         = 0xbebafeca;
        constexpr u32 magic_64                      = 0xcafebabf;
        constexpr u32 cigam_64                      = 0xbfbafeca;
    }

    namespace file {
        constexpr u32 magic                         = 0xfeedface;
        constexpr u32 cigam                         = 0xcefaedfe;
        constexpr u32 magic_64                      = 0xfeedfacf;
        constexpr u32 cigam_64                      = 0xcffaedfe;

        namespace type {
            constexpr u32 object                    = 0x1;
            constexpr u32 execute                   = 0x2;
            constexpr u32 fvm_lib                   = 0x3;
            constexpr u32 core                      = 0x4;
            constexpr u32 preload                   = 0x5;
            constexpr u32 dylib                     = 0x6;
            constexpr u32 dylinker                  = 0x7;
            constexpr u32 bundle                    = 0x8;
            constexpr u32 dylib_stub                = 0x9;
            constexpr u32 dsym                      = 0xa;
            constexpr u32 kext_bundle               = 0xb;
        }

        namespace flags {
            constexpr u32 none                      = 0;
            constexpr u32 no_undefs                 = 0x1;
            constexpr u32 incr_link                 = 0x2;
            constexpr u32 dyld_link                 = 0x4;
            constexpr u32 bin_dat_load              = 0x8;
            constexpr u32 pre_bound                 = 0x10;
            constexpr u32 split_segs                = 0x20;
            constexpr u32 lazy_hint                 = 0x40;
            constexpr u32 two_level                 = 0x80;
            constexpr u32 force_flat                = 0x100;
            constexpr u32 no_multi_defs             = 0x200;
            constexpr u32 no_fix_pre_binding        = 0x400;
            constexpr u32 pre_bindable              = 0x800;
            constexpr u32 all_mods_bound            = 0x1000;
            constexpr u32 subsections_via_symbols   = 0x2000;
            constexpr u32 canonical                 = 0x4000;
            constexpr u32 weak_defines              = 0x8000;
            constexpr u32 binds_to_weak             = 0x10000;
            constexpr u32 allow_stack_exec          = 0x20000;
            constexpr u32 root_safe                 = 0x40000;
            constexpr u32 setuid_safe               = 0x80000;
            constexpr u32 no_reexported_dylibs      = 0x100000;
            constexpr u32 pie                       = 0x200000;
            constexpr u32 dead_strippable_dylib     = 0x400000;
        }
    }

    namespace name {
        constexpr u32 undef                         = 0;
        constexpr u32 abs                           = 2;
        constexpr u32 ext                           = 1;
        constexpr u32 sect                          = 0xe;
        constexpr u32 weak_ref                      = 0x0040;
        constexpr u32 weak_def                      = 0x0080;
    }

    namespace segment {
        namespace type {
            constexpr u32 null                      = 0;    // (no name)
            constexpr u32 page_zero                 = 1;    // __PAGEZERO
            constexpr u32 text                      = 2;    // __TEXT
                                                            //  sections:
                                                            //      __text
                                                            //      __cstring
                                                            //      __picsymbol_stub
                                                            //      __symbol_stub
                                                            //      __const
                                                            //      __literal4
                                                            //      __literal8
            constexpr u32 data                      = 3;    // __DATA
                                                            //  sections:
                                                            //      __data
                                                            //      __la_symbol_ptr
                                                            //      __nl_symbol_ptr
                                                            //      __dyld
                                                            //      __const
                                                            //      __mod_init_func
                                                            //      __mod_term_func
                                                            //      __bss
                                                            //      __common
            constexpr u32 objc                      = 4;    // __OBJC
            constexpr u32 import                    = 5;    // __IMPORT
                                                            //  sections:
                                                            //      __jump_table
                                                            //      __pointers
            constexpr u32 link_edit                 = 6;    // __LINKEDIT
        }
    }

    namespace section {
        namespace type {
            constexpr u32 unknown                   = 0;
            constexpr u32 discard                   = 1;
            constexpr u32 text                      = 2;
            constexpr u32 stubs                     = 3;
            constexpr u32 ro_data                   = 4;
            constexpr u32 uw_info                   = 5;
            constexpr u32 nl_ptr                    = 6;
            constexpr u32 la_ptr                    = 7;
            constexpr u32 init                      = 8;
            constexpr u32 fini                      = 9;
            constexpr u32 rw_data                   = 10;
            constexpr u32 bss                       = 11;
            constexpr u32 link_edit                 = 12;
        }

        namespace flags {
            constexpr u32 regular                   = 0x0;
            constexpr u32 zero_fill                 = 0x1;
            constexpr u32 non_lazy_syms             = 0x06;
            constexpr u32 init_func_ptr             = 0x09;
            constexpr u32 term_func_ptr             = 0x0a;
            constexpr u32 attr_pure_instr           = 0x80000000;
            constexpr u32 attr_some_instr           = 0x00000400;
        }
    }

    namespace machine {
        namespace type {
            constexpr s32 unknown                   = 0;
            constexpr s32 any                       = -1;
            constexpr s32 aarch64                   = 12;
            constexpr s32 x86_64                    = 0x10000007;
        }

        namespace sub_type {
            constexpr u32 unknown                   = 0;
            constexpr u32 lib64                     = 0x80000000;
        }
    }

    namespace load_command {
        constexpr u32 req_dyld                      = 0x80000000;

        namespace type {
            constexpr u32 segment                   = 0x1;
            constexpr u32 symtab                    = 0x2;
            constexpr u32 symseg                    = 0x3;
            constexpr u32 thread                    = 0x4;
            constexpr u32 unix_thread               = 0x5;
            constexpr u32 load_fvm_lib              = 0x6;
            constexpr u32 id_fvm_lib                = 0x7;
            constexpr u32 ident                     = 0x8;
            constexpr u32 fvm_file                  = 0x9;
            constexpr u32 pre_page                  = 0xa;
            constexpr u32 dysymtab                  = 0xb;
            constexpr u32 load_dylib                = 0xc;
            constexpr u32 id_dylib                  = 0xd;
            constexpr u32 load_dylinker             = 0xe;
            constexpr u32 id_dylinker               = 0xf;
            constexpr u32 prebound_dylib            = 0x10;
            constexpr u32 routines                  = 0x11;
            constexpr u32 sub_framework             = 0x12;
            constexpr u32 sub_umbrella              = 0x13;
            constexpr u32 sub_client                = 0x14;
            constexpr u32 sub_library               = 0x15;
            constexpr u32 two_level_hints           = 0x16;
            constexpr u32 prebind_cksum             = 0x17;
            constexpr u32 load_weak_dylib           = (u32(0x18) | req_dyld);
            constexpr u32 segment_64                = 0x19;
            constexpr u32 routines_64               = 0x1a;
            constexpr u32 uuid                      = 0x1b;
            constexpr u32 rpath                     = (u32(0x1c) | req_dyld);
            constexpr u32 code_signature            = 0x1d;
            constexpr u32 segment_split_info        = 0x1e;
            constexpr u32 reexport_dylib            = (u32(0x1f) | req_dyld);
            constexpr u32 lazy_load_dylib           = 0x20;
            constexpr u32 encryption_info           = 0x21;
            constexpr u32 dyld_info                 = 0x22;
            constexpr u32 dyld_info_only            = (u32(0x22) | req_dyld);
            constexpr u32 main                      = (u32(0x28) | req_dyld);
        }
    }

    u0 free(macho_t& macho);

    status_t read(macho_t& macho, file_t& file);

    status_t write(macho_t& macho, file_t& file);

    status_t get_section_name(const module_t* module,
                              const binfmt::section_t* section,
                              str::slice_t& name);

    status_t init(macho_t& macho, const opts_t& opts);
}
