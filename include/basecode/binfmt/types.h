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

#define SYMBOL_DERIVED_TYPE(type)   (u32((type)) >> 16U & 0xffffU)
#define SYMBOL_TYPE(derived, type)  (u32((derived)) << 16U | u32((type)))

namespace basecode::binfmt {
    struct reloc_t;
    struct module_t;
    struct symbol_t;
    struct import_t;
    struct section_t;
    struct version_t;

    using import_id             = u32;
    using symbol_id             = u32;
    using module_id             = u32;
    using section_id            = u32;

    using id_list_t             = array_t<u32>;
    using reloc_list_t          = array_t<reloc_t>;
    using import_list_t         = array_t<import_t>;
    using symbol_list_t         = array_t<symbol_t>;
    using section_list_t        = array_t<section_t>;
    using section_ptr_list_t    = array_t<section_t*>;
    using symbol_table_t        = hashtab_t<intern_id, symbol_t>;

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
    };

    namespace symbol {
        struct type_t final {
            constexpr type_t()         : derived(0), base_type(0)
            {}
            constexpr type_t(u32 type) : derived(type >> 16U & 0xffffU), base_type(type & 0xffffU)
            {}
            constexpr operator u32() const              { return SYMBOL_TYPE(derived, base_type);   }
            explicit constexpr operator u16() const     { return u8(derived) << 8U | u8(base_type); }
            [[nodiscard]] constexpr b8 empty() const    { return base_type == 0 && derived == 0;    }
            static constexpr type_t none()              { return 0;                                 }
        private:
            u32                     derived:    16;
            u32                     base_type:  16;
        };

        namespace type::base {
            [[maybe_unused]] constexpr u16 null_            = 0b0000;
            [[maybe_unused]] constexpr u16 void_            = 0b0001;
            [[maybe_unused]] constexpr u16 char_            = 0b0010;
            [[maybe_unused]] constexpr u16 short_           = 0b0011;
            [[maybe_unused]] constexpr u16 int_             = 0b0100;
            [[maybe_unused]] constexpr u16 long_            = 0b0101;
            [[maybe_unused]] constexpr u16 float_           = 0b0110;
            [[maybe_unused]] constexpr u16 double_          = 0b0111;
            [[maybe_unused]] constexpr u16 struct_          = 0b1000;
            [[maybe_unused]] constexpr u16 union_           = 0b1001;
            [[maybe_unused]] constexpr u16 enum_            = 0b1010;
            [[maybe_unused]] constexpr u16 member_of_enum   = 0b1011;
            [[maybe_unused]] constexpr u16 uchar            = 0b1100;
            [[maybe_unused]] constexpr u16 uint             = 0b1101;
            [[maybe_unused]] constexpr u16 ulong            = 0b1110;
            [[maybe_unused]] constexpr u16 long_double      = 0b1111;
        }

        namespace type::derived {
            [[maybe_unused]] constexpr u16 none             = 0b0000;
            [[maybe_unused]] constexpr u16 pointer          = 0b0001;
            [[maybe_unused]] constexpr u16 function         = 0b0010;
            [[maybe_unused]] constexpr u16 array            = 0b0011;
        }
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
            u32                     code:       1;
            u32                     data:       1;
            u32                     init:       1;
            u32                     read:       1;
            u32                     exec:       1;
            u32                     write:      1;
            u32                     alloc:      1;
            u32                     can_free:   1;
            u32                     pad:        24;
        };
    }

    namespace storage {
        enum class class_t : u8 {
            null_,
            auto_,
            external_,
            static_,
            register_,
            extern_def,
            label,
            undef_label,
            member_of_struct,
            argument,
            struct_tag,
            member_of_union,
            union_tag,
            type_def,
            undef_static,
            enum_tag,
            member_of_enum,
            register_param,
            bit_field,
            auto_argument,
            dummy_entry,
            block,
            function,
            end_of_struct,
            file_name,
            line_number,
            alias,
            hidden,
            end_of_function,
        };
    }

    struct symbol_t final {
        intern_id               name        {};
        section_id              section     {};
        symbol::type_t          type        {};
        u32                     length      {};
        u32                     value       {};
        storage::class_t        sclass      {};

        b8 operator==(const symbol_t& other) const {
            return name == other.name;
        }
    };

    struct symbol_opts_t final {
        section_id              section     {};
        symbol::type_t          type        {};
        u32                     value       {};
        storage::class_t        sclass      {};
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

    struct version_t final {
        u16                     major;
        u16                     minor;
    };

    struct module_t final {
        alloc_t*                alloc;
        symbol_table_t          symbols;
        section_list_t          sections;
        module_id               id;
    };

    struct result_t final {
        u32                     id;
        status_t                status;
    };
}
