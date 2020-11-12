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
#include <basecode/core/str_array.h>

#define ELF64_R_SYM(i)              ((i) >> 32)
#define ELF64_R_TYPE(i)             ((i) & 0xffffffff)
#define ELF64_R_INFO(sym, type)     ((((u64) (sym)) << 32) + (type))
#define ELF64_ST_BIND(val)          (((u8) (val)) >> 4)
#define ELF64_ST_TYPE(val)          ((val) & 0xf)
#define ELF64_ST_VISIBILITY(o)      ((o) & 0x03)
#define ELF64_ST_INFO(bind, type)   (((bind) <<4 ) + ((type) & 0xf))

// XXX:
//  STT_GNU_IFUNC?
//
//  - .common section?
//  - .got section
//  - .plt section
//  - .gnu.version/.gnu.version.r
//  - init/fini array symbols
//
namespace basecode::binfmt::io::elf {
   struct file_header_t final {
        u8                      magic[16];
        u16                     type;
        u16                     machine;
        u32                     version;
        u64                     entry_point;
        u64                     pgm_hdr_offset;
        u64                     sect_hdr_offset;
        u32                     flags;
        u16                     header_size;
        u16                     pgm_hdr_size;
        u16                     pgm_hdr_count;
        u16                     sect_hdr_size;
        u16                     sect_hdr_count;
        u16                     strtab_ndx;
    };

    struct pgm_header_t final {
        u32                     type;
        u32                     flags;
        u64                     offset;
        u64                     virt_addr;
        u64                     phys_addr;
        u64                     file_size;
        u64                     mem_size;
        u64                     align;
    };

    struct sect_header_t final {
        u32                     name_offset;
        u32                     type;
        u64                     flags;
        u64                     addr;
        u64                     offset;
        u64                     size;
        u32                     link;
        u32                     info;
        u64                     addr_align;
        u64                     entity_size;
    };

    struct ver_t final {
        u16                     version;
        u16                     flags;
        u16                     index;
        u16                     aux_count;
        u32                     hash;
        u32                     aux_offset;
        u32                     next_offset;
    };

    struct var_aux_t final {
        u32                     name;
        u32                     next_offset;
    };

    struct ver_need_t final {
        u16                     version;
        u16                     aux_count;
        u32                     file_offset;
        u32                     aux_offset;
        u32                     next_offset;
    };

    struct ver_need_aux_t final {
        u32                     hash;
        u16                     flags;
        u16                     other;
        u32                     name_offset;
        u32                     next_offset;
    };

    struct sym_t final {
        u32                     name_offset;
        u8                      info;
        u8                      other;
        u16                     section_ndx;
        u64                     value;
        u64                     size;
    };

    struct sym_bind_t final {
        u16                     bound_to;
        u16                     flags;
    };

    struct rel_t final {
        u64                     offset;
        u64                     info;
    };

    struct rela_t final {
        u64                     offset;
        u64                     info;
        s64                     addend;
    };

    struct note_header_t final {
        u32                     name_size;
        u32                     desc_size;
        u32                     type;
    };

    struct dyn_t final {
        s64                     tag;
        u64                     value;
    };

    struct group_t final {
        u32                     flags;
        u32                     sect_hdr_indexes[0];
    };

    struct opts_t final {
        alloc_t*                alloc;
        file_t*                 file;
        symbol_t**              syms;
        str::slice_t*           strs;
        u64                     entry_point;
        u32                     strtab_idx;
        u32                     strtab_size;
        u32                     num_symbols;
        u32                     num_segments;
        u32                     num_sections;
        u32                     flags;
        u8                      clazz;
        u8                      os_abi;
        u8                      version;
        u8                      endianess;
        u8                      abi_version;
    };

    struct elf_t final {
        alloc_t*                alloc;
        const opts_t*           opts;
        file_header_t*          file_header;
        pgm_header_t*           segments;
        sect_header_t*          sections;
        struct {
            sect_header_t*      sect;
            u32                 offset;
        }                       strtab;
        struct {
            sect_header_t*      sect;
            u32                 offset;
        }                       symtab;
    };

    [[maybe_unused]] constexpr u32 class_64         = 2;
    [[maybe_unused]] constexpr u32 data_2lsb        = 1;
    [[maybe_unused]] constexpr u32 data_2msb        = 2;
    [[maybe_unused]] constexpr u32 os_abi_sysv      = 0;
    [[maybe_unused]] constexpr u32 os_abi_gnu       = 3;
    [[maybe_unused]] constexpr u32 version_current  = 1;

    namespace note {
        [[maybe_unused]] constexpr u32 header_size  = sizeof(note_header_t);
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

    namespace file {
        [[maybe_unused]] constexpr u16 header_size          = sizeof(file_header_t);
        [[maybe_unused]] constexpr u16 magic_class          = 4;
        [[maybe_unused]] constexpr u16 magic_data           = 5;
        [[maybe_unused]] constexpr u16 magic_version        = 6;
        [[maybe_unused]] constexpr u16 magic_os_abi         = 7;
        [[maybe_unused]] constexpr u16 magic_abi_version    = 8;

        namespace type {
            [[maybe_unused]] constexpr u8 none              = 0;
            [[maybe_unused]] constexpr u8 rel               = 1;
            [[maybe_unused]] constexpr u8 exec              = 2;
            [[maybe_unused]] constexpr u8 dyn               = 3;
            [[maybe_unused]] constexpr u8 core              = 4;
        }

        str::slice_t class_name(u8 cls);

        str::slice_t os_abi_name(u8 os_abi);

        str::slice_t version_name(u8 version);

        str::slice_t file_type_name(u16 type);

        str::slice_t endianess_name(u8 endianess);
    }

    namespace group {
        [[maybe_unused]] constexpr u32 entity_size          = sizeof(u32);
    }

    namespace relocs {
        [[maybe_unused]] constexpr u32 entity_size              = sizeof(rela_t);

        namespace x86_64 {
            [[maybe_unused]] constexpr u32 none                 = 0;
            [[maybe_unused]] constexpr u32 d64                  = 1;
            [[maybe_unused]] constexpr u32 pc32                 = 2;
            [[maybe_unused]] constexpr u32 got32                = 3;
            [[maybe_unused]] constexpr u32 plt32                = 4;
            [[maybe_unused]] constexpr u32 copy                 = 5;
            [[maybe_unused]] constexpr u32 glob_dat             = 6;
            [[maybe_unused]] constexpr u32 jmp_slot             = 7;
            [[maybe_unused]] constexpr u32 relative             = 8;
            [[maybe_unused]] constexpr u32 got_pc_rel           = 9;
            [[maybe_unused]] constexpr u32 dir32_zx             = 10;
            [[maybe_unused]] constexpr u32 dir32_sx             = 11;
            [[maybe_unused]] constexpr u32 dir16_zx             = 12;
            [[maybe_unused]] constexpr u32 pc_rel_16_sx         = 13;
            [[maybe_unused]] constexpr u32 dir8_sx              = 14;
            [[maybe_unused]] constexpr u32 pc_rel_8_sx          = 15;
            [[maybe_unused]] constexpr u32 dtp_mod_64           = 16;
            [[maybe_unused]] constexpr u32 dtp_off_64           = 17;
            [[maybe_unused]] constexpr u32 tp_off_64            = 18;
            [[maybe_unused]] constexpr u32 tls_gd               = 19;
            [[maybe_unused]] constexpr u32 tls_ld               = 20;
            [[maybe_unused]] constexpr u32 dtp_off_32           = 21;
            [[maybe_unused]] constexpr u32 got_tp_off           = 22;
            [[maybe_unused]] constexpr u32 tp_off_32            = 23;
            [[maybe_unused]] constexpr u32 pc64                 = 24;
            [[maybe_unused]] constexpr u32 got_off_64           = 25;
            [[maybe_unused]] constexpr u32 got_pc_32            = 26;
            [[maybe_unused]] constexpr u32 got_64               = 27;
            [[maybe_unused]] constexpr u32 got_pc_rel_64        = 28;
            [[maybe_unused]] constexpr u32 got_pc_64            = 29;
            [[maybe_unused]] constexpr u32 got_plt_64           = 30;
            [[maybe_unused]] constexpr u32 plt_off_64           = 31;
            [[maybe_unused]] constexpr u32 size32               = 32;
            [[maybe_unused]] constexpr u32 size64               = 33;
            [[maybe_unused]] constexpr u32 got_pc_32_tls_desc   = 34;
            [[maybe_unused]] constexpr u32 tls_desc_call        = 35;
            [[maybe_unused]] constexpr u32 tls_desc             = 36;
            [[maybe_unused]] constexpr u32 i_relative           = 37;
            [[maybe_unused]] constexpr u32 relative64           = 38;
            [[maybe_unused]] constexpr u32 got_pc_relx          = 41;
            [[maybe_unused]] constexpr u32 rex_got_pc_relx      = 42;
        }

        namespace aarch64 {
            [[maybe_unused]] constexpr u32 none                         = 0;
            [[maybe_unused]] constexpr u32 abs64                        = 257;
            [[maybe_unused]] constexpr u32 abs32                        = 258;
            [[maybe_unused]] constexpr u32 abs16                        = 259;
            [[maybe_unused]] constexpr u32 prel64                       = 260;
            [[maybe_unused]] constexpr u32 prel32                       = 261;
            [[maybe_unused]] constexpr u32 prel16                       = 262;
            [[maybe_unused]] constexpr u32 movw_uabs_g0                 = 263;
            [[maybe_unused]] constexpr u32 movw_uabs_g0_nc              = 264;
            [[maybe_unused]] constexpr u32 movw_uabs_g1                 = 265;
            [[maybe_unused]] constexpr u32 movw_uabs_g1_nc              = 266;
            [[maybe_unused]] constexpr u32 movw_uabs_g2                 = 267;
            [[maybe_unused]] constexpr u32 movw_uabs_g2_nc              = 268;
            [[maybe_unused]] constexpr u32 movw_uabs_g3                 = 269;
            [[maybe_unused]] constexpr u32 movw_sabs_g0                 = 270;
            [[maybe_unused]] constexpr u32 movw_sabs_g1                 = 271;
            [[maybe_unused]] constexpr u32 movw_sabs_g2                 = 272;
            [[maybe_unused]] constexpr u32 ld_prel_lo19                 = 273;
            [[maybe_unused]] constexpr u32 adr_prel_lo21                = 274;
            [[maybe_unused]] constexpr u32 adr_prel_pg_hi21             = 275;
            [[maybe_unused]] constexpr u32 adr_prel_pg_hi21_nc          = 276;
            [[maybe_unused]] constexpr u32 add_abs_lo12_nc              = 277;
            [[maybe_unused]] constexpr u32 ldst8_abs_lo12_nc            = 278;
            [[maybe_unused]] constexpr u32 tstbr14                      = 279;
            [[maybe_unused]] constexpr u32 condbr19                     = 280;
            [[maybe_unused]] constexpr u32 jump26                       = 282;
            [[maybe_unused]] constexpr u32 call26                       = 283;
            [[maybe_unused]] constexpr u32 ldst16_abs_lo12_nc           = 284;
            [[maybe_unused]] constexpr u32 ldst32_abs_lo12_nc           = 285;
            [[maybe_unused]] constexpr u32 ldst64_abs_lo12_nc           = 286;
            [[maybe_unused]] constexpr u32 movw_prel_g0                 = 287;
            [[maybe_unused]] constexpr u32 movw_prel_g0_nc              = 288;
            [[maybe_unused]] constexpr u32 movw_prel_g1                 = 289;
            [[maybe_unused]] constexpr u32 movw_prel_g1_nc              = 290;
            [[maybe_unused]] constexpr u32 movw_prel_g2                 = 291;
            [[maybe_unused]] constexpr u32 movw_prel_g2_nc              = 292;
            [[maybe_unused]] constexpr u32 movw_prel_g3                 = 293;
            [[maybe_unused]] constexpr u32 ldst128_abs_lo12_nc          = 299;
            [[maybe_unused]] constexpr u32 movw_gotoff_g0               = 300;
            [[maybe_unused]] constexpr u32 movw_gotoff_g0_nc            = 301;
            [[maybe_unused]] constexpr u32 movw_gotoff_g1               = 302;
            [[maybe_unused]] constexpr u32 movw_gotoff_g1_nc            = 303;
            [[maybe_unused]] constexpr u32 movw_gotoff_g2               = 304;
            [[maybe_unused]] constexpr u32 movw_gotoff_g2_nc            = 305;
            [[maybe_unused]] constexpr u32 movw_gotoff_g3               = 306;
            [[maybe_unused]] constexpr u32 gotrel64                     = 307;
            [[maybe_unused]] constexpr u32 gotrel32                     = 308;
            [[maybe_unused]] constexpr u32 got_ld_prel19                = 309;
            [[maybe_unused]] constexpr u32 ld64_gotoff_lo15             = 310;
            [[maybe_unused]] constexpr u32 adr_got_page                 = 311;
            [[maybe_unused]] constexpr u32 ld64_got_lo12_nc             = 312;
            [[maybe_unused]] constexpr u32 ld64_gotpage_lo15            = 313;
            [[maybe_unused]] constexpr u32 tlsgd_adr_prel21             = 512;
            [[maybe_unused]] constexpr u32 tlsgd_adr_page21             = 513;
            [[maybe_unused]] constexpr u32 tlsgd_add_lo12_nc            = 514;
            [[maybe_unused]] constexpr u32 tlsgd_movw_g1                = 515;
            [[maybe_unused]] constexpr u32 tlsgd_movw_g0_nc             = 516;
            [[maybe_unused]] constexpr u32 tlsld_adr_prel21             = 517;
            [[maybe_unused]] constexpr u32 tlsld_adr_page21             = 518;
            [[maybe_unused]] constexpr u32 tlsld_add_lo12_nc            = 519;
            [[maybe_unused]] constexpr u32 tlsld_movw_g1                = 520;
            [[maybe_unused]] constexpr u32 tlsld_movw_g0_nc             = 521;
            [[maybe_unused]] constexpr u32 tlsld_ld_prel19              = 522;
            [[maybe_unused]] constexpr u32 tlsld_movw_dtprel_g2         = 523;
            [[maybe_unused]] constexpr u32 tlsld_movw_dtprel_g1         = 524;
            [[maybe_unused]] constexpr u32 tlsld_movw_dtprel_g1_nc      = 525;
            [[maybe_unused]] constexpr u32 tlsld_movw_dtprel_g0         = 526;
            [[maybe_unused]] constexpr u32 tlsld_movw_dtprel_g0_nc      = 527;
            [[maybe_unused]] constexpr u32 tlsld_add_dtprel_hi12        = 528;
            [[maybe_unused]] constexpr u32 tlsld_add_dtprel_lo12        = 529;
            [[maybe_unused]] constexpr u32 tlsld_add_dtprel_lo12_nc     = 530;
            [[maybe_unused]] constexpr u32 tlsld_ldst8_dtprel_lo12      = 531;
            [[maybe_unused]] constexpr u32 tlsld_ldst8_dtprel_lo12_nc   = 532;
            [[maybe_unused]] constexpr u32 tlsld_ldst16_dtprel_lo12     = 533;
            [[maybe_unused]] constexpr u32 tlsld_ldst16_dtprel_lo12_nc  = 534;
            [[maybe_unused]] constexpr u32 tlsld_ldst32_dtprel_lo12     = 535;
            [[maybe_unused]] constexpr u32 tlsld_ldst32_dtprel_lo12_nc  = 536;
            [[maybe_unused]] constexpr u32 tlsld_ldst64_dtprel_lo12     = 537;
            [[maybe_unused]] constexpr u32 tlsld_ldst64_dtprel_lo12_nc  = 538;
            [[maybe_unused]] constexpr u32 tlsie_movw_gottprel_g1       = 539;
            [[maybe_unused]] constexpr u32 tlsie_movw_gottprel_g0_nc    = 540;
            [[maybe_unused]] constexpr u32 tlsie_adr_gottprel_page21    = 541;
            [[maybe_unused]] constexpr u32 tlsie_ld64_gottprel_lo12_nc  = 542;
            [[maybe_unused]] constexpr u32 tlsie_ld_gottprel_prel19     = 543;
            [[maybe_unused]] constexpr u32 tlsle_movw_tprel_g2          = 544;
            [[maybe_unused]] constexpr u32 tlsle_movw_tprel_g1          = 545;
            [[maybe_unused]] constexpr u32 tlsle_movw_tprel_g1_nc       = 546;
            [[maybe_unused]] constexpr u32 tlsle_movw_tprel_g0          = 547;
            [[maybe_unused]] constexpr u32 tlsle_movw_tprel_g0_nc       = 548;
            [[maybe_unused]] constexpr u32 tlsle_add_tprel_hi12         = 549;
            [[maybe_unused]] constexpr u32 tlsle_add_tprel_lo12         = 550;
            [[maybe_unused]] constexpr u32 tlsle_add_tprel_lo12_nc      = 551;
            [[maybe_unused]] constexpr u32 tlsle_ldst8_tprel_lo12       = 552;
            [[maybe_unused]] constexpr u32 tlsle_ldst8_tprel_lo12_nc    = 553;
            [[maybe_unused]] constexpr u32 tlsle_ldst16_tprel_lo12      = 554;
            [[maybe_unused]] constexpr u32 tlsle_ldst16_tprel_lo12_nc   = 555;
            [[maybe_unused]] constexpr u32 tlsle_ldst32_tprel_lo12      = 556;
            [[maybe_unused]] constexpr u32 tlsle_ldst32_tprel_lo12_nc   = 557;
            [[maybe_unused]] constexpr u32 tlsle_ldst64_tprel_lo12      = 558;
            [[maybe_unused]] constexpr u32 tlsle_ldst64_tprel_lo12_nc   = 559;
            [[maybe_unused]] constexpr u32 tlsdesc_ld_prel19            = 560;
            [[maybe_unused]] constexpr u32 tlsdesc_adr_prel21           = 561;
            [[maybe_unused]] constexpr u32 tlsdesc_adr_page21           = 562;
            [[maybe_unused]] constexpr u32 tlsdesc_ld64_lo12            = 563;
            [[maybe_unused]] constexpr u32 tlsdesc_add_lo12             = 564;
            [[maybe_unused]] constexpr u32 tlsdesc_off_g1               = 565;
            [[maybe_unused]] constexpr u32 tlsdesc_off_g0_nc            = 566;
            [[maybe_unused]] constexpr u32 tlsdesc_ldr                  = 567;
            [[maybe_unused]] constexpr u32 tlsdesc_add                  = 568;
            [[maybe_unused]] constexpr u32 tlsdesc_call                 = 569;
            [[maybe_unused]] constexpr u32 tlsle_ldst128_tprel_lo12     = 570;
            [[maybe_unused]] constexpr u32 tlsle_ldst128_tprel_lo12_nc  = 571;
            [[maybe_unused]] constexpr u32 tlsld_ldst128_dtprel_lo12    = 572;
            [[maybe_unused]] constexpr u32 tlsld_ldst128_dtprel_lo12_nc = 573;
            [[maybe_unused]] constexpr u32 copy                         = 1024;
            [[maybe_unused]] constexpr u32 glob_dat                     = 1025;
            [[maybe_unused]] constexpr u32 jump_slot                    = 1026;
            [[maybe_unused]] constexpr u32 relative                     = 1027;
            [[maybe_unused]] constexpr u32 tls_dtpmod64                 = 1028;
            [[maybe_unused]] constexpr u32 tls_dtprel64                 = 1029;
            [[maybe_unused]] constexpr u32 tls_tprel64                  = 1030;
            [[maybe_unused]] constexpr u32 tls_desc                     = 1031;
            [[maybe_unused]] constexpr u32 i_relative                   = 1032;
        }
    }

    namespace version {
        namespace def {
            [[maybe_unused]] constexpr u32 none     = 0;
            [[maybe_unused]] constexpr u32 current  = 1;
            [[maybe_unused]] constexpr u32 number   = 2;
        }

        namespace flags {
            [[maybe_unused]] constexpr u32 base     = 1;
            [[maybe_unused]] constexpr u32 weak     = 2;
        }

        namespace index {
            [[maybe_unused]] constexpr u32 local    = 0;
            [[maybe_unused]] constexpr u32 global   = 1;
        }
    }

    namespace machine {
        [[maybe_unused]] constexpr u16 x86_64       = 62;
        [[maybe_unused]] constexpr u16 aarch64      = 183;

        str::slice_t name(u16 type);
    }

    namespace dynamic {
        namespace type {
            [[maybe_unused]] constexpr u8 null              = 0;
            [[maybe_unused]] constexpr u8 needed            = 1;
            [[maybe_unused]] constexpr u8 plt_rel_size      = 2;
            [[maybe_unused]] constexpr u8 plt_got           = 3;
            [[maybe_unused]] constexpr u8 hash              = 4;
            [[maybe_unused]] constexpr u8 strtab            = 5;
            [[maybe_unused]] constexpr u8 symtab            = 6;
            [[maybe_unused]] constexpr u8 rela              = 7;
            [[maybe_unused]] constexpr u8 rela_size         = 8;
            [[maybe_unused]] constexpr u8 rela_ent          = 9;
            [[maybe_unused]] constexpr u8 strsz             = 10;
            [[maybe_unused]] constexpr u8 sym_ent           = 11;
            [[maybe_unused]] constexpr u8 init              = 12;
            [[maybe_unused]] constexpr u8 fini              = 13;
            [[maybe_unused]] constexpr u8 soname            = 14;
            [[maybe_unused]] constexpr u8 rpath             = 15;
            [[maybe_unused]] constexpr u8 symbolic          = 16;
            [[maybe_unused]] constexpr u8 rel               = 17;
            [[maybe_unused]] constexpr u8 rel_size          = 18;
            [[maybe_unused]] constexpr u8 rel_ent           = 19;
            [[maybe_unused]] constexpr u8 plt_rel           = 20;
            [[maybe_unused]] constexpr u8 debug             = 21;
            [[maybe_unused]] constexpr u8 text_rel          = 22;
            [[maybe_unused]] constexpr u8 jmp_rel           = 23;
            [[maybe_unused]] constexpr u8 encoding          = 32;
        }
    }

    namespace section {
        [[maybe_unused]] constexpr u16 header_size          = 64;

        namespace type {
            [[maybe_unused]] constexpr u32 null             = 0;
            [[maybe_unused]] constexpr u32 progbits         = 1;
            [[maybe_unused]] constexpr u32 symtab           = 2;
            [[maybe_unused]] constexpr u32 strtab           = 3;
            [[maybe_unused]] constexpr u32 rela             = 4;
            [[maybe_unused]] constexpr u32 hash             = 5;
            [[maybe_unused]] constexpr u32 dynamic          = 6;
            [[maybe_unused]] constexpr u32 note             = 7;
            [[maybe_unused]] constexpr u32 nobits           = 8;
            [[maybe_unused]] constexpr u32 rel              = 9;
            [[maybe_unused]] constexpr u32 shlib            = 10;
            [[maybe_unused]] constexpr u32 dynsym           = 11;
            [[maybe_unused]] constexpr u32 init_array       = 14;
            [[maybe_unused]] constexpr u32 fini_array       = 15;
            [[maybe_unused]] constexpr u32 pre_init_array   = 16;
            [[maybe_unused]] constexpr u32 group            = 17;
            [[maybe_unused]] constexpr u32 symtab_shndx     = 18;
            [[maybe_unused]] constexpr u32 low_os           = 0x60000000;
            [[maybe_unused]] constexpr u32 gnu_eh_frame     = 0x6474e550;
            [[maybe_unused]] constexpr u32 gnu_stack        = 0x6474e551;
            [[maybe_unused]] constexpr u32 gnu_rel_ro       = 0x6474e552;
            [[maybe_unused]] constexpr u32 gnu_attributes   = 0x6ffffff5;
            [[maybe_unused]] constexpr u32 gnu_hash         = 0x6ffffff6;
            [[maybe_unused]] constexpr u32 gnu_lib_list     = 0x6ffffff7;
            [[maybe_unused]] constexpr u32 checksum         = 0x6ffffff8;
            [[maybe_unused]] constexpr u32 gnu_ver_def      = 0x6ffffffd;
            [[maybe_unused]] constexpr u32 gnu_ver_need     = 0x6ffffffe;
            [[maybe_unused]] constexpr u32 gnu_ver_sym      = 0x6fffffff;
            [[maybe_unused]] constexpr u32 high_os          = 0x6fffffff;
            [[maybe_unused]] constexpr u32 low_proc         = 0x70000000;
            [[maybe_unused]] constexpr u32 x86_64_unwind    = 0x70000001;
            [[maybe_unused]] constexpr u32 high_proc        = 0x7fffffff;
            [[maybe_unused]] constexpr u32 low_user         = 0x80000000;
            [[maybe_unused]] constexpr u32 high_user        = 0x8fffffff;

            str::slice_t name(u32 type);
        }

        namespace flags {
            [[maybe_unused]] constexpr u32 write            = 0x1;
            [[maybe_unused]] constexpr u32 alloc            = 0x2;
            [[maybe_unused]] constexpr u32 exec_instr       = 0x4;
            [[maybe_unused]] constexpr u32 merge            = 0x10;
            [[maybe_unused]] constexpr u32 strings          = 0x20;
            [[maybe_unused]] constexpr u32 info_link        = 0x40;
            [[maybe_unused]] constexpr u32 link_order       = 0x80;
            [[maybe_unused]] constexpr u32 os_non_conform   = 0x100;
            [[maybe_unused]] constexpr u32 group            = 0x200;
            [[maybe_unused]] constexpr u32 tls              = 0x400;
            [[maybe_unused]] constexpr u32 compressed       = 0x800;
            [[maybe_unused]] constexpr u32 mask_os          = 0x0ff00000;
            [[maybe_unused]] constexpr u32 mask_proc        = 0xf0000000;
            [[maybe_unused]] constexpr u32 ordered          = 0x40000000;
            [[maybe_unused]] constexpr u32 exclude          = 0x80000000;

            const s8* name(u32 flag);

            u0 chars(u32 flags, s8* chars);

            u0 names(u32 flags, const s8* names[13]);
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
        [[maybe_unused]] constexpr u16 header_size          = 56;

        namespace type {
            [[maybe_unused]] constexpr u32 null             = 0x0;
            [[maybe_unused]] constexpr u32 load             = 0x1;
            [[maybe_unused]] constexpr u32 dynamic          = 0x2;
            [[maybe_unused]] constexpr u32 interp           = 0x3;
            [[maybe_unused]] constexpr u32 note             = 0x4;
            [[maybe_unused]] constexpr u32 shlib            = 0x5;
            [[maybe_unused]] constexpr u32 pgm_hdr          = 0x6;
            [[maybe_unused]] constexpr u32 tls              = 0x7;
            [[maybe_unused]] constexpr u32 gnu_eh_frame     = 0x6474e550;
            [[maybe_unused]] constexpr u32 gnu_property     = 0x6474e553;
        }

        namespace flags {
            [[maybe_unused]] constexpr u32 read             = 0x4;
            [[maybe_unused]] constexpr u32 exec             = 0x1;
            [[maybe_unused]] constexpr u32 write            = 0x2;
        }
    }

    namespace strtab {
        u32 add(elf_t& elf, str::slice_t str);

        const s8* get(const elf_t& elf, u32 offset);
    }

    namespace symtab {
        [[maybe_unused]] constexpr u32 entity_size          = 24;

        namespace type {
            [[maybe_unused]] constexpr u8 notype            = 0x0;
            [[maybe_unused]] constexpr u8 object            = 0x1;
            [[maybe_unused]] constexpr u8 func              = 0x2;
            [[maybe_unused]] constexpr u8 section           = 0x3;
            [[maybe_unused]] constexpr u8 file              = 0x4;
            [[maybe_unused]] constexpr u8 common            = 0x5;
            [[maybe_unused]] constexpr u8 tls               = 0x6;
        }

        namespace scope {
            [[maybe_unused]] constexpr u8 local             = 0x0;
            [[maybe_unused]] constexpr u8 global            = 0x1;
            [[maybe_unused]] constexpr u8 weak              = 0x2;
        }

        namespace visibility {
            [[maybe_unused]] constexpr u8 default_          = 0x0;
            [[maybe_unused]] constexpr u8 internal          = 0x1;
            [[maybe_unused]] constexpr u8 hidden            = 0x2;
            [[maybe_unused]] constexpr u8 protected_        = 0x3;
        }

        u64 hash_name(str::slice_t str);

        sym_t* get(const elf_t& elf, u32 sect_num, u32 sym_idx);
    }

    namespace hashtab {
        [[maybe_unused]] constexpr u32 entity_size          = sizeof(u32);
    }

    u0 free(elf_t& elf);

    status_t read(elf_t& elf, file_t& file);

    status_t write(elf_t& elf, file_t& file);

    status_t init(elf_t& elf, const opts_t& opts);

    status_t get_section_name(const module_t* module, const binfmt::section_t* section, str::slice_t& name);
}
