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
// Copyright (C) 2017-2021 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <basecode/binfmt/io.h>

#define ELF64_R_SYM(i)              ((i) >> u32(32))
#define ELF64_R_TYPE(i)             ((i) & u32(0xffffffff))
#define ELF64_R_INFO(sym, type)     ((((u64) (sym)) << u32(32)) + (type))
#define ELF64_ST_BIND(val)          (((u8) (val)) >> u32(4))
#define ELF64_ST_TYPE(val)          ((val) & u32(0xf))
#define ELF64_ST_VISIBILITY(o)      ((o) & u32(0x03))
#define ELF64_ST_INFO(bind, type)   (((bind) << u32(4)) + ((type) & u32(0xf)))

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

    struct rela_t final {
        u64                     offset;
        u64                     info;
        s64                     addend;
    };

    struct note_header_t final {
        u32                     name_size;
        u32                     desc_size;
        u32                     type;
        u8                      data[0];
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
        u64                     entry_point;
        u32                     header_offset;
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
    };

    constexpr u32 class_64         = 2;
    constexpr u32 data_2lsb        = 1;
    constexpr u32 data_2msb        = 2;
    constexpr u32 os_abi_sysv      = 0;
    constexpr u32 os_abi_gnu       = 3;
    constexpr u32 version_current  = 1;

    namespace note {
        constexpr u32 header_size  = sizeof(note_header_t);
        constexpr u32 pr_status    = 1;
        constexpr u32 pr_fp_reg    = 2;
        constexpr u32 pr_rs_info   = 3;
        constexpr u32 task_struct  = 4;
        constexpr u32 gnu_prop     = 5;
        constexpr u32 auxv         = 6;

        constexpr u32 file         = 0x46494c45;
        constexpr u32 sig_info     = 0x53494749;
        constexpr u32 prx_fp_reg   = 0x46e62b7f;

        constexpr u32 i386_tls     = 0x200;
    }

    namespace file {
        constexpr u16 header_size          = sizeof(file_header_t);
        constexpr u16 magic_class          = 4;
        constexpr u16 magic_data           = 5;
        constexpr u16 magic_version        = 6;
        constexpr u16 magic_os_abi         = 7;
        constexpr u16 magic_abi_version    = 8;

        namespace type {
            constexpr u8 none              = 0;
            constexpr u8 rel               = 1;
            constexpr u8 exec              = 2;
            constexpr u8 dyn               = 3;
            constexpr u8 core              = 4;
        }

        str::slice_t class_name(u8 cls);

        str::slice_t os_abi_name(u8 os_abi);

        str::slice_t version_name(u8 version);

        str::slice_t file_type_name(u16 type);

        str::slice_t endianess_name(u8 endianess);
    }

    namespace group {
        constexpr u32 entity_size          = sizeof(u32);
    }

    namespace relocs {
        constexpr u32 entity_size          = sizeof(rela_t);

        namespace x86_64 {
            constexpr u32 none                 = 0;
            constexpr u32 d64                  = 1;
            constexpr u32 pc32                 = 2;
            constexpr u32 got32                = 3;
            constexpr u32 plt32                = 4;
            constexpr u32 copy                 = 5;
            constexpr u32 glob_dat             = 6;
            constexpr u32 jmp_slot             = 7;
            constexpr u32 relative             = 8;
            constexpr u32 got_pc_rel           = 9;
            constexpr u32 dir32_zx             = 10;
            constexpr u32 dir32_sx             = 11;
            constexpr u32 dir16_zx             = 12;
            constexpr u32 pc_rel_16_sx         = 13;
            constexpr u32 dir8_sx              = 14;
            constexpr u32 pc_rel_8_sx          = 15;
            constexpr u32 dtp_mod_64           = 16;
            constexpr u32 dtp_off_64           = 17;
            constexpr u32 tp_off_64            = 18;
            constexpr u32 tls_gd               = 19;
            constexpr u32 tls_ld               = 20;
            constexpr u32 dtp_off_32           = 21;
            constexpr u32 got_tp_off           = 22;
            constexpr u32 tp_off_32            = 23;
            constexpr u32 pc64                 = 24;
            constexpr u32 got_off_64           = 25;
            constexpr u32 got_pc_32            = 26;
            constexpr u32 got_entry_64         = 27;
            constexpr u32 got_pc_rel_64        = 28;
            constexpr u32 got_pc_64            = 29;
            constexpr u32 got_plt_64           = 30;
            constexpr u32 plt_off_64           = 31;
            constexpr u32 size32               = 32;
            constexpr u32 size64               = 33;
            constexpr u32 got_pc_32_tls_desc   = 34;
            constexpr u32 tls_desc_call        = 35;
            constexpr u32 tls_desc             = 36;
            constexpr u32 i_relative           = 37;
            constexpr u32 relative64           = 38;
            constexpr u32 got_pc_relx          = 41;
            constexpr u32 rex_got_pc_relx      = 42;
        }

        namespace aarch64 {
            constexpr u32 none                         = 0;
            constexpr u32 abs64                        = 257;
            constexpr u32 abs32                        = 258;
            constexpr u32 abs16                        = 259;
            constexpr u32 prel64                       = 260;
            constexpr u32 prel32                       = 261;
            constexpr u32 prel16                       = 262;
            constexpr u32 movw_uabs_g0                 = 263;
            constexpr u32 movw_uabs_g0_nc              = 264;
            constexpr u32 movw_uabs_g1                 = 265;
            constexpr u32 movw_uabs_g1_nc              = 266;
            constexpr u32 movw_uabs_g2                 = 267;
            constexpr u32 movw_uabs_g2_nc              = 268;
            constexpr u32 movw_uabs_g3                 = 269;
            constexpr u32 movw_sabs_g0                 = 270;
            constexpr u32 movw_sabs_g1                 = 271;
            constexpr u32 movw_sabs_g2                 = 272;
            constexpr u32 ld_prel_lo19                 = 273;
            constexpr u32 adr_prel_lo21                = 274;
            constexpr u32 adr_prel_pg_hi21             = 275;
            constexpr u32 adr_prel_pg_hi21_nc          = 276;
            constexpr u32 add_abs_lo12_nc              = 277;
            constexpr u32 ldst8_abs_lo12_nc            = 278;
            constexpr u32 tstbr14                      = 279;
            constexpr u32 condbr19                     = 280;
            constexpr u32 jump26                       = 282;
            constexpr u32 call26                       = 283;
            constexpr u32 ldst16_abs_lo12_nc           = 284;
            constexpr u32 ldst32_abs_lo12_nc           = 285;
            constexpr u32 ldst64_abs_lo12_nc           = 286;
            constexpr u32 movw_prel_g0                 = 287;
            constexpr u32 movw_prel_g0_nc              = 288;
            constexpr u32 movw_prel_g1                 = 289;
            constexpr u32 movw_prel_g1_nc              = 290;
            constexpr u32 movw_prel_g2                 = 291;
            constexpr u32 movw_prel_g2_nc              = 292;
            constexpr u32 movw_prel_g3                 = 293;
            constexpr u32 ldst128_abs_lo12_nc          = 299;
            constexpr u32 movw_gotoff_g0               = 300;
            constexpr u32 movw_gotoff_g0_nc            = 301;
            constexpr u32 movw_gotoff_g1               = 302;
            constexpr u32 movw_gotoff_g1_nc            = 303;
            constexpr u32 movw_gotoff_g2               = 304;
            constexpr u32 movw_gotoff_g2_nc            = 305;
            constexpr u32 movw_gotoff_g3               = 306;
            constexpr u32 gotrel64                     = 307;
            constexpr u32 gotrel32                     = 308;
            constexpr u32 got_ld_prel19                = 309;
            constexpr u32 ld64_gotoff_lo15             = 310;
            constexpr u32 adr_got_page                 = 311;
            constexpr u32 ld64_got_lo12_nc             = 312;
            constexpr u32 ld64_gotpage_lo15            = 313;
            constexpr u32 tlsgd_adr_prel21             = 512;
            constexpr u32 tlsgd_adr_page21             = 513;
            constexpr u32 tlsgd_add_lo12_nc            = 514;
            constexpr u32 tlsgd_movw_g1                = 515;
            constexpr u32 tlsgd_movw_g0_nc             = 516;
            constexpr u32 tlsld_adr_prel21             = 517;
            constexpr u32 tlsld_adr_page21             = 518;
            constexpr u32 tlsld_add_lo12_nc            = 519;
            constexpr u32 tlsld_movw_g1                = 520;
            constexpr u32 tlsld_movw_g0_nc             = 521;
            constexpr u32 tlsld_ld_prel19              = 522;
            constexpr u32 tlsld_movw_dtprel_g2         = 523;
            constexpr u32 tlsld_movw_dtprel_g1         = 524;
            constexpr u32 tlsld_movw_dtprel_g1_nc      = 525;
            constexpr u32 tlsld_movw_dtprel_g0         = 526;
            constexpr u32 tlsld_movw_dtprel_g0_nc      = 527;
            constexpr u32 tlsld_add_dtprel_hi12        = 528;
            constexpr u32 tlsld_add_dtprel_lo12        = 529;
            constexpr u32 tlsld_add_dtprel_lo12_nc     = 530;
            constexpr u32 tlsld_ldst8_dtprel_lo12      = 531;
            constexpr u32 tlsld_ldst8_dtprel_lo12_nc   = 532;
            constexpr u32 tlsld_ldst16_dtprel_lo12     = 533;
            constexpr u32 tlsld_ldst16_dtprel_lo12_nc  = 534;
            constexpr u32 tlsld_ldst32_dtprel_lo12     = 535;
            constexpr u32 tlsld_ldst32_dtprel_lo12_nc  = 536;
            constexpr u32 tlsld_ldst64_dtprel_lo12     = 537;
            constexpr u32 tlsld_ldst64_dtprel_lo12_nc  = 538;
            constexpr u32 tlsie_movw_gottprel_g1       = 539;
            constexpr u32 tlsie_movw_gottprel_g0_nc    = 540;
            constexpr u32 tlsie_adr_gottprel_page21    = 541;
            constexpr u32 tlsie_ld64_gottprel_lo12_nc  = 542;
            constexpr u32 tlsie_ld_gottprel_prel19     = 543;
            constexpr u32 tlsle_movw_tprel_g2          = 544;
            constexpr u32 tlsle_movw_tprel_g1          = 545;
            constexpr u32 tlsle_movw_tprel_g1_nc       = 546;
            constexpr u32 tlsle_movw_tprel_g0          = 547;
            constexpr u32 tlsle_movw_tprel_g0_nc       = 548;
            constexpr u32 tlsle_add_tprel_hi12         = 549;
            constexpr u32 tlsle_add_tprel_lo12         = 550;
            constexpr u32 tlsle_add_tprel_lo12_nc      = 551;
            constexpr u32 tlsle_ldst8_tprel_lo12       = 552;
            constexpr u32 tlsle_ldst8_tprel_lo12_nc    = 553;
            constexpr u32 tlsle_ldst16_tprel_lo12      = 554;
            constexpr u32 tlsle_ldst16_tprel_lo12_nc   = 555;
            constexpr u32 tlsle_ldst32_tprel_lo12      = 556;
            constexpr u32 tlsle_ldst32_tprel_lo12_nc   = 557;
            constexpr u32 tlsle_ldst64_tprel_lo12      = 558;
            constexpr u32 tlsle_ldst64_tprel_lo12_nc   = 559;
            constexpr u32 tlsdesc_ld_prel19            = 560;
            constexpr u32 tlsdesc_adr_prel21           = 561;
            constexpr u32 tlsdesc_adr_page21           = 562;
            constexpr u32 tlsdesc_ld64_lo12            = 563;
            constexpr u32 tlsdesc_add_lo12             = 564;
            constexpr u32 tlsdesc_off_g1               = 565;
            constexpr u32 tlsdesc_off_g0_nc            = 566;
            constexpr u32 tlsdesc_ldr                  = 567;
            constexpr u32 tlsdesc_add                  = 568;
            constexpr u32 tlsdesc_call                 = 569;
            constexpr u32 tlsle_ldst128_tprel_lo12     = 570;
            constexpr u32 tlsle_ldst128_tprel_lo12_nc  = 571;
            constexpr u32 tlsld_ldst128_dtprel_lo12    = 572;
            constexpr u32 tlsld_ldst128_dtprel_lo12_nc = 573;
            constexpr u32 copy                         = 1024;
            constexpr u32 glob_dat                     = 1025;
            constexpr u32 jump_slot                    = 1026;
            constexpr u32 relative                     = 1027;
            constexpr u32 tls_dtpmod64                 = 1028;
            constexpr u32 tls_dtprel64                 = 1029;
            constexpr u32 tls_tprel64                  = 1030;
            constexpr u32 tls_desc                     = 1031;
            constexpr u32 i_relative                   = 1032;
        }
    }

    namespace version {
        namespace def {
            constexpr u32 none     = 0;
            constexpr u32 current  = 1;
            constexpr u32 number   = 2;
        }

        namespace flags {
            constexpr u32 base     = 1;
            constexpr u32 weak     = 2;
        }

        namespace index {
            constexpr u32 local    = 0;
            constexpr u32 global   = 1;
        }
    }

    namespace machine {
        constexpr u16 x86_64            = 62;
        constexpr u16 aarch64           = 183;
        constexpr u16 tilera_tile_pro   = 188;
        constexpr u16 tilera_tile_gx    = 191;
        constexpr u16 riscv             = 243;

        str::slice_t name(u16 type);
    }

    namespace dynamic {
        namespace type {
            constexpr u32 null                  = 0;
            constexpr u32 needed                = 1;
            constexpr u32 plt_rel_size          = 2;
            constexpr u32 plt_got               = 3;
            constexpr u32 hash                  = 4;
            constexpr u32 strtab                = 5;
            constexpr u32 symtab                = 6;
            constexpr u32 rela                  = 7;
            constexpr u32 rela_size             = 8;
            constexpr u32 rela_ent_size         = 9;
            constexpr u32 str_size              = 10;
            constexpr u32 sym_ent_size          = 11;
            constexpr u32 init                  = 12;
            constexpr u32 fini                  = 13;
            constexpr u32 soname                = 14;
            constexpr u32 rpath                 = 15;
            constexpr u32 symbolic              = 16;
            constexpr u32 rel                   = 17;
            constexpr u32 rel_size              = 18;
            constexpr u32 rel_ent_size          = 19;
            constexpr u32 plt_rel               = 20;
            constexpr u32 debug                 = 21;
            constexpr u32 text_rel              = 22;
            constexpr u32 jmp_rel               = 23;
            constexpr u32 bind_now              = 24;
            constexpr u32 init_array            = 25;
            constexpr u32 fini_array            = 26;
            constexpr u32 init_array_size       = 27;
            constexpr u32 fini_array_size       = 28;
            constexpr u32 run_path              = 29;
            constexpr u32 flags                 = 30;
            constexpr u32 pre_init_array        = 32;
            constexpr u32 pre_init_array_size   = 33;
            constexpr u32 low_os                = 0x6000000d;
            constexpr u32 high_os               = 0x6ffff000;
            constexpr u32 low_proc              = 0x70000000;
            constexpr u32 high_proc             = 0x7fffffff;

            str::slice_t name(u32 type);
        }
    }

    namespace section {
        constexpr u16 header_size          = 64;

        namespace type {
            constexpr u32 null             = 0;
            constexpr u32 progbits         = 1;
            constexpr u32 symtab           = 2;
            constexpr u32 strtab           = 3;
            constexpr u32 rela             = 4;
            constexpr u32 hash             = 5;
            constexpr u32 dynamic          = 6;
            constexpr u32 note             = 7;
            constexpr u32 nobits           = 8;
            constexpr u32 rel              = 9;
            constexpr u32 shlib            = 10;
            constexpr u32 dynsym           = 11;
            constexpr u32 init_array       = 14;
            constexpr u32 fini_array       = 15;
            constexpr u32 pre_init_array   = 16;
            constexpr u32 group            = 17;
            constexpr u32 symtab_shndx     = 18;
            constexpr u32 low_os           = 0x60000000;
            constexpr u32 gnu_eh_frame     = 0x6474e550;
            constexpr u32 gnu_stack        = 0x6474e551;
            constexpr u32 gnu_rel_ro       = 0x6474e552;
            constexpr u32 gnu_attributes   = 0x6ffffff5;
            constexpr u32 gnu_hash         = 0x6ffffff6;
            constexpr u32 gnu_lib_list     = 0x6ffffff7;
            constexpr u32 checksum         = 0x6ffffff8;
            constexpr u32 gnu_ver_def      = 0x6ffffffd;
            constexpr u32 gnu_ver_need     = 0x6ffffffe;
            constexpr u32 gnu_ver_sym      = 0x6fffffff;
            constexpr u32 high_os          = 0x6fffffff;
            constexpr u32 low_proc         = 0x70000000;
            constexpr u32 x86_64_unwind    = 0x70000001;
            constexpr u32 high_proc        = 0x7fffffff;
            constexpr u32 low_user         = 0x80000000;
            constexpr u32 high_user        = 0x8fffffff;

            str::slice_t name(u32 type);
        }

        namespace flags {
            constexpr u32 write            = 0x1;
            constexpr u32 alloc            = 0x2;
            constexpr u32 exec_instr       = 0x4;
            constexpr u32 merge            = 0x10;
            constexpr u32 strings          = 0x20;
            constexpr u32 info_link        = 0x40;
            constexpr u32 link_order       = 0x80;
            constexpr u32 os_non_conform   = 0x100;
            constexpr u32 group            = 0x200;
            constexpr u32 tls              = 0x400;
            constexpr u32 compressed       = 0x800;
            constexpr u32 mask_os          = 0x0ff00000;
            constexpr u32 mask_proc        = 0xf0000000;
            constexpr u32 ordered          = 0x40000000;
            constexpr u32 exclude          = 0x80000000;

            const s8* name(u32 flag);

            u0 chars(u32 flags, s8* chars);

            u0 names(u32 flags, const s8* names[13]);
        }

        namespace info {
        }

        namespace link {
        }

        namespace indexes {
            constexpr u32 undef            = 0;
            constexpr u32 live_abs         = 0xfff1;
            constexpr u32 live_patch       = 0xff20;
            constexpr u32 live_common      = 0xfff2;
        }
    }

    namespace segment {
        constexpr u16 header_size          = 56;

        namespace type {
            constexpr u32 null             = 0x0;
            constexpr u32 load             = 0x1;
            constexpr u32 dynamic          = 0x2;
            constexpr u32 interp           = 0x3;
            constexpr u32 note             = 0x4;
            constexpr u32 shlib            = 0x5;
            constexpr u32 pgm_hdr          = 0x6;
            constexpr u32 tls              = 0x7;
            constexpr u32 gnu_eh_frame     = 0x6474e550;
            constexpr u32 gnu_property     = 0x6474e553;
        }

        namespace flags {
            constexpr u32 read             = 0x4;
            constexpr u32 exec             = 0x1;
            constexpr u32 write            = 0x2;
        }
    }

    namespace symtab {
        constexpr u32 entity_size          = 24;

        namespace type {
            constexpr u8 notype            = 0x0;
            constexpr u8 object            = 0x1;
            constexpr u8 func              = 0x2;
            constexpr u8 section           = 0x3;
            constexpr u8 file              = 0x4;
            constexpr u8 common            = 0x5;
            constexpr u8 tls               = 0x6;
        }

        namespace scope {
            constexpr u8 local             = 0x0;
            constexpr u8 global            = 0x1;
            constexpr u8 weak              = 0x2;
        }

        namespace visibility {
            constexpr u8 default_          = 0x0;
            constexpr u8 internal          = 0x1;
            constexpr u8 hidden            = 0x2;
            constexpr u8 protected_        = 0x3;
        }

        u64 hash_name(str::slice_t str);

        sym_t* get(const elf_t& elf, u32 sect_num, u32 sym_idx);
    }

    namespace hashtab {
        constexpr u32 entity_size          = sizeof(u32);
    }

    u0 free(elf_t& elf);

    status_t read(elf_t& elf, file_t& file);

    status_t write(elf_t& elf, file_t& file);

    status_t init(elf_t& elf, const opts_t& opts);

    status_t get_section_name(const module_t* module,
                              const binfmt::section_t* section,
                              str::slice_t& name);

    u0 format_report(str_buf_t& buf, const elf_t& elf);

    u32 section_alignment(const binfmt::section_t* section);

    u32 section_file_size(const binfmt::section_t* section);
}
