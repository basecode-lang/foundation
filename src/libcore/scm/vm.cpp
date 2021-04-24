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

#include <basecode/core/scm/vm.h>

#define EXEC_NEXT()             SAFE_SCOPE(                                         \
    if (cycles == 0)    return status;                                              \
    if (cycles  > 0)    --cycles;                                                   \
    flags = (flag_register_t*) &F;                                                  \
    inst  = (encoded_inst_t*) reinterpret_cast<u64*>(PC);                           \
    data  = inst->data;                                                             \
    opers = (encoded_operand_t*) &data;                                             \
    goto *s_microcode[s_op_decode[inst->type][inst->is_signed][inst->encoding]];)

namespace basecode::scm::vm {
    namespace trap {
        static str::slice_t s_names[] = {
            [eq]            = "EQ?"_ss,
            [box]           = "BOX"_ss,
            [car]           = "CAR"_ss,
            [cdr]           = "CDR"_ss,
            [ffi]           = "FFI"_ss,
            [call]          = "CALL"_ss,
            [cons]          = "CONS"_ss,
            [read]          = "READ"_ss,
            [list]          = "LIST"_ss,
            [unbox]         = "UNBOX"_ss,
            [write]         = "WRITE"_ss,
            [quote]         = "QUOTE"_ss,
            [error]         = "ERROR"_ss,
            [debug]         = "DEBUG"_ss,
            [is_true]       = "TRUE?"_ss,
            [is_atom]       = "ATOM?"_ss,
            [compare]       = "COMPARE"_ss,
            [set_car]       = "SET_CAR"_ss,
            [set_cdr]       = "SET_CDR"_ss,
            [not_true]      = "NOT"_ss,
            [quasiquote]    = "QUASIQUOTE"_ss,
        };

        str::slice_t name(u8 type) {
            return s_names[u32(type)];
        }
    }

    namespace instruction {
        namespace type {
            static str::slice_t s_names[] = {
                [nop]       = "NOP"_ss,
                [clc]       = "CLC"_ss,
                [sec]       = "SEC"_ss,
                [add]       = "ADD"_ss,
                [mul]       = "MUL"_ss,
                [sub]       = "SUB"_ss,
                [div]       = "DIV"_ss,
                [pow]       = "POW"_ss,
                [mod]       = "MOD"_ss,
                [neg]       = "NEG"_ss,
                [addf]      = "ADDF"_ss,
                [mulf]      = "MULF"_ss,
                [subf]      = "SUBF"_ss,
                [divf]      = "DIVF"_ss,
                [powf]      = "POWF"_ss,
                [modf]      = "MODF"_ss,
                [negf]      = "NEGF"_ss,
                [not_]      = "NOT"_ss,
                [shl]       = "SHL"_ss,
                [shr]       = "SHR"_ss,
                [rol]       = "ROL"_ss,
                [ror]       = "ROR"_ss,
                [or_]       = "OR"_ss,
                [and_]      = "AND"_ss,
                [xor_]      = "XOR"_ss,
                [br]        = "BR"_ss,
                [blr]       = "BLR"_ss,
                [cmp]       = "CMP"_ss,
                [beq]       = "BEQ"_ss,
                [bne]       = "BNE"_ss,
                [b]         = "B"_ss,
                [bl]        = "BL"_ss,
                [ble]       = "BLE"_ss,
                [bg]        = "BG"_ss,
                [bge]       = "BGE"_ss,
                [seq]       = "SEQ"_ss,
                [sne]       = "SNE"_ss,
                [sl]        = "SL"_ss,
                [sle]       = "SLE"_ss,
                [sg]        = "SG"_ss,
                [sge]       = "SGE"_ss,
                [ret]       = "RET"_ss,
                [mma]       = "MMA"_ss,
                [push]      = "PUSH"_ss,
                [pop]       = "POP"_ss,
                [lea]       = "LEA"_ss,
                [move]      = "MOVE"_ss,
                [load]      = "LOAD"_ss,
                [store]     = "STORE"_ss,
                [exit]      = "EXIT"_ss,
                [trap]      = "TRAP"_ss,
            };

            str::slice_t name(reg_t op) {
                return s_names[u32(op)];
            }
        }
    }

    namespace register_file {
        static str::slice_t s_names[] = {
            [none]          = "NONE"_ss,
            [pc]            = "PC"_ss,
            [ep]            = "EP"_ss,
            [cp]            = "CP"_ss,
            [lp]            = "LP"_ss,
            [gp]            = "GP"_ss,
            [dp]            = "DP"_ss,
            [hp]            = "HP"_ss,
            [fp]            = "FP"_ss,
            [sp]            = "SP"_ss,
            [m]             = "M"_ss,
            [f]             = "F"_ss,
            [lr]            = "LR"_ss,
            [r0]            = "R0"_ss,
            [r1]            = "R1"_ss,
            [r2]            = "R2"_ss,
            [r3]            = "R3"_ss,
            [r4]            = "R4"_ss,
            [r5]            = "R5"_ss,
            [r6]            = "R6"_ss,
            [r7]            = "R7"_ss,
            [r8]            = "R8"_ss,
            [r9]            = "R9"_ss,
            [r10]           = "R10"_ss,
            [r11]           = "R11"_ss,
            [r12]           = "R12"_ss,
            [r13]           = "R13"_ss,
            [r14]           = "R14"_ss,
            [r15]           = "R15"_ss,
        };

        str::slice_t name(reg_t reg) {
            return s_names[u32(reg)];
        }
    }

    constexpr u8 op_nop             = 0;
    constexpr u8 op_clc             = 1;
    constexpr u8 op_sec             = 2;
    constexpr u8 op_add_imm         = 3;
    constexpr u8 op_add_reg2        = 4;
    constexpr u8 op_adds_imm        = 5;
    constexpr u8 op_adds_reg2       = 6;
    constexpr u8 op_mul_imm         = 7;
    constexpr u8 op_mul_reg2        = 8;
    constexpr u8 op_muls_imm        = 9;
    constexpr u8 op_muls_reg2       = 10;
    constexpr u8 op_sub_imm         = 11;
    constexpr u8 op_sub_reg2        = 12;
    constexpr u8 op_subs_imm        = 13;
    constexpr u8 op_subs_reg2       = 14;
    constexpr u8 op_div_imm         = 15;
    constexpr u8 op_div_reg2        = 16;
    constexpr u8 op_pow_imm         = 17;
    constexpr u8 op_pow_reg2        = 18;
    constexpr u8 op_mod_imm         = 19;
    constexpr u8 op_mod_reg2        = 20;
    constexpr u8 op_neg_reg1        = 21;
    constexpr u8 op_addf_imm        = 22;
    constexpr u8 op_addf_reg2       = 23;
    constexpr u8 op_mulf_imm        = 24;
    constexpr u8 op_mulf_reg2       = 25;
    constexpr u8 op_subf_imm        = 26;
    constexpr u8 op_subf_reg2       = 27;
    constexpr u8 op_divf_imm        = 28;
    constexpr u8 op_divf_reg2       = 29;
    constexpr u8 op_powf_imm        = 30;
    constexpr u8 op_powf_reg2       = 31;
    constexpr u8 op_modf_imm        = 32;
    constexpr u8 op_modf_reg2       = 33;
    constexpr u8 op_negf_reg1       = 34;
    constexpr u8 op_not_reg1        = 35;
    constexpr u8 op_shl_imm         = 36;
    constexpr u8 op_shl_reg2        = 37;
    constexpr u8 op_shr_imm         = 38;
    constexpr u8 op_shr_reg2        = 39;
    constexpr u8 op_rol_imm         = 40;
    constexpr u8 op_rol_reg2        = 41;
    constexpr u8 op_ror_imm         = 42;
    constexpr u8 op_ror_reg2        = 43;
    constexpr u8 op_or_imm          = 44;
    constexpr u8 op_or_reg2         = 45;
    constexpr u8 op_and_imm         = 46;
    constexpr u8 op_and_reg2        = 47;
    constexpr u8 op_xor_imm         = 48;
    constexpr u8 op_xor_reg2        = 49;
    constexpr u8 op_b_imm           = 50;
    constexpr u8 op_br_reg1         = 51;
    constexpr u8 op_blr_imm         = 52;
    constexpr u8 op_blr_reg1        = 53;
    constexpr u8 op_trap_imm        = 54;
    constexpr u8 op_trap_reg2       = 55;
    constexpr u8 op_cmp_imm         = 56;
    constexpr u8 op_cmp_reg2        = 57;
    constexpr u8 op_cmps_imm        = 58;
    constexpr u8 op_cmps_reg2       = 59;
    constexpr u8 op_beq_imm         = 60;
    constexpr u8 op_beqs_imm        = 61;
    constexpr u8 op_bne_imm         = 62;
    constexpr u8 op_bnes_imm        = 63;
    constexpr u8 op_bl_imm          = 64;
    constexpr u8 op_bls_imm         = 65;
    constexpr u8 op_ble_imm         = 66;
    constexpr u8 op_bles_imm        = 67;
    constexpr u8 op_bg_imm          = 68;
    constexpr u8 op_bgs_imm         = 69;
    constexpr u8 op_bge_imm         = 70;
    constexpr u8 op_bges_imm        = 71;
    constexpr u8 op_seq_reg1        = 72;
    constexpr u8 op_seqs_reg1       = 73;
    constexpr u8 op_sne_reg1        = 74;
    constexpr u8 op_snes_reg1       = 75;
    constexpr u8 op_sl_reg1         = 76;
    constexpr u8 op_sls_reg1        = 77;
    constexpr u8 op_sle_reg1        = 78;
    constexpr u8 op_sles_reg1       = 79;
    constexpr u8 op_sg_reg1         = 80;
    constexpr u8 op_sgs_reg1        = 81;
    constexpr u8 op_sge_reg1        = 82;
    constexpr u8 op_sges_reg1       = 83;
    constexpr u8 op_ret_reg1        = 84;
    constexpr u8 op_mma_imm         = 85;
    constexpr u8 op_push_imm        = 86;
    constexpr u8 op_push_reg2       = 87;
    constexpr u8 op_pop_reg2        = 88;
    constexpr u8 op_lea_imm         = 89;
    constexpr u8 op_lea_offs        = 90;
    constexpr u8 op_move_imm        = 91;
    constexpr u8 op_move_reg2       = 92;
    constexpr u8 op_load_reg2       = 93;
    constexpr u8 op_load_offs       = 94;
    constexpr u8 op_load_idx        = 95;
    constexpr u8 op_store_reg2      = 96;
    constexpr u8 op_store_offs      = 97;
    constexpr u8 op_store_idx       = 98;
    constexpr u8 op_exit_imm        = 99;
    constexpr u8 op_exit_reg1       = 100;
    constexpr u8 op_error           = 255;

    static u8 s_op_decode[][2][9] = {
        //  none            imm             reg1            reg2            reg3            reg4            offset          indexed             reg2_imm
        [instruction::type::nop] = {
            {op_nop,        op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_nop,        op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::clc] = {
            {op_clc,        op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::sec] = {
            {op_sec,        op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::add] = {
            {op_error,      op_add_imm,     op_error,       op_add_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_adds_imm,    op_error,       op_adds_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::mul] = {
            {op_error,      op_mul_imm,     op_error,       op_mul_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_muls_imm,    op_error,       op_muls_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::sub] = {
            {op_error,      op_sub_imm,     op_error,       op_sub_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_subs_imm,    op_error,       op_subs_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::div] = {
            {op_error,      op_div_imm,     op_error,       op_div_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::pow] = {
            {op_error,      op_pow_imm,     op_error,       op_pow_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::mod] = {
            {op_error,      op_mod_imm,     op_error,       op_mod_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::neg] = {
            {op_error,      op_error,       op_neg_reg1,    op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_neg_reg1,    op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::addf] = {
            {op_error,      op_addf_imm,    op_error,       op_addf_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::mulf] = {
            {op_error,      op_mulf_imm,    op_error,       op_mulf_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::subf] = {
            {op_error,      op_subf_imm,    op_error,       op_subf_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::divf] = {
            {op_error,      op_divf_imm,    op_error,       op_divf_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::powf] = {
            {op_error,      op_powf_imm,    op_error,       op_powf_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::modf] = {
            {op_error,      op_modf_imm,    op_error,       op_modf_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::negf] = {
            {op_error,      op_error,       op_negf_reg1,   op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::not_] = {
            {op_error,      op_error,       op_not_reg1,    op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_not_reg1,    op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::shl] = {
            {op_error,      op_shl_imm,     op_error,       op_shl_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::shr] = {
            {op_error,      op_shr_imm,     op_error,       op_shr_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::rol] = {
            {op_error,      op_rol_imm,     op_error,       op_rol_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::ror] = {
            {op_error,      op_ror_imm,     op_error,       op_ror_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::or_] = {
            {op_error,      op_or_imm,      op_error,       op_or_reg2,     op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::and_] = {
            {op_error,      op_and_imm,     op_error,       op_and_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::xor_] = {
            {op_error,      op_xor_imm,     op_error,       op_xor_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::b] = {
            {op_error,      op_b_imm,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::br] = {
            {op_error,      op_error,       op_br_reg1,     op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::blr] = {
            {op_error,      op_blr_imm,     op_blr_reg1,    op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::trap] = {
            {op_error,      op_trap_imm,    op_error,       op_trap_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::cmp] = {
            {op_error,      op_cmp_imm,     op_error,       op_cmp_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_cmps_imm,    op_error,       op_cmps_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::beq] = {
            {op_error,      op_beq_imm,     op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_beqs_imm,    op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::bne] = {
            {op_error,      op_bne_imm,     op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_bnes_imm,    op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::bl] = {
            {op_error,      op_bl_imm,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_bls_imm,     op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::ble] = {
            {op_error,      op_ble_imm,     op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_bles_imm,    op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::bg] = {
            {op_error,      op_bg_imm,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_bgs_imm,     op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::bge] = {
            {op_error,      op_bge_imm,     op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_bges_imm,    op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::seq] = {
            {op_error,      op_error,       op_seq_reg1,    op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_seqs_reg1,   op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::sne] = {
            {op_error,      op_error,       op_sne_reg1,    op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_snes_reg1,   op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::sl] = {
            {op_error,      op_error,       op_sl_reg1,     op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_sls_reg1,    op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::sle] = {
            {op_error,      op_error,       op_sle_reg1,    op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_sles_reg1,   op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::sg] = {
            {op_error,      op_error,       op_sg_reg1,     op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_sgs_reg1,    op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::sge] = {
            {op_error,      op_error,       op_sge_reg1,    op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_sges_reg1,   op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::ret] = {
            {op_error,      op_error,       op_ret_reg1,    op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::mma] = {
            {op_error,      op_mma_imm,     op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::push] = {
            {op_error,      op_push_imm,    op_error,       op_push_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::pop] = {
            {op_error,      op_error,       op_error,       op_pop_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::lea] = {
            {op_error,      op_lea_imm,     op_error,       op_error,       op_error,       op_error,       op_lea_offs,    op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::move] = {
            {op_error,      op_move_imm,    op_error,       op_move_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_move_imm,    op_error,       op_move_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::load] = {
            {op_error,      op_error,       op_error,       op_load_reg2,   op_error,       op_error,       op_load_offs,   op_load_idx,        op_error},
            {op_error,      op_error,       op_error,       op_load_reg2,   op_error,       op_error,       op_load_offs,   op_load_idx,        op_error},
        },
        [instruction::type::store] = {
            {op_error,      op_error,       op_error,       op_store_reg2,  op_error,       op_error,       op_store_offs,  op_store_idx,       op_error},
            {op_error,      op_error,       op_error,       op_store_reg2,  op_error,       op_error,       op_store_offs,  op_store_idx,       op_error},
        },
        [instruction::type::exit] = {
            {op_error,      op_exit_imm,    op_exit_reg1,   op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
   };

    namespace mem_area {
        status_t init(mem_area_t& area,
                      vm_t* vm,
                      u32 id,
                      mem_area_type_t type,
                      reg_t reg,
                      alloc_t* alloc,
                      u32 min_capacity,
                      b8 top) {
            area.id           = id;
            area.vm           = vm;
            area.pad          = {};
            area.top          = top;
            area.reg          = reg;
            area.data         = {};
            area.null         = {};
            area.size         = {};
            area.type         = type;
            area.alloc        = alloc;
            area.capacity     = {};
            area.min_capacity = min_capacity;
            if (area.min_capacity > 0) {
                reserve(area, area.min_capacity);
            }
            return status_t::ok;
        }

        u0 free(mem_area_t& area) {
            memory::free(area.alloc, area.data);
            area.data         = {};
            area.size         = {};
            area.capacity     = {};
            area.min_capacity = {};
        }

        u0 reset(mem_area_t& area, b8 zero_mem) {
            area.size = {};
            if (zero_mem) {
                std::memset(area.data,
                            0,
                            area.capacity * sizeof(u64));
            }
        }

        u0 resize(mem_area_t& area, u32 new_size) {
            new_size = std::max(new_size, area.min_capacity);
            grow(area, new_size);
            area.size = new_size;
        }

        u0 grow(mem_area_t& area, u32 new_capacity) {
            new_capacity = std::max(std::max(new_capacity, area.capacity),
                                    area.min_capacity);
            reserve(area, new_capacity * 2 + 8);
        }

        str::slice_t type_name(mem_area_type_t type) {
            switch (type) {
                case mem_area_type_t::none:         return "none"_ss;
                case mem_area_type_t::heap:         return "heap"_ss;
                case mem_area_type_t::code:         return "code"_ss;
                case mem_area_type_t::gc_stack:     return "gc_stack"_ss;
                case mem_area_type_t::env_stack:    return "env_stack"_ss;
                case mem_area_type_t::code_stack:   return "code_stack"_ss;
                case mem_area_type_t::data_stack:   return "data_stack"_ss;
                case mem_area_type_t::register_file:return "register_file"_ss;
            }
        }

        u0 reserve(mem_area_t& area, u32 new_capacity) {
            if (new_capacity == 0) {
                memory::free(area.alloc, area.data);
                area.data     = {};
                area.capacity = area.size = {};
                return;
            }

            if (new_capacity == area.capacity)
                return;

            new_capacity = std::max(std::max(area.size, new_capacity),
                                    area.min_capacity);
            area.data = (u64*) memory::realloc(
                area.alloc,
                area.data,
                new_capacity * sizeof(u64),
                alignof(u64));
            area.capacity = new_capacity;
            if (area.reg != register_file::none) {
                auto& vm = *area.vm;
                G(area.reg) = mem_area::curr_addr(area);
            }
        }

        u0 shrink_to_size(mem_area_t& area, u32 new_size) {
            auto& vm = *area.vm;
            area.size = new_size;
            if (area.reg != register_file::none)
                G(area.reg) = curr_addr(area);
        }
    }

    u0 free(vm_t& vm) {
        for (auto area : vm.mem_map)
            mem_area::free(*area);
        hashtab::free(vm.traptab);
        stable_array::free(vm.mem_map);
    }

    u0 reset(vm_t& vm) {
        for (auto area : vm.mem_map) {
            if (area->type != mem_area_type_t::register_file)
                mem_area::reset(*area, true);
            if (area->reg != register_file::none)
                G(area->reg) = mem_area::base_addr(*area);
        }
        F  = 0;
        M  = 0;
        PC = 0;
    }

    mem_area_t* add_mem_area(vm_t& vm,
                             mem_area_type_t type,
                             reg_t reg,
                             alloc_t* alloc,
                             u32 min_capacity,
                             b8 top) {
        auto area = &stable_array::append(vm.mem_map);
        mem_area::init(*area,
                       &vm,
                       vm.mem_map.size,
                       type,
                       reg,
                       alloc,
                       min_capacity,
                       top);
        if (area->reg != register_file::none)
            vm.area_by_reg[area->reg] = area;

        // N.B. "code" areas are a bit special because there
        //      can be an arbitrary number of them.  since the
        //      mem_area_t* will likely live on the proc_t, it's better
        //      to access these directly instead of an indirect lookup.
        if (type != mem_area_type_t::code)
            vm.area_by_type[u32(type)] = area;

        return area;
    }

    b8 remove_mem_area(vm_t& vm, mem_area_t* area) {
        if (!area)
            return false;
        mem_area::free(*area);
        return stable_array::erase(vm.mem_map, area);
    }

    status_t step(vm_t& vm, ctx_t* ctx, s32 cycles) {
        static u0* s_microcode[] = {
            [op_nop]                = &&nop,
            [op_clc]                = &&clc,
            [op_sec]                = &&sec,
            [op_add_imm]            = &&add_imm,
            [op_add_reg2]           = &&add_reg2,
            [op_adds_imm]           = &&adds_imm,
            [op_adds_reg2]          = &&adds_reg2,
            [op_mul_imm]            = &&mul_imm,
            [op_mul_reg2]           = &&mul_reg2,
            [op_muls_imm]           = &&muls_imm,
            [op_muls_reg2]          = &&muls_reg2,
            [op_sub_imm]            = &&sub_imm,
            [op_sub_reg2]           = &&sub_reg2,
            [op_subs_imm]           = &&subs_imm,
            [op_subs_reg2]          = &&subs_reg2,
            [op_div_imm]            = &&div_imm,
            [op_div_reg2]           = &&div_reg2,
            [op_pow_imm]            = &&pow_imm,
            [op_pow_reg2]           = &&pow_reg2,
            [op_mod_imm]            = &&mod_imm,
            [op_mod_reg2]           = &&mod_reg2,
            [op_neg_reg1]           = &&neg_reg1,
            [op_addf_imm]           = &&addf_imm,
            [op_addf_reg2]          = &&addf_reg2,
            [op_mulf_imm]           = &&mulf_imm,
            [op_mulf_reg2]          = &&mulf_reg2,
            [op_subf_imm]           = &&subf_imm,
            [op_subf_reg2]          = &&subf_reg2,
            [op_divf_imm]           = &&divf_imm,
            [op_divf_reg2]          = &&divf_reg2,
            [op_powf_imm]           = &&powf_imm,
            [op_powf_reg2]          = &&powf_reg2,
            [op_modf_imm]           = &&modf_imm,
            [op_modf_reg2]          = &&modf_reg2,
            [op_negf_reg1]          = &&negf_reg1,
            [op_not_reg1]           = &&not_reg1,
            [op_shl_imm]            = &&shl_imm,
            [op_shl_reg2]           = &&shl_reg2,
            [op_shr_imm]            = &&shr_imm,
            [op_shr_reg2]           = &&shr_reg2,
            [op_rol_imm]            = &&rol_imm,
            [op_rol_reg2]           = &&rol_reg2,
            [op_ror_imm]            = &&ror_imm,
            [op_ror_reg2]           = &&ror_reg2,
            [op_or_imm]             = &&or_imm,
            [op_or_reg2]            = &&or_reg2,
            [op_and_imm]            = &&and_imm,
            [op_and_reg2]           = &&and_reg2,
            [op_xor_imm]            = &&xor_imm,
            [op_xor_reg2]           = &&xor_reg2,
            [op_b_imm]              = &&b_imm,
            [op_br_reg1]            = &&br_reg1,
            [op_blr_imm]            = &&blr_imm,
            [op_blr_reg1]           = &&blr_reg1,
            [op_trap_imm]           = &&trap_imm,
            [op_trap_reg2]          = &&trap_reg2,
            [op_cmp_imm]            = &&cmp_imm,
            [op_cmp_reg2]           = &&cmp_reg2,
            [op_cmps_imm]           = &&cmps_imm,
            [op_cmps_reg2]          = &&cmps_reg2,
            [op_beq_imm]            = &&beq_imm,
            [op_beqs_imm]           = &&beqs_imm,
            [op_bne_imm]            = &&bne_imm,
            [op_bnes_imm]           = &&bnes_imm,
            [op_bl_imm]             = &&bl_imm,
            [op_bls_imm]            = &&bls_imm,
            [op_ble_imm]            = &&ble_imm,
            [op_bles_imm]           = &&bles_imm,
            [op_bg_imm]             = &&bg_imm,
            [op_bgs_imm]            = &&bgs_imm,
            [op_bge_imm]            = &&bge_imm,
            [op_bges_imm]           = &&bges_imm,
            [op_seq_reg1]           = &&seq_reg1,
            [op_seqs_reg1]          = &&seqs_reg1,
            [op_sne_reg1]           = &&sne_reg1,
            [op_snes_reg1]          = &&snes_reg1,
            [op_sl_reg1]            = &&sl_reg1,
            [op_sls_reg1]           = &&sls_reg1,
            [op_sle_reg1]           = &&sle_reg1,
            [op_sles_reg1]          = &&sles_reg1,
            [op_sg_reg1]            = &&sg_reg1,
            [op_sgs_reg1]           = &&sgs_reg1,
            [op_sge_reg1]           = &&sge_reg1,
            [op_sges_reg1]          = &&sges_reg1,
            [op_ret_reg1]           = &&ret_reg1,
            [op_mma_imm]            = &&mma_imm,
            [op_push_imm]           = &&push_imm,
            [op_push_reg2]          = &&push_reg2,
            [op_pop_reg2]           = &&pop_reg2,
            [op_lea_imm]            = &&lea_imm,
            [op_lea_offs]           = &&lea_offs,
            [op_move_imm]           = &&move_imm,
            [op_move_reg2]          = &&move_reg2,
            [op_load_reg2]          = &&load_reg2,
            [op_load_offs]          = &&load_offset,
            [op_load_idx]           = &&load_indexed,
            [op_store_reg2]         = &&store_reg2,
            [op_store_offs]         = &&store_offset,
            [op_store_idx]          = &&store_indexed,
            [op_exit_imm]           = &&exit_imm,
            [op_exit_reg1]          = &&exit_reg1,
            [op_error]              = &&error,
        };

#ifdef __linux__
        // N.B. only linux...for some reason the platform typedefs
    //      on linux don't align with clang's expectations for
    //      the arithmetic builtins.  on windows and macOS they're
    //      fine.  this redefines u64/s64 in this scope so they're in
    //      agreement with clang's expectations
    using u64 = unsigned long long;
    using s64 = long long;
#endif
        u64                 data    {};
        u64                 carry   {};
        encoded_inst_t*     inst    {};
        flag_register_t*    flags   {};
        encoded_operand_t*  opers   {};
        status_t            status  {};

        EXEC_NEXT();

        nop:
        {
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        clc:
        {
            flags->c = false;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        sec:
        {
            flags->c = true;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        add_imm:
        {
            auto sum = __builtin_addcll(G(opers->imm.dst),
                                        opers->imm.src,
                                        flags->c,
                                        &carry);
            flags->z = sum == 0;
            flags->c = carry > 0;
            flags->n = s64(sum) < 0;
            G(opers->imm.dst) = sum;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        add_reg2:
        {
            auto sum = __builtin_addcll(G(opers->reg2.dst),
                                        G(opers->reg2.src),
                                        flags->c,
                                        &carry);
            flags->z = sum == 0;
            flags->c = carry > 0;
            flags->n = s64(sum) < 0;
            G(opers->reg2.dst) = sum;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        adds_imm:
        {
            s64 sum{};
            flags->v = __builtin_saddll_overflow(G(opers->imm.dst),
                                                 opers->imm.src,
                                                 &sum);
            flags->z = sum == 0;
            flags->c = false;
            flags->n = sum < 0;
            G(opers->imm.dst) = sum;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        adds_reg2:
        {
            s64 sum{};
            flags->v = __builtin_saddll_overflow(G(opers->reg2.dst),
                                                 G(opers->reg2.src),
                                                 &sum);
            flags->z = sum == 0;
            flags->c = false;
            flags->n = sum < 0;
            G(opers->reg2.dst) = sum;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        mul_imm:
        {
            u64 prod{};
            flags->c = __builtin_umulll_overflow(G(opers->imm.dst),
                                                 opers->imm.src,
                                                 &prod);
            flags->z = prod == 0;
            flags->v = false;
            flags->n = s64(prod) < 0;
            G(opers->imm.dst) = prod;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        mul_reg2:
        {
            u64 prod{};
            flags->c = __builtin_umulll_overflow(G(opers->reg2.dst),
                                                 G(opers->reg2.src),
                                                 &prod);
            flags->z = prod == 0;
            flags->v = false;
            flags->n = s64(prod) < 0;
            G(opers->reg2.dst) = prod;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        muls_imm:
        {
            s64 prod{};
            flags->v = __builtin_smulll_overflow(G(opers->imm.dst),
                                                 opers->imm.src,
                                                 &prod);
            flags->z = prod == 0;
            flags->c = false;
            flags->n = prod < 0;
            G(opers->imm.dst) = prod;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        muls_reg2:
        {
            s64 prod{};
            flags->v = __builtin_smulll_overflow(G(opers->reg2.dst),
                                                 G(opers->reg2.src),
                                                 &prod);
            flags->z = prod == 0;
            flags->c = false;
            flags->n = prod < 0;
            G(opers->reg2.dst) = prod;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        sub_imm:
        {
            auto diff = __builtin_subcll(G(opers->imm.dst),
                                         opers->imm.src,
                                         flags->c,
                                         &carry);
            flags->z = diff == 0;
            flags->c = carry > 0;
            flags->n = s64(diff) < 0;
            G(opers->imm.dst) = diff;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        sub_reg2:
        {
            auto diff = __builtin_subcll(G(opers->reg2.dst),
                                         G(opers->reg2.src),
                                         flags->c,
                                         &carry);
            flags->z = diff == 0;
            flags->c = carry > 0;
            flags->n = s64(diff) < 0;
            G(opers->reg2.dst) = diff;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        subs_imm:
        {
            s64 diff{};
            flags->v = __builtin_ssubll_overflow(G(opers->imm.dst),
                                                 opers->imm.src,
                                                 &diff);
            flags->z = diff == 0;
            flags->c = false;
            flags->n = diff < 0;
            G(opers->imm.dst) = diff;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        subs_reg2:
        {
            s64 diff{};
            flags->v = __builtin_ssubll_overflow(G(opers->reg2.dst),
                                                 G(opers->reg2.src),
                                                 &diff);
            flags->z = diff == 0;
            flags->c = false;
            flags->n = diff < 0;
            G(opers->reg2.dst) = diff;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        div_imm:
        {
            auto quotient = opers->imm.src == 0 ? 0 : G(opers->imm.dst) / opers->imm.src;
            flags->v = false;
            flags->c = false;
            flags->z = quotient == 0;
            flags->n = s64(quotient) < 0;
            G(opers->imm.dst) = quotient;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        div_reg2:
        {
            auto src = G(opers->reg2.src);
            auto quotient = src == 0 ? 0 : G(opers->reg2.dst) / src;
            flags->v = false;
            flags->c = false;
            flags->z = quotient == 0;
            flags->n = s64(quotient) < 0;
            G(opers->reg2.dst) = quotient;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        pow_imm:
        {
            u64 e = pow(G(opers->imm.dst), opers->imm.src);
            flags->v = false;
            flags->c = false;
            flags->z = e == 0;
            flags->n = s64(e) < 0;
            G(opers->imm.dst) = e;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        pow_reg2:
        {
            u64 e = pow(G(opers->reg2.dst), G(opers->reg2.src));
            flags->v = false;
            flags->c = false;
            flags->z = e == 0;
            flags->n = s64(e) < 0;
            G(opers->reg2.dst) = e;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        mod_imm:
        {
            u64 e = G(opers->imm.dst) % opers->imm.src;
            flags->v = false;
            flags->c = false;
            flags->z = e == 0;
            flags->n = s64(e) < 0;
            G(opers->imm.dst) = e;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        mod_reg2:
        {
            u64 e = G(opers->reg2.dst) % G(opers->reg2.src);
            flags->v = false;
            flags->c = false;
            flags->z = e == 0;
            flags->n = s64(e) < 0;
            G(opers->reg2.dst) = e;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        neg_reg1:
        {
            s64 negated = -G(opers->reg1.dst);
            flags->v = false;
            flags->c = false;
            flags->z = negated == 0;
            flags->n = negated < 0;
            G(opers->reg1.dst) = negated;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        addf_imm:
        {
            auto sum = std::bit_cast<f64>(G(opers->imm.dst))
                       + std::bit_cast<f64>(opers->imm.src);
            flags->z = sum == 0;
            flags->c = carry > 0;
            flags->n = sum < 0;
            G(opers->imm.dst) = sum;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        addf_reg2:
        {
            auto sum = std::bit_cast<f64>(G(opers->reg2.dst))
                       + std::bit_cast<f64>(G(opers->reg2.src));
            flags->z = sum == 0;
            flags->c = carry > 0;
            flags->n = sum < 0;
            G(opers->reg2.dst) = std::bit_cast<f64>(sum);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        mulf_imm:
        {
            auto prod = std::bit_cast<f64>(G(opers->imm.dst))
                        * std::bit_cast<f64>(opers->imm.src);
            flags->z = prod == 0;
            flags->v = false;
            flags->n = prod < 0;
            G(opers->imm.dst) = std::bit_cast<f64>(prod);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        mulf_reg2:
        {
            auto prod = std::bit_cast<f64>(G(opers->reg2.dst))
                        * std::bit_cast<f64>(G(opers->reg2.src));
            flags->z = prod == 0;
            flags->v = false;
            flags->n = prod < 0;
            G(opers->reg2.dst) = std::bit_cast<u64>(prod);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        subf_imm:
        {
            auto diff = std::bit_cast<f64>(G(opers->imm.dst))
                        - std::bit_cast<f64>(opers->imm.src);
            flags->z = diff == 0;
            flags->c = carry > 0;
            flags->n = diff < 0;
            G(opers->imm.dst) = std::bit_cast<u64>(diff);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        subf_reg2:
        {
            auto diff = std::bit_cast<f64>(G(opers->reg2.dst))
                        - std::bit_cast<f64>(G(opers->reg2.src));
            flags->z = diff == 0;
            flags->c = carry > 0;
            flags->n = diff < 0;
            G(opers->reg2.dst) = std::bit_cast<u64>(diff);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        divf_imm:
        {
            auto src = std::bit_cast<f64>(opers->imm.src);
            auto dst = std::bit_cast<f64>(G(opers->imm.dst));
            auto quotient = src == 0 ? 0 : dst / src;
            flags->v = false;
            flags->c = false;
            flags->z = quotient == 0;
            flags->n = quotient < 0;
            G(opers->imm.dst) = std::bit_cast<u64>(quotient);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        divf_reg2:
        {
            auto src = std::bit_cast<f64>(G(opers->reg2.src));
            auto quotient = src == 0 ? 0 : std::bit_cast<f64>(G(opers->reg2.dst)) / src;
            flags->v = false;
            flags->c = false;
            flags->z = quotient == 0;
            flags->n = quotient < 0;
            G(opers->reg2.dst) = std::bit_cast<u64>(quotient);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        powf_imm:
        {
            u64 e = std::pow(std::bit_cast<f64>(G(opers->imm.dst)),
                             opers->imm.src);
            flags->v = false;
            flags->c = false;
            flags->z = e == 0;
            flags->n = e < 0;
            G(opers->imm.dst) = std::bit_cast<u64>(e);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        powf_reg2:
        {
            f64 e = std::pow(std::bit_cast<f64>(G(opers->reg2.dst)),
                             G(opers->reg2.src));
            flags->v = false;
            flags->c = false;
            flags->z = e == 0;
            flags->n = e < 0;
            G(opers->reg2.dst) = std::bit_cast<u64>(e);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        modf_imm:
        {
            f64 e = std::fmod(std::bit_cast<f64>(G(opers->imm.dst)),
                              std::bit_cast<f64>(opers->imm.src));
            flags->v = false;
            flags->c = false;
            flags->z = e == 0;
            flags->n = e < 0;
            G(opers->imm.dst) = std::bit_cast<u64>(e);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        modf_reg2:
        {
            f64 e = std::fmod(std::bit_cast<f64>(G(opers->reg2.dst)),
                              std::bit_cast<f64>(G(opers->reg2.src)));
            flags->v = false;
            flags->c = false;
            flags->z = e == 0;
            flags->n = e < 0;
            G(opers->reg2.dst) = std::bit_cast<u64>(e);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        negf_reg1:
        {
            f64 negated = -std::bit_cast<f64>(G(opers->reg1.dst));
            flags->v = false;
            flags->c = false;
            flags->z = negated == 0;
            flags->n = negated < 0;
            G(opers->reg1.dst) = std::bit_cast<u64>(negated);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        not_reg1:
        {
            u64 bin_not = ~G(opers->reg1.dst);
            flags->v = false;
            flags->c = false;
            flags->z = bin_not == 0;
            flags->n = s64(bin_not) < 0;
            G(opers->reg1.dst) = bin_not;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        shl_imm:
        {
            auto count = opers->imm.src;
            auto shifted = G(opers->imm.dst) << count;
            auto msb = shifted & (1ULL << 63U);
            flags->v = msb ^ (count & 0x1fU);
            flags->c = msb;
            flags->z = shifted == 0;
            flags->n = s64(shifted) < 0;
            G(opers->imm.dst) = shifted;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        shl_reg2:
        {
            auto count = G(opers->reg2.src);
            auto shifted = G(opers->reg2.dst) << count;
            auto msb = shifted & (1ULL << 63U);
            flags->v = msb ^ (count & 0x1fU);
            flags->c = msb;
            flags->z = shifted == 0;
            flags->n = s64(shifted) < 0;
            G(opers->reg2.dst) = shifted;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        shr_imm:
        {
            auto count = opers->imm.src;
            auto dv = G(opers->imm.dst);
            auto shifted = dv >> count;
            flags->v = dv & (1ULL << 63U);
            flags->c = shifted & 0x01U;
            flags->z = shifted == 0;
            flags->n = s64(shifted) < 0;
            G(opers->imm.dst) = shifted;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        shr_reg2:
        {
            auto count = G(opers->reg2.src);
            auto dv = G(opers->reg2.dst);
            auto shifted = dv >> count;
            flags->v = dv & (1ULL << 63U);
            flags->c = shifted & 0x01U;
            flags->z = shifted == 0;
            flags->n = s64(shifted) < 0;
            G(opers->reg2.dst) = shifted;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        rol_imm:
        {
            auto count = opers->imm.src;
            auto shifted = __builtin_rotateleft64(G(opers->imm.dst), count);
            auto msb = shifted & (1ULL << 63U);
            flags->v = msb ^ (count & 0x1fU);
            flags->c = msb;
            flags->z = shifted == 0;
            flags->n = s64(shifted) < 0;
            G(opers->imm.dst) = shifted;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        rol_reg2:
        {
            auto count = G(opers->reg2.src);
            auto shifted = __builtin_rotateleft64(G(opers->reg2.dst), count);
            auto msb = shifted & (1ULL << 63U);
            flags->v = msb ^ (count & 0x1fU);
            flags->c = msb;
            flags->z = shifted == 0;
            flags->n = s64(shifted) < 0;
            G(opers->reg2.dst) = shifted;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        ror_imm:
        {
            auto count = opers->imm.src;
            auto dv = G(opers->imm.dst);
            auto shifted = __builtin_rotateright64(dv, count);
            flags->v = dv & (1ULL << 63U);
            flags->c = shifted & 0x01U;
            flags->z = shifted == 0;
            flags->n = s64(shifted) < 0;
            G(opers->imm.dst) = shifted;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        ror_reg2:
        {
            auto count = G(opers->reg2.src);
            auto dv = G(opers->reg2.dst);
            auto shifted = __builtin_rotateright64(dv, count);
            flags->v = dv & (1ULL << 63U);
            flags->c = shifted & 0x01U;
            flags->z = shifted == 0;
            flags->n = s64(shifted) < 0;
            G(opers->reg2.dst) = shifted;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        or_imm:
        {
            auto e = G(opers->imm.dst) | opers->imm.src;
            flags->v = false;
            flags->c = false;
            flags->z = e == 0;
            flags->n = s64(e) < 0;
            G(opers->imm.dst) = e;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        or_reg2:
        {
            auto e = G(opers->reg2.dst) | G(opers->reg2.src);
            flags->v = false;
            flags->c = false;
            flags->z = e == 0;
            flags->n = s64(e) < 0;
            G(opers->reg2.dst) = e;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        and_imm:
        {
            auto e = G(opers->imm.dst) & opers->imm.src;
            flags->v = false;
            flags->c = false;
            flags->z = e == 0;
            flags->n = s64(e) < 0;
            G(opers->imm.dst) = e;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        and_reg2:
        {
            auto e = G(opers->reg2.dst) & G(opers->reg2.src);
            flags->v = false;
            flags->c = false;
            flags->z = e == 0;
            flags->n = s64(e) < 0;
            G(opers->reg2.dst) = e;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        xor_imm:
        {
            auto e = G(opers->imm.dst) ^ opers->imm.src;
            flags->v = false;
            flags->c = false;
            flags->z = e == 0;
            flags->n = s64(e) < 0;
            G(opers->imm.dst) = e;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        xor_reg2:
        {
            auto e = G(opers->reg2.dst) ^ G(opers->reg2.src);
            flags->v = false;
            flags->c = false;
            flags->z = e == 0;
            flags->n = s64(e) < 0;
            G(opers->reg2.dst) = e;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        b_imm:
        {
            PC += s64(opers->imm.src);
            EXEC_NEXT();
        }
        br_reg1:
        {
            PC += s64(G(opers->reg1.dst));
            EXEC_NEXT();
        }
        blr_imm:
        {
            LR = PC + sizeof(encoded_inst_t);
            PC += s64(opers->imm.src);
            EXEC_NEXT();
        }
        blr_reg1:
        {
            LR = PC + sizeof(encoded_inst_t);
            PC += s64(G(opers->reg1.dst));
            EXEC_NEXT();
        }
        trap_imm:
        {
            auto id       = opers->imm.src;
            auto arg      = G(opers->imm.dst);
            auto callback = hashtab::find(vm.traptab, u32(id));
            status = callback && callback(vm, arg) ? status_t::ok :
                     status_t::fail;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        trap_reg2:
        {
            auto id       = G(opers->reg2.src);
            auto arg      = G(opers->reg2.dst);
            auto callback = hashtab::find(vm.traptab, u32(id));
            status = callback && callback(vm, arg) ? status_t::ok :
                     status_t::fail;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        cmp_imm:
        {
            s64 diff = __builtin_subcll(G(opers->imm.dst),
                                        opers->imm.src,
                                        flags->c,
                                        &carry);
            flags->v = false;
            flags->c = carry > 0;
            flags->z = diff == 0;
            flags->n = diff < 0;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        cmp_reg2:
        {
            s64 diff = __builtin_subcll(G(opers->reg2.dst),
                                        G(opers->reg2.src),
                                        flags->c,
                                        &carry);
            flags->v = false;
            flags->c = carry > 0;
            flags->z = diff == 0;
            flags->n = diff < 0;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        cmps_imm:
        {
            s64 diff{};
            flags->v = __builtin_ssubll_overflow(G(opers->imm.dst),
                                                 opers->imm.src,
                                                 &diff);
            flags->c = false;
            flags->z = diff == 0;
            flags->n = diff < 0;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        cmps_reg2:
        {
            s64 diff{};
            flags->v = __builtin_ssubll_overflow(G(opers->reg2.dst),
                                                 G(opers->reg2.src),
                                                 &diff);
            flags->c = false;
            flags->z = diff == 0;
            flags->n = diff < 0;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        beq_imm:
        {
            PC += s64(flags->z ? opers->imm.src : sizeof(encoded_inst_t));
            EXEC_NEXT();
        }
        beqs_imm:
        {
            PC += s64(flags->z ? opers->imm.src : sizeof(encoded_inst_t));
            EXEC_NEXT();
        }
        bne_imm:
        {
            PC += s64(!flags->z ? opers->imm.src : sizeof(encoded_inst_t));
            EXEC_NEXT();
        }
        bnes_imm:
        {
            PC += s64(!flags->z ? opers->imm.src : sizeof(encoded_inst_t));
            EXEC_NEXT();
        }
        bl_imm:
        {
            PC += s64(!flags->c ? opers->imm.src : sizeof(encoded_inst_t));
            EXEC_NEXT();
        }
        bls_imm:
        {
            PC += s64(flags->n != flags->v ? opers->imm.src : sizeof(encoded_inst_t));
            EXEC_NEXT();
        }
        ble_imm:
        {
            PC += s64(!flags->c || flags->z ? opers->imm.src : sizeof(encoded_inst_t));
            EXEC_NEXT();
        }
        bles_imm:
        {
            PC += s64(flags->z || flags->n != flags->v ? opers->imm.src : sizeof(encoded_inst_t));
            EXEC_NEXT();
        }
        bg_imm:
        {
            PC += s64(flags->c && !flags->z ? opers->imm.src : sizeof(encoded_inst_t));
            EXEC_NEXT();
        }
        bgs_imm:
        {
            PC += s64(!flags->z && flags->n == flags->v ? opers->imm.src : sizeof(encoded_inst_t));
            EXEC_NEXT();
        }
        bge_imm:
        {
            PC += s64(flags->c ? opers->imm.src : sizeof(encoded_inst_t));
            EXEC_NEXT();
        }
        bges_imm:
        {
            PC += s64(flags->n == flags->v ? opers->imm.src : sizeof(encoded_inst_t));
            EXEC_NEXT();
        }
        seq_reg1:
        {
            G(opers->reg1.dst) = flags->z;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        seqs_reg1:
        {
            G(opers->reg1.dst) = flags->z;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        sne_reg1:
        {
            G(opers->reg1.dst) = !flags->z;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        snes_reg1:
        {
            G(opers->reg1.dst) = !flags->z;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        sl_reg1:
        {
            G(opers->reg1.dst) = !flags->c;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        sls_reg1:
        {
            G(opers->reg1.dst) = flags->n != flags->v;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        sle_reg1:
        {
            G(opers->reg1.dst) = !flags->c || flags->z;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        sles_reg1:
        {
            G(opers->reg1.dst) = flags->z || flags->n != flags->v;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        sg_reg1:
        {
            G(opers->reg1.dst) = flags->c && !flags->z;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        sgs_reg1:
        {
            G(opers->reg1.dst) = !flags->z || flags->n == flags->v;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        sge_reg1:
        {
            G(opers->reg1.dst) = flags->c;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        sges_reg1:
        {
            G(opers->reg1.dst) = flags->n == flags->v;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        ret_reg1:
        {
            PC = G(opers->reg1.dst);
            EXEC_NEXT();
        }
        mma_imm:
        {
            // FIXME: not sure if this op is really necessary
//            auto& area = vm.mem_map[opers->imm.src];
//            G(opers->imm.dst) = mem_area::base_addr(area);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        push_imm:
        {
            auto area = get_mem_area_by_reg(vm, opers->imm.dst);
            mem_area::push(*area, opers->imm.src);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        push_reg2:
        {
            auto area = get_mem_area_by_reg(vm, opers->reg2.dst);
            mem_area::push(*area, G(opers->reg2.src));
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        pop_reg2:
        {
            auto area = get_mem_area_by_reg(vm, opers->reg2.src);
            G(opers->reg2.dst) = mem_area::pop<u64>(*area);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        lea_imm:
        {
            G(opers->imm.dst) = PC + s32(opers->imm.src);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        lea_offs:
        {
            G(opers->offset.dst) = G(opers->offset.src) + s32(opers->offset.offs);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        move_imm:
        {
            G(opers->imm.dst) = opers->imm.src;
            flags->v = false;
            flags->c = false;
            flags->z = opers->imm.src == 0;
            flags->n = s32(opers->imm.src) < 0;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        move_reg2:
        {
            auto src = G(opers->reg2.src);
            G(opers->reg2.dst) = src;
            flags->v = false;
            flags->c = false;
            flags->z = src == 0;
            flags->n = s32(src) < 0;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        load_reg2:
        {
            auto src = HU(G(opers->reg2.src));
            G(opers->reg2.dst) = src;
            flags->v = false;
            flags->c = false;
            flags->z = src == 0;
            flags->n = s32(src) < 0;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        load_offset:
        {
            auto src = HU(G(opers->offset.src) + opers->offset.offs);
            G(opers->offset.dst) = src;
            flags->v = false;
            flags->c = false;
            flags->z = src == 0;
            flags->n = s32(src) < 0;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        load_indexed:
        {
            const auto ndx_offs = G(opers->indexed.ndx) * sizeof(u64);
            auto src = HU(G(opers->indexed.base) + ndx_offs + opers->indexed.offs);
            G(opers->indexed.dst) = src;
            flags->v = false;
            flags->c = false;
            flags->z = src == 0;
            flags->n = s32(src) < 0;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        store_reg2:
        {
            auto src = G(opers->reg2.src);
            HU(G(opers->reg2.dst)) = src;
            flags->v = false;
            flags->c = false;
            flags->z = src == 0;
            flags->n = s32(src) < 0;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        store_offset:
        {
            auto src = G(opers->offset.src);
            HU(G(opers->offset.dst) + opers->offset.offs) = src;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        store_indexed:
        {
            auto src = G(opers->indexed.dst);
            const auto ndx_offs = G(opers->indexed.ndx) * sizeof(u64);
            HU(G(opers->indexed.base) + ndx_offs + opers->indexed.offs) = src;
            flags->v = false;
            flags->c = false;
            flags->z = src == 0;
            flags->n = s32(src) < 0;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        exit_imm:
        {
            auto success = b8(opers->imm.src);
            if (!success)
                status = status_t::fail;
            vm.exited = true;
            return status;
        }
        exit_reg1:
        {
            auto success = b8(G(opers->reg1.dst));
            if (!success)
                status = status_t::fail;
            vm.exited = true;
            return status;
        }
        error:
        {
            return status_t::fail;
        }
    }

    status_t init(vm_t& vm, alloc_t* alloc) {
        vm.alloc = alloc;
        hashtab::init(vm.traptab, vm.alloc);
        stable_array::init(vm.mem_map, vm.alloc);
        vm.reg_file = add_mem_area(vm,
                                   mem_area_type_t::register_file,
                                   register_file::none,
                                   vm.alloc,
                                   register_file::max);
        vm.reg_file->size = register_file::max;
        return status_t::ok;
    }

    mem_area_t* get_mem_area_by_reg(vm_t& vm, reg_t reg) {
        return vm.area_by_reg[reg];
    }

    mem_area_t* get_mem_area_by_type(vm_t& vm, mem_area_type_t type) {
        return vm.area_by_type[u32(type)];
    }
}
