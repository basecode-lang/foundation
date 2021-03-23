// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
// V I R T U A L  M A C H I N E  P R O J E C T
//
// Copyright (C) 2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <basecode/core/wasm.h>
#include <basecode/core/leb128.h>
#include <basecode/core/string.h>

#define FILE_READ_SLEB128(t, v)             SAFE_SCOPE(                 \
    static_assert(std::is_same_v<decltype(v), t>);                      \
    if (!OK(leb::decode_signed(file.crsr, (v))))                        \
        return status_t::read_error;)
#define FILE_READ_ULEB128(t, v)             SAFE_SCOPE(                 \
    static_assert(std::is_same_v<decltype(v), t>);                      \
    if (!OK(leb::decode_unsigned(file.crsr, (v))))                      \
        return status_t::read_error;)
#define FILE_READ_SLICE(s)                  SAFE_SCOPE(                 \
    static_assert(std::is_same_v<decltype(s), str::slice_t>);           \
    FILE_READ_ULEB128(u32, (s).length);                                 \
    (s).data = FILE_PTR();                                              \
    FILE_SEEK_FWD((s).length);)

namespace basecode::wasm {
    namespace type {
        str::slice_t name(core_type_t type) {
            switch (type) {
                case core_type_t::i32:
                    return "i32"_ss;
                case core_type_t::i64:
                    return "i64"_ss;
                case core_type_t::f32:
                    return "f32"_ss;
                case core_type_t::f64:
                    return "f64"_ss;
                case core_type_t::func_ref:
                    return "func_ref"_ss;
                case core_type_t::extern_ref:
                    return "extern_ref"_ss;
                case core_type_t::exn_ref:
                    return "exn_ref"_ss;
                case core_type_t::func:
                    return "func"_ss;
                case core_type_t::empty_block:
                    return "empty_block"_ss;
                default:
                    return "unknown"_ss;
            }
        }

        valtype_t make_valtype(core_type_t type) {
            switch (type) {
                case core_type_t::i32:
                case core_type_t::i64:
                case core_type_t::f32:
                case core_type_t::f64:
                    return valtype_t{type, valtype_kind_t::num};
                case core_type_t::exn_ref:
                case core_type_t::func_ref:
                case core_type_t::extern_ref:
                    return valtype_t{type, valtype_kind_t::ref};
                default:
                    return valtype_t{type, valtype_kind_t::none};
            }
        }
    }

    namespace section {
        namespace type_group {
            str::slice_t name(type_group_t group) {
                switch (group) {
                    case type_group_t::function:    return "function"_ss;
                    case type_group_t::module:      return "module"_ss;
                    case type_group_t::instance:    return "instance"_ss;
                    default:                        return "unknown"_ss;
                }
            }
        }

        static str::slice_t s_names[] = {
            [u32(section_type_t::custom)]       = "custom"_ss,
            [u32(section_type_t::type)]         = "type"_ss,
            [u32(section_type_t::import)]       = "import"_ss,
            [u32(section_type_t::function)]     = "function"_ss,
            [u32(section_type_t::table)]        = "table"_ss,
            [u32(section_type_t::memory)]       = "memory"_ss,
            [u32(section_type_t::global)]       = "global"_ss,
            [u32(section_type_t::export_)]      = "export"_ss,
            [u32(section_type_t::start)]        = "start"_ss,
            [u32(section_type_t::element)]      = "element"_ss,
            [u32(section_type_t::code)]         = "code"_ss,
            [u32(section_type_t::data)]         = "data"_ss,
            [u32(section_type_t::data_count)]   = "data count"_ss,
        };

        u0 free(section_t& sect) {
            switch (sect.type) {
                case section_type_t::type: {
                    auto sc = &sect.subclass.type;
                    switch (sc->group) {
                        case type_group_t::function:
                            for (auto& func_def : sc->funcs) {
                                array::free(func_def.params);
                                array::free(func_def.returns);
                            }
                            array::free(sc->funcs);
                            break;
                        case type_group_t::module:
                        case type_group_t::instance:
                        default:
                            break;
                    }
                    break;
                }
                case section_type_t::code: {
                    auto sc = &sect.subclass.code;
                    for (auto& body : sc->bodies) {
                        array::free(body.locals);
                        for (auto& inst : body.instructions) {
                            if (inst.op_code == op_code_t::br_table)
                                array::free(inst.subclass.br_table.targets_table);
                        }
                    }
                    array::free(sc->bodies);
                    break;
                }
                case section_type_t::data: {
                    auto sc = &sect.subclass.data;
                    for (auto& segment : sc->segments) {
                        array::free(segment.offset);
                    }
                    array::free(sc->segments);
                    break;
                }
                case section_type_t::table: {
                    auto sc = &sect.subclass.table;
                    array::free(sc->elements);
                    break;
                }
                case section_type_t::custom: {
                    if (sect.name == "name"_ss) {
                        auto sc = &sect.subclass.custom;
                        array::free(sc->names);
                    }
                    break;
                }
                case section_type_t::start:
                case section_type_t::import:
                case section_type_t::memory:
                case section_type_t::global:
                case section_type_t::export_:
                case section_type_t::element:
                case section_type_t::function:
                case section_type_t::data_count:
                default:
                    break;
            }
        }

        str::slice_t name(section_type_t type) {
            return s_names[u32(type)];
        }

        static status_t read_code(section_t* sect) {
            auto& file = *sect->module;
            auto sc = &sect->subclass.code;
            array::init(sc->bodies, sect->module->wasm->alloc);
            u32 count;  FILE_READ_ULEB128(u32, count);
            for (u32 i = 0; i < count; ++i) {
                auto& body = array::append(sc->bodies);
                array::init(body.locals, sc->bodies.alloc);
                array::init(body.instructions, sc->bodies.alloc);
                u32 body_size;      FILE_READ_ULEB128(u32, body_size);
                u32 local_count;    FILE_READ_ULEB128(u32, local_count);
                for (u32 j = 0; j < local_count; ++j) {
                    auto& local = array::append(body.locals);
                    FILE_READ_ULEB128(u32, local.count);
                    u8 type_code; FILE_READ(u8, type_code);
                    local.value_type = type::make_valtype(core_type_t(type_code));
                }
                auto status = instruction::read_body(*sect->module, body.instructions);
                if (!OK(status))
                    return status;
            }
            return status_t::ok;
        }

        static status_t read_data(section_t* sect) {
            auto& file = *sect->module;
            auto sc = &sect->subclass.data;
            array::init(sc->segments, sect->module->wasm->alloc);
            u32 count;  FILE_READ_ULEB128(u32, count);
            for (u32 i = 0; i < count; ++i) {
                auto& segment = array::append(sc->segments);
                FILE_READ_ULEB128(u32, segment.index);
                auto status = instruction::read_body(*sect->module, segment.offset);
                if (!OK(status))
                    return status;
                FILE_READ_ULEB128(u32, segment.size);
                segment.data = FILE_PTR();
                FILE_SEEK_FWD(segment.size);
            }
            return status_t::ok;
        }

        static status_t read_start(section_t* sect) {
            auto& file = *sect->module;
            FILE_READ_ULEB128(u32, sect->subclass.start_func_idx);
            return status_t::ok;
        }

        static status_t read_table(section_t* sect) {
            auto& file = *sect->module;
            auto sc = &sect->subclass.table;
            array::init(sc->elements, sect->module->wasm->alloc);

            u32 num_entries;
            FILE_READ_ULEB128(u32, num_entries);
            for (u32 i = 0; i < num_entries; ++i) {
                auto& elem = array::append(sc->elements);
                u8 elem_type; FILE_READ(u8, elem_type);
                elem.type = elem_type_t(elem_type);
                FILE_READ(u8, elem.limits.flags);
                FILE_READ_ULEB128(u32, elem.limits.initial);
                if ((elem.limits.flags & 0x1U) != 0)
                    FILE_READ_ULEB128(u32, elem.limits.maximum);
            }

            return status_t::ok;
        }

        static status_t read_types(section_t* sect) {
            auto& file = *sect->module;
            auto sc = &sect->subclass.type;
            array::init(sc->funcs, sect->module->wasm->alloc);

            u32 num_types{};
            FILE_READ_ULEB128(u32, num_types);

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
                        FILE_READ_ULEB128(u32, len);
                        for (u32 j = 0; j < len; ++j) {
                            u8 type_code;
                            FILE_READ(u8, type_code);
                            array::append(func.params, type::make_valtype(core_type_t(type_code)));
                        }

                        FILE_READ_ULEB128(u32, len);
                        for (u32 j = 0; j < len; ++j) {
                            u8 type_code;
                            FILE_READ(u8, type_code);
                            array::append(func.returns, type::make_valtype(core_type_t(type_code)));
                        }
                        break;
                    }
                    case type_group_t::module: {
                        u32 modules_len;
                        FILE_READ_ULEB128(u32, modules_len);
                        for (u32 j = 0; j < modules_len; ++j) {
                            str::slice_t module_name;
                            FILE_READ_SLICE(module_name);
                            str::slice_t field_name;
                            FILE_READ_SLICE(field_name);
                            u8 extern_kind;
                            FILE_READ_ULEB128(u8, extern_kind);
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

        static status_t read_memory(section_t* sect) {
            auto& file = *sect->module;
            format::print("section: memory\n");
            FILE_SEEK_FWD(sect->size);
            return status_t::ok;
        }

        static status_t read_custom(section_t* sect) {
            auto& file = *sect->module;
            format::print("section: custom\n");
            FILE_READ_SLICE(sect->name);
            if (sect->name != "name"_ss)
                return status_t::ok;
            auto sc = &sect->subclass.custom;
            array::init(sc->names, sect->module->wasm->alloc);
            u8 name_type; FILE_READ_ULEB128(u8, name_type);
            sc->type = name_type_t(name_type);
            FILE_READ_ULEB128(u32, sc->size);
            switch (sc->type) {
                case name_type_t::local: {
                    break;
                }
                case name_type_t::module: {
                    auto& name = array::append(sc->names);
                    FILE_READ_SLICE(name.payload);
                    break;
                }
                case name_type_t::function: {
                    break;
                }
                default: {
                    return status_t::error;
                }
            }
            return status_t::ok;
        }

        static status_t read_global(section_t* sect) {
            auto& file = *sect->module;
            format::print("section: global\n");
            FILE_SEEK_FWD(sect->size);
            return status_t::ok;
        }

        static status_t read_exports(section_t* sect) {
            auto& file = *sect->module;
            format::print("section: export\n");
            FILE_SEEK_FWD(sect->size);
            return status_t::ok;
        }

        static status_t read_imports(section_t* sect) {
            auto& file = *sect->module;
            auto sc = &sect->subclass.import;
            array::init(sc->entries, sect->module->wasm->alloc);
            u32 num_entries;
            FILE_READ_ULEB128(u32, num_entries);
            for (u32 i = 0; i < num_entries; ++i) {
                auto& imp = array::append(sc->entries);
                FILE_READ_SLICE(imp.module_name);
                FILE_READ_SLICE(imp.field_name);
                u8 ext_kind; FILE_READ(u8, ext_kind);
                imp.kind = external_kind_t(ext_kind);
                switch (imp.kind) {
                    case external_kind_t::table: {
                        auto isc = &imp.subclass.table;
                        u8 elem_type; FILE_READ(u8, elem_type);
                        isc->type = elem_type_t(elem_type);
                        FILE_READ(u8, isc->limits.flags);
                        FILE_READ_ULEB128(u32, isc->limits.initial);
                        if ((isc->limits.flags & 0x1U) != 0)
                            FILE_READ_ULEB128(u32, isc->limits.maximum);
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
                        FILE_READ_ULEB128(u32, msc->initial);
                        if ((msc->flags & 0x1U) != 0)
                            FILE_READ_ULEB128(u32, msc->maximum);
                        break;
                    }
                    case external_kind_t::function:
                        FILE_READ_ULEB128(u32, imp.subclass.index);
                        break;
                }
            }
            return status_t::ok;
        }

        static status_t read_elements(section_t* sect) {
            auto& file = *sect->module;
            format::print("section: element\n");
            FILE_SEEK_FWD(sect->size);
            return status_t::ok;
        }

        static status_t read_functions(section_t* sect) {
            auto& file = *sect->module;
            format::print("section: function\n");
            FILE_SEEK_FWD(sect->size);
            return status_t::ok;
        }

        static status_t read_data_count(section_t* sect) {
            auto& file = *sect->module;
            format::print("section: custom\n");
            FILE_SEEK_FWD(sect->size);
            return status_t::ok;
        }
    }

    namespace module {
        u0 free(module_t& module) {
            for (auto& sect : module.sections)
                section::free(sect);
            buf::cursor::free(module.crsr);
            buf::free(module.buf);
            array::free(module.sections);
        }

        status_t decode(module_t& module) {
            auto& file = module;

            FILE_READ(u32, module.magic);
            FILE_READ(u32, module.version);

            while (CRSR_MORE(module.crsr)) {
                section_t* sect{};
                auto status = make_section(module, &sect);
                if (!OK(status))
                    return status;
            }

            return status_t::ok;
        }

        status_t init(module_t& module, wasm_t* wasm) {
            module.wasm  = wasm;
            module.magic = module.version = {};
            buf::init(module.buf, module.wasm->alloc);
            buf::cursor::init(module.crsr, module.buf);
            array::init(module.sections, module.wasm->alloc);
            return status_t::ok;
        }

        status_t make_section(module_t& module, section_t** section) {
            auto& file = module;
            auto sect = &array::append(module.sections);
            sect->module = &module;

            u8 type_code; FILE_READ_ULEB128(u8, type_code);
            sect->type = section_type_t(type_code);
            FILE_READ_ULEB128(u32, sect->size);
            sect->payload = FILE_PTR();

            auto status = status_t::ok;

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

            *section = sect;
            return status;
        }
    }

    namespace name_type {
        static str::slice_t s_names[] = {
            [u32(name_type_t::module)]                  = "module"_ss,
            [u32(name_type_t::function)]                = "function"_ss,
            [u32(name_type_t::local)]                   = "local"_ss,
        };

        str::slice_t name(name_type_t type) {
            return s_names[u32(type)];
        }
    }

    namespace instruction {
        static str::slice_t s_names[] = {
            [u32(op_code_t::unreachable)]               = "unreachable"_ss,
            [u32(op_code_t::nop)]                       = "nop"_ss,
            [u32(op_code_t::block)]                     = "block"_ss,
            [u32(op_code_t::loop)]                      = "loop"_ss,
            [u32(op_code_t::if_)]                       = "if"_ss,
            [u32(op_code_t::else_)]                     = "else"_ss,
            [u32(op_code_t::try_)]                      = "try"_ss,
            [u32(op_code_t::catch_)]                    = "catch"_ss,
            [u32(op_code_t::throw_)]                    = "throw"_ss,
            [u32(op_code_t::rethrow)]                   = "rethrow"_ss,
            [u32(op_code_t::unwind)]                    = "unwind"_ss,
            [u32(op_code_t::end)]                       = "end"_ss,
            [u32(op_code_t::br)]                        = "br"_ss,
            [u32(op_code_t::br_if)]                     = "br_if"_ss,
            [u32(op_code_t::br_table)]                  = "br_table"_ss,
            [u32(op_code_t::return_)]                   = "return"_ss,
            [u32(op_code_t::call)]                      = "call"_ss,
            [u32(op_code_t::calli)]                     = "call_indirect"_ss,
            [u32(op_code_t::return_call)]               = "return.call"_ss,
            [u32(op_code_t::return_calli)]              = "return.call_indirect"_ss,
            [u32(op_code_t::delegate)]                  = "delegate"_ss,
            [u32(op_code_t::null)]                      = "ref.null"_ss,
            [u32(op_code_t::is_null)]                   = "ref.is_null"_ss,
            [u32(op_code_t::ref_func)]                  = "ref.func"_ss,
            [u32(op_code_t::drop)]                      = "drop"_ss,
            [u32(op_code_t::select)]                    = "select"_ss,
            [u32(op_code_t::typed_select)]              = "typed.select"_ss,
            [u32(op_code_t::local_get)]                 = "local.get"_ss,
            [u32(op_code_t::local_set)]                 = "local.set"_ss,
            [u32(op_code_t::local_tee)]                 = "local.tee"_ss,
            [u32(op_code_t::global_get)]                = "global.get"_ss,
            [u32(op_code_t::global_set)]                = "global.set"_ss,
            [u32(op_code_t::table_get)]                 = "table.get"_ss,
            [u32(op_code_t::table_set)]                 = "table.set"_ss,
            [u32(op_code_t::i32_load)]                  = "i32.load"_ss,
            [u32(op_code_t::i64_load)]                  = "i64.load"_ss,
            [u32(op_code_t::f32_load)]                  = "f32.load"_ss,
            [u32(op_code_t::f64_load)]                  = "f64.load"_ss,
            [u32(op_code_t::i32_load8_s)]               = "i32.load8_s"_ss,
            [u32(op_code_t::i32_load8_u)]               = "i32_load8_u"_ss,
            [u32(op_code_t::i32_load16_s)]              = "i32.load16_s"_ss,
            [u32(op_code_t::i32_load16_u)]              = "i32.load16_u"_ss,
            [u32(op_code_t::i64_load8_s)]               = "i64.load8_s"_ss,
            [u32(op_code_t::i64_load8_u)]               = "i64.load8_u"_ss,
            [u32(op_code_t::i64_load16_s)]              = "i64.load16_s"_ss,
            [u32(op_code_t::i64_load16_u)]              = "i64.load16_u"_ss,
            [u32(op_code_t::i64_load32_s)]              = "i64.load32_s"_ss,
            [u32(op_code_t::i64_load32_u)]              = "i64.load32_u"_ss,
            [u32(op_code_t::i32_store)]                 = "i32.store"_ss,
            [u32(op_code_t::i64_store)]                 = "i64.store"_ss,
            [u32(op_code_t::f32_store)]                 = "f32.store"_ss,
            [u32(op_code_t::f64_store)]                 = "f64.store"_ss,
            [u32(op_code_t::i32_store8)]                = "i32.store8"_ss,
            [u32(op_code_t::i64_store8)]                = "i64.store8"_ss,
            [u32(op_code_t::i32_store16)]               = "i32.store16"_ss,
            [u32(op_code_t::i64_store16)]               = "i64.store16"_ss,
            [u32(op_code_t::i64_store32)]               = "i64.store32"_ss,
            [u32(op_code_t::memory_size)]               = "memory.size"_ss,
            [u32(op_code_t::memory_grow)]               = "memory.grow"_ss,
            [u32(op_code_t::i32_const)]                 = "i32.const"_ss,
            [u32(op_code_t::i64_const)]                 = "i64.const"_ss,
            [u32(op_code_t::f32_const)]                 = "f32.const"_ss,
            [u32(op_code_t::f64_const)]                 = "f64.const"_ss,
            [u32(op_code_t::i32_eqz)]                   = "i32.eqz"_ss,
            [u32(op_code_t::i32_eq)]                    = "i32.eq"_ss,
            [u32(op_code_t::i32_ne)]                    = "i32.ne"_ss,
            [u32(op_code_t::i32_lt_s)]                  = "i32.lt_s"_ss,
            [u32(op_code_t::i32_lt_u)]                  = "i32.lt_u"_ss,
            [u32(op_code_t::i32_gt_s)]                  = "i32.gt_s"_ss,
            [u32(op_code_t::i32_gt_u)]                  = "i32.gt_u"_ss,
            [u32(op_code_t::i32_le_s)]                  = "i32.le_s"_ss,
            [u32(op_code_t::i32_le_u)]                  = "i32.le_u"_ss,
            [u32(op_code_t::i32_ge_s)]                  = "i32.ge_s"_ss,
            [u32(op_code_t::i32_ge_u)]                  = "i32.ge_u"_ss,
            [u32(op_code_t::i64_eqz)]                   = "i64.eqz"_ss,
            [u32(op_code_t::i64_eq)]                    = "i64.eq"_ss,
            [u32(op_code_t::i64_ne)]                    = "i64.ne"_ss,
            [u32(op_code_t::i64_lt_s)]                  = "i64.lt_s"_ss,
            [u32(op_code_t::i64_lt_u)]                  = "i64.lt_u"_ss,
            [u32(op_code_t::i64_gt_s)]                  = "i64.gt_s"_ss,
            [u32(op_code_t::i64_gt_u)]                  = "i64.gt_u"_ss,
            [u32(op_code_t::i64_le_s)]                  = "i64.le_s"_ss,
            [u32(op_code_t::i64_le_u)]                  = "i64.le_u"_ss,
            [u32(op_code_t::i64_ge_s)]                  = "i64.ge_s"_ss,
            [u32(op_code_t::i64_ge_u)]                  = "i64.ge_u"_ss,
            [u32(op_code_t::f32_eq)]                    = "f32.eq"_ss,
            [u32(op_code_t::f32_ne)]                    = "f32.ne"_ss,
            [u32(op_code_t::f32_lt)]                    = "f32.lt"_ss,
            [u32(op_code_t::f32_gt)]                    = "f32.gt"_ss,
            [u32(op_code_t::f32_le)]                    = "f32.le"_ss,
            [u32(op_code_t::f32_ge)]                    = "f32.ge"_ss,
            [u32(op_code_t::f64_eq)]                    = "f64.eq"_ss,
            [u32(op_code_t::f64_ne)]                    = "f64.ne"_ss,
            [u32(op_code_t::f64_lt)]                    = "f64.lt"_ss,
            [u32(op_code_t::f64_gt)]                    = "f64.gt"_ss,
            [u32(op_code_t::f64_le)]                    = "f64.le"_ss,
            [u32(op_code_t::f64_ge)]                    = "f64.ge"_ss,
            [u32(op_code_t::i32_clz)]                   = "i32.clz"_ss,
            [u32(op_code_t::i32_ctz)]                   = "i32.ctz"_ss,
            [u32(op_code_t::i32_popcnt)]                = "i32.popcnt"_ss,
            [u32(op_code_t::i32_add)]                   = "i32.add"_ss,
            [u32(op_code_t::i32_sub)]                   = "i32.sub"_ss,
            [u32(op_code_t::i32_mul)]                   = "i32.mul"_ss,
            [u32(op_code_t::i32_div_s)]                 = "i32.div_s"_ss,
            [u32(op_code_t::i32_div_u)]                 = "i32.div_u"_ss,
            [u32(op_code_t::i32_rem_s)]                 = "i32.rem_s"_ss,
            [u32(op_code_t::i32_rem_u)]                 = "i32.rem_u"_ss,
            [u32(op_code_t::i32_and)]                   = "i32.and"_ss,
            [u32(op_code_t::i32_or)]                    = "i32.or"_ss,
            [u32(op_code_t::i32_xor)]                   = "i32.xor"_ss,
            [u32(op_code_t::i32_shl)]                   = "i32.shl"_ss,
            [u32(op_code_t::i32_shr_s)]                 = "i32.shr_s"_ss,
            [u32(op_code_t::i32_shr_u)]                 = "i32.shr_u"_ss,
            [u32(op_code_t::i32_rotl)]                  = "i32.rotl"_ss,
            [u32(op_code_t::i32_rotr)]                  = "i32.rotr"_ss,
            [u32(op_code_t::i64_clz)]                   = "i64.clz"_ss,
            [u32(op_code_t::i64_ctz)]                   = "i64.ctz"_ss,
            [u32(op_code_t::i64_popcnt)]                = "i64.popcnt"_ss,
            [u32(op_code_t::i64_add)]                   = "i64.add"_ss,
            [u32(op_code_t::i64_sub)]                   = "i64.sub"_ss,
            [u32(op_code_t::i64_mul)]                   = "i64.mul"_ss,
            [u32(op_code_t::i64_div_s)]                 = "i64.div_s"_ss,
            [u32(op_code_t::i64_div_u)]                 = "i64.div_u"_ss,
            [u32(op_code_t::i64_rem_s)]                 = "i64.rem_s"_ss,
            [u32(op_code_t::i64_rem_u)]                 = "i64.rem_u"_ss,
            [u32(op_code_t::i64_and)]                   = "i64.and"_ss,
            [u32(op_code_t::i64_or)]                    = "i64.or"_ss,
            [u32(op_code_t::i64_xor)]                   = "i64.xor"_ss,
            [u32(op_code_t::i64_shl)]                   = "i64.shl"_ss,
            [u32(op_code_t::i64_shr_s)]                 = "i64.shr_s"_ss,
            [u32(op_code_t::i64_shr_u)]                 = "i64.shr_u"_ss,
            [u32(op_code_t::i64_rotl)]                  = "i64.rotl"_ss,
            [u32(op_code_t::i64_rotr)]                  = "i64.rotr"_ss,
            [u32(op_code_t::f32_abs)]                   = "f32.abs"_ss,
            [u32(op_code_t::f32_neg)]                   = "f32.neg"_ss,
            [u32(op_code_t::f32_ceil)]                  = "f32.ceil"_ss,
            [u32(op_code_t::f32_floor)]                 = "f32.floor"_ss,
            [u32(op_code_t::f32_trunc)]                 = "f32.trunc"_ss,
            [u32(op_code_t::f32_nearest)]               = "f32.nearest"_ss,
            [u32(op_code_t::f32_sqrt)]                  = "f32.sqrt"_ss,
            [u32(op_code_t::f32_add)]                   = "f32.add"_ss,
            [u32(op_code_t::f32_sub)]                   = "f32.sub"_ss,
            [u32(op_code_t::f32_mul)]                   = "f32.mul"_ss,
            [u32(op_code_t::f32_div)]                   = "f32.div"_ss,
            [u32(op_code_t::f32_min)]                   = "f32.min"_ss,
            [u32(op_code_t::f32_max)]                   = "f32.max"_ss,
            [u32(op_code_t::f32_copysign)]              = "f32.copysign"_ss,
            [u32(op_code_t::f64_abs)]                   = "f64.abs"_ss,
            [u32(op_code_t::f64_neg)]                   = "f64.neg"_ss,
            [u32(op_code_t::f64_ceil)]                  = "f64.ceil"_ss,
            [u32(op_code_t::f64_floor)]                 = "f64.floor"_ss,
            [u32(op_code_t::f64_trunc)]                 = "f64.trunc"_ss,
            [u32(op_code_t::f64_nearest)]               = "f64.nearest"_ss,
            [u32(op_code_t::f64_sqrt)]                  = "f64.sqrt"_ss,
            [u32(op_code_t::f64_add)]                   = "f64.add"_ss,
            [u32(op_code_t::f64_sub)]                   = "f64.sub"_ss,
            [u32(op_code_t::f64_mul)]                   = "f64.mul"_ss,
            [u32(op_code_t::f64_div)]                   = "f64.div"_ss,
            [u32(op_code_t::f64_min)]                   = "f64.min"_ss,
            [u32(op_code_t::f64_max)]                   = "f64.max"_ss,
            [u32(op_code_t::f64_copysign)]              = "f64.copysign"_ss,
            [u32(op_code_t::i32_wrap_i64)]              = "i32.wrap_i64"_ss,
            [u32(op_code_t::i32_trunc_f32_s)]           = "i32.trunc_f32_s"_ss,
            [u32(op_code_t::i32_trunc_f32_u)]           = "i32.trunc_f32_u"_ss,
            [u32(op_code_t::i32_trunc_f64_s)]           = "i32.trunc_f64_s"_ss,
            [u32(op_code_t::i32_trunc_f64_u)]           = "i32.trunc_f64_u"_ss,
            [u32(op_code_t::i64_extend_i32_s)]          = "i64.extend_i32_s"_ss,
            [u32(op_code_t::i64_extend_i32_u)]          = "i64.extend_i32_u"_ss,
            [u32(op_code_t::i64_trunc_f32_s)]           = "i64.trunc_f32_s"_ss,
            [u32(op_code_t::i64_trunc_f32_u)]           = "i64.trunc_f32_u"_ss,
            [u32(op_code_t::i64_trunc_f64_s)]           = "i64.trunc_f64_s"_ss,
            [u32(op_code_t::i64_trunc_f64_u)]           = "i64.trunc_f64_u"_ss,
            [u32(op_code_t::f32_convert_i32_s)]         = "f32.convert_i32_s"_ss,
            [u32(op_code_t::f32_convert_i32_u)]         = "f32.convert_i32_u"_ss,
            [u32(op_code_t::f32_convert_i64_s)]         = "f32.convert_i64_s"_ss,
            [u32(op_code_t::f32_convert_i64_u)]         = "f32.convert_i64_u"_ss,
            [u32(op_code_t::f32_demote_f64)]            = "f32.demote_f64"_ss,
            [u32(op_code_t::f64_convert_i32_s)]         = "f64.convert_i32_s"_ss,
            [u32(op_code_t::f64_convert_i32_u)]         = "f64.convert_i32_u"_ss,
            [u32(op_code_t::f64_convert_i64_s)]         = "f64.convert_i64_s"_ss,
            [u32(op_code_t::f64_convert_i64_u)]         = "f64.convert_i64_u"_ss,
            [u32(op_code_t::f64_promote_f32)]           = "f64.promote_f32"_ss,
            [u32(op_code_t::i32_reinterpret_f32)]       = "i32.reinterpret_f32"_ss,
            [u32(op_code_t::i64_reinterpret_f64)]       = "i64.reinterpret_f64"_ss,
            [u32(op_code_t::f32_reinterpret_i32)]       = "f32.reinterpret_i32"_ss,
            [u32(op_code_t::f64_reinterpret_i64)]       = "f64.reinterpret_i64"_ss,
            [u32(op_code_t::i32_extend8_s)]             = "i32.extend8_s"_ss,
            [u32(op_code_t::i32_extend16_s)]            = "i32.extend16_s"_ss,
            [u32(op_code_t::i64_extend8_s)]             = "i64.extend8_s"_ss,
            [u32(op_code_t::i64_extend16_s)]            = "i64.extend16_s"_ss,
            [u32(op_code_t::i64_extend32_s)]            = "i64.extend32_s"_ss,
        };

        str::slice_t name(op_code_t op) {
            switch (op) {
                case op_code_t::unreachable ... op_code_t::ref_func:
                    return s_names[u32(op)];
                case op_code_t::memory_init:         return "memory.init"_ss;
                case op_code_t::data_drop:           return "data.drop"_ss;
                case op_code_t::memory_copy:         return "memory.copy"_ss;
                case op_code_t::memory_fill:         return "memory.fill"_ss;
                case op_code_t::table_init:          return "table.init"_ss;
                case op_code_t::elem_drop:           return "elem.drop"_ss;
                case op_code_t::table_copy:          return "table.copy"_ss;
                case op_code_t::table_grow:          return "table.grow"_ss;
                case op_code_t::table_size:          return "table.size"_ss;
                case op_code_t::table_fill:          return "table.fill"_ss;
                case op_code_t::i32_trunc_sat_f32_s: return "i32.trunc_sat_f32_s"_ss;
                case op_code_t::i32_trunc_sat_f32_u: return "i32.trunc_sat_f32_u"_ss;
                case op_code_t::i32_trunc_sat_f64_s: return "i32.trunc_sat_f64_s"_ss;
                case op_code_t::i32_trunc_sat_f64_u: return "i32.trunc_sat_f64_u"_ss;
                case op_code_t::i64_trunc_sat_f32_s: return "i64.trunc_sat_f32_s"_ss;
                case op_code_t::i64_trunc_sat_f32_u: return "i64.trunc_sat_f32_u"_ss;
                case op_code_t::i64_trunc_sat_f64_s: return "i64.trunc_sat_f64_s"_ss;
                case op_code_t::i64_trunc_sat_f64_u: return "i64.trunc_sat_f64_u"_ss;
            }
        }

        status_t read_body(module_t& module, instruction_array_t& list) {
            auto& file = module;
            b8 exit{};
            while (!exit) {
                u8 byte_code;   FILE_READ(u8, byte_code);
                u16 prefixed = byte_code;
                switch (byte_code) {
                    case 0xff:  // reserved
                        break;
                    case 0xfe:  // threads
                        break;
                    case 0xfd:  // SIMD V128
                        break;
                    case 0xfc: {  // misc
                        FILE_READ(u8, byte_code);
                        prefixed <<= 8U;
                        prefixed |= byte_code;
                        break;
                    }
                    default: {
                        break;
                    }
                }
                auto& inst = array::append(list);
                inst.op_code = op_code_t(prefixed);
                format::print("inst.op_code = {}\n", instruction::name(inst.op_code));
                switch (inst.op_code) {
                    case op_code_t::end:
                        exit = true;
                        break;
                    case op_code_t::nop:
                    case op_code_t::null:
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
                    case op_code_t::if_:
                    case op_code_t::try_:
                    case op_code_t::loop:
                    case op_code_t::block: {
                        FILE_READ(u8, inst.subclass.sig_imm.block_type);
                        break;
                    }
                    case op_code_t::catch_:
                    case op_code_t::throw_:
                    case op_code_t::rethrow:
                    case op_code_t::ref_func: {
                        FILE_READ_ULEB128(u32, inst.subclass.idx_imm.idx);
                        break;
                    }
                    case op_code_t::br:
                    case op_code_t::br_if: {
                        FILE_READ_ULEB128(u32, inst.subclass.br_imm.rel_depth);
                        break;
                    }
                    case op_code_t::typed_select: {
                        // XXX:
                        FILE_READ(u8, inst.subclass.sig_imm.block_type);
                        break;
                    }
                    case op_code_t::br_table: {
                        auto sc = &inst.subclass.br_table;
                        u32 count; FILE_READ_ULEB128(u32, count);
                        array::init(sc->targets_table, module.wasm->alloc);
                        for (u32 i = 0; i < count; ++i) {
                            u32 target; FILE_READ_ULEB128(u32, target);
                            array::append(sc->targets_table, target);
                        }
                        FILE_READ_ULEB128(u32, sc->default_target);
                        break;
                    }
                    case op_code_t::call: {
                        FILE_READ_ULEB128(u32, inst.subclass.idx_imm.idx);
                        break;
                    }
                    case op_code_t::calli: {
                        FILE_READ_ULEB128(u32, inst.subclass.calli_imm.type_idx);
                        FILE_READ(u8, inst.subclass.calli_imm.reserved);
                        break;
                    }
                    case op_code_t::delegate:
                    case op_code_t::return_call:
                        FILE_READ_ULEB128(u32, inst.subclass.idx_imm.idx);
                        break;
                    case op_code_t::return_calli:
                        break;
                    case op_code_t::local_get:
                    case op_code_t::local_set:
                    case op_code_t::local_tee:
                    case op_code_t::table_get:
                    case op_code_t::table_set:
                    case op_code_t::global_get:
                    case op_code_t::global_set: {
                        FILE_READ_ULEB128(u32, inst.subclass.idx_imm.idx);
                        break;
                    }
                    case op_code_t::memory_size:
                    case op_code_t::memory_grow: {
                        u8 discard; FILE_READ(u8, discard);
                        FILE_READ_ULEB128(u32, inst.subclass.mem_imm.offset);
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
                        FILE_READ_ULEB128(u32, inst.subclass.mem_imm.flags);
                        FILE_READ_ULEB128(u32, inst.subclass.mem_imm.offset);
                        break;
                    }
                    case op_code_t::i32_const: {
                        FILE_READ_ULEB128(u32, inst.subclass.i32_f32_imm.value);
                        break;
                    }
                    case op_code_t::f32_const: {
                        FILE_READ(u32, inst.subclass.i32_f32_imm.value);
                        break;
                    }
                    case op_code_t::i64_const: {
                        FILE_READ_ULEB128(u64, inst.subclass.i64_f64_imm.value);
                        break;
                    }
                    case op_code_t::f64_const: {
                        FILE_READ(u64, inst.subclass.i64_f64_imm.value);
                        break;
                    }
                    case op_code_t::table_grow:
                        break;
                    case op_code_t::table_size:
                        break;
                    case op_code_t::table_fill:
                        break;
                    case op_code_t::elem_drop:
                        break;
                    case op_code_t::table_init:
                        break;
                    case op_code_t::table_copy:
                        break;
                    case op_code_t::data_drop:
                        break;
                    case op_code_t::memory_init:
                        break;
                    case op_code_t::memory_copy:
                        break;
                    case op_code_t::memory_fill:
                        break;
                }
            }
            format::print("\n");
            return status_t::ok;
        }
    }

    namespace external_kind {
        static str::slice_t s_names[] = {
            [u32(external_kind_t::function)]            = "function"_ss,
            [u32(external_kind_t::table)]               = "table"_ss,
            [u32(external_kind_t::memory)]              = "memory"_ss,
            [u32(external_kind_t::global)]              = "global"_ss,
        };

        str::slice_t name(external_kind_t kind) {
            return s_names[u32(kind)];
        }
    }

    u0 free(wasm_t& wasm) {
        for (const auto& pair : wasm.modules)
            module::free(const_cast<module_t&>(pair.value));
        hashtab::free(wasm.modules);
    }

    status_t init(wasm_t& wasm, alloc_t* alloc) {
        wasm.alloc = alloc;
        hashtab::init(wasm.modules, wasm.alloc);
        return status_t::error;
    }

    module_t* load_module(wasm_t& wasm, const path_t& path) {
        auto slice = string::interned::fold(path.str);
        auto mod   = hashtab::emplace(wasm.modules, slice);
        mod->path = slice;
        if (!OK(module::init(*mod, &wasm)))
            return nullptr;
        if (!OK(buf::map_existing(mod->buf, path)))
            return nullptr;
        return mod;
    }

    module_t* load_module(wasm_t& wasm, str::slice_t name, const u8* data, u32 size) {
        auto slice = string::interned::fold(name);
        auto mod   = hashtab::emplace(wasm.modules, slice);
        mod->path = slice;
        if (!OK(module::init(*mod, &wasm)))
            return nullptr;
        if (!OK(buf::load(mod->buf, data, size)))
            return nullptr;
        return mod;
    }
}
