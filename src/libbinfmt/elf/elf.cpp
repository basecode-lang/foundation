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
#include <basecode/binfmt/binfmt.h>

namespace basecode::binfmt::io::elf {
    enum section_map_status_t : u8 {
        ok,
        skip,
        error,
        not_supported,
    };

    struct section_map_t final {
        binfmt::section::flags_t    flags;
        binfmt::section::type_t     type;
        section_map_status_t        status;
    };

    static section_map_t s_section_maps[64] = {
        [section::type::null]           = {
            .status = section_map_status_t::skip
        },

        [section::type::progbits]       = {
            .flags = {
                .code = false,
                .init = true,
                .alloc = true,
            },
            .type = binfmt::section::type_t::data,
            .status = section_map_status_t::ok,
        },

        [section::type::symtab]         = {
            .type = binfmt::section::type_t::symtab,
            .status = section_map_status_t::ok
        },

        [section::type::strtab]         = {
            .type = binfmt::section::type_t::strtab,
            .status = section_map_status_t::ok
        },

        [section::type::rela]           = {
            .type = binfmt::section::type_t::reloc,
            .status = section_map_status_t::ok,
        },

        [section::type::hash]           = {
            .status = section_map_status_t::not_supported,
        },

        [section::type::dynamic]        = {
            .status = section_map_status_t::not_supported,
        },

        [section::type::note]           = {
            .type = binfmt::section::type_t::note,
            .status = section_map_status_t::ok,
        },

        [section::type::nobits]         = {
            .flags = {
                .code = false,
                .init = false,
                .write = true,
                .alloc = true,
            },
            .type = binfmt::section::type_t::data,
            .status = section_map_status_t::ok,
        },

        [section::type::rel]            = {
            .status = section_map_status_t::not_supported
        },

        [section::type::shlib]          = {
            .status = section_map_status_t::not_supported,
        },

        [section::type::dynsym]         = {
            .status = section_map_status_t::not_supported,
        },

        [section::type::init_array]     = {
            .flags = {
                .code = true,
                .init = true,
                .write = true,
                .alloc = true,
            },
            .type = binfmt::section::type_t::init,
            .status = section_map_status_t::ok,
        },

        [section::type::fini_array]     = {
            .flags = {
                .code = true,
                .init = true,
                .write = true,
                .alloc = true,
            },
            .type = binfmt::section::type_t::fini,
            .status = section_map_status_t::ok,
        },

        [section::type::pre_init_array] = {
            .flags = {
                .code = true,
                .init = true,
                .write = true,
                .alloc = true,
            },
            .type = binfmt::section::type_t::pre_init,
            .status = section_map_status_t::ok,
        },

        [section::type::group]          = {
            .type = binfmt::section::type_t::group,
            .status = section_map_status_t::ok,
        },

        [section::type::symtab_shndx]   = {
            .status = section_map_status_t::not_supported,
        },

        [12] = { .status = section_map_status_t::skip },
        [13] = { .status = section_map_status_t::skip },

        [30] = {
            .flags = {
                .code = false,
                .init = true,
                .alloc = true,
            },
            .type = binfmt::section::type_t::unwind,
            .status = section_map_status_t::ok,
        },

        [31] = {
            .type = binfmt::section::type_t::custom,
            .status = section_map_status_t::ok,
        }
    };

    using r_arm64_t = binfmt::machine::aarch64::reloc::type_t;
    using r_amd64_t = binfmt::machine::x86_64::reloc::type_t;

    static u32 s_arm64_to_elf_relocs[] = {
        [u32(r_arm64_t::none)]                        = relocs::aarch64::none,
        [u32(r_arm64_t::abs_64)]                      = relocs::aarch64::none,
        [u32(r_arm64_t::abs_32)]                      = relocs::aarch64::none,
        [u32(r_arm64_t::abs_16)]                      = relocs::aarch64::none,
        [u32(r_arm64_t::pc_rel_64)]                   = relocs::aarch64::none,
        [u32(r_arm64_t::pc_rel_32)]                   = relocs::aarch64::none,
        [u32(r_arm64_t::pc_rel_16)]                   = relocs::aarch64::none,
        [u32(r_arm64_t::movw_uabs_g0)]                = relocs::aarch64::none,
        [u32(r_arm64_t::movw_uabs_g0_nc)]             = relocs::aarch64::none,
        [u32(r_arm64_t::movw_uabs_g1)]                = relocs::aarch64::none,
        [u32(r_arm64_t::movw_uabs_g1_nc)]             = relocs::aarch64::none,
        [u32(r_arm64_t::movw_uabs_g2)]                = relocs::aarch64::none,
        [u32(r_arm64_t::movw_uabs_g2_nc)]             = relocs::aarch64::none,
        [u32(r_arm64_t::movw_uabs_g3)]                = relocs::aarch64::none,
        [u32(r_arm64_t::movw_sabs_g0)]                = relocs::aarch64::none,
        [u32(r_arm64_t::movw_sabs_g1)]                = relocs::aarch64::none,
        [u32(r_arm64_t::movw_sabs_g2)]                = relocs::aarch64::none,
        [u32(r_arm64_t::ld_pc_rel_lo_19)]             = relocs::aarch64::none,
        [u32(r_arm64_t::adr_pc_rel_lo21)]             = relocs::aarch64::none,
        [u32(r_arm64_t::adr_pc_rel_pg_hi_21)]         = relocs::aarch64::none,
        [u32(r_arm64_t::adr_pc_rel_pg_hi_21_nc)]      = relocs::aarch64::none,
        [u32(r_arm64_t::add_abs_lo_12_nc)]            = relocs::aarch64::none,
        [u32(r_arm64_t::ldst_8_abs_lo_12_nc)]         = relocs::aarch64::none,
        [u32(r_arm64_t::tst_br_14)]                   = relocs::aarch64::none,
        [u32(r_arm64_t::cond_br_19)]                  = relocs::aarch64::none,
        [u32(r_arm64_t::jump_26)]                     = relocs::aarch64::none,
        [u32(r_arm64_t::call_26)]                     = relocs::aarch64::none,
        [u32(r_arm64_t::got_rel_64)]                  = relocs::aarch64::none,
        [u32(r_arm64_t::got_rel_32)]                  = relocs::aarch64::none,
        [u32(r_arm64_t::adr_got_page)]                = relocs::aarch64::none,
        [u32(r_arm64_t::copy)]                        = relocs::aarch64::none,
        [u32(r_arm64_t::glob_dat)]                    = relocs::aarch64::none,
        [u32(r_arm64_t::jump_slot)]                   = relocs::aarch64::none,
        [u32(r_arm64_t::relative)]                    = relocs::aarch64::none,
        [u32(r_arm64_t::tls_dtp_mod_64)]              = relocs::aarch64::none,
        [u32(r_arm64_t::tls_dtp_rel64)]               = relocs::aarch64::none,
        [u32(r_arm64_t::tls_tp_rel_64)]               = relocs::aarch64::none,
        [u32(r_arm64_t::tls_desc)]                    = relocs::aarch64::none,
    };

    static binfmt::machine::aarch64::reloc::type_t s_elf_to_arm64_relocs[] = {
        [relocs::aarch64::none]                             = r_arm64_t::none,
        [relocs::aarch64::abs64]                            = r_arm64_t::abs_64,
        [relocs::aarch64::abs32]                            = r_arm64_t::abs_32,
        [relocs::aarch64::abs16]                            = r_arm64_t::abs_16,
        [relocs::aarch64::prel64]                           = r_arm64_t::pc_rel_64,
        [relocs::aarch64::prel32]                           = r_arm64_t::pc_rel_32,
        [relocs::aarch64::prel16]                           = r_arm64_t::pc_rel_16,
        [relocs::aarch64::movw_uabs_g0]                     = r_arm64_t::movw_uabs_g0,
        [relocs::aarch64::movw_uabs_g0_nc]                  = r_arm64_t::movw_uabs_g0_nc,
        [relocs::aarch64::movw_uabs_g1]                     = r_arm64_t::movw_uabs_g1,
        [relocs::aarch64::movw_uabs_g1_nc]                  = r_arm64_t::movw_uabs_g1_nc,
        [relocs::aarch64::movw_uabs_g2]                     = r_arm64_t::movw_uabs_g2,
        [relocs::aarch64::movw_uabs_g2_nc]                  = r_arm64_t::movw_uabs_g2_nc,
        [relocs::aarch64::movw_uabs_g3]                     = r_arm64_t::movw_uabs_g3,
        [relocs::aarch64::movw_sabs_g0]                     = r_arm64_t::movw_sabs_g0,
        [relocs::aarch64::movw_sabs_g1]                     = r_arm64_t::movw_sabs_g1,
        [relocs::aarch64::movw_sabs_g2]                     = r_arm64_t::movw_sabs_g2,
        [relocs::aarch64::ld_prel_lo19]                     = r_arm64_t::ld_pc_rel_lo_19,
        [relocs::aarch64::adr_prel_lo21]                    = r_arm64_t::adr_pc_rel_lo21,
        [relocs::aarch64::adr_prel_pg_hi21]                 = r_arm64_t::adr_pc_rel_pg_hi_21,
        [relocs::aarch64::adr_prel_pg_hi21_nc]              = r_arm64_t::adr_pc_rel_pg_hi_21_nc,
        [relocs::aarch64::add_abs_lo12_nc]                  = r_arm64_t::add_abs_lo_12_nc,
        [relocs::aarch64::ldst8_abs_lo12_nc]                = r_arm64_t::ldst_8_abs_lo_12_nc,
        [relocs::aarch64::tstbr14]                          = r_arm64_t::tst_br_14,
        [relocs::aarch64::condbr19]                         = r_arm64_t::cond_br_19,
        [relocs::aarch64::jump26]                           = r_arm64_t::jump_26,
        [relocs::aarch64::call26]                           = r_arm64_t::call_26,
        [relocs::aarch64::ldst16_abs_lo12_nc]               = r_arm64_t::none,
        [relocs::aarch64::ldst32_abs_lo12_nc]               = r_arm64_t::none,
        [relocs::aarch64::ldst64_abs_lo12_nc]               = r_arm64_t::none,
        [relocs::aarch64::movw_prel_g0]                     = r_arm64_t::none,
        [relocs::aarch64::movw_prel_g0_nc]                  = r_arm64_t::none,
        [relocs::aarch64::movw_prel_g1]                     = r_arm64_t::none,
        [relocs::aarch64::movw_prel_g1_nc]                  = r_arm64_t::none,
        [relocs::aarch64::movw_prel_g2]                     = r_arm64_t::none,
        [relocs::aarch64::movw_prel_g2_nc]                  = r_arm64_t::none,
        [relocs::aarch64::movw_prel_g3]                     = r_arm64_t::none,
        [relocs::aarch64::ldst128_abs_lo12_nc]              = r_arm64_t::none,
        [relocs::aarch64::movw_gotoff_g0]                   = r_arm64_t::none,
        [relocs::aarch64::movw_gotoff_g0_nc]                = r_arm64_t::none,
        [relocs::aarch64::movw_gotoff_g1]                   = r_arm64_t::none,
        [relocs::aarch64::movw_gotoff_g1_nc]                = r_arm64_t::none,
        [relocs::aarch64::movw_gotoff_g2]                   = r_arm64_t::none,
        [relocs::aarch64::movw_gotoff_g2_nc]                = r_arm64_t::none,
        [relocs::aarch64::movw_gotoff_g3]                   = r_arm64_t::none,
        [relocs::aarch64::gotrel64]                         = r_arm64_t::got_rel_64,
        [relocs::aarch64::gotrel32]                         = r_arm64_t::got_rel_32,
        [relocs::aarch64::got_ld_prel19]                    = r_arm64_t::none,
        [relocs::aarch64::ld64_gotoff_lo15]                 = r_arm64_t::none,
        [relocs::aarch64::adr_got_page]                     = r_arm64_t::adr_got_page,
        [relocs::aarch64::ld64_got_lo12_nc]                 = r_arm64_t::none,
        [relocs::aarch64::ld64_gotpage_lo15]                = r_arm64_t::none,
        [relocs::aarch64::tlsgd_adr_prel21]                 = r_arm64_t::none,
        [relocs::aarch64::tlsgd_adr_page21]                 = r_arm64_t::none,
        [relocs::aarch64::tlsgd_add_lo12_nc]                = r_arm64_t::none,
        [relocs::aarch64::tlsgd_movw_g1]                    = r_arm64_t::none,
        [relocs::aarch64::tlsgd_movw_g0_nc]                 = r_arm64_t::none,
        [relocs::aarch64::tlsld_adr_prel21]                 = r_arm64_t::none,
        [relocs::aarch64::tlsld_adr_page21]                 = r_arm64_t::none,
        [relocs::aarch64::tlsld_add_lo12_nc]                = r_arm64_t::none,
        [relocs::aarch64::tlsld_movw_g1]                    = r_arm64_t::none,
        [relocs::aarch64::tlsld_movw_g0_nc]                 = r_arm64_t::none,
        [relocs::aarch64::tlsld_ld_prel19]                  = r_arm64_t::none,
        [relocs::aarch64::tlsld_movw_dtprel_g2]             = r_arm64_t::none,
        [relocs::aarch64::tlsld_movw_dtprel_g1]             = r_arm64_t::none,
        [relocs::aarch64::tlsld_movw_dtprel_g1_nc]          = r_arm64_t::none,
        [relocs::aarch64::tlsld_movw_dtprel_g0]             = r_arm64_t::none,
        [relocs::aarch64::tlsld_movw_dtprel_g0_nc]          = r_arm64_t::none,
        [relocs::aarch64::tlsld_add_dtprel_hi12]            = r_arm64_t::none,
        [relocs::aarch64::tlsld_add_dtprel_lo12]            = r_arm64_t::none,
        [relocs::aarch64::tlsld_add_dtprel_lo12_nc]         = r_arm64_t::none,
        [relocs::aarch64::tlsld_ldst8_dtprel_lo12]          = r_arm64_t::none,
        [relocs::aarch64::tlsld_ldst8_dtprel_lo12_nc]       = r_arm64_t::none,
        [relocs::aarch64::tlsld_ldst16_dtprel_lo12]         = r_arm64_t::none,
        [relocs::aarch64::tlsld_ldst16_dtprel_lo12_nc]      = r_arm64_t::none,
        [relocs::aarch64::tlsld_ldst32_dtprel_lo12]         = r_arm64_t::none,
        [relocs::aarch64::tlsld_ldst32_dtprel_lo12_nc]      = r_arm64_t::none,
        [relocs::aarch64::tlsld_ldst64_dtprel_lo12]         = r_arm64_t::none,
        [relocs::aarch64::tlsld_ldst64_dtprel_lo12_nc]      = r_arm64_t::none,
        [relocs::aarch64::tlsie_movw_gottprel_g1]           = r_arm64_t::none,
        [relocs::aarch64::tlsie_movw_gottprel_g0_nc]        = r_arm64_t::none,
        [relocs::aarch64::tlsie_adr_gottprel_page21]        = r_arm64_t::none,
        [relocs::aarch64::tlsie_ld64_gottprel_lo12_nc]      = r_arm64_t::none,
        [relocs::aarch64::tlsie_ld_gottprel_prel19]         = r_arm64_t::none,
        [relocs::aarch64::tlsle_movw_tprel_g2]              = r_arm64_t::none,
        [relocs::aarch64::tlsle_movw_tprel_g1]              = r_arm64_t::none,
        [relocs::aarch64::tlsle_movw_tprel_g1_nc]           = r_arm64_t::none,
        [relocs::aarch64::tlsle_movw_tprel_g0]              = r_arm64_t::none,
        [relocs::aarch64::tlsle_movw_tprel_g0_nc]           = r_arm64_t::none,
        [relocs::aarch64::tlsle_add_tprel_hi12]             = r_arm64_t::none,
        [relocs::aarch64::tlsle_add_tprel_lo12]             = r_arm64_t::none,
        [relocs::aarch64::tlsle_add_tprel_lo12_nc]          = r_arm64_t::none,
        [relocs::aarch64::tlsle_ldst8_tprel_lo12]           = r_arm64_t::none,
        [relocs::aarch64::tlsle_ldst8_tprel_lo12_nc]        = r_arm64_t::none,
        [relocs::aarch64::tlsle_ldst16_tprel_lo12]          = r_arm64_t::none,
        [relocs::aarch64::tlsle_ldst16_tprel_lo12_nc]       = r_arm64_t::none,
        [relocs::aarch64::tlsle_ldst32_tprel_lo12]          = r_arm64_t::none,
        [relocs::aarch64::tlsle_ldst32_tprel_lo12_nc]       = r_arm64_t::none,
        [relocs::aarch64::tlsle_ldst64_tprel_lo12]          = r_arm64_t::none,
        [relocs::aarch64::tlsle_ldst64_tprel_lo12_nc]       = r_arm64_t::none,
        [relocs::aarch64::tlsdesc_ld_prel19]                = r_arm64_t::none,
        [relocs::aarch64::tlsdesc_adr_prel21]               = r_arm64_t::none,
        [relocs::aarch64::tlsdesc_adr_page21]               = r_arm64_t::none,
        [relocs::aarch64::tlsdesc_ld64_lo12]                = r_arm64_t::none,
        [relocs::aarch64::tlsdesc_add_lo12]                 = r_arm64_t::none,
        [relocs::aarch64::tlsdesc_off_g1]                   = r_arm64_t::none,
        [relocs::aarch64::tlsdesc_off_g0_nc]                = r_arm64_t::none,
        [relocs::aarch64::tlsdesc_ldr]                      = r_arm64_t::none,
        [relocs::aarch64::tlsdesc_add]                      = r_arm64_t::none,
        [relocs::aarch64::tlsdesc_call]                     = r_arm64_t::none,
        [relocs::aarch64::tlsle_ldst128_tprel_lo12]         = r_arm64_t::none,
        [relocs::aarch64::tlsle_ldst128_tprel_lo12_nc]      = r_arm64_t::none,
        [relocs::aarch64::tlsld_ldst128_dtprel_lo12]        = r_arm64_t::none,
        [relocs::aarch64::tlsld_ldst128_dtprel_lo12_nc]     = r_arm64_t::none,
        [relocs::aarch64::copy]                             = r_arm64_t::copy,
        [relocs::aarch64::glob_dat]                         = r_arm64_t::glob_dat,
        [relocs::aarch64::jump_slot]                        = r_arm64_t::jump_slot,
        [relocs::aarch64::relative]                         = r_arm64_t::relative,
        [relocs::aarch64::tls_dtpmod64]                     = r_arm64_t::tls_dtp_mod_64,
        [relocs::aarch64::tls_dtprel64]                     = r_arm64_t::tls_dtp_rel64,
        [relocs::aarch64::tls_tprel64]                      = r_arm64_t::tls_tp_rel_64,
        [relocs::aarch64::tls_desc]                         = r_arm64_t::tls_desc,
    };

    static u32 s_amd64_to_elf_relocs[] = {
        [u32(r_amd64_t::none)]                = relocs::x86_64::none,
        [u32(r_amd64_t::direct_64)]           = relocs::x86_64::d64,
        [u32(r_amd64_t::pc_rel_32)]           = relocs::x86_64::pc32,
        [u32(r_amd64_t::got_32)]              = relocs::x86_64::got32,
        [u32(r_amd64_t::plt_32)]              = relocs::x86_64::plt32,
        [u32(r_amd64_t::copy)]                = relocs::x86_64::copy,
        [u32(r_amd64_t::glob_dat)]            = relocs::x86_64::glob_dat,
        [u32(r_amd64_t::jump_slot)]           = relocs::x86_64::jmp_slot,
        [u32(r_amd64_t::relative)]            = relocs::x86_64::relative,
        [u32(r_amd64_t::got_pc_rel)]          = relocs::x86_64::got_pc_rel,
        [u32(r_amd64_t::direct_32)]           = relocs::x86_64::dir32_zx,
        [u32(r_amd64_t::direct_32_signed)]    = relocs::x86_64::dir32_sx,
        [u32(r_amd64_t::direct_16)]           = relocs::x86_64::dir16_zx,
        [u32(r_amd64_t::pc_rel_16_signed)]    = relocs::x86_64::pc_rel_16_sx,
        [u32(r_amd64_t::direct_8_signed)]     = relocs::x86_64::dir8_sx,
        [u32(r_amd64_t::pc_rel_8_signed)]     = relocs::x86_64::pc_rel_8_sx,
        [u32(r_amd64_t::dtp_mod_64)]          = relocs::x86_64::dtp_mod_64,
        [u32(r_amd64_t::dtp_offset_64)]       = relocs::x86_64::dtp_off_64,
        [u32(r_amd64_t::tp_offset_64)]        = relocs::x86_64::tp_off_64,
        [u32(r_amd64_t::tls_gd_signed)]       = relocs::x86_64::tls_gd,
        [u32(r_amd64_t::tls_ld_signed)]       = relocs::x86_64::tls_ld,
        [u32(r_amd64_t::dtp_offset_32)]       = relocs::x86_64::dtp_off_32,
        [u32(r_amd64_t::got_tp_offset_signed)]= relocs::x86_64::got_tp_off,
        [u32(r_amd64_t::tp_offset_32)]        = relocs::x86_64::tp_off_32,
        [u32(r_amd64_t::pc_rel_64)]           = relocs::x86_64::pc64,
        [u32(r_amd64_t::got_offset_64)]       = relocs::x86_64::got_off_64,
        [u32(r_amd64_t::got_pc_32)]           = relocs::x86_64::got_pc_32,
        [u32(r_amd64_t::got_entry_64)]        = relocs::x86_64::got_entry_64,
        [u32(r_amd64_t::got_pc_rel_64)]       = relocs::x86_64::got_pc_rel_64,
        [u32(r_amd64_t::got_pc_64)]           = relocs::x86_64::got_pc_64,
        [u32(r_amd64_t::got_plt_64)]          = relocs::x86_64::got_plt_64,
        [u32(r_amd64_t::plt_offset_64)]       = relocs::x86_64::plt_off_64,
        [u32(r_amd64_t::size_sym_32)]         = relocs::x86_64::size32,
        [u32(r_amd64_t::size_sym_64)]         = relocs::x86_64::size64,
        [u32(r_amd64_t::got_pc_32_tls_desc)]  = relocs::x86_64::got_pc_32_tls_desc,
        [u32(r_amd64_t::tls_desc_call)]       = relocs::x86_64::tls_desc_call,
        [u32(r_amd64_t::tls_desc)]            = relocs::x86_64::tls_desc,
        [u32(r_amd64_t::i_relative)]          = relocs::x86_64::i_relative,
        [u32(r_amd64_t::relative_64)]         = relocs::x86_64::relative64,
        [u32(r_amd64_t::got_pc_relx)]         = relocs::x86_64::got_pc_relx,
        [u32(r_amd64_t::rex_got_pc_relx)]     = relocs::x86_64::rex_got_pc_relx,
    };

    static binfmt::machine::x86_64::reloc::type_t s_elf_to_amd64_relocs[] = {
        [relocs::x86_64::none]                 = r_amd64_t::none,
        [relocs::x86_64::d64]                  = r_amd64_t::direct_64,
        [relocs::x86_64::pc32]                 = r_amd64_t::pc_rel_32,
        [relocs::x86_64::got32]                = r_amd64_t::got_32,
        [relocs::x86_64::plt32]                = r_amd64_t::plt_32,
        [relocs::x86_64::copy]                 = r_amd64_t::copy,
        [relocs::x86_64::glob_dat]             = r_amd64_t::glob_dat,
        [relocs::x86_64::jmp_slot]             = r_amd64_t::jump_slot,
        [relocs::x86_64::relative]             = r_amd64_t::relative,
        [relocs::x86_64::got_pc_rel]           = r_amd64_t::got_pc_rel,
        [relocs::x86_64::dir32_zx]             = r_amd64_t::direct_32,
        [relocs::x86_64::dir32_sx]             = r_amd64_t::direct_32_signed,
        [relocs::x86_64::dir16_zx]             = r_amd64_t::direct_16,
        [relocs::x86_64::pc_rel_16_sx]         = r_amd64_t::pc_rel_16_signed,
        [relocs::x86_64::dir8_sx]              = r_amd64_t::direct_8_signed,
        [relocs::x86_64::pc_rel_8_sx]          = r_amd64_t::pc_rel_8_signed,
        [relocs::x86_64::dtp_mod_64]           = r_amd64_t::dtp_mod_64,
        [relocs::x86_64::dtp_off_64]           = r_amd64_t::dtp_offset_64,
        [relocs::x86_64::tp_off_64]            = r_amd64_t::tp_offset_64,
        [relocs::x86_64::tls_gd]               = r_amd64_t::tls_gd_signed,
        [relocs::x86_64::tls_ld]               = r_amd64_t::tls_ld_signed,
        [relocs::x86_64::dtp_off_32]           = r_amd64_t::dtp_offset_32,
        [relocs::x86_64::got_tp_off]           = r_amd64_t::got_tp_offset_signed,
        [relocs::x86_64::tp_off_32]            = r_amd64_t::tp_offset_32,
        [relocs::x86_64::pc64]                 = r_amd64_t::pc_rel_64,
        [relocs::x86_64::got_off_64]           = r_amd64_t::got_offset_64,
        [relocs::x86_64::got_pc_32]            = r_amd64_t::got_pc_32,
        [relocs::x86_64::got_entry_64]         = r_amd64_t::got_entry_64,
        [relocs::x86_64::got_pc_rel_64]        = r_amd64_t::got_pc_rel_64,
        [relocs::x86_64::got_pc_64]            = r_amd64_t::got_pc_64,
        [relocs::x86_64::got_plt_64]           = r_amd64_t::got_plt_64,
        [relocs::x86_64::plt_off_64]           = r_amd64_t::plt_offset_64,
        [relocs::x86_64::size32]               = r_amd64_t::size_sym_32,
        [relocs::x86_64::size64]               = r_amd64_t::size_sym_64,
        [relocs::x86_64::got_pc_32_tls_desc]   = r_amd64_t::got_pc_32_tls_desc,
        [relocs::x86_64::tls_desc_call]        = r_amd64_t::tls_desc_call,
        [relocs::x86_64::tls_desc]             = r_amd64_t::tls_desc,
        [relocs::x86_64::i_relative]           = r_amd64_t::i_relative,
        [relocs::x86_64::relative64]           = r_amd64_t::relative_64,
        [relocs::x86_64::got_pc_relx]          = r_amd64_t::got_pc_relx,
        [relocs::x86_64::rex_got_pc_relx]      = r_amd64_t::rex_got_pc_relx,
    };

    static section_map_t* find_section_map(u32 type) {
        switch (type) {
            case section::type::null ... section::type::rel:
            case section::type::shlib ... section::type::symtab_shndx: {
                return &s_section_maps[type];
            }
            case elf::section::type::x86_64_unwind:
                return &s_section_maps[30];
            case elf::section::type::gnu_hash:
            case elf::section::type::gnu_stack:
            case elf::section::type::gnu_rel_ro:
            case elf::section::type::gnu_ver_def:
            case elf::section::type::gnu_ver_sym:
            case elf::section::type::gnu_ver_need:
            case elf::section::type::gnu_eh_frame:
            case elf::section::type::gnu_lib_list:
            case elf::section::type::gnu_attributes:
                break;
            default:
                return &s_section_maps[31];
        }
        return nullptr;
    }

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
                'W', 'A', 'X', 'M', 'S',
                'I', 'L', 'O', 'G', 'T',
                'C', 'o', 'E'
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

    static status_t read_section(elf_t& elf,
                                 file_t& file,
                                 u8* buf,
                                 binfmt::module_t* module,
                                 u32 num) {
        auto section = binfmt::module::get_section(*module, num - 1);
        if (section->type != binfmt::section::type_t::none)
            return status_t::ok;

        const auto& hdr = elf.sections[num];
        auto map = find_section_map(hdr.type);
        if (!map)
            return status_t::read_error;

        if (map->status == section_map_status_t::skip)
            return status_t::ok;

        if (map->status == section_map_status_t::not_supported)
            return status_t::elf_unsupported_section;

        auto sect_type = map->type;
        binfmt::section_opts_t sect_opts{};
        sect_opts.info          = hdr.info;
        sect_opts.size          = hdr.size;
        sect_opts.flags         = map->flags;
        sect_opts.align         = hdr.addr_align;
        sect_opts.name_offset   = hdr.name_offset;
        sect_opts.flags.tls     = (hdr.flags & section::flags::tls) != 0;
        sect_opts.flags.alloc   = (hdr.flags & section::flags::alloc) != 0;
        sect_opts.flags.write   = (hdr.flags & section::flags::write) != 0;
        sect_opts.flags.group   = (hdr.flags & section::flags::group) != 0;
        sect_opts.flags.merge   = (hdr.flags & section::flags::merge) != 0;
        sect_opts.flags.exclude = (hdr.flags & section::flags::exclude) != 0;
        sect_opts.flags.strings = (hdr.flags & section::flags::strings) != 0;

        if (hdr.flags & section::flags::exec_instr) {
            sect_opts.flags.code  = true;
            sect_opts.flags.exec  = true;
            sect_type = binfmt::section::type_t::code;
        }

        if (sect_type == binfmt::section::type_t::strtab
        ||  sect_opts.flags.strings) {
            sect_opts.strtab.buf           = buf + hdr.offset;
            sect_opts.strtab.size_in_bytes = hdr.size;
        }

        if (!OK(binfmt::section::init(section, sect_type, sect_opts)))
            return status_t::read_error;

        if (hdr.link > 0) {
            auto status = read_section(elf, file, buf, module, hdr.link);
            if (!OK(status))
                return status;
            section->link = binfmt::module::get_section(*module, hdr.link - 1);
        }

        auto msc = &module->subclass.object;

        switch (section->type) {
            case binfmt::section::type_t::data: {
                if (section->flags.init
                &&  !section->flags.strings) {
                    section->subclass.data = buf + hdr.offset;
                }
                break;
            }
            case binfmt::section::type_t::code:
            case binfmt::section::type_t::init:
            case binfmt::section::type_t::fini:
            case binfmt::section::type_t::unwind:
            case binfmt::section::type_t::pre_init: {
                section->subclass.data = buf + hdr.offset;
                break;
            }
            case binfmt::section::type_t::note: {
                auto note_hdr = (note_header_t*) (buf + hdr.offset);
                UNUSED(note_hdr);
                // XXX: need to finish
                break;
            }
            case binfmt::section::type_t::reloc: {
                auto& relocs = section->subclass.relocs;
                auto rels = (rela_t*) (buf + hdr.offset);
                auto num_rels = hdr.size / hdr.entity_size;
                array::resize(relocs, num_rels);
                for (u32 j = 0; j < num_rels; ++j) {
                    const auto& rela = rels[j];
                    auto sym  = ELF64_R_SYM(rela.info);
                    auto type = ELF64_R_TYPE(rela.info);

                    auto& reloc = relocs[j];
                    reloc.offset = rela.offset;
                    reloc.addend = rela.addend;
                    reloc.symbol = binfmt::section::get_symbol(section->link, sym - 1);

                    switch (file.machine) {
                        case binfmt::machine::type_t::unknown:
                            break;
                        case binfmt::machine::type_t::x86_64:
                            reloc.x86_64_type = s_elf_to_amd64_relocs[type];
                            break;
                        case binfmt::machine::type_t::aarch64:
                            reloc.aarch64_type = s_elf_to_arm64_relocs[type];
                            break;
                    }
                }
                break;
            }
            case binfmt::section::type_t::group: {
                auto gsc = &section->subclass.group;
                auto group = (group_t*) (buf + hdr.offset);
                auto num_groups = hdr.size / hdr.entity_size;
                gsc->flags = group->flags;
                array::resize(gsc->sections, num_groups);
                for (u32 j = 0; j < num_groups; ++j)
                    gsc->sections[j] = group->sect_hdr_indexes[j];
                break;
            }
            case binfmt::section::type_t::custom: {
                section->subclass.data = buf + hdr.offset;
                section->ext_type      = hdr.type;
                break;
            }
            case binfmt::section::type_t::strtab: {
                if (num == elf.file_header->strtab_ndx)
                    msc->strtab = section;
                break;
            }
            case binfmt::section::type_t::symtab: {
                if (hdr.link > 0
                &&  hdr.link == elf.file_header->strtab_ndx) {
                    msc->symtab = section;
                }
                const auto symtab_count = hdr.size / hdr.entity_size;
                for (u32 j = 1; j < symtab_count; ++j) {
                    auto sym = elf::symtab::get(elf, num, j);

                    binfmt::symbol_opts_t opts{};
                    opts.size        = sym->size;
                    opts.value       = sym->value;
                    opts.section     = binfmt::module::get_section(*module, sym->section_ndx - 1);
                    opts.name_offset = sym->name_offset;
                    auto symbol = binfmt::section::add_symbol(section, opts);

                    switch (ELF64_ST_TYPE(sym->info)) {
                        default:
                        case elf::symtab::type::notype:
                            symbol->type = symbol::type_t::none;
                            break;
                        case elf::symtab::type::tls:
                            symbol->type = symbol::type_t::tls;
                            break;
                        case elf::symtab::type::file:
                            symbol->type = symbol::type_t::file;
                            break;
                        case elf::symtab::type::common:
                            symbol->type = symbol::type_t::common;
                            break;
                        case elf::symtab::type::object:
                            symbol->type = symbol::type_t::object;
                            break;
                        case elf::symtab::type::section:
                            symbol->type = symbol::type_t::section;
                            break;
                        case elf::symtab::type::func:
                            symbol->type = symbol::type_t::function;
                            break;
                    }

                    switch (ELF64_ST_BIND(sym->info)) {
                        default:
                        case elf::symtab::scope::local:
                            symbol->scope = symbol::scope_t::local;
                            break;
                        case elf::symtab::scope::global:
                            symbol->scope = symbol::scope_t::global;
                            break;
                        case elf::symtab::scope::weak:
                            symbol->scope = symbol::scope_t::weak;
                            break;
                    }

                    switch (ELF64_ST_VISIBILITY(sym->other)) {
                        default:
                        case elf::symtab::visibility::default_:
                            symbol->visibility = symbol::visibility_t::default_;
                            break;
                        case elf::symtab::visibility::internal:
                            symbol->visibility = symbol::visibility_t::internal_;
                            break;
                        case elf::symtab::visibility::hidden:
                            symbol->visibility = symbol::visibility_t::hidden;
                            break;
                        case elf::symtab::visibility::protected_:
                            symbol->visibility = symbol::visibility_t::protected_;
                            break;
                    }
                }
                break;
            }
            case binfmt::section::type_t::none:
            case binfmt::section::type_t::rsrc:
            case binfmt::section::type_t::debug:
            case binfmt::section::type_t::import:
            case binfmt::section::type_t::export_: {
                break;
            }
        }

        return status_t::ok;
    }

    u0 free(elf_t& elf) {
    }

    status_t read(elf_t& elf, file_t& file) {
        auto buf = FILE_PTR();

        elf.file_header = (file_header_t*) (buf);
        if (std::memcmp(elf.file_header->magic, "\177ELF", 4) != 0)
            return status_t::read_error;

        u8 expected_type{};
        switch (file.file_type) {
            case file_type_t::none:
            case file_type_t::obj:
                expected_type = elf::file::type::rel;
                break;
            case file_type_t::exe:
                expected_type = elf::file::type::exec;
                break;
            case file_type_t::dll:
                expected_type = elf::file::type::dyn;
                break;
        }
        if (elf.file_header->type != expected_type)
            return status_t::invalid_input_type;

        switch (elf.file_header->machine) {
            case elf::machine::x86_64:
                file.machine = binfmt::machine::type_t::x86_64;
                break;
            case elf::machine::aarch64:
                file.machine = binfmt::machine::type_t::aarch64;
                break;
            default:
                return status_t::invalid_machine_type;
        }

        if (elf.file_header->pgm_hdr_count > 0) {
            elf.segments = (pgm_header_t*) (buf + elf.file_header->pgm_hdr_offset);
        }

        if (elf.file_header->sect_hdr_count > 0) {
            elf.sections = (sect_header_t*) (buf + elf.file_header->sect_hdr_offset);
        }

        file.module = binfmt::system::make_module(module_type_t::object);
        if (!file.module) {
            return status_t::read_error;
        }

        const auto num_sections = elf.file_header->sect_hdr_count;

        if (!OK(binfmt::module::reserve_sections(*file.module, num_sections - 1))) {
            return status_t::read_error;
        }

        for (u32 i = 1; i < num_sections; ++i) {
            auto status = read_section(elf, file, buf, file.module, i);
            if (!OK(status)) {
                return status;
            }
        }

        return status_t::ok;
    }

    status_t write(elf_t& elf, file_t& file) {
        using machine_type_t = binfmt::machine::type_t;
        using section_type_t = binfmt::section::type_t;

        auto buf = FILE_PTR();
        const auto& opts = *elf.opts;
        auto fh     = elf.file_header;
        auto module = file.module;
        auto msc    = &module->subclass.object;

        fh->magic[0]                       = 0x7f;
        fh->magic[1]                       = 'E';
        fh->magic[2]                       = 'L';
        fh->magic[3]                       = 'F';
        fh->magic[file::magic_class]       = opts.clazz;
        fh->magic[file::magic_data]        = opts.endianess;
        fh->magic[file::magic_version]     = opts.version;
        fh->magic[file::magic_os_abi]      = opts.os_abi;
        fh->magic[file::magic_abi_version] = opts.abi_version;

        fh->flags       = opts.flags;
        fh->version     = 1;
        fh->header_size = file::header_size;

        b8 is_obj = file.file_type == file_type_t::obj;
        if (!is_obj)
            fh->entry_point = opts.entry_point == 0 ? 0x00400000 : opts.entry_point;
        else
            fh->entry_point = 0;

        u32 num_segments = 0;

        if (msc->sections.size > 0) {
            fh->sect_hdr_count  = msc->sections.size + 1;
            fh->sect_hdr_size   = section::header_size;
            fh->sect_hdr_offset = opts.header_offset;
            elf.sections = (sect_header_t*) (buf + fh->sect_hdr_offset);
        }

        if (num_segments > 0) {
            fh->pgm_hdr_count  = num_segments + 1;
            fh->pgm_hdr_size   = segment::header_size;
            fh->pgm_hdr_offset = fh->sect_hdr_offset + (fh->sect_hdr_count * fh->sect_hdr_size);
            elf.segments = (pgm_header_t*) (buf + fh->pgm_hdr_offset);
        }

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

        u64 virt_addr   = fh->entry_point;
        u64 data_offset = fh->header_size;
        u32 sect_idx    = 1;
        b8  inc_vaddr   = false;

        for (const auto section : msc->sections) {
            auto& hdr = elf.sections[sect_idx];
            hdr.flags       = {};
            hdr.addr        = virt_addr;
            hdr.link        = section->link ? section->link->number : 0;
            hdr.info        = section->info;
            hdr.size        = section_file_size(section);
            hdr.offset      = data_offset;
            hdr.addr_align  = section_alignment(section);
            hdr.name_offset = section->name_offset;

            if (section->flags.exec)     hdr.flags |= section::flags::exec_instr;
            if (section->flags.alloc)    hdr.flags |= section::flags::alloc;
            if (section->flags.write)    hdr.flags |= section::flags::write;
            if (section->flags.group)    hdr.flags |= section::flags::group;
            if (section->flags.merge)    hdr.flags |= section::flags::merge;
            if (section->flags.exclude)  hdr.flags |= section::flags::exclude;
            if (section->flags.strings)  hdr.flags |= section::flags::strings;

            u8* data = buf + hdr.offset;
            const auto alignment = hdr.addr_align;

            switch (section->type) {
                case section_type_t::data: {
                    if (!section->flags.init) {
                        hdr.type = section::type::nobits;
                    } else if (section->flags.strings) {
                        std::memcpy(data, section->subclass.strtab.buf.data, hdr.size);
                        hdr.type        = section::type::progbits;
                        hdr.entity_size = 1;
                        data_offset = align(data_offset + hdr.size, alignment);
                    } else {
                        std::memcpy(data, section->subclass.data, hdr.size);
                        hdr.type = section::type::progbits;
                        data_offset = align(data_offset + hdr.size, alignment);
                    }
                    inc_vaddr = !is_obj;
                    break;
                }
                case section_type_t::init: {
                    std::memcpy(data, section->subclass.data, hdr.size);
                    hdr.type = section::type::init_array;
                    data_offset = align(data_offset + hdr.size, alignment);
                    inc_vaddr   = !is_obj;
                    break;
                }
                case section_type_t::fini: {
                    std::memcpy(data, section->subclass.data, hdr.size);
                    hdr.type = section::type::fini_array;
                    data_offset = align(data_offset + hdr.size, alignment);
                    inc_vaddr   = !is_obj;
                    break;
                }
                case section_type_t::code: {
                    std::memcpy(data, section->subclass.data, hdr.size);
                    hdr.type = section::type::progbits;
                    data_offset = align(data_offset + hdr.size, alignment);
                    inc_vaddr   = !is_obj;
                    break;
                }
                case section_type_t::unwind: {
                    std::memcpy(data, section->subclass.data, hdr.size);
                    hdr.type = section::type::x86_64_unwind;
                    data_offset = align(data_offset + hdr.size, alignment);
                    break;
                }
                case section_type_t::reloc: {
                    hdr.type        = section::type::rela;
                    hdr.entity_size = relocs::entity_size;
                    auto rela = (rela_t*) data;
                    for (u32 j = 0; j < section->subclass.relocs.size; ++j) {
                        const auto& reloc = section->subclass.relocs[j];
                        u32 type{};
                        switch (file.machine) {
                            case binfmt::machine::type_t::unknown:
                                break;
                            case binfmt::machine::type_t::x86_64:
                                type = s_amd64_to_elf_relocs[u32(reloc.x86_64_type)];
                                break;
                            case binfmt::machine::type_t::aarch64:
                                type = s_arm64_to_elf_relocs[u32(reloc.aarch64_type)];
                                break;
                        }
                        rela[j].info   = ELF64_R_INFO(reloc.symbol->ndx + 1, type);
                        rela[j].offset = reloc.offset;
                        rela[j].addend = reloc.addend;
                    }
                    data_offset = align(data_offset + hdr.size, alignment);
                    break;
                }
                case section_type_t::group: {
                    hdr.type        = section::type::group;
                    hdr.entity_size = group::entity_size;
                    auto group = (group_t*) data;
                    group->flags = section->subclass.group.flags;
                    for (u32 j = 0; j < section->subclass.group.sections.size; ++j)
                        group->sect_hdr_indexes[j] = section->subclass.group.sections[j];
                    data_offset = align(data_offset + hdr.size, alignment);
                    break;
                }
                case section_type_t::custom: {
                    std::memcpy(data, section->subclass.data, hdr.size);
                    hdr.type = section->ext_type != 0 ? section->ext_type : section::type::progbits;
                    data_offset = align(data_offset + hdr.size, alignment);
                    inc_vaddr   = !is_obj;
                    break;
                }
                case section_type_t::strtab: {
                    std::memcpy(data, section->subclass.strtab.buf.data, hdr.size);
                    hdr.type        = section::type::strtab;
                    hdr.entity_size = 1;
                    if (fh->strtab_ndx == 0
                    &&  msc->strtab
                    &&  msc->strtab->number == sect_idx) {
                        fh->strtab_ndx = sect_idx;
                    }
                    data_offset = align(data_offset + hdr.size, 8);
                    inc_vaddr   = !is_obj && section->flags.alloc;
                    break;
                }
                case section_type_t::symtab: {
                    hdr.type        = section::type::symtab;
                    hdr.entity_size = symtab::entity_size;

                    auto sym_data = (sym_t*) (buf + hdr.offset);
                    u32 i       {};
                    u32 scope   {};
                    u32 type    {};
                    u32 vis     {};
                    for (const auto symbol : section->subclass.symtab.symbols) {
                        auto& sym = sym_data[i + 1];

                        switch (symbol->type) {
                            case symbol::type_t::none:
                                type = elf::symtab::type::notype;
                                break;
                            case symbol::type_t::tls:
                                type = elf::symtab::type::tls;
                                break;
                            case symbol::type_t::file:
                                type = elf::symtab::type::file;
                                break;
                            case symbol::type_t::common:
                                type = elf::symtab::type::common;
                                break;
                            case symbol::type_t::object:
                                type = elf::symtab::type::object;
                                break;
                            case symbol::type_t::section:
                                type = elf::symtab::type::section;
                                break;
                            case symbol::type_t::function:
                                type = elf::symtab::type::func;
                                break;
                        }

                        switch (symbol->scope) {
                            case symbol::scope_t::none:
                            case symbol::scope_t::weak:
                                scope = elf::symtab::scope::weak;
                                if (!hdr.info)
                                    hdr.info = i + 1;
                                break;
                            case symbol::scope_t::local:
                                scope = elf::symtab::scope::local;
                                break;
                            case symbol::scope_t::global:
                                scope = elf::symtab::scope::global;
                                if (!hdr.info)
                                    hdr.info = i + 1;
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

                        sym.size        = symbol->size;
                        sym.value       = symbol->value;
                        sym.info        = ELF64_ST_INFO(scope, type);
                        sym.other       = ELF64_ST_VISIBILITY(vis);
                        sym.name_offset = symbol->name_offset;
                        sym.section_ndx = symbol->section ? symbol->section->number + 1 : 0;

                        ++i;
                    }

                    data_offset = align(data_offset + hdr.size, 8);
                    inc_vaddr   = false;
                    break;
                }
                default:
                    break;
            }

            if (inc_vaddr)
                virt_addr = align(virt_addr + hdr.size, hdr.addr_align);

            ++sect_idx;
        }

        return status_t::ok;
    }

    status_t init(elf_t& elf, const opts_t& opts) {
        auto& file = *opts.file;
        elf.alloc       = opts.alloc;
        elf.opts        = &opts;
        elf.file_header = (file_header_t*) (FILE_PTR());
        return status_t::ok;
    }

    u0 format_report(str_buf_t& buf, const elf_t& elf) {
        format::format_to(buf, "ELF Header:\n");
        format::format_to(buf, "  Magic:    ");
        format::format_hex_dump(buf, elf.file_header->magic, 16, false, false);
        format::format_to(buf, "  Class:                             {}\n", elf::file::class_name(elf.file_header->magic[elf::file::magic_class]));
        format::format_to(buf, "  Data:                              {}\n", elf::file::endianess_name(elf.file_header->magic[elf::file::magic_data]));
        format::format_to(buf, "  Version:                           {}\n", elf::file::version_name(elf.file_header->magic[elf::file::magic_version]));
        format::format_to(buf, "  OS/ABI:                            {}\n", elf::file::os_abi_name(elf.file_header->magic[elf::file::magic_os_abi]));
        format::format_to(buf, "  ABI Version:                       {}\n", elf.file_header->magic[elf::file::magic_abi_version]);
        format::format_to(buf, "  Type:                              {}\n", elf::file::file_type_name(elf.file_header->type));
        format::format_to(buf, "  Machine:                           {}\n", elf::machine::name(elf.file_header->machine));
        format::format_to(buf, "  Version:                           0x{:x}\n", elf.file_header->version);
        format::format_to(buf, "  Entry point address:               0x{:x}\n", elf.file_header->entry_point);
        format::format_to(buf, "  Start of program headers:          {} (bytes into file)\n", elf.file_header->pgm_hdr_offset);
        format::format_to(buf, "  Start of section headers:          {} (bytes into file)\n", elf.file_header->sect_hdr_offset);
        format::format_to(buf, "  Flags:                             0x{:x}\n", elf.file_header->flags);
        format::format_to(buf, "  Size of this header:               {} (bytes)\n", file::header_size);
        format::format_to(buf, "  Size of program headers:           {} (bytes)\n", elf.file_header->pgm_hdr_size);
        format::format_to(buf, "  Number of program headers:         {}\n", elf.file_header->pgm_hdr_count);
        format::format_to(buf, "  Size of section headers:           {} (bytes)\n", elf.file_header->sect_hdr_size);
        format::format_to(buf, "  Number of section headers:         {}\n", elf.file_header->sect_hdr_count);
        format::format_to(buf, "  Section header string table index: {}\n\n", elf.file_header->strtab_ndx);
        format::format_to(buf, "Section Headers:\n");
        format::format_to(buf, "  [Nr] Name              Type             Address           Offset\n");
        format::format_to(buf, "       Size              EntSize          Flags  Link  Info  Align\n");

        s8 name[17];
        name[16] = '\0';

        s8 flag_chars[14];
        auto elf_buf = (u8*) elf.file_header;
        auto strtab_sect = &elf.sections[elf.file_header->strtab_ndx];
        auto strtab = elf_buf + strtab_sect->offset;

        for (u32 i = 0; i < elf.file_header->sect_hdr_count; ++i) {
            const auto& hdr = elf.sections[i];
            std::memcpy(name, strtab + hdr.name_offset, 16);
            elf::section::flags::chars(hdr.flags, flag_chars);

            format::format_to(buf,
                              " [{:>3}] {:<17} {:<16} {:016x} {:08x}\n",
                              i,
                              name,
                              section::type::name(hdr.type),
                              hdr.addr,
                              hdr.offset);
            format::format_to(buf,
                              "       {:016x}  {:016x} {:<7}{:>4}  {:>4}    {:<}\n",
                              hdr.size,
                              hdr.entity_size,
                              flag_chars,
                              hdr.link,
                              hdr.info,
                              hdr.addr_align);
        }

        auto symtab_ndx = elf.file_header->sect_hdr_count - 1;
        auto symtab_sect = &elf.sections[symtab_ndx];

        const auto symtab_size = symtab_sect->size / symtab_sect->entity_size;

        format::format_to(buf, "\nSymbol table '{}' contains {} entries:\n", ".symtab", symtab_size);
        format::format_to(buf, "  Num:     Value         Size Type     Bind   Vis       Ndx Name\n");
        for (u32 i = 0; i < symtab_size; ++i) {
            auto sym = elf::symtab::get(elf, symtab_ndx, i);
            std::memcpy(name, strtab + sym->name_offset, 16);
            format::format_to(buf,
                              "{:>5}: {:016x} {:>5} NOTYPE   LOCAL   DEFAULT  {} {}\n",
                              i,
                              sym->value,
                              sym->size,
                              sym->section_ndx,
                              name);
        }
    }

    u32 section_alignment(const binfmt::section_t* section) {
        using section_type_t = binfmt::section::type_t;
        u32 alignment = section->align;
        if (alignment == 0) {
            switch (section->type) {
                case section_type_t::code:
                    alignment = 16;
                    break;
                case section_type_t::group:
                    alignment = 4;
                    break;
                case section_type_t::strtab:
                    alignment = 1;
                    break;
                default:
                    alignment = 8;
                    break;
            }
        }
        return alignment;
    }

    u32 section_file_size(const binfmt::section_t* section) {
        using section_type_t = binfmt::section::type_t;

        if (section->type == section_type_t::data
        &&  !section->flags.init) {
            return 0;
        }

        u32 size = section->size;
        if (size == 0) {
            switch (section->type) {
                case section_type_t::strtab: {
                    size = section->subclass.strtab.buf.size;
                    break;
                }
                case section_type_t::symtab: {
                    size = (section->subclass.symtab.symbols.size + 1) * elf::symtab::entity_size;
                    break;
                }
                case section_type_t::group: {
                    size = (section->subclass.group.sections.size + 1) * elf::group::entity_size;
                    break;
                }
                case section_type_t::reloc: {
                    size = (section->subclass.relocs.size) * elf::relocs::entity_size;
                    break;
                }
                default:
                    break;
            }
        }

        return size;
    }
}
