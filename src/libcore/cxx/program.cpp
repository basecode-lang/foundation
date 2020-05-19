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

#include <basecode/core/cxx/cxx.h>
#include <basecode/core/memory/system/proxy.h>

namespace basecode::cxx::program {
    static b8 format_record(format_type_t type, cursor_t& c, fmt_buf_t& buf, u0* ctx) {
        auto pgm = (program_t*) ctx;
        if (type == format_type_t::field) {
            auto value = c.field->value;
            format::format_to(
                buf,
                "\t{:<16} = {}",
                element::field::name(c.field->type),
                c.field->value);
            switch (c.field->type) {
                case element::field::revision: {
                    format::format_to(
                        buf,
                        "\n\t{:<16} = '{}'",
                        " .name",
                        program::revision_name((revision_t) value));
                    break;
                }
                case element::field::type: {
                    switch (c.header->type) {
                        case element::header::type: {
                            auto base_type = (meta_type_t) BASE_TYPE(value);
                            format::format_to(
                                buf,
                                "\n\t{:<16} = '{}'",
                                " .name",
                                scope::type::meta_name(base_type));
                            break;
                        }
                        case element::header::statement: {
                            auto base_type = (statement_type_t) BASE_TYPE(value);
                            format::format_to(
                                buf,
                                "\n\t{:<16} = '{}'",
                                " .name",
                                scope::stmt::name(base_type));
                            break;
                        }
                        case element::header::expression: {
                            auto base_type = (expression_type_t) BASE_TYPE(value);
                            auto sub_type = SUB_TYPE(value);
                            format::format_to(
                                buf,
                                "\n\t{:<16} = '{}'",
                                " .name",
                                scope::expr::name(base_type));
                            str::slice_t sub_type_name = "unknown"_ss;
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
                            format::format_to(buf, "\n\t{:<16} = '{}'", " .sub_type", sub_type_name);
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                    break;
                }
                case element::field::intern: {
                    auto rc = intern::get(pgm->intern, value);
                    format::format_to(buf, "\n\t{:<16} = '{}'", " .slice", rc.slice);
                    break;
                }
                default: {
                    break;
                }
            }
            format::format_to(buf, "\n");
        } else {
            format::format_to(
                buf,
                "{}(bytes={}, field_capacity={}) {{\n",
                element::header::name(c.header->type),
                c.header->value,
                RECORD_FIELD_COUNT(c.header->value));
        }
        return true;
    }

    u0 free(program_t& pgm) {
        str::free(pgm.scratch);
        bass::free(pgm.storage);
        for (auto& module : pgm.modules)
            module::free(module);
        array::free(pgm.modules);
        intern::free(pgm.intern);
    }

    status_t finalize(program_t& pgm) {
        cursor_t list_cursor{};
        bass::seek_current(pgm.storage, list_cursor);
        bass::new_record(list_cursor, element::header::list, pgm.modules.size + 1);
        bass::write_field(list_cursor, element::field::parent, pgm.id);
        for (const auto& module : pgm.modules)
            bass::write_field(list_cursor, element::field::child, module.id);

        cursor_t pgm_cursor{};
        u32 value{};
        bass::seek_record(pgm.storage, pgm.id, pgm_cursor);
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

    u0 debug_dump(program_t& pgm, fmt_buf_t& buf) {
        cursor_t cursor{};
        if (!bass::seek_first(pgm.storage, cursor))
            return;
        do {
            bass::format_record(pgm.storage, buf, cursor.id, format_record, &pgm);
            format::format_to(buf, "}}\n");
        } while (bass::next_record(cursor));
    }

    module_t& get_module(program_t& pgm, u32 module_idx) {
        return pgm.modules[module_idx];
    }

    u0 init(program_t& pgm, alloc_t* alloc, u32 num_modules) {
        intern::init(pgm.intern, alloc);
        array::init(pgm.modules, alloc);
        array::reserve(pgm.modules, num_modules);
        str::init(pgm.scratch, alloc);
        str::reserve(pgm.scratch, 64);
        bass::init(pgm.storage, alloc);
        cursor_t cursor{};
        bass::seek_current(pgm.storage, cursor);
        bass::new_record(cursor, element::header::program, 1);
        bass::write_field(cursor, element::field::list, 0);
        pgm.id = cursor.id;
        pgm.alloc = alloc;
    }

    module_t& add_module(program_t& pgm, str::slice_t filename, cxx::revision_t rev) {
        auto& mod = array::append(pgm.modules);
        mod.idx = pgm.modules.size - 1;
        module::init(mod, pgm, filename, rev, pgm.alloc);
        return mod;
    }
}
