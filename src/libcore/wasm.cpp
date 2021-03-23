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

#define FILE_READ_SLEB128(t, v)             SAFE_SCOPE(     \
    static_assert(std::is_same_v<decltype(v), t>);          \
    if (!OK(leb::decode_signed(file.crsr, (v))))            \
        return status_t::read_error;)
#define FILE_READ_ULEB128(t, v)             SAFE_SCOPE(     \
    static_assert(std::is_same_v<decltype(v), t>);          \
    if (!OK(leb::decode_unsigned(file.crsr, (v))))          \
        return status_t::read_error;)
#define FILE_READ_PSTR(s)       SAFE_SCOPE(                     \
    u32 l;                                                      \
    FILE_READ_ULEB128(u32, l);                                  \
    str::resize((s), l);                                        \
    if (!OK(buf::cursor::read_str(file.crsr, (s).data, (l))))   \
        return status_t::read_error;)

namespace basecode::wasm {
    namespace types {
        str::slice_t name(u8 type) {
            switch (type) {
                case i32:
                    return "i32"_ss;
                case i64:
                    return "i64"_ss;
                case f32:
                    return "f32"_ss;
                case f64:
                    return "f64"_ss;
                case func_ref:
                    return "func_ref"_ss;
                case extern_ref:
                    return "extern_ref"_ss;
                case exn_ref:
                    return "exn_ref"_ss;
                case func:
                    return "func"_ss;
                case empty_block:
                    return "empty_block"_ss;
                default:
                    return "unknown"_ss;
            }
        }
    }

    namespace sections {
        static str::slice_t s_names[] = {
            [u32(custom)]                 = "custom"_ss,
            [u32(type)]                   = "type"_ss,
            [u32(import)]                 = "import"_ss,
            [u32(function)]               = "function"_ss,
            [u32(table)]                  = "table"_ss,
            [u32(memory)]                 = "memory"_ss,
            [u32(global)]                 = "global"_ss,
            [u32(export_)]                = "export"_ss,
            [u32(start)]                  = "start"_ss,
            [u32(element)]                = "element"_ss,
            [u32(code)]                   = "code"_ss,
            [u32(data)]                   = "data"_ss,
            [u32(data_count)]             = "data count"_ss,
        };

        str::slice_t name(u8 type) {
            return s_names[u32(type)];
        }
    }

    namespace instructions {
        static str::slice_t s_names[] = {
            [u32(unreachable)]            = "unreachable"_ss,
            [u32(nop)]                    = "nop"_ss,
            [u32(block)]                  = "block"_ss,
            [u32(loop)]                   = "loop"_ss,
            [u32(if_)]                    = "if"_ss,
            [u32(else_)]                  = "else"_ss,
            [u32(try_)]                   = "try"_ss,
            [u32(catch_)]                 = "catch"_ss,
            [u32(throw_)]                 = "throw"_ss,
            [u32(rethrow)]                = "rethrow"_ss,
            [u32(unwind)]                 = "unwind"_ss,
            [u32(end)]                    = "end"_ss,
            [u32(br)]                     = "br"_ss,
            [u32(br_if)]                  = "br_if"_ss,
            [u32(br_table)]               = "br_table"_ss,
            [u32(return_)]                = "return"_ss,
            [u32(call)]                   = "call"_ss,
            [u32(calli)]                  = "call_indirect"_ss,
            [u32(return_call)]            = "return.call"_ss,
            [u32(return_calli)]           = "return.call_indirect"_ss,
            [u32(delegate)]               = "delegate"_ss,
            [u32(null)]                   = "ref.null"_ss,
            [u32(is_null)]                = "ref.is_null"_ss,
            [u32(ref_func)]               = "ref.func"_ss,
            [u32(drop)]                   = "drop"_ss,
            [u32(select)]                 = "select"_ss,
            [u32(typed_select)]           = "typed.select"_ss,
            [u32(local_get)]              = "local.get"_ss,
            [u32(local_set)]              = "local.set"_ss,
            [u32(local_tee)]              = "local.tee"_ss,
            [u32(global_get)]             = "global.get"_ss,
            [u32(global_set)]             = "global.set"_ss,
            [u32(table_get)]              = "table.get"_ss,
            [u32(table_set)]              = "table.set"_ss,
            [u32(table_init)]             = "table.init"_ss,
            [u32(elem_drop)]              = "elem.drop"_ss,
            [u32(table_copy)]             = "table.copy"_ss,
            [u32(table_grow)]             = "table.grow"_ss,
            [u32(table_size)]             = "table.size"_ss,
            [u32(table_fill)]             = "table.fill"_ss,
            [u32(i32_load)]               = "i32.load"_ss,
            [u32(i64_load)]               = "i64.load"_ss,
            [u32(f32_load)]               = "f32.load"_ss,
            [u32(f64_load)]               = "f64.load"_ss,
            [u32(i32_load8_s)]            = "i32.load8_s"_ss,
            [u32(i32_load8_u)]            = "i32_load8_u"_ss,
            [u32(i32_load16_s)]           = "i32.load16_s"_ss,
            [u32(i32_load16_u)]           = "i32.load16_u"_ss,
            [u32(i64_load8_s)]            = "i64.load8_s"_ss,
            [u32(i64_load8_u)]            = "i64.load8_u"_ss,
            [u32(i64_load16_s)]           = "i64.load16_s"_ss,
            [u32(i64_load16_u)]           = "i64.load16_u"_ss,
            [u32(i64_load32_s)]           = "i64.load32_s"_ss,
            [u32(i64_load32_u)]           = "i64.load32_u"_ss,
            [u32(i32_store)]              = "i32.store"_ss,
            [u32(i64_store)]              = "i64.store"_ss,
            [u32(f32_store)]              = "f32.store"_ss,
            [u32(f64_store)]              = "f64.store"_ss,
            [u32(i32_store8)]             = "i32.store8"_ss,
            [u32(i64_store8)]             = "i64.store8"_ss,
            [u32(i32_store16)]            = "i32.store16"_ss,
            [u32(i64_store16)]            = "i64.store16"_ss,
            [u32(i64_store32)]            = "i64.store32"_ss,
            [u32(memory_size)]            = "memory.size"_ss,
            [u32(memory_grow)]            = "memory.grow"_ss,
            [u32(memory_init)]            = "memory.init"_ss,
            [u32(data_drop)]              = "data.drop"_ss,
            [u32(memory_copy)]            = "memory.copy"_ss,
            [u32(memory_fill)]            = "memory.fill"_ss,
            [u32(i32_const)]              = "i32.const"_ss,
            [u32(i64_const)]              = "i64.const"_ss,
            [u32(f32_const)]              = "f32.const"_ss,
            [u32(f64_const)]              = "f64.const"_ss,
            [u32(i32_eqz)]                = "i32.eqz"_ss,
            [u32(i32_eq)]                 = "i32.eq"_ss,
            [u32(i32_ne)]                 = "i32.ne"_ss,
            [u32(i32_lt_s)]               = "i32.lt_s"_ss,
            [u32(i32_lt_u)]               = "i32.lt_u"_ss,
            [u32(i32_gt_s)]               = "i32.gt_s"_ss,
            [u32(i32_gt_u)]               = "i32.gt_u"_ss,
            [u32(i32_le_s)]               = "i32.le_s"_ss,
            [u32(i32_le_u)]               = "i32.le_u"_ss,
            [u32(i32_ge_s)]               = "i32.ge_s"_ss,
            [u32(i32_ge_u)]               = "i32.ge_u"_ss,
            [u32(i64_eqz)]                = "i64.eqz"_ss,
            [u32(i64_eq)]                 = "i64.eq"_ss,
            [u32(i64_ne)]                 = "i64.ne"_ss,
            [u32(i64_lt_s)]               = "i64.lt_s"_ss,
            [u32(i64_lt_u)]               = "i64.lt_u"_ss,
            [u32(i64_gt_s)]               = "i64.gt_s"_ss,
            [u32(i64_gt_u)]               = "i64.gt_u"_ss,
            [u32(i64_le_s)]               = "i64.le_s"_ss,
            [u32(i64_le_u)]               = "i64.le_u"_ss,
            [u32(i64_ge_s)]               = "i64.ge_s"_ss,
            [u32(i64_ge_u)]               = "i64.ge_u"_ss,
            [u32(f32_eq)]                 = "f32.eq"_ss,
            [u32(f32_ne)]                 = "f32.ne"_ss,
            [u32(f32_lt)]                 = "f32.lt"_ss,
            [u32(f32_gt)]                 = "f32.gt"_ss,
            [u32(f32_le)]                 = "f32.le"_ss,
            [u32(f32_ge)]                 = "f32.ge"_ss,
            [u32(f64_eq)]                 = "f64.eq"_ss,
            [u32(f64_ne)]                 = "f64.ne"_ss,
            [u32(f64_lt)]                 = "f64.lt"_ss,
            [u32(f64_gt)]                 = "f64.gt"_ss,
            [u32(f64_le)]                 = "f64.le"_ss,
            [u32(f64_ge)]                 = "f64.ge"_ss,
            [u32(i32_clz)]                = "i32.clz"_ss,
            [u32(i32_ctz)]                = "i32.ctz"_ss,
            [u32(i32_popcnt)]             = "i32.popcnt"_ss,
            [u32(i32_add)]                = "i32.add"_ss,
            [u32(i32_sub)]                = "i32.sub"_ss,
            [u32(i32_mul)]                = "i32.mul"_ss,
            [u32(i32_div_s)]              = "i32.div_s"_ss,
            [u32(i32_div_u)]              = "i32.div_u"_ss,
            [u32(i32_rem_s)]              = "i32.rem_s"_ss,
            [u32(i32_rem_u)]              = "i32.rem_u"_ss,
            [u32(i32_and)]                = "i32.and"_ss,
            [u32(i32_or)]                 = "i32.or"_ss,
            [u32(i32_xor)]                = "i32.xor"_ss,
            [u32(i32_shl)]                = "i32.shl"_ss,
            [u32(i32_shr_s)]              = "i32.shr_s"_ss,
            [u32(i32_shr_u)]              = "i32.shr_u"_ss,
            [u32(i32_rotl)]               = "i32.rotl"_ss,
            [u32(i32_rotr)]               = "i32.rotr"_ss,
            [u32(i64_clz)]                = "i64.clz"_ss,
            [u32(i64_ctz)]                = "i64.ctz"_ss,
            [u32(i64_popcnt)]             = "i64.popcnt"_ss,
            [u32(i64_add)]                = "i64.add"_ss,
            [u32(i64_sub)]                = "i64.sub"_ss,
            [u32(i64_mul)]                = "i64.mul"_ss,
            [u32(i64_div_s)]              = "i64.div_s"_ss,
            [u32(i64_div_u)]              = "i64.div_u"_ss,
            [u32(i64_rem_s)]              = "i64.rem_s"_ss,
            [u32(i64_rem_u)]              = "i64.rem_u"_ss,
            [u32(i64_and)]                = "i64.and"_ss,
            [u32(i64_or)]                 = "i64.or"_ss,
            [u32(i64_xor)]                = "i64.xor"_ss,
            [u32(i64_shl)]                = "i64.shl"_ss,
            [u32(i64_shr_s)]              = "i64.shr_s"_ss,
            [u32(i64_shr_u)]              = "i64.shr_u"_ss,
            [u32(i64_rotl)]               = "i64.rotl"_ss,
            [u32(i64_rotr)]               = "i64.rotr"_ss,
            [u32(f32_abs)]                = "f32.abs"_ss,
            [u32(f32_neg)]                = "f32.neg"_ss,
            [u32(f32_ceil)]               = "f32.ceil"_ss,
            [u32(f32_floor)]              = "f32.floor"_ss,
            [u32(f32_trunc)]              = "f32.trunc"_ss,
            [u32(f32_nearest)]            = "f32.nearest"_ss,
            [u32(f32_sqrt)]               = "f32.sqrt"_ss,
            [u32(f32_add)]                = "f32.add"_ss,
            [u32(f32_sub)]                = "f32.sub"_ss,
            [u32(f32_mul)]                = "f32.mul"_ss,
            [u32(f32_div)]                = "f32.div"_ss,
            [u32(f32_min)]                = "f32.min"_ss,
            [u32(f32_max)]                = "f32.max"_ss,
            [u32(f32_copysign)]           = "f32.copysign"_ss,
            [u32(f64_abs)]                = "f64.abs"_ss,
            [u32(f64_neg)]                = "f64.neg"_ss,
            [u32(f64_ceil)]               = "f64.ceil"_ss,
            [u32(f64_floor)]              = "f64.floor"_ss,
            [u32(f64_trunc)]              = "f64.trunc"_ss,
            [u32(f64_nearest)]            = "f64.nearest"_ss,
            [u32(f64_sqrt)]               = "f64.sqrt"_ss,
            [u32(f64_add)]                = "f64.add"_ss,
            [u32(f64_sub)]                = "f64.sub"_ss,
            [u32(f64_mul)]                = "f64.mul"_ss,
            [u32(f64_div)]                = "f64.div"_ss,
            [u32(f64_min)]                = "f64.min"_ss,
            [u32(f64_max)]                = "f64.max"_ss,
            [u32(f64_copysign)]           = "f64.copysign"_ss,
            [u32(i32_wrap_i64)]           = "i32.wrap_i64"_ss,
            [u32(i32_trunc_f32_s)]        = "i32.trunc_f32_s"_ss,
            [u32(i32_trunc_f32_u)]        = "i32.trunc_f32_u"_ss,
            [u32(i32_trunc_f64_s)]        = "i32.trunc_f64_s"_ss,
            [u32(i32_trunc_f64_u)]        = "i32.trunc_f64_u"_ss,
            [u32(i64_extend_i32_s)]       = "i64.extend_i32_s"_ss,
            [u32(i64_extend_i32_u)]       = "i64.extend_i32_u"_ss,
            [u32(i64_trunc_f32_s)]        = "i64.trunc_f32_s"_ss,
            [u32(i64_trunc_f32_u)]        = "i64.trunc_f32_u"_ss,
            [u32(i64_trunc_f64_s)]        = "i64.trunc_f64_s"_ss,
            [u32(i64_trunc_f64_u)]        = "i64.trunc_f64_u"_ss,
            [u32(f32_convert_i32_s)]      = "f32.convert_i32_s"_ss,
            [u32(f32_convert_i32_u)]      = "f32.convert_i32_u"_ss,
            [u32(f32_convert_i64_s)]      = "f32.convert_i64_s"_ss,
            [u32(f32_convert_i64_u)]      = "f32.convert_i64_u"_ss,
            [u32(f32_demote_f64)]         = "f32.demote_f64"_ss,
            [u32(f64_convert_i32_s)]      = "f64.convert_i32_s"_ss,
            [u32(f64_convert_i32_u)]      = "f64.convert_i32_u"_ss,
            [u32(f64_convert_i64_s)]      = "f64.convert_i64_s"_ss,
            [u32(f64_convert_i64_u)]      = "f64.convert_i64_u"_ss,
            [u32(f64_promote_f32)]        = "f64.promote_f32"_ss,
            [u32(i32_reinterpret_f32)]    = "i32.reinterpret_f32"_ss,
            [u32(i64_reinterpret_f64)]    = "i64.reinterpret_f64"_ss,
            [u32(f32_reinterpret_i32)]    = "f32.reinterpret_i32"_ss,
            [u32(f64_reinterpret_i64)]    = "f64.reinterpret_i64"_ss,
            [u32(i32_extend8_s)]          = "i32.extend8_s"_ss,
            [u32(i32_extend16_s)]         = "i32.extend16_s"_ss,
            [u32(i64_extend8_s)]          = "i64.extend8_s"_ss,
            [u32(i64_extend16_s)]         = "i64.extend16_s"_ss,
            [u32(i64_extend32_s)]         = "i64.extend32_s"_ss,
            [u32(i32_trunc_sat_f32_s)]    = "i32.trunc_sat_f32_s"_ss,
            [u32(i32_trunc_sat_f32_u)]    = "i32.trunc_sat_f32_u"_ss,
            [u32(i32_trunc_sat_f64_s)]    = "i32.trunc_sat_f64_s"_ss,
            [u32(i32_trunc_sat_f64_u)]    = "i32.trunc_sat_f64_u"_ss,
            [u32(i64_trunc_sat_f32_s)]    = "i64.trunc_sat_f32_s"_ss,
            [u32(i64_trunc_sat_f32_u)]    = "i64.trunc_sat_f32_u"_ss,
            [u32(i64_trunc_sat_f64_s)]    = "i64.trunc_sat_f64_s"_ss,
            [u32(i64_trunc_sat_f64_u)]    = "i64.trunc_sat_f64_u"_ss,
        };

        str::slice_t name(u16 op) {
            return s_names[u32(op)];
        }
    }

    namespace module {
        u0 free(module_t& module) {
            buf::cursor::free(module.crsr);
            buf::free(module.buf);
            array::free(module.sections);
        }

        status_t decode(module_t& module) {
            auto& file = module;

            FILE_READ(u32, module.magic);
            FILE_READ(u32, module.version);

            b8 ok = true;

            while (ok && CRSR_MORE(module.crsr)) {
                u32 size{};
                s8 id{};

                FILE_READ_ULEB128(s8, id);
                FILE_READ_ULEB128(u32, size);

                switch (id) {
                    case sections::type: {
                        format::print("section: type\n");

                        u32 num_types{};
                        FILE_READ_ULEB128(u32, num_types);

                        for (u32 i = 0; i < num_types; ++i) {
                            u8 type_code;
                            FILE_READ(u8, type_code);

                            switch (type_code) {
                                case 0x60: {
                                    u32 params_len{};
                                    u32 returns_len{};
                                    FILE_READ_ULEB128(u32, params_len);

                                    format::print("func@{}(", i);
                                    for (u32 j = 0; j < params_len; ++j) {
                                        u8 code;
                                        FILE_READ(u8, code);
                                        if (j > 0 && j < params_len) format::print(", ");
                                        format::print("{}", types::name(code));
                                    }

                                    FILE_READ_ULEB128(u32, returns_len);
                                    if (returns_len > 0) {
                                        format::print(") : ");
                                        for (u32 j = 0; j < returns_len; ++j) {
                                            u8 code;
                                            FILE_READ(u8, code);
                                            if (j > 0) format::print(", ");
                                            format::print("{}", types::name(code));
                                        }
                                    } else {
                                        format::print(")");
                                    }

                                    format::print("\n");
                                    break;
                                }
                                case 0x61: {
                                    u32 modules_len{};
                                    FILE_READ_ULEB128(u32, modules_len);
                                    for (u32 j = 0; j < modules_len; ++j) {
                                        str::slice_t field_name{};
                                        str::slice_t module_name{};
                                        {
                                            u32 len{};
                                            FILE_READ_ULEB128(u32, len);

                                            s8 buf[len];
                                            FILE_READ_STR(buf, len);

                                            module_name = string::interned::fold(buf, len);
                                        }
                                        {
                                            u32 len{};
                                            FILE_READ_ULEB128(u32, len);

                                            s8 buf[len];
                                            FILE_READ_STR(buf, len);

                                            field_name = string::interned::fold(buf, len);
                                        }
                                        u8 extern_kind{};
                                        FILE_READ_ULEB128(u8, extern_kind);
                                        switch (extern_kind) {
                                            default:
                                                break;
                                        }
                                    }
                                    // read module type
                                    break;
                                }
                                case 0x62: {
                                    // read instance type
                                    break;
                                }
                                default: {
                                    break;
                                }
                            }
                        }
                        break;
                    }
                    case sections::code: {
                        format::print("section: code\n");
                        FILE_SEEK_FWD(size);
                        break;
                    }
                    case sections::data: {
                        format::print("section: data\n");
                        FILE_SEEK_FWD(size);
                        break;
                    }
                    case sections::table: {
                        format::print("section: table\n");
                        u32 num_entries{};
                        FILE_READ_ULEB128(u32, num_entries);
                        for (u32 i = 0; i < num_entries; ++i) {
                            u8 element_type{};
                            u32 flags{};
                            u32 min{};
                            u32 max{};

                            FILE_READ(u8, element_type);
                            FILE_READ_ULEB128(u32, flags);
                            FILE_READ_ULEB128(u32, min);
                            if ((flags & 0x1U) != 0)
                                FILE_READ_ULEB128(u32, max);
                            format::print(
                                "\telement_type = {}, flags = {}, min = {}, max = {}\n",
                                types::name(element_type),
                                flags,
                                min,
                                max);
                        }
                        break;
                    }
                    case sections::start: {
                        format::print("section: start\n");
                        FILE_SEEK_FWD(size);
                        break;
                    }
                    case sections::import: {
                        format::print("section: import\n");
                        FILE_SEEK_FWD(size);
                        break;
                    }
                    case sections::memory: {
                        format::print("section: memory\n");
                        FILE_SEEK_FWD(size);
                        break;
                    }
                    case sections::global: {
                        format::print("section: global\n");
                        FILE_SEEK_FWD(size);
                        break;
                    }
                    case sections::custom: {
                        format::print("section: custom\n");
                        FILE_SEEK_FWD(size);
                        break;
                    }
                    case sections::export_: {
                        format::print("section: export\n");
                        FILE_SEEK_FWD(size);
                        break;
                    }
                    case sections::element: {
                        format::print("section: element\n");
                        FILE_SEEK_FWD(size);
                        break;
                    }
                    case sections::function: {
                        format::print("section: function\n");
                        FILE_SEEK_FWD(size);
                        break;
                    }
                    case sections::data_count: {
                        format::print("section: data count\n");
                        FILE_SEEK_FWD(size);
                        break;
                    }
                    default: {
                        format::print("section: unknown\n");
                        ok = false;
                        break;
                    }
                }
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
