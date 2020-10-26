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

#include <basecode/core/path.h>
#include <basecode/core/hashtab.h>
#include <basecode/objfmt/configure.h>
#include <basecode/objfmt/container.h>
#include <basecode/core/memory/system/slab.h>

#define SYMBOL_DERIVED_TYPE(type)   (u32((type)) >> 16U & 0xffffU)
#define SYMBOL_TYPE(derived, type)  (u32((derived)) << 16U | u32((type)))

namespace basecode {
    struct symbol_t;
    struct import_t;
    struct section_t;
    struct version_t;
    struct obj_file_t;

    using import_list_t         = array_t<import_t>;
    using symbol_list_t         = array_t<symbol_t*>;
    using section_list_t        = array_t<section_t*>;
    using symbol_table_t        = hashtab_t<str::slice_t, symbol_t*>;

    struct symbol_type_t final {
        constexpr symbol_type_t()         : derived(0), base_type(0)
        {}
        constexpr symbol_type_t(u32 type) : derived(type >> 16U & 0xffffU), base_type(type & 0xffffU)
        {}
        constexpr operator u32() const              { return SYMBOL_TYPE(derived, base_type);   }
        [[nodiscard]] constexpr b8 empty() const    { return base_type == 0 && derived == 0;    }
        static constexpr symbol_type_t none()       { return 0;                                 }
    private:
        u32                     derived:    16;
        u32                     base_type:  16;
    };

    enum class machine_type_t : u16 {
    };

    enum class section_type_t : u8 {
        data,
        uninit,
        import
    };

    enum class relocation_type_t : u8 {
    };

    struct symbol_t final {
        str::slice_t            name        {};
        section_t*              section     {};
        symbol_type_t           type        {};

        b8 operator==(const symbol_t& other) const {
            return name == other.name;
        }
    };

    struct import_t final {
        const symbol_t*         module;
        const section_t*        section;
        symbol_list_t           symbols;
        struct {
            u32                 load:   1;
            u32                 pad:    31;
        }                       flags;
    };

    struct address_t final {
        u64                     physical;
        u64                     virtual_;
    };

    struct relo_entry_t final {
        address_t               address;
        symbol_t*               symbol;
        relocation_type_t       type;
    };

    struct line_number_entry_t final {
        address_t               address;
        symbol_t*               symbol;
        u16                     line_number;
    };

    union section_subclass_t {
        u64                     size;
        str::slice_t            data;
        import_list_t           imports;
    };

    struct section_t final {
        alloc_t*                alloc;
        const symbol_t*         symbol;
        const obj_file_t*       file;
        address_t               address;
        symbol_list_t           symbols;
        section_subclass_t      subclass;
        struct {
            u32                 code:   1;
            u32                 data:   1;
            u32                 read:   1;
            u32                 exec:   1;
            u32                 write:  1;
            u32                 pad:    26;
        }                       flags;
        section_type_t          type;
    };

    struct version_t final {
        u16                     major;
        u16                     minor;
    };

    struct obj_file_t final {
        alloc_t*                alloc       {};
        alloc_t*                symbol_slab {};
        alloc_t*                section_slab{};
        path_t                  path        {};
        symbol_table_t          symbols     {};
        section_list_t          sections    {};
        version_t               version     {};
        machine_type_t          machine     {};
    };

    namespace objfmt {
        enum class status_t : u32 {
            ok                              = 0,
            read_error                      = 2000,
            write_error                     = 2001,
            import_failure                  = 2002,
            duplicate_import                = 2008,
            duplicate_symbol                = 2003,
            symbol_not_found                = 2004,
            config_eval_error               = 2005,
            invalid_section_type            = 2006,
            container_init_error            = 2007,
        };

        namespace system {
            u0 fini();

            status_t init(alloc_t* alloc = context::top()->alloc);
        }

        namespace import {
            status_t add_symbol(import_t* import, const symbol_t* symbol);
        }

        namespace section {
            u0 free(section_t* section);

            u0 reserve(section_t* section, u64 size);

            u0 data(section_t* section, const u8* data, u32 length);

            status_t init(section_t* section, section_type_t type, const symbol_t* symbol);

            status_t import_module(section_t* section, import_t** result, const symbol_t* module);
        }

        namespace obj_file {
            u0 free(obj_file_t& file);

            status_t init(obj_file_t& file);

            status_t find_section(obj_file_t& file, section_t** result, const symbol_t* symbol);

            status_t find_symbol(obj_file_t& file, symbol_t** result, const s8* name, s32 len = -1);

            status_t make_symbol(obj_file_t& file, symbol_t** result, const s8* name, s32 len = -1);

            status_t find_symbol(obj_file_t& file, symbol_t** result, const String_Concept auto& name) {
                return find_symbol(file, result, (const s8*) name.data, name.length);
            }

            status_t make_symbol(obj_file_t& file, symbol_t** result, const String_Concept auto& name) {
                return make_symbol(file, result, (const s8*) name.data, name.length);
            }

            status_t make_section(obj_file_t& file, section_t** result, section_type_t type, const symbol_t* symbol);
        }
    }
}
