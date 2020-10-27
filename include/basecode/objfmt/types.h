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
#include <basecode/core/intern.h>
#include <basecode/core/hashtab.h>
#include <basecode/objfmt/configure.h>
#include <basecode/core/memory/system/slab.h>

#define SYMBOL_DERIVED_TYPE(type)   (u32((type)) >> 16U & 0xffffU)
#define SYMBOL_TYPE(derived, type)  (u32((derived)) << 16U | u32((type)))

namespace basecode::objfmt {
    struct file_t;
    struct symbol_t;
    struct import_t;
    struct section_t;
    struct version_t;

    using import_id             = u32;
    using symbol_id             = u32;
    using section_id            = u32;

    using id_list_t             = array_t<u32>;
    using import_list_t         = array_t<import_t>;
    using symbol_list_t         = array_t<symbol_t>;
    using section_list_t        = array_t<section_t>;
    using symbol_table_t        = hashtab_t<intern_id, symbol_t>;

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
        invalid_container_type          = 2009,
    };

    namespace machine {
        enum class type_t : u32 {
        };
    }

    namespace section {
        enum class type_t : u8 {
            data,
            uninit,
            import
        };
    }

    namespace relocation {
        enum class type_t : u8 {
        };
    }

    struct symbol_t final {
        intern_id               name        {};
        section_id              section     {};
        symbol_type_t           type        {};

        b8 operator==(const symbol_t& other) const {
            return name == other.name;
        }
    };

    struct import_t final {
        id_list_t               symbols;
        symbol_id               module;
        section_id              section;
        import_id               id;
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
        symbol_id               symbol;
        relocation::type_t      type;
    };

    struct lineno_entry_t final {
        address_t               address;
        symbol_id               symbol;
        u16                     line_number;
    };

    union section_subclass_t {
        u64                     size;
        str::slice_t            data;
        import_list_t           imports;
    };

    struct section_t final {
        alloc_t*                alloc;
        const file_t*           file;
        address_t               address;
        symbol_list_t           symbols;
        section_subclass_t      subclass;
        symbol_id               symbol;
        section_id              id;
        struct {
            u32                 code:   1;
            u32                 data:   1;
            u32                 read:   1;
            u32                 exec:   1;
            u32                 write:  1;
            u32                 pad:    26;
        }                       flags;
        section::type_t         type;
    };

    struct version_t final {
        u16                     major;
        u16                     minor;
    };

    struct file_t final {
        alloc_t*                alloc       {};
        path_t                  path        {};
        symbol_table_t          symbols     {};
        section_list_t          sections    {};
        version_t               version     {};
        machine::type_t         machine     {};
    };

    struct result_t final {
        u32                     id;
        status_t                status;
    };
}
