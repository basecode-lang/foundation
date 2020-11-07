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

#define FLAG_CHK(v, f) (((v) & u32((f))) == f)

namespace basecode::binfmt::io::coff {
    struct sym_t;
    struct reloc_t;
    struct line_num_t;
    struct section_hdr_t;

    using sym_list_t            = array_t<sym_t>;
    using str_list_t            = array_t<str::slice_t>;
    using section_hdr_list_t    = array_t<section_hdr_t>;

    struct rva_t final {
        u32                     base;
        u32                     size;
    };

    struct raw_t final {
        u32                     offset;
        u32                     size;
    };

    // .debug$S (symbolic info)
    // .debug$T (type info)
    struct debug_t final {
    };

    struct reloc_t final {
        u32                     rva;
        u32                     symtab_idx;
        u16                     type;
    };

    struct line_num_t final {
        u32                     one;
        u32                     two;
    };

    // .pdata
    struct exception_t final {
        u32                     begin_rva;
        u32                     end_rva;
        u32                     unwind_info;
    };

    enum class sym_type_t : u8 {
        sym,
        aux_xf,
        aux_file,
        aux_section,
        aux_func_def,
        aux_weak_extern,
    };

    struct sym_t final {
        union {
            s8                  aux_file[18];
            struct {
                u32             ptr_next_func;
                u16             line_num;
            }                   aux_xf;
            struct {
                u32             tag_idx;
                u32             flags;
            }                   aux_weak_extern;
            struct {
                u32             tag_idx;
                u32             total_size;
                u32             ptr_line_num;
                u32             ptr_next_func;
            }                   aux_func_def;
            struct {
                u32             len;
                u32             check_sum;
                u16             num_relocs;
                u16             num_lines;
                u16             sect_num;
                u8              comdat_sel;
            }                   aux_section;
            struct {
                u64             strtab_offset;
                str::slice_t    name;
                struct {
                    u32         idx;
                    u32         len;
                }               aux;
                u32             value;
                u16             type;
                s16             section;
                u8              sclass;
            }                   sym;
        }                       subclass;
        u32                     id;
        sym_type_t              type;
    };

    struct section_hdr_t final {
        const section_t*        section;
        u32                     symbol;
        rva_t                   rva;
        raw_t                   file;
        struct {
            raw_t               file;
        }                       relocs;
        struct {
            raw_t               file;
        }                       line_nums;
        u32                     number;
        u32                     flags;
    };

    struct coff_t final {
        alloc_t*                alloc;
        u8*                     buf;
        section_hdr_list_t      headers;
        u32                     rva;
        u32                     offset;
        u32                     timestamp;
        struct {
            raw_t               file;
            str_list_t          list;
        }                       strtab;
        struct {
            raw_t               file;
            sym_list_t          list;
            u32                 num_symbols;
        }                       symtab;
        struct {
            u32                 image;
            u32                 headers;
            u16                 opt_hdr;
        }                       size;
        struct {
            u16                 image;
        }                       flags;
        struct {
            u32                 file;
            u32                 section;
        }                       align;
        rva_t                   code;
        rva_t                   relocs;
        rva_t                   init_data;
        rva_t                   uninit_data;
        u16                     machine;
    };

    [[maybe_unused]] constexpr u32 header_size = 0x14;

    namespace debug {
        [[maybe_unused]] constexpr u32 unknown   = 0;
        [[maybe_unused]] constexpr u32 code_view = 2;
    }

    namespace reloc {
        [[maybe_unused]] constexpr u32 entry_size = 10;

        namespace type::x86_64 {
            [[maybe_unused]] constexpr u16 absolute             = 0;
            [[maybe_unused]] constexpr u16 addr64               = 1;
            [[maybe_unused]] constexpr u16 addr32               = 2;
            [[maybe_unused]] constexpr u16 addr32nb             = 3;
            [[maybe_unused]] constexpr u16 rel32                = 4;
            [[maybe_unused]] constexpr u16 rel32_1              = 5;
            [[maybe_unused]] constexpr u16 rel32_2              = 6;
            [[maybe_unused]] constexpr u16 rel32_3              = 7;
            [[maybe_unused]] constexpr u16 rel32_4              = 8;
            [[maybe_unused]] constexpr u16 rel32_5              = 9;
            [[maybe_unused]] constexpr u16 section              = 10;
            [[maybe_unused]] constexpr u16 section_rel          = 11;
            [[maybe_unused]] constexpr u16 section_rel_7        = 12;
            [[maybe_unused]] constexpr u16 token                = 13;
            [[maybe_unused]] constexpr u16 rel32_signed         = 14;
            [[maybe_unused]] constexpr u16 pair                 = 15;
            [[maybe_unused]] constexpr u16 span32_signed        = 16;

            str::slice_t name(u16 type);
        }

        namespace type::aarch64 {
            [[maybe_unused]] constexpr u16 absolute             = 0;
            [[maybe_unused]] constexpr u16 addr32               = 1;
            [[maybe_unused]] constexpr u16 addr32nb             = 2;
            [[maybe_unused]] constexpr u16 branch26             = 3;
            [[maybe_unused]] constexpr u16 page_base_rel_21     = 4;
            [[maybe_unused]] constexpr u16 rel_21               = 5;
            [[maybe_unused]] constexpr u16 page_offset_12a      = 6;
            [[maybe_unused]] constexpr u16 page_offset_12l      = 7;
            [[maybe_unused]] constexpr u16 section_rel          = 8;
            [[maybe_unused]] constexpr u16 section_rel_low12a   = 9;
            [[maybe_unused]] constexpr u16 section_rel_high12a  = 10;
            [[maybe_unused]] constexpr u16 section_rel_loe12l   = 11;
            [[maybe_unused]] constexpr u16 token                = 12;
            [[maybe_unused]] constexpr u16 section              = 13;
            [[maybe_unused]] constexpr u16 addr64               = 14;
            [[maybe_unused]] constexpr u16 branch19             = 15;
            [[maybe_unused]] constexpr u16 branch14             = 16;
            [[maybe_unused]] constexpr u16 rel_32               = 17;

            str::slice_t name(u16 type);
        }

        reloc_t get(const coff_t& coff, const section_hdr_t& hdr, u32 idx);
    }

    namespace machine {
        [[maybe_unused]] constexpr u16 amd64 = 0x8664;
        [[maybe_unused]] constexpr u16 arm64 = 0xaa64;
    }

    namespace section {
        [[maybe_unused]] constexpr u32 header_size         = 0x28;
        [[maybe_unused]] constexpr u32 no_pad              = 0x00000008;
        [[maybe_unused]] constexpr u32 content_code        = 0x00000020;
        [[maybe_unused]] constexpr u32 data_init           = 0x00000040;
        [[maybe_unused]] constexpr u32 data_uninit         = 0x00000080;
        [[maybe_unused]] constexpr u32 link_info           = 0x00000200;
        [[maybe_unused]] constexpr u32 link_remove         = 0x00000800;
        [[maybe_unused]] constexpr u32 link_comdat         = 0x00001000;
        [[maybe_unused]] constexpr u32 gp_relative         = 0x00008000;
        [[maybe_unused]] constexpr u32 memory_purgeable    = 0x00020000;
        [[maybe_unused]] constexpr u32 memory_locked       = 0x00040000;
        [[maybe_unused]] constexpr u32 memory_preload      = 0x00080000;
        [[maybe_unused]] constexpr u32 memory_align_1      = 0x00100000;
        [[maybe_unused]] constexpr u32 memory_align_2      = 0x00200000;
        [[maybe_unused]] constexpr u32 memory_align_4      = 0x00300000;
        [[maybe_unused]] constexpr u32 memory_align_8      = 0x00400000;
        [[maybe_unused]] constexpr u32 memory_align_16     = 0x00500000; // default
        [[maybe_unused]] constexpr u32 memory_align_32     = 0x00600000;
        [[maybe_unused]] constexpr u32 memory_align_64     = 0x00700000;
        [[maybe_unused]] constexpr u32 memory_align_128    = 0x00800000;
        [[maybe_unused]] constexpr u32 memory_align_256    = 0x00900000;
        [[maybe_unused]] constexpr u32 memory_align_512    = 0x00a00000;
        [[maybe_unused]] constexpr u32 memory_align_1024   = 0x00b00000;
        [[maybe_unused]] constexpr u32 memory_align_2048   = 0x00c00000;
        [[maybe_unused]] constexpr u32 memory_align_4096   = 0x00d00000;
        [[maybe_unused]] constexpr u32 memory_align_8192   = 0x00e00000;
        [[maybe_unused]] constexpr u32 link_reloc_overflow = 0x01000000;
        [[maybe_unused]] constexpr u32 memory_discard      = 0x02000000;
        [[maybe_unused]] constexpr u32 memory_not_cached   = 0x04000000;
        [[maybe_unused]] constexpr u32 memory_not_paged    = 0x08000000;
        [[maybe_unused]] constexpr u32 memory_shared       = 0x10000000;
        [[maybe_unused]] constexpr u32 memory_execute      = 0x20000000;
        [[maybe_unused]] constexpr u32 memory_read         = 0x40000000;
        [[maybe_unused]] constexpr u32 memory_write        = 0x80000000;

        sym_t* get_symbol(coff_t& coff, section_hdr_t& hdr);
    }

    namespace flags {
        [[maybe_unused]] constexpr u32 relocs_stripped         = 0x0001;
        [[maybe_unused]] constexpr u32 executable_type         = 0x0002;
        [[maybe_unused]] constexpr u32 line_nums_stripped      = 0x0004;
        [[maybe_unused]] constexpr u32 local_syms_stripped     = 0x0008;
        [[maybe_unused]] constexpr u32 aggressive_ws_trim      = 0x0010;
        [[maybe_unused]] constexpr u32 large_address_aware     = 0x0020;
        [[maybe_unused]] constexpr u32 reserved                = 0x0040;
        [[maybe_unused]] constexpr u32 bytes_reversed_lo       = 0x0080;
        [[maybe_unused]] constexpr u32 machine_32bit           = 0x0100;
        [[maybe_unused]] constexpr u32 debug_stripped          = 0x0200;
        [[maybe_unused]] constexpr u32 removable_run_from_swap = 0x0400;
        [[maybe_unused]] constexpr u32 net_run_from_swap       = 0x0800;
        [[maybe_unused]] constexpr u32 system_type             = 0x1000;
        [[maybe_unused]] constexpr u32 dll_type                = 0x2000;
        [[maybe_unused]] constexpr u32 up_system_only          = 0x4000;
        [[maybe_unused]] constexpr u32 bytes_reversed_hi       = 0x8000;
    }

    namespace strtab {
        u0 free(coff_t& coff);

        u0 init(coff_t& coff, alloc_t* alloc);

        u32 add(coff_t& coff, str::slice_t str);

        u32 thunk_len(const u64_bytes_t& thunk);

        const s8* get(coff_t& coff, u64 offset);
    }

    namespace symtab {
        [[maybe_unused]] constexpr u32 entry_size = 0x12;

        namespace type {
            [[maybe_unused]] constexpr u8 none             = 0;
            [[maybe_unused]] constexpr u8 function         = 0x20;
        }

        namespace sclass {
            [[maybe_unused]] constexpr u8 null_            = 0;
            [[maybe_unused]] constexpr u8 auto_            = 1;
            [[maybe_unused]] constexpr u8 external_        = 2;
            [[maybe_unused]] constexpr u8 static_          = 3;
            [[maybe_unused]] constexpr u8 register_        = 4;
            [[maybe_unused]] constexpr u8 extern_def       = 5;
            [[maybe_unused]] constexpr u8 label            = 6;
            [[maybe_unused]] constexpr u8 undef_label      = 7;
            [[maybe_unused]] constexpr u8 member_of_struct = 8;
            [[maybe_unused]] constexpr u8 argument         = 9;
            [[maybe_unused]] constexpr u8 struct_tag       = 10;
            [[maybe_unused]] constexpr u8 member_of_union  = 11;
            [[maybe_unused]] constexpr u8 union_tag        = 12;
            [[maybe_unused]] constexpr u8 type_def         = 13;
            [[maybe_unused]] constexpr u8 undef_static     = 14;
            [[maybe_unused]] constexpr u8 enum_tag         = 15;
            [[maybe_unused]] constexpr u8 member_of_enum   = 16;
            [[maybe_unused]] constexpr u8 register_param   = 17;
            [[maybe_unused]] constexpr u8 bit_field        = 18;
            [[maybe_unused]] constexpr u8 block            = 100;
            [[maybe_unused]] constexpr u8 function         = 101;
            [[maybe_unused]] constexpr u8 end_of_struct    = 102;
            [[maybe_unused]] constexpr u8 file             = 103;
            [[maybe_unused]] constexpr u8 section          = 104;
            [[maybe_unused]] constexpr u8 weak_external    = 105;
            [[maybe_unused]] constexpr u8 clr_token        = 107;
            [[maybe_unused]] constexpr u8 end_of_function  = 0xff;
        }

        u0 free(coff_t& sym);

        u0 init(coff_t& sym, alloc_t* alloc);

        sym_t* make_symbol(coff_t& coff);

        sym_t* find_symbol(coff_t& coff, u64 name);

        sym_t* get_aux(coff_t& coff, sym_t* sym, u32 idx);

        sym_t* make_symbol(coff_t& coff, str::slice_t name);

        sym_t* make_symbol(coff_t& coff, u64 name, u32 offset);

        sym_t* make_aux(coff_t& coff, sym_t* sym, sym_type_t type);
    }

    u0 free(coff_t& coff);

    u0 update_symbol_table(coff_t& coff);

    u0 write_header(file_t& file, coff_t& coff);

    status_t read_header(file_t& file, coff_t& coff);

    u0 write_string_table(file_t& file, coff_t& coff);

    u0 write_symbol_table(file_t& file, coff_t& coff);

    status_t build_sections(file_t& file, coff_t& coff);

    u0 write_section_headers(file_t& file, coff_t& coff);

    u0 set_section_flags(file_t& file, section_hdr_t& hdr);

    status_t read_string_table(file_t& file, coff_t& coff);

    status_t read_symbol_table(file_t& file, coff_t& coff);

    status_t write_sections_data(file_t& file, coff_t& coff);

    status_t init(coff_t& coff, file_t& file, alloc_t* alloc);

    status_t read_section_headers(file_t& file, coff_t& coff);

    status_t build_section(file_t& file, coff_t& coff, section_hdr_t& hdr);

    u0 write_section_header(file_t& file, coff_t& coff, section_hdr_t& hdr);

    status_t write_section_data(file_t& file, coff_t& coff, section_hdr_t& hdr);

    status_t get_section_name(const binfmt::section_t* section, str::slice_t& name);
}
