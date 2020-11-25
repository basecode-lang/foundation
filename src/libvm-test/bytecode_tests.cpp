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

#include <catch2/catch.hpp>
#include <basecode/vm/bytecode.h>
#include <basecode/core/format.h>

using namespace basecode;

TEST_CASE("basecode::vm::bytecode encoding") {
    using namespace basecode::vm;

    format::print("static u16 s_decode_table[] = {{\n");

    u32 op_idx{};
    for (u8 i = 0; i < bytecode::group::logical; ++i) {
        format::print("    /* {} */\n", bytecode::group::name(i));
        const u8 num_ops = bytecode::group::group_size(i);
        for (u8 j = 0; j < num_ops; ++j) {
            u16 valid_sizes       {};
            u16 valid_operands    {};
            u16 valid_num_types   {};
            u16 valid_cond_codes  {};
            str::slice_t uop_name {};

            switch (i) {
                case bytecode::group::alu: {
                    uop_name         = bytecode::alu::name(j);
                    valid_sizes      = bytecode::alu::valid_sizes(j);
                    valid_operands   = bytecode::alu::valid_operands(j);
                    valid_num_types  = bytecode::alu::valid_number_types(j);
                    valid_cond_codes = bytecode::alu::valid_cond_codes(j);
                    break;
                }
                case bytecode::group::cond: {
                    uop_name         = bytecode::cond::name(j);
                    valid_sizes      = bytecode::cond::valid_sizes(j);
                    valid_operands   = bytecode::cond::valid_operands(j);
                    valid_num_types  = bytecode::cond::valid_number_types(j);
                    valid_cond_codes = bytecode::cond::valid_cond_codes(j);
                    break;
                }
                case bytecode::group::system: {
                    uop_name         = bytecode::system::name(j);
                    valid_sizes      = bytecode::system::valid_sizes(j);
                    valid_operands   = bytecode::system::valid_operands(j);
                    valid_num_types  = bytecode::system::valid_number_types(j);
                    valid_cond_codes = bytecode::system::valid_cond_codes(j);
                    break;
                }
                case bytecode::group::memory: {
                    uop_name         = bytecode::memory::name(j);
                    valid_sizes      = bytecode::memory::valid_sizes(j);
                    valid_operands   = bytecode::memory::valid_operands(j);
                    valid_num_types  = bytecode::memory::valid_number_types(j);
                    valid_cond_codes = bytecode::memory::valid_cond_codes(j);
                    break;
                }
                case bytecode::group::branch: {
                    uop_name         = bytecode::branch::name(j);
                    valid_sizes      = bytecode::branch::valid_sizes(j);
                    valid_operands   = bytecode::branch::valid_operands(j);
                    valid_num_types  = bytecode::branch::valid_number_types(j);
                    valid_cond_codes = bytecode::branch::valid_cond_codes(j);
                    break;
                }
                case bytecode::group::logical: {
                    uop_name         = bytecode::logical::name(j);
                    valid_sizes      = bytecode::logical::valid_sizes(j);
                    valid_operands   = bytecode::logical::valid_operands(j);
                    valid_num_types  = bytecode::logical::valid_number_types(j);
                    valid_cond_codes = bytecode::logical::valid_cond_codes(j);
                    break;
                }
                default:
                    break;
            }

            for (u16 k = 0; k < 16; ++k) {
                if (!TST_MASK(valid_sizes, k))
                    continue;
                for (u16 l = 0; l < 16; ++l) {
                    if (!TST_MASK(valid_num_types, l))
                        continue;
                    for (u16 m = 0; m < 16; ++m) {
                        if (!TST_MASK(valid_cond_codes, m))
                            continue;
                        for (u16 n = 0; n < 16; ++n) {
                            if (!TST_MASK(valid_operands, n))
                                continue;
                            auto uop     = UOP(i, j, k, m, l, n);
                            auto op_name = format::format("{}{}{}{}{}{}",
                                                          uop_name,
                                                          l > 0 ? bytecode::num_type::name(l) : ""_ss,
                                                          k > 0 ? "." : "",
                                                          k > 0 ? bytecode::size::name(k) : ""_ss,
                                                                  m > 0 ? "." : "",
                                                                  m > 0 ? bytecode::cond_code::name(m) : ""_ss);
                            format::print("    [0x{:08x}] = 0x{:04x}, /* {:<10} {:<32} */\n",
                                          uop,
                                          op_idx++,
                                          op_name,
                                          bytecode::operand::name(n));
                        }
                    }
                }
            }
        }
    }

    format::print("}};\n");
}
