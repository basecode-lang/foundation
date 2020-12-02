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

#include <basecode/core/str.h>
#include <basecode/core/types.h>

namespace basecode {
    struct gp_register_t;

    // 37 + 109
    // sizes: qw, dw, w, b
    //
    // addressing modes:
    // - r0                     register direct
    // - (r0)                   register indirect
    // - (r0)+, -(r2)           register indirect with post-incrementing or pre-decrementing
    // - d(r0), d(r1,r2)        register indirect with displacement
    //                          register indirect with indexing and displacement
    // - abs                    absolute addressing
    // - d(pc), d(pc,r3)        program counter relative addressing with displacement
    //                          program counter relative addressing with index register and displacement
    //
    // - imm                    immediate value
    //
    // add                      byte, word, dword, qword    10
    // addx                     byte, word, dword, qword    2
    // and                      byte, word, dword, qword    10
    // asl                      byte, word, dword, qword    7
    // asr                      byte, word, dword, qword    7
    // bge  <---+
    // bgt      | signed
    // ble      | compare/branch
    // blt  <---+
    // bhs  <---+
    // bhi      | unsigned
    // bls      | compare/branch
    // blo  <---+
    // clr                      byte, word, dword, qword    7
    // cmp                      byte, word, dword, qword    10
    // divs                     "                           9
    // divu                     "                           9
    // eor                      "
    // exts       sign extend   byte, word, dword
    // extz       zero extend   byte, word, dword
    // jmp                      qword                       2
    // jsr                      qword                       2
    // link                     qword
    // unlk                     qword
    // lea                      qword
    // lsl                      byte, word, dword, qword    7
    // lsr                      "                           7
    // move                     "                           10
    // movem                    "                           10
    // muls                     "                           9
    // mulu                     "                           9
    // neg                      "                           7
    // negx                     "                           2
    // nop
    // not                      "                           7
    // or                       "                           10
    // rol                      "                           7
    // ror                      "                           7
    // rts
    // scc <------+
    // scs        |
    // seq        |
    // sge        |
    // sgt        | condition set ops
    // shi        | byte size only
    // sle        |
    // sls        |
    // slt        |
    // smi        |
    // sne        |
    // spl        |
    // svc        |
    // svs <------+
    // stop
    // sub                      "                           10
    // subx                     "                           2
    // swap                     byte, word, dword
    // trap

    union operand_data_t final {
        struct {
            u64                 imm:        32;
            u64                 rd:         5;
            u64                 pad:        18;
        }                       imm_rd;
        struct {
            u64                 rs:         5;
            u64                 rs_inc:     1;
            u64                 rs_dec:     1;
            u64                 rs_pre:     1;
            u64                 rs_post:    1;
            u64                 rs_disp:    16;
            u64                 rd:         5;
            u64                 pad:        25;
        }                       rs_ippid_rd;
        struct {
            u64                 rs:         5;
            u64                 rs_inc:     1;
            u64                 rs_dec:     1;
            u64                 rs_pre:     1;
            u64                 rs_post:    1;
            u64                 rs_disp:    16;
            u64                 rd:         5;
            u64                 rd_inc:     1;
            u64                 rd_dec:     1;
            u64                 rd_pre:     1;
            u64                 rd_post:    1;
            u64                 rd_disp:    16;
            u64                 pad:        5;
        }                       rs_ippid_rd_ippid;
    };

    struct instruction_t final {
        u64                     op:         9;
        u64                     data:       55;
    };
    static_assert(sizeof(instruction_t) <= 8, "instruction_t is now greater than 8 bytes!");

    namespace vm {
        namespace bytecode {
            namespace size {
                constexpr u8 none               = 0;
                constexpr u8 byte               = 1;
                constexpr u8 word               = 2;
                constexpr u8 dword              = 3;
                constexpr u8 qword              = 4;

                str::slice_t name(u8 size);
            }

            namespace instructions {
                constexpr u16 add_b             = 0;
                constexpr u16 add_w             = 1;
                constexpr u16 add_dw            = 2;
                constexpr u16 add_qw            = 3;

                constexpr u16 addx_b            = 4;
                constexpr u16 addx_w            = 5;
                constexpr u16 addx_dw           = 6;
                constexpr u16 addx_qw           = 7;

                constexpr u16 and_b             = 8;
                constexpr u16 and_w             = 9;
                constexpr u16 and_dw            = 10;
                constexpr u16 and_qw            = 11;

                constexpr u16 asl_b             = 12;
                constexpr u16 asl_w             = 13;
                constexpr u16 asl_dw            = 14;
                constexpr u16 asl_qw            = 15;

                constexpr u16 asr_b             = 16;
                constexpr u16 asr_w             = 17;
                constexpr u16 asr_dw            = 18;
                constexpr u16 asr_qw            = 19;

                constexpr u16 bge_dw            = 20;

                constexpr u16 bgt_dw            = 21;

                constexpr u16 ble_dw            = 22;

                constexpr u16 blt_dw            = 23;

                constexpr u16 bhs_dw            = 24;

                constexpr u16 bhi_dw            = 25;

                constexpr u16 bls_dw            = 26;

                constexpr u16 blo_dw            = 27;

                constexpr u16 cas_b             = 28;
                constexpr u16 cas_w             = 29;
                constexpr u16 cas_dw            = 30;
                constexpr u16 cas_qw            = 31;

                constexpr u16 clr_b             = 32;
                constexpr u16 clr_w             = 33;
                constexpr u16 clr_dw            = 34;
                constexpr u16 clr_qw            = 35;

                constexpr u16 cmp_b             = 36;
                constexpr u16 cmp_w             = 37;
                constexpr u16 cmp_dw            = 38;
                constexpr u16 cmp_qw            = 39;

                constexpr u16 cmpm_b            = 40;
                constexpr u16 cmpm_w            = 41;
                constexpr u16 cmpm_dw           = 42;
                constexpr u16 cmpm_qw           = 43;

                constexpr u16 divs_b            = 44;
                constexpr u16 divs_w            = 45;
                constexpr u16 divs_dw           = 46;
                constexpr u16 divs_qw           = 47;

                constexpr u16 divu_b            = 48;
                constexpr u16 divu_w            = 49;
                constexpr u16 divu_dw           = 50;
                constexpr u16 divu_qw           = 51;

                constexpr u16 eor_b             = 52;
                constexpr u16 eor_w             = 53;
                constexpr u16 eor_dw            = 54;
                constexpr u16 eor_qw            = 55;

                constexpr u16 exts_b            = 56;
                constexpr u16 exts_w            = 57;
                constexpr u16 exts_dw           = 58;

                constexpr u16 extz_b            = 59;
                constexpr u16 extz_w            = 60;
                constexpr u16 extz_dw           = 61;

                constexpr u16 jmp_dw            = 62;

                constexpr u16 jsr_dw            = 63;

                constexpr u16 link_dw           = 64;

                constexpr u16 unlk_none         = 65;

                constexpr u16 lea_qw            = 66;

                constexpr u16 lsl_b             = 67;
                constexpr u16 lsl_w             = 68;
                constexpr u16 lsl_dw            = 69;
                constexpr u16 lsl_qw            = 70;

                constexpr u16 lsr_b             = 71;
                constexpr u16 lsr_w             = 72;
                constexpr u16 lsr_dw            = 73;
                constexpr u16 lsr_qw            = 74;

                constexpr u16 move_b            = 75;
                constexpr u16 move_w            = 76;
                constexpr u16 move_dw           = 77;
                constexpr u16 move_qw           = 78;

                constexpr u16 movem_b           = 79;
                constexpr u16 movem_w           = 80;
                constexpr u16 movem_dw          = 81;
                constexpr u16 movem_qw          = 82;

                constexpr u16 muls_b            = 83;
                constexpr u16 muls_w            = 84;
                constexpr u16 muls_dw           = 85;
                constexpr u16 muls_qw           = 86;

                constexpr u16 mulu_b            = 87;
                constexpr u16 mulu_w            = 88;
                constexpr u16 mulu_dw           = 89;
                constexpr u16 mulu_qw           = 90;

                constexpr u16 neg_b             = 91;
                constexpr u16 neg_w             = 92;
                constexpr u16 neg_dw            = 93;
                constexpr u16 neg_qw            = 94;

                constexpr u16 negx_b            = 95;
                constexpr u16 negx_w            = 96;
                constexpr u16 negx_dw           = 97;
                constexpr u16 negx_qw           = 98;

                constexpr u16 nop_none          = 99;

                constexpr u16 not_b             = 100;
                constexpr u16 not_w             = 101;
                constexpr u16 not_dw            = 102;
                constexpr u16 not_qw            = 103;

                constexpr u16 or_b              = 104;
                constexpr u16 or_w              = 105;
                constexpr u16 or_dw             = 106;
                constexpr u16 or_qw             = 107;

                constexpr u16 rol_b             = 108;
                constexpr u16 rol_w             = 109;
                constexpr u16 rol_dw            = 110;
                constexpr u16 rol_qw            = 111;

                constexpr u16 ror_b             = 112;
                constexpr u16 ror_w             = 113;
                constexpr u16 ror_dw            = 114;
                constexpr u16 ror_qw            = 115;

                constexpr u16 rts_none          = 116;

                constexpr u16 scc_b             = 117;

                constexpr u16 scs_b             = 118;

                constexpr u16 seq_b             = 119;

                constexpr u16 sge_b             = 120;

                constexpr u16 sgt_b             = 121;

                constexpr u16 shi_b             = 122;

                constexpr u16 sle_b             = 123;

                constexpr u16 sls_b             = 124;

                constexpr u16 slt_b             = 125;

                constexpr u16 smi_b             = 126;

                constexpr u16 sne_b             = 127;

                constexpr u16 spl_b             = 128;

                constexpr u16 svc_b             = 129;

                constexpr u16 svs_b             = 130;

                constexpr u16 stop_none         = 131;

                constexpr u16 sub_b             = 132;
                constexpr u16 sub_w             = 133;
                constexpr u16 sub_dw            = 134;
                constexpr u16 sub_qw            = 135;

                constexpr u16 subx_b            = 136;
                constexpr u16 subx_w            = 137;
                constexpr u16 subx_dw           = 138;
                constexpr u16 subx_qw           = 139;

                constexpr u16 swap_b            = 140;
                constexpr u16 swap_w            = 141;
                constexpr u16 swap_dw           = 142;

                constexpr u16 trap_none         = 143;

                str::slice_t name(u8 op);
            }

            namespace register_file {
                constexpr u8 pc                 = 0;
                constexpr u8 sp                 = 1;
                constexpr u8 r1                 = 1;
                constexpr u8 r2                 = 2;
                constexpr u8 r3                 = 3;
                constexpr u8 r4                 = 4;
                constexpr u8 r5                 = 5;
                constexpr u8 r6                 = 6;
                constexpr u8 r7                 = 7;
                constexpr u8 r8                 = 8;
                constexpr u8 r9                 = 9;
                constexpr u8 r10                = 10;
                constexpr u8 r11                = 11;
                constexpr u8 r12                = 12;
                constexpr u8 r13                = 13;
                constexpr u8 r14                = 14;
                constexpr u8 r15                = 15;
                constexpr u8 r16                = 16;
                constexpr u8 r17                = 17;
                constexpr u8 r18                = 18;
                constexpr u8 r19                = 19;
                constexpr u8 r20                = 20;
                constexpr u8 r21                = 21;
                constexpr u8 r22                = 22;
                constexpr u8 r23                = 23;
                constexpr u8 r24                = 24;
                constexpr u8 r25                = 25;
                constexpr u8 r26                = 26;
                constexpr u8 r27                = 27;
                constexpr u8 r28                = 28;
                constexpr u8 r29                = 29;
                constexpr u8 r30                = 30;

                str::slice_t name(u8 reg);
            }
        }
    }
}
