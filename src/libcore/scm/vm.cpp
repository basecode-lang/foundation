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
            [hash]       = "HASH"_ss,
            [functor]    = "FUNCTOR"_ss,
        };

        str::slice_t name(u8 type) {
            return s_names[u32(type)];
        }
    }

    namespace instruction {
        namespace type {
            static str::slice_t s_names[] = {
                [nop]       = "NOP"_ss,
                [add]       = "ADD"_ss,
                [mul]       = "MUL"_ss,
                [sub]       = "SUB"_ss,
                [div]       = "DIV"_ss,
                [pow]       = "POW"_ss,
                [mod]       = "MOD"_ss,
                [neg]       = "NEG"_ss,
                [not_]      = "NOT"_ss,
                [shl]       = "SHL"_ss,
                [shr]       = "SHR"_ss,
                [or_]       = "OR"_ss,
                [and_]      = "AND"_ss,
                [xor_]      = "XOR"_ss,
                [br]        = "BR"_ss,
                [blr]       = "BLR"_ss,
                [cmp]       = "CMP"_ss,
                [beq]       = "BEQ"_ss,
                [bne]       = "BNE"_ss,
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
                [pop]       = "POP"_ss,
                [get]       = "GET"_ss,
                [set]       = "SET"_ss,
                [push]      = "PUSH"_ss,
                [move]      = "MOVE"_ss,
                [load]      = "LOAD"_ss,
                [store]     = "STORE"_ss,
                [exit]      = "EXIT"_ss,
                [trap]      = "TRAP"_ss,
                [lea]       = "LEA"_ss,
                [bra]       = "BRA"_ss,
                [car]       = "CAR"_ss,
                [cdr]       = "CDR"_ss,
                [setcar]    = "SETCAR"_ss,
                [setcdr]    = "SETCDR"_ss,
                [fix]       = "FIX"_ss,
                [flo]       = "FLO"_ss,
                [cons]      = "CONS"_ss,
                [env]       = "ENV"_ss,
                [type]      = "TYPE"_ss,
                [list]      = "LIST"_ss,
                [eval]      = "EVAL"_ss,
                [error]     = "ERROR"_ss,
                [write]     = "WRITE"_ss,
                [qt]        = "QT"_ss,
                [qq]        = "QQ"_ss,
                [collect]   = "COLLECT"_ss,
                [apply]     = "APPLY"_ss,
                [const_]    = "CONST"_ss,
                [ladd]      = "LADD"_ss,
                [lsub]      = "LSUB"_ss,
                [lmul]      = "LMUL"_ss,
                [ldiv]      = "LDIV"_ss,
                [lmod]      = "LMOD"_ss,
                [lnot]      = "LNOT"_ss,
                [pairp]     = "PAIRP"_ss,
                [symp]      = "SYMP"_ss,
                [atomp]     = "ATOMP"_ss,
                [truep]     = "TRUEP"_ss,
                [falsep]    = "FALSEP"_ss,
                [lcmp]      = "LCMP"_ss,
                [clc]       = "CLC"_ss,
                [sec]       = "SEC"_ss,
                [read]      = "READ"_ss,
                [define]    = "DEFINE"_ss,
            };

            str::slice_t name(reg_t op) {
                return s_names[u32(op)];
            }
        }
    }

    namespace register_file {
        static str::slice_t s_names[] = {
            [none]  = "NONE"_ss,
            [pc]    = "PC"_ss,
            [gp]    = "GP"_ss,
            [ep]    = "EP"_ss,
            [dp]    = "DP"_ss,
            [hp]    = "HP"_ss,
            [sp]    = "SP"_ss,
            [fp]    = "FP"_ss,
            [m]     = "M"_ss,
            [f]     = "F"_ss,
            [lr]    = "LR"_ss,
            [r0]    = "R0"_ss,
            [r1]    = "R1"_ss,
            [r2]    = "R2"_ss,
            [r3]    = "R3"_ss,
            [r4]    = "R4"_ss,
            [r5]    = "R5"_ss,
            [r6]    = "R6"_ss,
            [r7]    = "R7"_ss,
            [r8]    = "R8"_ss,
            [r9]    = "R9"_ss,
            [r10]   = "R10"_ss,
            [r11]   = "R11"_ss,
            [r12]   = "R12"_ss,
            [r13]   = "R13"_ss,
            [r14]   = "R14"_ss,
            [r15]   = "R15"_ss,
        };

        str::slice_t name(reg_t reg) {
            return s_names[u32(reg)];
        }
    }

    constexpr u8 op_nop             = 0;
    constexpr u8 op_add_imm         = 1;
    constexpr u8 op_add_reg2        = 2;
    constexpr u8 op_adds_imm        = 3;
    constexpr u8 op_adds_reg2       = 4;
    constexpr u8 op_mul_imm         = 5;
    constexpr u8 op_mul_reg2        = 6;
    constexpr u8 op_muls_imm        = 7;
    constexpr u8 op_muls_reg2       = 8;
    constexpr u8 op_sub_imm         = 9;
    constexpr u8 op_sub_reg2        = 10;
    constexpr u8 op_subs_imm        = 11;
    constexpr u8 op_subs_reg2       = 12;
    constexpr u8 op_div_imm         = 13;
    constexpr u8 op_div_reg2        = 14;
    constexpr u8 op_pow_imm         = 15;
    constexpr u8 op_pow_reg2        = 16;
    constexpr u8 op_mod_imm         = 17;
    constexpr u8 op_mod_reg2        = 18;
    constexpr u8 op_neg_reg1        = 19;
    constexpr u8 op_not_reg1        = 20;
    constexpr u8 op_shl_imm         = 21;
    constexpr u8 op_shl_reg2        = 22;
    constexpr u8 op_shr_imm         = 23;
    constexpr u8 op_shr_reg2        = 24;
    constexpr u8 op_or_imm          = 25;
    constexpr u8 op_or_reg2         = 26;
    constexpr u8 op_and_imm         = 27;
    constexpr u8 op_and_reg2        = 28;
    constexpr u8 op_xor_imm         = 29;
    constexpr u8 op_xor_reg2        = 30;
    constexpr u8 op_br_imm          = 31;
    constexpr u8 op_br_reg1         = 32;
    constexpr u8 op_blr_imm         = 33;
    constexpr u8 op_blr_reg1        = 34;
    constexpr u8 op_cmp_imm         = 35;
    constexpr u8 op_cmp_reg2        = 36;
    constexpr u8 op_cmps_imm        = 37;
    constexpr u8 op_cmps_reg2       = 38;
    constexpr u8 op_beq_imm         = 39;
    constexpr u8 op_beqs_imm        = 40;
    constexpr u8 op_bne_imm         = 41;
    constexpr u8 op_bnes_imm        = 42;
    constexpr u8 op_bl_imm          = 43;
    constexpr u8 op_bls_imm         = 44;
    constexpr u8 op_ble_imm         = 45;
    constexpr u8 op_bles_imm        = 46;
    constexpr u8 op_bg_imm          = 47;
    constexpr u8 op_bgs_imm         = 48;
    constexpr u8 op_bge_imm         = 49;
    constexpr u8 op_bges_imm        = 50;
    constexpr u8 op_seq_reg1        = 51;
    constexpr u8 op_seqs_reg1       = 52;
    constexpr u8 op_sne_reg1        = 53;
    constexpr u8 op_snes_reg1       = 54;
    constexpr u8 op_sl_reg1         = 55;
    constexpr u8 op_sls_reg1        = 56;
    constexpr u8 op_sle_reg1        = 57;
    constexpr u8 op_sles_reg1       = 58;
    constexpr u8 op_sg_reg1         = 59;
    constexpr u8 op_sgs_reg1        = 60;
    constexpr u8 op_sge_reg1        = 61;
    constexpr u8 op_sges_reg1       = 62;
    constexpr u8 op_ret_reg1        = 63;
    constexpr u8 op_mma_imm         = 64;
    constexpr u8 op_pop_reg2        = 65;
    constexpr u8 op_get_reg2        = 66;
    constexpr u8 op_set_reg2        = 67;
    constexpr u8 op_push_imm        = 68;
    constexpr u8 op_push_reg2       = 69;
    constexpr u8 op_move_imm        = 70;
    constexpr u8 op_move_reg2       = 71;
    constexpr u8 op_load_reg2       = 72;
    constexpr u8 op_load_offs       = 73;
    constexpr u8 op_load_idx        = 74;
    constexpr u8 op_store_reg2      = 75;
    constexpr u8 op_store_offs      = 76;
    constexpr u8 op_store_idx       = 77;
    constexpr u8 op_exit_imm        = 78;
    constexpr u8 op_exit_reg1       = 79;
    constexpr u8 op_trap_imm        = 80;
    constexpr u8 op_trap_reg2       = 81;
    constexpr u8 op_lea_imm         = 82;
    constexpr u8 op_lea_offs        = 109;
    constexpr u8 op_bra_imm         = 83;
    constexpr u8 op_bra_reg1        = 84;
    constexpr u8 op_car_reg2        = 85;
    constexpr u8 op_cdr_reg2        = 86;
    constexpr u8 op_setcar_reg2     = 87;
    constexpr u8 op_setcdr_reg2     = 88;
    constexpr u8 op_fix_reg2        = 89;
    constexpr u8 op_fix_imm2        = 90;
    constexpr u8 op_flo_reg2        = 91;
    constexpr u8 op_flo_imm2        = 92;
    constexpr u8 op_cons_reg3       = 93;
    constexpr u8 op_env_reg2        = 94;
    constexpr u8 op_type_reg2       = 95;
    constexpr u8 op_list_reg2_imm   = 96;
    constexpr u8 op_eval_reg2       = 97;
    constexpr u8 op_error_reg2      = 98;
    constexpr u8 op_write_imm2      = 99;
    constexpr u8 op_write_reg2      = 100;
    constexpr u8 op_qt_reg2         = 101;
    constexpr u8 op_qq_reg2         = 102;
    constexpr u8 op_clc             = 103;
    constexpr u8 op_sec             = 104;
    constexpr u8 op_collect         = 105;
    constexpr u8 op_apply_reg2      = 106;
    constexpr u8 op_const_reg2      = 107;
    constexpr u8 op_const_imm2      = 108;
    constexpr u8 op_ladd_reg2_imm   = 110;
    constexpr u8 op_lsub_reg2_imm   = 111;
    constexpr u8 op_lmul_reg2_imm   = 112;
    constexpr u8 op_ldiv_reg2_imm   = 113;
    constexpr u8 op_lmod_reg2_imm   = 114;
    constexpr u8 op_lnot_reg2       = 115;
    constexpr u8 op_pairp_reg2      = 116;
    constexpr u8 op_listp_reg2      = 117;
    constexpr u8 op_symp_reg2       = 118;
    constexpr u8 op_atomp_reg2      = 119;
    constexpr u8 op_truep_reg1      = 120;
    constexpr u8 op_falsep_reg1     = 121;
    constexpr u8 op_lcmp_reg2       = 122;
    constexpr u8 op_lnot_reg1       = 123;
    constexpr u8 op_get_imm         = 124;
    constexpr u8 op_set_imm         = 125;
    constexpr u8 op_read_reg2       = 126;
    constexpr u8 op_define_imm      = 127;
    constexpr u8 op_define_reg2     = 128;
    constexpr u8 op_error           = 255;

    static u8 s_op_decode[][2][9] = {
        //  none            imm             reg1            reg2            reg3            reg4            offset          indexed             reg2_imm
        [instruction::type::nop] = {
            {op_nop,        op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_nop,        op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
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
        [instruction::type::br] = {
            {op_error,      op_br_imm,      op_br_reg1,     op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::blr] = {
            {op_error,      op_blr_imm,     op_blr_reg1,    op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
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
        [instruction::type::pop] = {
            {op_error,      op_error,       op_error,       op_pop_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::get] = {
            {op_error,      op_get_imm,     op_error,       op_get_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::set] = {
            {op_error,      op_set_imm,     op_set_reg2,    op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::push] = {
            {op_error,      op_push_imm,    op_error,       op_push_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
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
        [instruction::type::trap] = {
            {op_error,      op_trap_imm,    op_error,       op_trap_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::lea] = {
            {op_error,      op_lea_imm,     op_error,       op_error,       op_error,       op_error,       op_lea_offs,    op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::bra] = {
            {op_error,      op_bra_imm,     op_bra_reg1,    op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::car] = {
            {op_error,      op_error,       op_error,       op_car_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::cdr] = {
            {op_error,      op_error,       op_error,       op_cdr_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::setcar] = {
            {op_error,      op_error,       op_error,       op_setcar_reg2, op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::setcdr] = {
            {op_error,      op_error,       op_error,       op_setcdr_reg2, op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::fix] = {
            {op_error,      op_fix_imm2,    op_error,       op_fix_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::flo] = {
            {op_error,      op_flo_imm2,    op_error,       op_flo_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::cons] = {
            {op_error,      op_error,       op_error,       op_error,       op_cons_reg3,   op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::env] = {
            {op_error,      op_error,       op_error,       op_env_reg2,    op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::type] = {
            {op_error,      op_error,       op_error,       op_type_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::list] = {
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_list_reg2_imm},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::eval] = {
            {op_error,      op_error,       op_error,       op_eval_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::error] = {
            {op_error,      op_error,       op_error,       op_error_reg2,  op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::write] = {
            {op_error,      op_write_imm2,  op_error,       op_write_reg2,  op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::qt] = {
            {op_error,      op_error,       op_error,       op_qt_reg2,     op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::qq] = {
            {op_error,      op_error,       op_error,       op_qq_reg2,     op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::apply] = {
            {op_error,      op_error,       op_error,       op_apply_reg2,  op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::const_] = {
            {op_error,      op_const_imm2,  op_error,       op_const_reg2,  op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::ladd] = {
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_ladd_reg2_imm},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::lsub] = {
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_lsub_reg2_imm},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::lmul] = {
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_lmul_reg2_imm},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::ldiv] = {
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_ldiv_reg2_imm},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::lmod] = {
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_lmod_reg2_imm},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::lnot] = {
            {op_error,      op_error,       op_lnot_reg1,   op_lnot_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::pairp] = {
            {op_error,      op_error,       op_error,       op_pairp_reg2,  op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::listp] = {
            {op_error,      op_error,       op_error,       op_listp_reg2,  op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::symp] = {
            {op_error,      op_error,       op_error,       op_symp_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::atomp] = {
            {op_error,      op_error,       op_error,       op_atomp_reg2,  op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::truep] = {
            {op_error,      op_error,       op_truep_reg1,  op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::falsep] = {
            {op_error,      op_error,       op_falsep_reg1, op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::lcmp] = {
            {op_error,      op_error,       op_error,       op_lcmp_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::clc] = {
            {op_clc,        op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::sec] = {
            {op_sec,        op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::collect] = {
            {op_collect,    op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::read] = {
            {op_error,      op_error,       op_error,       op_read_reg2,   op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::define] = {
            {op_error,      op_define_imm,  op_error,       op_define_reg2, op_error,       op_error,       op_error,       op_error,           op_error},
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
                G(area.reg) = mem_area::base_addr(area);
            }
        }

        u0 shrink_to_size(mem_area_t& area, u32 new_size) {
            auto& vm = *area.vm;
            area.size = new_size;
            G(area.reg) = base_addr(area);
        }
    }

    u0 free(vm_t& vm) {
        for (auto& area : vm.mem_map)
            mem_area::free(area);
        array::free(vm.mem_map);
        hashtab::free(vm.traptab);
    }

    u0 reset(vm_t& vm) {
        for (auto& area : vm.mem_map) {
            if (area.type != mem_area_type_t::register_file)
                mem_area::reset(area, true);
            if (area.reg != register_file::none)
                G(area.reg) = mem_area::base_addr(area);
        }
        F  = 0;
        M  = 0;
        PC = 0;
    }

    mem_area_t& add_mem_area(vm_t& vm,
                             mem_area_type_t type,
                             reg_t reg,
                             alloc_t* alloc,
                             u32 min_capacity,
                             b8 top) {
        auto& area = array::append(vm.mem_map);
        mem_area::init(area,
                       &vm,
                       vm.mem_map.size,
                       type,
                       reg,
                       alloc,
                       min_capacity,
                       top);
        if (area.reg != register_file::none) {
            vm.area_by_reg[area.reg] = area.id;
        }
        return area;
    }

    status_t step(vm_t& vm, ctx_t* ctx, s32 cycles) {
        static u0* s_microcode[] = {
            [op_nop]                = &&nop,
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
            [op_not_reg1]           = &&not_reg1,
            [op_shl_imm]            = &&shl_imm,
            [op_shl_reg2]           = &&shl_reg2,
            [op_shr_imm]            = &&shr_imm,
            [op_shr_reg2]           = &&shr_reg2,
            [op_or_imm]             = &&or_imm,
            [op_or_reg2]            = &&or_reg2,
            [op_and_imm]            = &&and_imm,
            [op_and_reg2]           = &&and_reg2,
            [op_xor_imm]            = &&xor_imm,
            [op_xor_reg2]           = &&xor_reg2,
            [op_br_imm]             = &&br_imm,
            [op_br_reg1]            = &&br_reg1,
            [op_blr_imm]            = &&blr_imm,
            [op_blr_reg1]           = &&blr_reg1,
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
            [op_pop_reg2]           = &&pop_reg2,
            [op_get_reg2]           = &&get_reg2,
            [op_set_reg2]           = &&set_reg2,
            [op_push_imm]           = &&push_imm,
            [op_push_reg2]          = &&push_reg2,
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
            [op_trap_imm]           = &&trap_imm,
            [op_trap_reg2]          = &&trap_reg2,
            [op_lea_imm]            = &&lea_imm,
            [op_lea_offs]           = &&lea_offs,
            [op_bra_imm]            = &&bra_imm,
            [op_bra_reg1]           = &&bra_reg1,
            [op_car_reg2]           = &&car_reg2,
            [op_cdr_reg2]           = &&cdr_reg2,
            [op_setcar_reg2]        = &&setcar_reg2,
            [op_setcdr_reg2]        = &&setcdr_reg2,
            [op_fix_imm2]           = &&fix_imm2,
            [op_fix_reg2]           = &&fix_reg2,
            [op_flo_imm2]           = &&flo_imm2,
            [op_flo_reg2]           = &&flo_reg2,
            [op_cons_reg3]          = &&cons_reg3,
            [op_env_reg2]           = &&env_reg2,
            [op_type_reg2]          = &&type_reg2,
            [op_list_reg2_imm]      = &&list_reg2_imm,
            [op_eval_reg2]          = &&eval_reg2,
            [op_error_reg2]         = &&error_reg2,
            [op_write_imm2]         = &&write_imm2,
            [op_write_reg2]         = &&write_reg2,
            [op_qt_reg2]            = &&qt_reg2,
            [op_qq_reg2]            = &&qq_reg2,
            [op_apply_reg2]         = &&apply_reg2,
            [op_const_reg2]         = &&const_reg2,
            [op_const_imm2]         = &&const_imm2,
            [op_ladd_reg2_imm]      = &&ladd_reg2_imm,
            [op_lsub_reg2_imm]      = &&lsub_reg2_imm,
            [op_lmul_reg2_imm]      = &&lmul_reg2_imm,
            [op_ldiv_reg2_imm]      = &&ldiv_reg2_imm,
            [op_lmod_reg2_imm]      = &&lmod_reg2_imm,
            [op_lnot_reg1]          = &&lnot_reg1,
            [op_lnot_reg2]          = &&lnot_reg2,
            [op_pairp_reg2]         = &&pairp_reg2,
            [op_listp_reg2]         = &&listp_reg2,
            [op_symp_reg2]          = &&symp_reg2,
            [op_atomp_reg2]         = &&atomp_reg2,
            [op_truep_reg1]         = &&truep_reg1,
            [op_falsep_reg1]        = &&falsep_reg1,
            [op_lcmp_reg2]          = &&lcmp_reg2,
            [op_get_imm]            = &&get_imm,
            [op_set_imm]            = &&set_imm,
            [op_clc]                = &&clc,
            [op_sec]                = &&sec,
            [op_collect]            = &&collect,
            [op_read_reg2]          = &&read_reg2,
            [op_define_imm]         = &&define_imm,
            [op_define_reg2]        = &&define_reg2,
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
        collect:
        {
            collect_garbage(ctx);
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
        br_imm:
        {
            PC += s64(opers->imm.src);
            EXEC_NEXT();
        }
        br_reg1:
        {
            PC += s64(G(opers->reg1.dst));
            EXEC_NEXT();
        }
        bra_imm:
        {
            PC = u64(opers->imm.src);
            EXEC_NEXT();
        }
        bra_reg1:
        {
            PC = G(opers->reg1.dst);
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
            auto& area = vm.mem_map[opers->imm.src];
            G(opers->imm.dst) = mem_area::base_addr(area);
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
        fix_imm2:
        {
            G(opers->imm.dst) = u64(make_fixnum(ctx, G(opers->imm.src)));
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        fix_reg2:
        {
            G(opers->reg2.dst) = u64(make_fixnum(ctx, G(opers->reg2.src)));
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        flo_imm2:
        {
            G(opers->imm.dst) = u64(make_flonum(ctx, f32(G(opers->imm.src))));
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        flo_reg2:
        {
            G(opers->reg2.dst) = u64(make_flonum(ctx, f32(G(opers->reg2.src))));
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        cons_reg3:
        {
            auto c = cons(ctx,
                          (obj_t*) G(opers->reg3.a),
                          (obj_t*) G(opers->reg3.b));
            flags->c = false;
            flags->z = !IS_NIL(c);
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg3.c) = u64(c);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        env_reg2:
        {
            auto curr_env = (obj_t*) HU(G(opers->reg2.src));
            G(opers->reg2.dst) = u64(make_environment(ctx, curr_env));
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        type_reg2:
        {
            auto t = TYPE((obj_t*) G(opers->reg2.src));
            flags->c = false;
            flags->z = t != obj_type_t::nil;
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg2.dst) = u64(t);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        list_reg2_imm:
        {
            auto base  = G(opers->reg2_imm.a);
            auto size  = s32(opers->reg2_imm.imm);
            auto count = size / sizeof(u64);
            auto offs  = size - sizeof(u64);
            auto lst   = ctx->nil;
            auto gc    = save_gc(ctx);
            for (u32 i = count - 1; i >= 0; --i) {
                lst = cons(ctx, (obj_t*) HU(base + offs), lst);
                offs -= sizeof(u64);
            }
            restore_gc(ctx, gc);
            push_gc(ctx, lst);
            G(opers->reg2_imm.b) = u64(lst);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        error_reg2:
        {
            G(opers->reg2.dst) = u64(make_error(ctx,
                                                (obj_t*) G(opers->reg2.src),
                                                ctx->call_list));
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        read_reg2:
        {
            // XXX: FIXME
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        write_imm2:
        {
            auto m = G(opers->imm.src);
            switch (m) {
                case 0:
                    write(stdout, ctx, (obj_t*) G(opers->imm.dst));
                    break;
                case 1:
                    print(stdout, ctx, (obj_t*) G(opers->imm.dst));
                    break;
                case 2:
                    break;
                default:
                    break;
            }
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        write_reg2:
        {
            auto m = G(opers->reg2.src);
            switch (m) {
                case 0:
                    write(stdout, ctx, (obj_t*) G(opers->reg2.dst));
                    break;
                case 1:
                    print(stdout, ctx, (obj_t*) G(opers->reg2.dst));
                    break;
                case 2:
                    break;
                default:
                    break;
            }
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        qt_reg2:
        {
            auto v = (obj_t*) G(opers->reg2.src);
            flags->c = false;
            flags->z = !IS_NIL(v);
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg2.dst) = u64(CDR(v));
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        qq_reg2:
        {
            auto v = quasiquote(ctx, (obj_t*) G(opers->reg2.src));
            flags->c = false;
            flags->z = !IS_NIL(v);
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg2.dst) = u64(v);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        eval_reg2:
        {
            // XXX:
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        apply_reg2:
        {
            // XXX:
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        const_imm2:
        {
            auto v = OBJ_AT(G(opers->imm.src));
            flags->c = false;
            flags->z = !IS_NIL(v);
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->imm.dst) = u64(v);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        const_reg2:
        {
            auto v = OBJ_AT(G(opers->reg2.src));
            flags->c = false;
            flags->z = !IS_NIL(v);
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg2.dst) = u64(v);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        car_reg2:
        {
            G(opers->reg2.dst) = u64(CAR((obj_t*) G(opers->reg2.src)));
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        cdr_reg2:
        {
            G(opers->reg2.dst) = u64(CDR((obj_t*) G(opers->reg2.src)));
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        setcar_reg2:
        {
            SET_CAR((obj_t*) G(opers->reg2.dst), (obj_t*) G(opers->reg2.src));
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        setcdr_reg2:
        {
            SET_CDR((obj_t*) G(opers->reg2.dst), (obj_t*) G(opers->reg2.src));
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        get_imm:
        {
            auto key = (obj_t*) OBJ_AT(G(opers->imm.src));
            auto v   = get(ctx, key);
            flags->c = false;
            flags->z = !IS_NIL(v);
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->imm.dst) = u64(v);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        get_reg2:
        {
            auto v = get(ctx, (obj_t*) G(opers->reg2.src));
            flags->c = false;
            flags->z = !IS_NIL(v);
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg2.dst) = u64(v);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        define_imm:
        {
            define(ctx, (obj_t*) G(opers->imm.dst), OBJ_AT(G(opers->imm.src)));
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        define_reg2:
        {
            define(ctx, (obj_t*) G(opers->reg2.dst), (obj_t*) G(opers->reg2.src));
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        set_imm:
        {
            set(ctx, (obj_t*) G(opers->imm.dst), OBJ_AT(G(opers->imm.src)));
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        set_reg2:
        {
            set(ctx, (obj_t*) G(opers->reg2.dst), (obj_t*) G(opers->reg2.src));
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        lnot_reg1:
        {
            auto v = (obj_t*) G(opers->reg1.dst);
            flags->c = false;
            flags->z = !IS_TRUE(v);
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg1.dst) = u64(flags->z ? ctx->true_ : ctx->false_);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        lnot_reg2:
        {
            auto v = (obj_t*) G(opers->reg2.src);
            flags->c = false;
            flags->z = !IS_TRUE(v);
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg2.dst) = u64(flags->z ? ctx->true_ : ctx->false_);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        truep_reg1:
        {
            auto r = G(opers->reg1.dst);
            auto v = r == 1 ? ctx->true_ : (obj_t*) r;
            flags->c = false;
            flags->z = IS_TRUE(v);
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg1.dst) = u64(flags->z ? ctx->true_ : ctx->false_);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        falsep_reg1:
        {
            auto r = G(opers->reg1.dst);
            auto v = r == 0 ? ctx->false_ : (obj_t*) r;
            flags->c = false;
            flags->z = IS_FALSE(v);
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg1.dst) = u64(flags->z ? ctx->true_ : ctx->false_);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        pairp_reg2:
        {
            auto v = (obj_t*) G(opers->reg2.src);
            flags->c = false;
            flags->z = TYPE(v) == obj_type_t::pair;
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg2.dst) = u64(flags->z ? ctx->true_ : ctx->false_);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        listp_reg2:
        {
            auto v = (obj_t*) G(opers->reg2.src);
            flags->c = false;
            flags->z = is_list(ctx, v);
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg2.dst) = u64(flags->z ? ctx->true_ : ctx->false_);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        symp_reg2:
        {
            auto v = (obj_t*) G(opers->reg2.src);
            flags->c = false;
            flags->z = TYPE(v) == obj_type_t::symbol;
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg2.dst) = u64(flags->z ? ctx->true_ : ctx->false_);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        atomp_reg2:
        {
            auto v = (obj_t*) G(opers->reg2.src);
            flags->c = false;
            flags->z = TYPE(v) != obj_type_t::pair;
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg2.dst) = u64(flags->z ? ctx->true_ : ctx->false_);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        ladd_reg2_imm:
        {
            auto base = G(opers->reg2_imm.a);
            auto size = s32(opers->reg2_imm.imm) / sizeof(u64);
            auto offs = 0;
            auto acc  = to_flonum((obj_t*) HU(base + offs));
            auto gc   = save_gc(ctx);
            base += sizeof(u64);
            for (u32 i = 0; i < size - 1; ++i) {
                acc += to_flonum((obj_t*) HU(base + offs));
                offs += sizeof(u64);
            }
            restore_gc(ctx, gc);
            auto res = make_flonum(ctx, acc);
            push_gc(ctx, res);
            G(opers->reg2_imm.b) = u64(res);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        lsub_reg2_imm:
        {
            auto base = G(opers->reg2_imm.a);
            auto size = s32(opers->reg2_imm.imm) / sizeof(u64);
            auto offs = 0;
            auto acc  = to_flonum((obj_t*) HU(base + offs));
            auto gc   = save_gc(ctx);
            base += sizeof(u64);
            for (u32 i = 0; i < size - 1; ++i) {
                acc -= to_flonum((obj_t*) HU(base + offs));
                offs += sizeof(u64);
            }
            restore_gc(ctx, gc);
            auto res = make_flonum(ctx, acc);
            push_gc(ctx, res);
            G(opers->reg2_imm.b) = u64(res);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        lmul_reg2_imm:
        {
            auto base = G(opers->reg2_imm.a);
            auto size = s32(opers->reg2_imm.imm) / sizeof(u64);
            auto offs = 0;
            auto acc  = to_flonum((obj_t*) HU(base + offs));
            auto gc   = save_gc(ctx);
            base += sizeof(u64);
            for (u32 i = 0; i < size - 1; ++i) {
                acc *= to_flonum((obj_t*) HU(base + offs));
                offs += sizeof(u64);
            }
            restore_gc(ctx, gc);
            auto res = make_flonum(ctx, acc);
            push_gc(ctx, res);
            G(opers->reg2_imm.b) = u64(res);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        ldiv_reg2_imm:
        {
            auto base = G(opers->reg2_imm.a);
            auto size = s32(opers->reg2_imm.imm) / sizeof(u64);
            auto offs = 0;
            auto acc  = to_flonum((obj_t*) HU(base + offs));
            auto gc   = save_gc(ctx);
            base += sizeof(u64);
            for (u32 i = 0; i < size - 1; ++i) {
                acc /= to_flonum((obj_t*) HU(base + offs));
                offs += sizeof(u64);
            }
            restore_gc(ctx, gc);
            auto res = make_flonum(ctx, acc);
            push_gc(ctx, res);
            G(opers->reg2_imm.b) = u64(res);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        lmod_reg2_imm:
        {
            auto base = G(opers->reg2_imm.a);
            auto size = s32(opers->reg2_imm.imm) / sizeof(u64);
            auto offs = 0;
            auto acc  = to_fixnum((obj_t*) HU(base + offs));
            auto gc   = save_gc(ctx);
            base += sizeof(u64);
            for (u32 i = 0; i < size - 1; ++i) {
                acc %= to_fixnum((obj_t*) HU(base + offs));
                offs += sizeof(u64);
            }
            restore_gc(ctx, gc);
            auto res = make_fixnum(ctx, acc);
            push_gc(ctx, res);
            G(opers->reg2_imm.b) = u64(res);
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        lcmp_reg2:
        {
            auto cmp = scm::compare(ctx,
                                    (obj_t*) G(opers->reg2.src),
                                    (obj_t*) G(opers->reg2.src));
            flags->c = cmp > 0;
            flags->z = cmp == 0;
            flags->n = cmp < 0;
            flags->i = false;
            flags->v = false;
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
        trap_imm:
        {
            auto id       = opers->imm.src;
            auto arg      = G(opers->imm.dst);
            auto callback = hashtab::find(vm.traptab, u32(id));
            status = callback && callback(vm, arg) ? status_t::ok : status_t::fail;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        trap_reg2:
        {
            auto id       = G(opers->reg2.src);
            auto arg      = G(opers->reg2.dst);
            auto callback = hashtab::find(vm.traptab, u32(id));
            status = callback && callback(vm, arg) ? status_t::ok : status_t::fail;
            PC += sizeof(encoded_inst_t);
            EXEC_NEXT();
        }
        error:
        {
            return status_t::fail;
        }
    }

    status_t init(vm_t& vm, alloc_t* alloc) {
        vm.alloc = alloc;
        array::init(vm.mem_map, vm.alloc);
        hashtab::init(vm.traptab, vm.alloc);
        vm.reg_file = &add_mem_area(vm,
                                    mem_area_type_t::register_file,
                                    register_file::none,
                                    vm.alloc,
                                    register_file::max);
        vm.reg_file->size = register_file::max;
        return status_t::ok;
    }

    mem_area_t* get_mem_area(vm_t& vm, u32 id) {
        return id == 0 || id > vm.mem_map.size ? nullptr : &vm.mem_map[id - 1];
    }

    mem_area_t* get_mem_area_by_reg(vm_t& vm, reg_t reg) {
        const auto id = vm.area_by_reg[reg];
        return get_mem_area(vm, id);
    }
}
