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

#include <basecode/core/wasm/formatter.h>

namespace basecode::wasm {
    namespace instruction {
        u0 format_body(const func_body_t& body, u32 addr, u32 idx) {
            format::print(
                "{:06} func[{}]  <{}>:\n",
                addr,
                idx,
                "__unknown__");
            u32 stack_slot{};
            for (const auto& local : body.locals) {
                format::print(
                    " {:06}: {:02x} {:02x}                           | local",
                    addr,
                    local.count,
                    local.value_type.code);
                if (local.count > 1) {
                    format::print(
                        "[{}..{}]",
                        stack_slot,
                        (stack_slot + local.count) - 1);
                } else {
                    format::print("[{}]", stack_slot);
                }
                format::print(" type={}\n", type::name(local.value_type.code));
                stack_slot += local.count;
                addr += 2;
            }
            u32 block_depth{};
            for (const auto& inst : body.instructions) {
                format(inst, addr, block_depth);
                addr += inst.size;
            }
        }

        u0 format(const instruction_t& inst, u32 addr, u32& block_depth) {
            if (inst.op_code == op_code_t::end) {
                if (block_depth > 0)
                    --block_depth;
            }
            format::print(" {:06x}: ", addr);
            u32 len{};
            for (u32 i = 0; i < inst.size; ++i) {
                if (i > 0) {
                    if ((i % 9) == 0) {
                        format::print("{:<{}}|\n", " ", 32 - len);
                        format::print(" {:06x}: ", addr);
                        len = {};
                    } else {
                        format::print(" ");
                        ++len;
                    }
                }
                format::print("{:02x}", inst.data[i]);
                len += 2;
            }
            format::print("{:<{}}", " ", 32 - len);
            format::print(
                "|{:<{}}{} ",
                " ",
                block_depth * 2, name(inst.op_code));

            switch (inst.op_code) {
                case op_code_t::null:
                    format::print("{}", inst.subclass.type);
                    break;
                case op_code_t::if_:
                case op_code_t::try_:
                case op_code_t::loop:
                case op_code_t::block: {
                    if (inst.subclass.type != 0x40)
                        format::print("{}", inst.subclass.type);
                    ++block_depth;
                    break;
                }
                case op_code_t::br:
                case op_code_t::call:
                case op_code_t::br_if:
                case op_code_t::catch_:
                case op_code_t::throw_:
                case op_code_t::rethrow:
                case op_code_t::ref_func:
                case op_code_t::delegate:
                case op_code_t::local_get:
                case op_code_t::local_set:
                case op_code_t::local_tee:
                case op_code_t::table_get:
                case op_code_t::table_set:
                case op_code_t::elem_drop:
                case op_code_t::data_drop:
                case op_code_t::global_get:
                case op_code_t::global_set:
                case op_code_t::table_grow:
                case op_code_t::table_size:
                case op_code_t::table_fill:
                case op_code_t::memory_fill:
                case op_code_t::return_call: {
                    format::print("{}", inst.subclass.dw);
                    break;
                }
                case op_code_t::calli: {
                    auto sc = &inst.subclass.calli_imm;
                    format::print("{} {}", sc->type_idx, sc->reserved);
                    break;
                }
                case op_code_t::br_table: {
                    auto sc = &inst.subclass.br_table;
                    for (u32 i = 0; i < sc->targets_table.size; ++i) {
                        if (i > 0 && i < sc->targets_table.size)
                            format::print(" ");
                        format::print("{}", sc->targets_table[i]);
                    }
                    format::print(" {}", sc->default_target);
                    break;
                }
                case op_code_t::i32_const:
                    format::print("{}", s32(inst.subclass.dw));
                    break;
                case op_code_t::i64_const: {
                    format::print("{}", s64(inst.subclass.qw));
                    break;
                }
                case op_code_t::f32_const: {
                    format::print("{:a}", f32(inst.subclass.dw));
                    break;
                }
                case op_code_t::f64_const: {
                    format::print("{:a}", f64(inst.subclass.qw));
                    break;
                }
                case op_code_t::typed_select: {
                    format::print(
                        "{} {}",
                        inst.subclass.typed_select.results,
                        inst.subclass.typed_select.type);
                    break;
                }
                case op_code_t::return_calli: {
                    auto sc = &inst.subclass.ret_calli_imm;
                    format::print("{} {}", sc->index, sc->table_index);
                    break;
                }
                case op_code_t::memory_size:
                case op_code_t::memory_grow: {
                    auto sc = &inst.subclass.alloc_imm;
                    format::print("{} {}", sc->byte, sc->memory);
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
                    format::print("{} {}", sc->align, sc->offset);
                    break;
                }
                case op_code_t::table_init:
                case op_code_t::memory_init: {
                    auto sc = &inst.subclass.seg_idx_imm;
                    format::print("{} {}", sc->segment, sc->index);
                    break;
                }
                case op_code_t::table_copy:
                case op_code_t::memory_copy: {
                    auto sc = &inst.subclass.copy_imm;
                    format::print("{} {}", sc->dst, sc->src);
                    break;
                }
                default:
                    break;
            }
            format::print("\n");
        }
    }
}
