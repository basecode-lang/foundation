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

namespace basecode::binfmt::io::coff {
    struct symbol_t;
    struct aux_record_t;
    struct section_hdr_t;

    using symbol_list_t         = array_t<symbol_t>;
    using string_list_t         = array_t<str::slice_t>;
    using aux_list_t            = array_t<aux_record_t>;
    using section_hdr_list_t    = array_t<section_hdr_t>;

    struct rva_t final {
        u32                     base;
        u32                     size;
    };

    struct raw_t final {
        u32                     offset;
        u32                     size;
    };

    // .pdata
    struct exception_t final {
        u32                     begin_rva;
        u32                     end_rva;
        u32                     unwind_info;
    };

    enum class aux_record_type_t : u8 {
        xf,
        file,
        section,
        func_def,
        weak_extern,
    };

    struct aux_record_t final {
        union {
            struct {
                symbol_t*       sym;
            }                   file;
            struct {
                u32             ptr_next_func;
                u16             line_num;
            }                   xf;
            struct {
                u32             tag_idx;
                u32             flags;
            }                   weak_extern;
            struct {
                u32             tag_idx;
                u32             total_size;
                u32             ptr_line_num;
                u32             ptr_next_func;
            }                   func_def;
            struct {
                u32             len;
                u32             check_sum;
                u16             num_relocs;
                u16             num_lines;
                u16             sect_num;
                u8              comdat_sel;
            }                   section;
        };
        aux_record_type_t       type;
    };

    struct symbol_t final {
        aux_list_t              aux_records;
        union {
            u64                 offset;
            str::slice_t        slice;
        }                       name;
        u32                     value;
        u16                     type;
        s16                     section;
        u8                      sclass;
        b8                      inlined;
    };

    struct section_hdr_t final {
        const section_t*        section;
        symbol_t*               symbol;
        u32                     rva;
        u32                     size;
        u32                     number;
        u32                     offset;
        raw_t                   relocs;
        raw_t                   line_nums;
    };

    // .debug$S (symbolic info)
    // .debug$T (type info)
    struct debug_t final {
    };

    struct coff_t final {
        alloc_t*                alloc;
        section_hdr_list_t      headers;
        u32                     rva;
        u32                     offset;
        struct {
            raw_t               file;
            string_list_t       list;
        }                       string_table;
        struct {
            raw_t               file;
            symbol_list_t       list;
        }                       symbol_table;
        struct {
            u32                 image;
            u32                 headers;
        }                       size;
        struct {
            u32                 image;
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

    namespace machine {
        [[maybe_unused]] constexpr u16 amd64 = 0x8664;
        [[maybe_unused]] constexpr u16 arm64 = 0xaa64;
    }

    namespace section {
        [[maybe_unused]] constexpr u32 header_size         = 0x28;
        [[maybe_unused]] constexpr u32 code                = 0x00000020;
        [[maybe_unused]] constexpr u32 init_data           = 0x00000040;
        [[maybe_unused]] constexpr u32 uninit_data         = 0x00000080;
        [[maybe_unused]] constexpr u32 align_1             = 0x00100000;
        [[maybe_unused]] constexpr u32 align_2             = 0x00200000;
        [[maybe_unused]] constexpr u32 align_4             = 0x00300000;
        [[maybe_unused]] constexpr u32 align_8             = 0x00400000;
        [[maybe_unused]] constexpr u32 align_16            = 0x00500000; // default
        [[maybe_unused]] constexpr u32 align_32            = 0x00600000;
        [[maybe_unused]] constexpr u32 align_64            = 0x00700000;
        [[maybe_unused]] constexpr u32 align_128           = 0x00800000;
        [[maybe_unused]] constexpr u32 align_256           = 0x00900000;
        [[maybe_unused]] constexpr u32 align_512           = 0x00a00000;
        [[maybe_unused]] constexpr u32 align_1024          = 0x00b00000;
        [[maybe_unused]] constexpr u32 align_2048          = 0x00c00000;
        [[maybe_unused]] constexpr u32 align_4096          = 0x00d00000;
        [[maybe_unused]] constexpr u32 align_8192          = 0x00e00000;
        [[maybe_unused]] constexpr u32 link_reloc_overflow = 0x01000000;
        [[maybe_unused]] constexpr u32 memory_can_discard  = 0x02000000;
        [[maybe_unused]] constexpr u32 memory_not_cached   = 0x04000000;
        [[maybe_unused]] constexpr u32 memory_not_paged    = 0x08000000;
        [[maybe_unused]] constexpr u32 memory_shared       = 0x10000000;
        [[maybe_unused]] constexpr u32 memory_execute      = 0x20000000;
        [[maybe_unused]] constexpr u32 memory_read         = 0x40000000;
        [[maybe_unused]] constexpr u32 memory_write        = 0x80000000;
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

    namespace storage_class {
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

        u8 coff_storage_class(storage::class_t cls);
    }

    namespace string_table {
        u0 free(coff_t& coff);

        u0 init(coff_t& coff, alloc_t* alloc);

        u32 add(coff_t& coff, str::slice_t str);
    }

    namespace symbol_table {
        [[maybe_unused]] constexpr u32 entry_size = 0x12;

        u0 free(coff_t& sym);

        u0 init(coff_t& sym, alloc_t* alloc);

        aux_record_t* make_aux_record(coff_t& coff,
                                      symbol_t* sym,
                                      aux_record_type_t type);

        symbol_t* make_symbol(coff_t& coff, str::slice_t name);
    }

    u0 free(coff_t& coff);

    u0 update_symbol_table(coff_t& coff);

    u0 write_string_table(session_t& s, coff_t& coff);

    u0 write_symbol_table(session_t& s, coff_t& coff);

    status_t build_sections(session_t& s, coff_t& coff);

    u0 write_section_headers(session_t& s, coff_t& coff);

    status_t write_sections_data(session_t& s, coff_t& coff);

    status_t init(coff_t& coff, file_t& file, alloc_t* alloc);

    u0 write_aux_record(session_t& s, const aux_record_t& record);

    u0 write_header(session_t& s, coff_t& coff, u16 opt_hdr_size = {});

    status_t build_section(session_t& s, coff_t& coff, section_hdr_t& hdr);

    u0 write_section_header(session_t& s, coff_t& coff, section_hdr_t& hdr);

    status_t write_section_data(session_t& s, coff_t& coff, section_hdr_t& hdr);

    status_t get_section_name(const binfmt::section_t* section, str::slice_t& name);
}
