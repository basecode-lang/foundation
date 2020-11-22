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

#include <basecode/core/types.h>

namespace basecode {
    namespace bytecode {
        namespace group {
            constexpr u8 alu        = 0;
            constexpr u8 misc       = 1;
            constexpr u8 flag       = 2;
            constexpr u8 memory     = 3;
            constexpr u8 branch     = 4;
        }

        namespace type {
            constexpr u8 nop        = 0;
            constexpr u8 add        = 1;
            constexpr u8 adc        = 2;
            constexpr u8 mul        = 3;
            constexpr u8 sub        = 4;
            constexpr u8 sbc        = 5;
            constexpr u8 div        = 6;
            constexpr u8 pow        = 7;
            constexpr u8 mod        = 8;
            constexpr u8 neg        = 9;
            constexpr u8 not_       = 10;
            constexpr u8 lsl        = 11;
            constexpr u8 lsr        = 12;
            constexpr u8 rol        = 13;
            constexpr u8 rcl        = 14;
            constexpr u8 ror        = 15;
            constexpr u8 rcr        = 16;
            constexpr u8 or_        = 17;
            constexpr u8 and_       = 18;
            constexpr u8 xor_       = 19;
            constexpr u8 br         = 20;
            constexpr u8 blr        = 21;
            constexpr u8 bra        = 22;
            constexpr u8 cmp        = 23;
            constexpr u8 beq        = 24;
            constexpr u8 bne        = 25;
            constexpr u8 bl         = 26;
            constexpr u8 ble        = 27;
            constexpr u8 bg         = 28;
            constexpr u8 bge        = 29;
            constexpr u8 seq        = 30;
            constexpr u8 sne        = 31;
            constexpr u8 sl         = 32;
            constexpr u8 sle        = 34;
            constexpr u8 sg         = 35;
            constexpr u8 sge        = 36;
            constexpr u8 ret        = 37;
            constexpr u8 mov        = 38;
            constexpr u8 lea        = 39;
            constexpr u8 ldr        = 40;
            constexpr u8 str        = 41;
            constexpr u8 halt       = 42;
            constexpr u8 trap       = 43;

            constexpr u8 fadd       = 44;
            constexpr u8 fsub       = 45;
            constexpr u8 fmul       = 46;
            constexpr u8 fdiv       = 47;
            constexpr u8 fneg       = 48;
            constexpr u8 fcmp       = 49;
        }

        namespace encoding {
            constexpr u8 none       = 0;
            constexpr u8 imm        = 1;
            constexpr u8 reg1       = 2;
            constexpr u8 reg2       = 3;
            constexpr u8 reg3       = 4;
            constexpr u8 reg4       = 5;
            constexpr u8 offset     = 6;
            constexpr u8 indexed    = 7;
        }
    }

    union operand_encoding_t final {
        struct {
            u64                 src:        32;
            u64                 dest:       6;
            u64                 pad:        8;
        }                       imm;
        struct {
            u64                 dest:       6;
            u64                 pad:        40;
        }                       reg1;
        struct {
            u64                 src:        6;
            u64                 dest:       6;
            u64                 aux:        32;
            u64                 pad:        2;
        }                       reg2;
        struct {
            u64                 src:        6;
            u64                 dest1:      6;
            u64                 dest2:      6;
            u64                 pad:        28;
        }                       reg3;
        struct {
            u64                 src:        6;
            u64                 dest1:      6;
            u64                 dest2:      6;
            u64                 dest3:      6;
            u64                 pad:        22;
        }                       reg4;
        struct {
            u64                 offs:       32;
            u64                 src:        6;
            u64                 dest:       6;
            u64                 pad:        2;
        }                       offset;
        struct {
            u64                 offs:       24;
            u64                 base:       6;
            u64                 index:      6;
            u64                 dest:       6;
            u64                 pad:        4;
        }                       indexed;
    };

    struct instruction_t final {
        u64                     group:      3;
        u64                     op:         5;
        u64                     size:       2;
        u64                     is_float:   1;
        u64                     is_signed:  1;
        u64                     encoding:   4;
        u64                     pad:        2;
        u64                     data:       46;
    };
    static_assert(sizeof(instruction_t) <= 8, "instruction_t is now greater than 8 bytes!");
}
