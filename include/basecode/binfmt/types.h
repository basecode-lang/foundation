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
#include <basecode/binfmt/configure.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode::binfmt {
    struct reloc_t;
    struct module_t;
    struct symbol_t;
    struct import_t;
    struct member_t;
    struct section_t;
    struct version_t;
    struct symbol_offs_t;

    using import_id             = u32;
    using symbol_id             = u32;
    using module_id             = u32;
    using member_id             = u32;
    using section_id            = u32;

    using id_list_t             = array_t<u32>;
    using reloc_list_t          = array_t<reloc_t>;
    using import_list_t         = array_t<import_t>;
    using symbol_list_t         = array_t<symbol_t>;
    using member_list_t         = array_t<member_t>;
    using section_list_t        = array_t<section_t>;
    using symbol_table_t        = hashtab_t<intern_id, symbol_t>;
    using section_ptr_list_t    = array_t<section_t*>;
    using symbol_offs_list_t    = array_t<symbol_offs_t>;

    enum class status_t : u32 {
        ok                              = 0,
        read_error                      = 2000,
        write_error                     = 2001,
        import_failure                  = 2002,
        duplicate_import                = 2003,
        duplicate_symbol                = 2004,
        symbol_not_found                = 2005,
        config_eval_error               = 2006,
        invalid_section_type            = 2007,
        container_init_error            = 2008,
        invalid_container_type          = 2009,
        spec_section_custom_name        = 2010,
        not_implemented                 = 2011,
        invalid_machine_type            = 2012,
        invalid_output_type             = 2013,
        cannot_map_section_name         = 2014,
        section_not_found               = 2015,
        invalid_input_type              = 2016,
    };

    enum class module_type_t : u8 {
        archive,
        object,
    };

    namespace symbol {
        enum class type_t : u8 {
            none,
            file,
            object,
            function,
        };

        enum class scope_t : u8 {
            none,
            local,
            global,
            weak
        };

        enum class visibility_t : u8 {
            default_,
            internal_,
            hidden,
            protected_,
        };
    }

    namespace machine {
        enum class type_t : u32 {
            unknown,
            x86_64,
            aarch64,
        };

        namespace x86_64::reloc {
            enum class type_t : u8 {
                absolute,
                addr64,
                addr32,
                addr32nb,
                rel32,
                section,
                section_rel,
                pair
            };
        }

        namespace aarch64::reloc {
            enum class type_t : u8 {
                absolute,
                addr32,
                addr32nb,
                branch24,
                branch11,
                rel32,
                section,
                section_rel,
                mov32,
                pair,
            };
        }
    }

    namespace section {
        // XXX: ELF symbol table: SHT_HASH, SHT_SYMTAB, SHT_STRTAB
        enum class type_t : u8 {
            tls,                    // XXX: win only?
            code,                   // XXX: SHT_PROGBITS
            data,                   // XXX: SHT_PROGBITS or SHT_NOBITS
            debug,                  // XXX: SHT_PROGBITS
            reloc,                  // XXX: SHT_RELA, SHT_REL
            import,                 // XXX: SHT_DYNAMIC, SHT_DYNSYM
            export_,
            resource,               // XXX: win only?
            exception,              // XXX: win only?
            custom
        };

        struct flags_t final {
            u32                 code:       1;
            u32                 data:       1;
            u32                 init:       1;
            u32                 read:       1;
            u32                 exec:       1;
            u32                 write:      1;
            u32                 alloc:      1;
            u32                 can_free:   1;
            u32                 pad:        24;
        };
    }

    struct symbol_t final {
        u64                     size;
        u64                     value;
        intern_id               name;
        section_id              section;
        symbol::type_t          type;
        symbol::scope_t         scope;
        symbol::visibility_t    visibility;

        b8 operator==(const symbol_t& other) const {
            return name == other.name;
        }
    };

    struct symbol_opts_t final {
        u64                     size;
        u64                     value;
        section_id              section;
        symbol::type_t          type;
        symbol::scope_t         scope;
        symbol::visibility_t    visibility;
    };

    struct symbol_offs_t final {
        symbol_id               symbol;
        u32                     offset;
    };

    struct import_t final {
        id_list_t               symbols;
        symbol_id               module_symbol;
        section_id              section;
        import_id               id;
        struct {
            u32                 load:   1;
            u32                 pad:    31;
        }                       flags;
    };

    struct reloc_t final {
        u64                     virtual_addr;
        symbol_id               symbol;
        u8                      type;
    };

    union section_subclass_t {
        u64                     size;
        str::slice_t            data;
        import_list_t           imports;
    };

    struct section_t final {
        alloc_t*                alloc;
        const module_t*         module;
        symbol_list_t           symbols;
        section_subclass_t      subclass;
        u32                     align;
        symbol_id               symbol;
        section_id              id;
        section::flags_t        flags;
        section::type_t         type;
    };

    struct section_opts_t final {
        symbol_id               symbol;
        section::flags_t        flags;
        u32                     align;
    };

    enum class member_type_t : u8 {
        module,
        raw
    };

    union member_subclass_t final {
        str::slice_t            raw;
        module_t*               module;
    };

    struct member_t final {
        str::slice_t            name;
        member_subclass_t       subclass;
        member_id               id;
        member_type_t           type;
    };

    struct version_t final {
        u16                     major;
        u16                     minor;
    };

    union module_subclass_t final {
        struct {
            section_list_t      sections;
        }                       object;
        struct {
            member_list_t       members;
            symbol_offs_list_t  offsets;
        }                       archive;
    };

    struct module_t final {
        alloc_t*                alloc;
        symbol_table_t          symbols;
        module_subclass_t       subclass;
        module_id               id;
        module_type_t           type;
    };

    struct result_t final {
        u32                     id;
        status_t                status;
    };
}
