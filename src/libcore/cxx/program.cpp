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

#include "../memory/proxy_system.h"
#include "cxx.h"

namespace basecode::cxx::program {
    u0 free(program_t& pgm) {
        bass::free(pgm.storage);
        for (auto& module : pgm.modules)
            module::free(module);
        array::free(pgm.modules);
        intern::free(pgm.intern);
    }

    u0 debug_dump(program_t& pgm) {
        u32 value{};
        auto cursor = bass::first_header(pgm.storage);
        while (cursor.ok) {
            format::print(
                "{}(bytes={}, field_capacity={}) {{\n",
                element::header::name(cursor.header->type),
                cursor.header->value,
                (cursor.header->value / sizeof(field_t)) - 1);

            while (bass::next_field(cursor, value)) {
                format::print(
                    "\t{:<16} = {}",
                    element::field::name(cursor.field->type),
                    cursor.field->value);
                switch (cursor.field->type) {
                    case element::field::revision: {
                        format::print(
                            "\n\t{:<16} = '{}'",
                            " .name",
                            program::revision_name((revision_t) value));
                        break;
                    }
                    case element::field::type: {
                        switch (cursor.header->type) {
                            case element::header::type: {
                                auto base_type = (meta_type_t) BASE_TYPE(value);
                                format::print(
                                    "\n\t{:<16} = '{}'",
                                    " .name",
                                    scope::type::meta_name(base_type));
                                break;
                            }
                            case element::header::statement: {
                                auto base_type = (statement_type_t) BASE_TYPE(value);
                                format::print(
                                    "\n\t{:<16} = '{}'",
                                    " .name",
                                    scope::stmt::name(base_type));
                                break;
                            }
                            case element::header::expression: {
                                auto base_type = (expression_type_t) BASE_TYPE(value);
                                auto sub_type = SUB_TYPE(value);
                                format::print(
                                    "\n\t{:<16} = '{}'",
                                    " .name",
                                    scope::expr::name(base_type));
                                string::slice_t sub_type_name;
                                switch (base_type) {
                                    case expression_type_t::raw: {
                                        sub_type_name = "none"_ss;
                                        break;
                                    }
                                    case expression_type_t::initializer: {
                                        sub_type_name = cxx::scope::expr::init::name((initializer_type_t) sub_type);
                                        break;
                                    }
                                    case expression_type_t::assignment: {
                                        sub_type_name = cxx::scope::expr::assign::name((assignment_type_t) sub_type);
                                        break;
                                    }
                                    case expression_type_t::binary: {
                                        sub_type_name = cxx::scope::expr::binary::name((binary_op_type_t) sub_type);
                                        break;
                                    }
                                    case expression_type_t::unary: {
                                        sub_type_name = cxx::scope::expr::unary::name((unary_op_type_t) sub_type);
                                        break;
                                    }
                                }
                                format::print("\n\t{:<16} = '{}'", " .sub_type", sub_type_name);
                                break;
                            }
                            default: {
                                break;
                            }
                        }
                        break;
                    }
                    case element::field::intern: {
                        auto rc = intern::get(pgm.intern, value);
                        format::print("\n\t{:<16} = '{}'", " .slice", rc.slice);
                        break;
                    }
                    default: {
                        break;
                    }
                }
                format::print("\n");
            }

            format::print("}}\n");
            bass::next_header(cursor);
        }
    }

    status_t finalize(program_t& pgm) {
        auto list_cursor = bass::write_header(pgm.storage, element::header::list, pgm.modules.size + 1);
        bass::write_field(list_cursor, element::field::parent, pgm.id);
        for (const auto& module : pgm.modules)
            bass::write_field(list_cursor, element::field::child, module.id);

        u32 value{};
        auto pgm_cursor = bass::get_header(pgm.storage, pgm.id);
        if (!bass::next_field(pgm_cursor, value, element::field::list))
            return status_t::list_not_found;
        bass::write_field(pgm_cursor, element::field::list, list_cursor.id);

        for (auto& module : pgm.modules) {
            auto status = module::finalize(module);
            if (!OK(status))
                return status;
        }

        return status_t::ok;
    }

    module_t& get_module(program_t& pgm, u32 module_idx) {
        return pgm.modules[module_idx];
    }

    u0 init(program_t& pgm, alloc_t* alloc, u32 num_modules) {
        intern::init(pgm.intern, alloc);
        array::init(pgm.modules, alloc);
        array::reserve(pgm.modules, num_modules);
        bass::init(pgm.storage, alloc);
        auto cursor = bass::write_header(pgm.storage, element::header::program, 1);
        bass::write_field(cursor, element::field::list, 0);
        pgm.id = cursor.id;
        pgm.alloc = alloc;
    }

    module_t& add_module(program_t& pgm, string::slice_t filename, cxx::revision_t rev) {
        auto& mod = array::append(pgm.modules);
        mod.idx = pgm.modules.size - 1;
        module::init(mod, pgm, filename, rev, pgm.alloc);
        return mod;
    }
}
