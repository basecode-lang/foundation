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

#define SYMBOL_DERIVED_TYPE(type)   (u32((type)) >> 16U & 0xffffU)
#define SYMBOL_TYPE(derived, type)  (u32((derived)) << 16U | u32((type)))

namespace basecode {
    struct symbol_t;
    struct section_t;
    struct version_t;
    struct obj_file_t;

    using symbol_table_t        = hashtab_t<u32, symbol_t>;
    using section_list_t        = array_t<section_t>;

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

    enum class section_type_t : u8 {
    };

    enum class relocation_type_t : u8 {
    };

    struct symbol_t final {
        str::slice_t            name        {};
        u32                     id          {};
        section_t*              section     {};
        symbol_type_t           type        {};
    };

    struct address_t final {
        u64                     physical;
        u64                     virtual_;
    };

    struct relocation_t final {
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
    };

    struct section_t final {
        alloc_t*                alloc;
        str::slice_t            name;
        address_t               address;
        u64                     size;
        symbol_table_t          symbols;
        section_subclass_t      subclass;
        section_type_t          type;
    };

    struct version_t final {
        u16                     major;
        u16                     minor;
    };

    struct obj_file_t final {
        alloc_t*                alloc       {};
        path_t                  path;
        section_list_t          sections    {};
        version_t               version     {};
    };

    namespace objfmt {
        enum class status_t : u32 {
            ok                  = 0,
            read_error          = 2001,
            write_error         = 2002,
            config_eval_error   = 2000
        };

        namespace system {
            u0 fini();

            status_t init(alloc_t* alloc = context::top()->alloc);
        }

        status_t read(container::type_t type, obj_file_t& file);

        status_t write(container::type_t type, obj_file_t& file);
    }
}
