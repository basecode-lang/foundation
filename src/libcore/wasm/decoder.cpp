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
// Copyright (C) 2017-2021 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <basecode/core/buf.h>
#include <basecode/core/leb128.h>
#include <basecode/core/wasm/decoder.h>

namespace basecode::wasm {
    namespace module {
        status_t decode(module_t& module) {
            FILE_ALIAS(module);
            FILE_READ(u32, module.magic);
            FILE_READ(u32, module.version);
            while (CRSR_MORE(module.crsr)) {
                auto sect = make_section(module);
                auto status = section::decode(sect);
                if (!OK(status))
                    return status;
            }
            return status_t::ok;
        }
    }

    namespace section {
        status_t decode(section_t* sect) {
            FILE_ALIAS(*sect->module);

            u32 size;
            u8 type_code; FILE_READ_ULEB128(u8, type_code, size);
            sect->type = section_type_t(type_code);
            FILE_READ_ULEB128(u32, sect->size, size);
            sect->payload = FILE_PTR();

            status_t status;
            switch (sect->type) {
                case section_type_t::type:
                    status = section::read_types(sect);
                    break;
                case section_type_t::code:
                    status = section::read_code(sect);
                    break;
                case section_type_t::data:
                    status = section::read_data(sect);
                    break;
                case section_type_t::table:
                    status = section::read_table(sect);
                    break;
                case section_type_t::start:
                    status = section::read_start(sect);
                    break;
                case section_type_t::import:
                    status = section::read_imports(sect);
                    break;
                case section_type_t::memory:
                    status = section::read_memory(sect);
                    break;
                case section_type_t::global:
                    status = section::read_global(sect);
                    break;
                case section_type_t::custom:
                    status = section::read_custom(sect);
                    break;
                case section_type_t::export_:
                    status = section::read_exports(sect);
                    break;
                case section_type_t::element:
                    status = section::read_elements(sect);
                    break;
                case section_type_t::function:
                    status = section::read_functions(sect);
                    break;
                case section_type_t::data_count:
                    status = section::read_data_count(sect);
                    break;
                default:
                    return status_t::invalid_section;
            }

            return status;
        }

        status_t read_code(section_t* sect) {
            FILE_ALIAS(*sect->module);
            auto sc = &sect->subclass.code;
            array::init(sc->bodies, sect->module->wasm->alloc);
            u32 size;
            u32 count;  FILE_READ_ULEB128(u32, count, size);
            for (u32 i = 0; i < count; ++i) {
                auto& body = array::append(sc->bodies);
                array::init(body.locals, sc->bodies.alloc);
                array::init(body.instructions, sc->bodies.alloc);
                u32 body_size;      FILE_READ_ULEB128(u32, body_size, size);
                u32 local_count;    FILE_READ_ULEB128(u32, local_count, size);
                for (u32 j = 0; j < local_count; ++j) {
                    auto& local = array::append(body.locals);
                    FILE_READ_ULEB128(u32, local.count, size);
                    u8 type_code; FILE_READ(u8, type_code);
                    local.value_type = type::make_valtype(core_type_t(type_code));
                }
                auto status = instruction::read_body(*sect->module, body.instructions);
                if (!OK(status))
                    return status;
            }
            return status_t::ok;
        }

        status_t read_data(section_t* sect) {
            FILE_ALIAS(*sect->module);
            auto sc = &sect->subclass.data;
            array::init(sc->segments, sect->module->wasm->alloc);
            u32 size;
            u32 count;  FILE_READ_ULEB128(u32, count, size);
            for (u32 i = 0; i < count; ++i) {
                auto& segment = array::append(sc->segments);
                array::init(segment.offset, sc->segments.alloc);
                FILE_READ_ULEB128(u32, segment.index, size);
                auto status = instruction::read_body(*sect->module, segment.offset);
                if (!OK(status))
                    return status;
                FILE_READ_ULEB128(u32, segment.size, size);
                segment.data = FILE_PTR();
                FILE_SEEK_FWD(segment.size);
            }
            return status_t::ok;
        }

        status_t read_start(section_t* sect) {
            FILE_ALIAS(*sect->module);
            u32 size;
            FILE_READ_ULEB128(u32, sect->subclass.start_func_idx, size);
            return status_t::ok;
        }

        status_t read_table(section_t* sect) {
            FILE_ALIAS(*sect->module);
            auto sc = &sect->subclass.table;
            array::init(sc->elements, sect->module->wasm->alloc);
            u32 size;
            u32 num_entries;
            FILE_READ_ULEB128(u32, num_entries, size);
            for (u32 i = 0; i < num_entries; ++i) {
                auto& elem = array::append(sc->elements);
                u8 elem_type; FILE_READ(u8, elem_type);
                elem.type = elem_type_t(elem_type);
                FILE_READ(u8, elem.limits.flags);
                FILE_READ_ULEB128(u32, elem.limits.initial, size);
                if ((elem.limits.flags & 0x1U) != 0)
                    FILE_READ_ULEB128(u32, elem.limits.maximum, size);
            }

            return status_t::ok;
        }

        status_t read_types(section_t* sect) {
            FILE_ALIAS(*sect->module);
            auto sc = &sect->subclass.type;
            array::init(sc->funcs, sect->module->wasm->alloc);
            u32 size;
            u32 num_types{};
            FILE_READ_ULEB128(u32, num_types, size);
            for (u32 i = 0; i < num_types; ++i) {
                u8 type_group; FILE_READ(u8, type_group);
                sc->group = type_group_t(type_group);
                switch (sc->group) {
                    case type_group_t::function: {
                        auto& func = array::append(sc->funcs);
                        func.index = i;
                        array::init(func.params, sc->funcs.alloc);
                        array::init(func.returns, sc->funcs.alloc);

                        u32 len;
                        FILE_READ_ULEB128(u32, len, size);
                        for (u32 j = 0; j < len; ++j) {
                            u8 type_code;
                            FILE_READ(u8, type_code);
                            array::append(func.params, type::make_valtype(core_type_t(type_code)));
                        }

                        FILE_READ_ULEB128(u32, len, size);
                        for (u32 j = 0; j < len; ++j) {
                            u8 type_code;
                            FILE_READ(u8, type_code);
                            array::append(func.returns, type::make_valtype(core_type_t(type_code)));
                        }
                        break;
                    }
                    case type_group_t::module: {
                        u32 modules_len;
                        FILE_READ_ULEB128(u32, modules_len, size);
                        for (u32 j = 0; j < modules_len; ++j) {
                            str::slice_t module_name;
                            FILE_READ_SLICE(module_name, size);
                            str::slice_t field_name;
                            FILE_READ_SLICE(field_name, size);
                            u8 extern_kind;
                            FILE_READ_ULEB128(u8, extern_kind, size);
                            switch (extern_kind) {
                                default:
                                    break;
                            }
                        }
                        // read module type
                        break;
                    }
                    case type_group_t::instance: {
                        // read instance type
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }

            return status_t::ok;
        }

        status_t read_memory(section_t* sect) {
            FILE_ALIAS(*sect->module);
            auto sc = &sect->subclass.memory;
            array::init(sc->limits, sect->module->wasm->alloc);
            u32 size;
            u32 count; FILE_READ_ULEB128(u32, count, size);
            for (u32 i = 0; i < count; ++i) {
                auto& limits = array::append(sc->limits);
                FILE_READ(u8, limits.flags);
                FILE_READ_ULEB128(u32, limits.initial, size);
                if ((limits.flags & 0x1U) != 0)
                    FILE_READ_ULEB128(u32, limits.maximum, size);
            }
            return status_t::ok;
        }

        status_t read_custom(section_t* sect) {
            FILE_ALIAS(*sect->module);
            u32 size;
            FILE_READ_SLICE(sect->name, size);
            if (sect->name != "name"_ss)
                return status_t::ok;
            auto sc = &sect->subclass.custom;
            array::init(sc->names, sect->module->wasm->alloc);
            u8 name_type; FILE_READ_ULEB128(u8, name_type, size);
            sc->type = name_type_t(name_type);
            FILE_READ_ULEB128(u32, sc->size, size);
            switch (sc->type) {
                case name_type_t::local: {
                    break;
                }
                case name_type_t::module: {
                    auto& name = array::append(sc->names);
                    FILE_READ_SLICE(name.payload, size);
                    break;
                }
                case name_type_t::function: {
                    u32 count; FILE_READ_ULEB128(u32, count, size);
                    array::resize(sc->names, count);
                    for (u32 i = 0; i < count; ++i) {
                        u32 index; FILE_READ_ULEB128(u32, index, size);
                        auto& name = sc->names[index];
                        FILE_READ_SLICE(name.payload, size);
                    }
                    break;
                }
                default: {
                    return status_t::error;
                }
            }
            return status_t::ok;
        }

        status_t read_global(section_t* sect) {
            FILE_ALIAS(*sect->module);
            auto sc = &sect->subclass.global;
            array::init(sc->vars, sect->module->wasm->alloc);
            u32 size;
            u32 count; FILE_READ_ULEB128(u32, count, size);
            for (u32 i = 0; i < count; ++i) {
                auto& var = array::append(sc->vars);
                array::init(var.init, sc->vars.alloc);
                u8 type_code; FILE_READ(u8, type_code);
                var.type.content_type = type::make_valtype(core_type_t(type_code));
                FILE_READ(b8, var.type.mutability);
                auto status = instruction::read_body(*sect->module, var.init);
                if (!OK(status))
                    return status;
            }
            return status_t::ok;
        }

        status_t read_exports(section_t* sect) {
            FILE_ALIAS(*sect->module);
            auto sc = &sect->subclass.export_;
            array::init(sc->entries, sect->module->wasm->alloc);
            u32 size;
            u32 count; FILE_READ_ULEB128(u32, count, size);
            for (u32 i = 0; i < count; ++i) {
                auto& entry = array::append(sc->entries);
                FILE_READ_SLICE(entry.field_name, size);
                u8 kind; FILE_READ(u8, kind);
                entry.kind = external_kind_t(kind);
                FILE_READ_ULEB128(u32, entry.index, size);
            }
            return status_t::ok;
        }

        status_t read_imports(section_t* sect) {
            FILE_ALIAS(*sect->module);
            auto sc = &sect->subclass.import;
            array::init(sc->entries, sect->module->wasm->alloc);
            u32 size;
            u32 num_entries;
            FILE_READ_ULEB128(u32, num_entries, size);
            for (u32 i = 0; i < num_entries; ++i) {
                auto& imp = array::append(sc->entries);
                FILE_READ_SLICE(imp.module_name, size);
                FILE_READ_SLICE(imp.field_name, size);
                u8 ext_kind; FILE_READ(u8, ext_kind);
                imp.kind = external_kind_t(ext_kind);
                switch (imp.kind) {
                    case external_kind_t::table: {
                        auto isc = &imp.subclass.table;
                        u8 elem_type; FILE_READ(u8, elem_type);
                        isc->type = elem_type_t(elem_type);
                        FILE_READ(u8, isc->limits.flags);
                        FILE_READ_ULEB128(u32, isc->limits.initial, size);
                        if ((isc->limits.flags & 0x1U) != 0)
                            FILE_READ_ULEB128(u32, isc->limits.maximum, size);
                        break;
                    }
                    case external_kind_t::global: {
                        auto gsc = &imp.subclass.global;
                        u8 type_code; FILE_READ(u8, type_code);
                        gsc->content_type = type::make_valtype(core_type_t(type_code));
                        FILE_READ(b8, gsc->mutability);
                        break;
                    }
                    case external_kind_t::memory: {
                        auto msc = &imp.subclass.memory;
                        FILE_READ(u8, msc->flags);
                        FILE_READ_ULEB128(u32, msc->initial, size);
                        if ((msc->flags & 0x1U) != 0)
                            FILE_READ_ULEB128(u32, msc->maximum, size);
                        break;
                    }
                    case external_kind_t::function:
                        FILE_READ_ULEB128(u32, imp.subclass.index, size);
                        break;
                }
            }
            return status_t::ok;
        }

        status_t read_elements(section_t* sect) {
            FILE_ALIAS(*sect->module);
            auto sc = &sect->subclass.element;
            array::init(sc->entries, sect->module->wasm->alloc);
            u32 size;
            u32 count; FILE_READ_ULEB128(u32, count, size);
            for (u32 i = 0; i < count; ++i) {
                auto& segment = array::append(sc->entries);
                array::init(segment.funcs, sc->entries.alloc);
                array::init(segment.offset, sc->entries.alloc);
                FILE_READ_ULEB128(u32, segment.tab_index, size);
                auto status = instruction::read_body(*sect->module, segment.offset);
                if (!OK(status))
                    return status;
                u32 num_elem; FILE_READ_ULEB128(u32, num_elem, size);
                for (u32 j = 0; j < num_elem; ++j) {
                    u32 index; FILE_READ_ULEB128(u32, index, size);
                    array::append(segment.funcs, index);
                }
            }
            return status_t::ok;
        }

        status_t read_functions(section_t* sect) {
            FILE_ALIAS(*sect->module);
            auto& sc = sect->subclass.function;
            array::init(sc, sect->module->wasm->alloc);
            u32 size;
            u32 count; FILE_READ_ULEB128(u32, count, size);
            for (u32 i = 0; i < count; ++i) {
                u32 index; FILE_READ_ULEB128(u32, index, size);
                array::append(sc, index);
            }
            return status_t::ok;
        }

        status_t read_data_count(section_t* sect) {
            FILE_ALIAS(*sect->module);
            auto sc = &sect->subclass;
            u32 size;
            FILE_READ_ULEB128(u32, sc->ds_count, size);
            return status_t::ok;
        }
    }

    namespace instruction {
        status_t read_body(module_t& module, instruction_array_t& list) {
            FILE_ALIAS(module);
            u8* data = FILE_PTR();
            u32 block_depth {};
            b8  exit{};
            while (!exit) {
                u32 size{};
                u8 byte_code;   FILE_READ(u8, byte_code);
                u16 prefixed = byte_code;
                ++size;
                switch (op_prefix_t(byte_code)) {
                    case op_prefix_t::misc: {
                        FILE_READ(u8, byte_code);
                        prefixed <<= 8U;
                        prefixed |= byte_code;
                        ++size;
                        break;
                    }
                    case op_prefix_t::simd:
                    case op_prefix_t::threads:
                    case op_prefix_t::reserved:
                        return status_t::unsupported_op_prefix;
                    default:
                        break;
                }
                auto& inst = array::append(list);
                inst.data    = data;
                inst.op_code = op_code_t(prefixed);
                switch (inst.op_code) {
                    case op_code_t::end:
                        if (block_depth > 0)
                            --block_depth;
                        else
                            exit = true;
                        break;
                    case op_code_t::nop:
                    case op_code_t::drop:
                    case op_code_t::else_:
                    case op_code_t::select:
                    case op_code_t::unwind:
                    case op_code_t::i32_eq:
                    case op_code_t::i32_ne:
                    case op_code_t::i64_eq:
                    case op_code_t::i64_ne:
                    case op_code_t::f32_eq:
                    case op_code_t::f32_ne:
                    case op_code_t::f32_lt:
                    case op_code_t::f32_gt:
                    case op_code_t::f32_le:
                    case op_code_t::f32_ge:
                    case op_code_t::f64_eq:
                    case op_code_t::f64_ne:
                    case op_code_t::f64_lt:
                    case op_code_t::f64_gt:
                    case op_code_t::f64_le:
                    case op_code_t::f64_ge:
                    case op_code_t::i32_or:
                    case op_code_t::i64_or:
                    case op_code_t::i64_and:
                    case op_code_t::i64_xor:
                    case op_code_t::i64_shl:
                    case op_code_t::f32_abs:
                    case op_code_t::f32_neg:
                    case op_code_t::f32_add:
                    case op_code_t::f32_sub:
                    case op_code_t::f32_mul:
                    case op_code_t::f32_div:
                    case op_code_t::f32_min:
                    case op_code_t::f32_max:
                    case op_code_t::f64_abs:
                    case op_code_t::f64_neg:
                    case op_code_t::i32_and:
                    case op_code_t::i32_xor:
                    case op_code_t::i32_shl:
                    case op_code_t::i64_clz:
                    case op_code_t::i64_ctz:
                    case op_code_t::i64_add:
                    case op_code_t::i64_sub:
                    case op_code_t::i64_mul:
                    case op_code_t::return_:
                    case op_code_t::is_null:
                    case op_code_t::i64_eqz:
                    case op_code_t::i32_eqz:
                    case op_code_t::i32_clz:
                    case op_code_t::i32_ctz:
                    case op_code_t::i32_add:
                    case op_code_t::i32_sub:
                    case op_code_t::i32_mul:
                    case op_code_t::f64_add:
                    case op_code_t::f64_sub:
                    case op_code_t::f64_mul:
                    case op_code_t::f64_div:
                    case op_code_t::f64_min:
                    case op_code_t::f64_max:
                    case op_code_t::f64_ceil:
                    case op_code_t::i64_rotl:
                    case op_code_t::i64_rotr:
                    case op_code_t::f32_ceil:
                    case op_code_t::f32_sqrt:
                    case op_code_t::i32_lt_s:
                    case op_code_t::i32_lt_u:
                    case op_code_t::i32_gt_s:
                    case op_code_t::i32_gt_u:
                    case op_code_t::i32_le_s:
                    case op_code_t::i32_le_u:
                    case op_code_t::i32_ge_s:
                    case op_code_t::i32_ge_u:
                    case op_code_t::i64_lt_s:
                    case op_code_t::i64_lt_u:
                    case op_code_t::i64_gt_s:
                    case op_code_t::i64_gt_u:
                    case op_code_t::i64_le_s:
                    case op_code_t::i64_le_u:
                    case op_code_t::i64_ge_s:
                    case op_code_t::i64_ge_u:
                    case op_code_t::i32_rotl:
                    case op_code_t::i32_rotr:
                    case op_code_t::f64_sqrt:
                    case op_code_t::i32_div_s:
                    case op_code_t::i32_div_u:
                    case op_code_t::i32_rem_s:
                    case op_code_t::i32_rem_u:
                    case op_code_t::i32_shr_s:
                    case op_code_t::i32_shr_u:
                    case op_code_t::i64_div_s:
                    case op_code_t::i64_div_u:
                    case op_code_t::i64_rem_s:
                    case op_code_t::i64_rem_u:
                    case op_code_t::i64_shr_s:
                    case op_code_t::i64_shr_u:
                    case op_code_t::f32_floor:
                    case op_code_t::f32_trunc:
                    case op_code_t::f64_floor:
                    case op_code_t::f64_trunc:
                    case op_code_t::catch_all:
                    case op_code_t::i32_popcnt:
                    case op_code_t::i64_popcnt:
                    case op_code_t::unreachable:
                    case op_code_t::f32_nearest:
                    case op_code_t::f64_nearest:
                    case op_code_t::f32_copysign:
                    case op_code_t::f64_copysign:
                    case op_code_t::i32_wrap_i64:
                    case op_code_t::i32_extend8_s:
                    case op_code_t::i64_extend8_s:
                    case op_code_t::i32_extend16_s:
                    case op_code_t::i64_extend16_s:
                    case op_code_t::i64_extend32_s:
                    case op_code_t::f32_demote_f64:
                    case op_code_t::f64_promote_f32:
                    case op_code_t::i32_trunc_f32_s:
                    case op_code_t::i32_trunc_f32_u:
                    case op_code_t::i32_trunc_f64_s:
                    case op_code_t::i32_trunc_f64_u:
                    case op_code_t::i64_trunc_f32_s:
                    case op_code_t::i64_trunc_f32_u:
                    case op_code_t::i64_trunc_f64_s:
                    case op_code_t::i64_trunc_f64_u:
                    case op_code_t::i64_extend_i32_s:
                    case op_code_t::i64_extend_i32_u:
                    case op_code_t::f32_convert_i32_s:
                    case op_code_t::f32_convert_i32_u:
                    case op_code_t::f32_convert_i64_s:
                    case op_code_t::f32_convert_i64_u:
                    case op_code_t::f64_convert_i32_s:
                    case op_code_t::f64_convert_i32_u:
                    case op_code_t::f64_convert_i64_s:
                    case op_code_t::f64_convert_i64_u:
                    case op_code_t::i32_reinterpret_f32:
                    case op_code_t::i64_reinterpret_f64:
                    case op_code_t::f32_reinterpret_i32:
                    case op_code_t::f64_reinterpret_i64:
                    case op_code_t::i32_trunc_sat_f32_s:
                    case op_code_t::i32_trunc_sat_f32_u:
                    case op_code_t::i32_trunc_sat_f64_s:
                    case op_code_t::i32_trunc_sat_f64_u:
                    case op_code_t::i64_trunc_sat_f32_s:
                    case op_code_t::i64_trunc_sat_f32_u:
                    case op_code_t::i64_trunc_sat_f64_s:
                    case op_code_t::i64_trunc_sat_f64_u:
                        break;
                    case op_code_t::null: {
                        FILE_READ(u8, inst.subclass.type);
                        ++size;
                        break;
                    }
                    case op_code_t::if_:
                    case op_code_t::try_:
                    case op_code_t::loop:
                    case op_code_t::block: {
                        FILE_READ(u8, inst.subclass.type);
                        ++size;
                        ++block_depth;
                        break;
                    }
                    case op_code_t::rethrow: {
                        u32 c;
                        FILE_READ_ULEB128(u32, inst.subclass.dw, c);
                        size += c;
                        break;
                    }
                    case op_code_t::catch_:
                    case op_code_t::throw_:
                    case op_code_t::ref_func: {
                        u32 c;
                        FILE_READ_ULEB128(u32, inst.subclass.dw, c);
                        size += c;
                        break;
                    }
                    case op_code_t::br:
                    case op_code_t::br_if: {
                        u32 c;
                        FILE_READ_ULEB128(u32, inst.subclass.dw, c);
                        size += c;
                        break;
                    }
                    case op_code_t::typed_select: {
                        auto sc = &inst.subclass.typed_select;
                        u32 c;
                        FILE_READ_ULEB128(u32, sc->results, c);
                        FILE_READ(u8, sc->type);
                        size += c + 1;
                        break;
                    }
                    case op_code_t::br_table: {
                        auto sc = &inst.subclass.br_table;
                        u32 c;
                        u32 count; FILE_READ_ULEB128(u32, count, c);
                        size += c;
                        array::init(sc->targets_table, module.wasm->alloc);
                        for (u32 i = 0; i < count; ++i) {
                            u32 target; FILE_READ_ULEB128(u32, target, c);
                            size += c;
                            array::append(sc->targets_table, target);
                        }
                        FILE_READ_ULEB128(u32, sc->default_target, c);
                        size += c;
                        break;
                    }
                    case op_code_t::call: {
                        u32 c;
                        FILE_READ_ULEB128(u32, inst.subclass.dw, c);
                        size += c;
                        break;
                    }
                    case op_code_t::calli: {
                        u32 c;
                        FILE_READ_ULEB128(u32, inst.subclass.calli_imm.type_idx, c);
                        FILE_READ(u8, inst.subclass.calli_imm.reserved);
                        size += c + 1;
                        break;
                    }
                    case op_code_t::delegate:
                    case op_code_t::return_call: {
                        u32 c;
                        FILE_READ_ULEB128(u32, inst.subclass.dw, c);
                        size += c;
                        break;
                    }
                    case op_code_t::return_calli: {
                        auto sc = &inst.subclass.ret_calli_imm;
                        u32 c;
                        FILE_READ_ULEB128(u32, sc->index, c);       size += c;
                        FILE_READ_ULEB128(u32, sc->table_index, c); size += c;
                        break;
                    }
                    case op_code_t::local_get:
                    case op_code_t::local_set:
                    case op_code_t::local_tee:
                    case op_code_t::table_get:
                    case op_code_t::table_set:
                    case op_code_t::global_get:
                    case op_code_t::global_set: {
                        u32 c;
                        FILE_READ_ULEB128(u32, inst.subclass.dw, c);
                        size += c;
                        break;
                    }
                    case op_code_t::memory_size:
                    case op_code_t::memory_grow: {
                        auto sc = &inst.subclass.alloc_imm;
                        FILE_READ(u8, sc->byte);
                        u32 c;
                        FILE_READ_ULEB128(u32, sc->memory, c);
                        size += c + 1;
                        break;
                    }
                    case op_code_t::i32_load:
                    case op_code_t::i64_load:
                    case op_code_t::f32_load:
                    case op_code_t::f64_load:
                    case op_code_t::i32_store:
                    case op_code_t::i64_store:
                    case op_code_t::f32_store:
                    case op_code_t::f64_store:
                    case op_code_t::i32_store8:
                    case op_code_t::i64_store8:
                    case op_code_t::i32_load8_s:
                    case op_code_t::i32_load8_u:
                    case op_code_t::i64_load8_s:
                    case op_code_t::i64_load8_u:
                    case op_code_t::i32_store16:
                    case op_code_t::i64_store16:
                    case op_code_t::i64_store32:
                    case op_code_t::i32_load16_s:
                    case op_code_t::i32_load16_u:
                    case op_code_t::i64_load16_s:
                    case op_code_t::i64_load16_u:
                    case op_code_t::i64_load32_s:
                    case op_code_t::i64_load32_u: {
                        auto sc = &inst.subclass.mem_imm;
                        u32 c;
                        sc->memory = 0;
                        u32 flags; FILE_READ_ULEB128(u32, flags, c); size += c;
                        FILE_READ_ULEB128(u32, sc->offset, c); size += c;
                        if ((flags & (u32(1) << 6U)) != 0) {
                            flags ^= u32(1) << 6U;
                            FILE_READ_ULEB128(u32, sc->memory, c); size += c;
                        }
                        sc->align = flags;
                        break;
                    }
                    case op_code_t::i32_const: {
                        u32 c;
                        FILE_READ_SLEB128(u32, inst.subclass.dw, c);
                        size += c;
                        break;
                    }
                    case op_code_t::f32_const: {
                        FILE_READ(u32, inst.subclass.dw);
                        size += 4;
                        break;
                    }
                    case op_code_t::i64_const: {
                        u32 c;
                        FILE_READ_SLEB128(u64, inst.subclass.qw, c);
                        size += c;
                        break;
                    }
                    case op_code_t::f64_const: {
                        FILE_READ(u64, inst.subclass.qw);
                        size += 8;
                        break;
                    }
                    case op_code_t::elem_drop: {
                        u32 c;
                        FILE_READ_ULEB128(u32, inst.subclass.dw, c);
                        size += c;
                        break;
                    }
                    case op_code_t::data_drop: {
                        u32 c;
                        FILE_READ_ULEB128(u32, inst.subclass.dw, c);
                        size += c;
                        break;
                    }
                    case op_code_t::table_grow: {
                        u32 c;
                        FILE_READ_ULEB128(u32, inst.subclass.dw, c);
                        size += c;
                        break;
                    }
                    case op_code_t::table_size: {
                        u32 c;
                        FILE_READ_ULEB128(u32, inst.subclass.dw, c);
                        size += c;
                        break;
                    }
                    case op_code_t::table_fill: {
                        u32 c;
                        FILE_READ_ULEB128(u32, inst.subclass.dw, c);
                        size += c;
                        break;
                    }
                    case op_code_t::table_init: {
                        auto sc = &inst.subclass.seg_idx_imm;
                        u32 c;
                        FILE_READ_ULEB128(u32, sc->segment, c); size += c;
                        FILE_READ_ULEB128(u32, sc->index, c); size += c;
                        break;
                    }
                    case op_code_t::table_copy: {
                        auto sc = &inst.subclass.copy_imm;
                        u32 c;
                        FILE_READ_ULEB128(u32, sc->dst, c); size += c;
                        FILE_READ_ULEB128(u32, sc->src, c); size += c;
                        break;
                    }
                    case op_code_t::memory_init: {
                        auto sc = &inst.subclass.seg_idx_imm;
                        u32 c;
                        FILE_READ_ULEB128(u32, sc->segment, c); size += c;
                        FILE_READ_ULEB128(u32, sc->index, c); size += c;
                        break;
                    }
                    case op_code_t::memory_copy: {
                        auto sc = &inst.subclass.copy_imm;
                        u32 c;
                        FILE_READ_ULEB128(u32, sc->dst, c); size += c;
                        FILE_READ_ULEB128(u32, sc->src, c); size += c;
                        break;
                    }
                    case op_code_t::memory_fill: {
                        u32 c;
                        FILE_READ_ULEB128(u32, inst.subclass.dw, c);
                        size += c;
                        break;
                    }
                }
                inst.size = size;
                data += size;
            }
            return status_t::ok;
        }
    }
}
