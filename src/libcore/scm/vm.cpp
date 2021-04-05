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
#include <basecode/core/scm/scm.h>
#include <basecode/core/scm/bytecode.h>

#define EXEC_NEXT()             SAFE_SCOPE(                                         \
    if (cycles > 0)     --cycles;                                                   \
    if (cycles == 0)    return status;                                              \
    env   = (obj_t*) H(G(register_file::ep));                                       \
    inst  = (instruction_t*) &H(PC);                                                \
    data  = inst->data;                                                             \
    flags = (flag_register_t*) &F;                                                  \
    opers = (operand_encoding_t*) &data;                                            \
    goto *s_microcode[s_op_decode[inst->type][inst->is_signed][inst->encoding]];)

namespace basecode::scm::vm {
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
    constexpr u8 op_gc              = 103;
    constexpr u8 op_gc_reg1         = 104;
    constexpr u8 op_gc_reg2         = 105;
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
        [instruction::type::gc] = {
            {op_gc,         op_error,       op_gc_reg1,     op_gc_reg2,     op_error,       op_error,       op_error,       op_error,           op_error},
            {op_error,      op_error,       op_error,       op_error,       op_error,       op_error,       op_error,       op_error,           op_error},
        },
        [instruction::type::apply] = {
            {op_gc,         op_error,       op_error,       op_apply_reg2,  op_error,       op_error,       op_error,       op_error,           op_error},
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
    };

    u0 free(vm_t& vm) {
        hashtab::free(vm.traptab);
        memory::free(vm.alloc, vm.heap);
    }

    u0 reset(vm_t& vm) {
        std::memset(vm.memory_map.reg_to_entry, -1, sizeof(s32) * 32);
        u64 addr = 0;
        for (u32 i = 0; i < max_memory_areas; ++i) {
            auto& area = vm.memory_map.entries[i];
            if (!area.valid)
                continue;
            u64 end_addr = addr + area.size;
            area.offs  = addr;
            if (area.top) {
                area.addr = end_addr;
            } else {
                area.addr = addr > 0 ? addr - 1 : addr;
            }
            G(area.reg) = area.addr;
            vm.memory_map.reg_to_entry[area.reg] = i;
            addr = end_addr;
        }

        M  = 0;
        LR = 0;
        PC = LP;
    }

    u32 heap_top(vm_t& vm) {
        return vm.memory_map.heap_size - 1;
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
            [op_gc]                 = &&gc,
            [op_gc_reg1]            = &&gc_reg1,
            [op_gc_reg2]            = &&gc_reg2,
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
        u64                 data;
        u64                 carry_out;
        obj_t*              env;
        instruction_t*      inst;
        flag_register_t*    flags;
        operand_encoding_t* opers;
        status_t            status      {};

        EXEC_NEXT();

        nop:
        {
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        add_imm:
        {
            auto sum = __builtin_addcll(G(opers->imm.dst),
                                        opers->imm.src,
                                        0,
                                        &carry_out);
            flags->z = sum == 0;
            flags->c = carry_out > 0;
            G(opers->imm.dst) = sum;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        add_reg2:
        {
            auto sum = __builtin_addcll(G(opers->reg2.dst),
                                        G(opers->reg2.src),
                                        0,
                                        &carry_out);
            flags->z = sum == 0;
            flags->c = carry_out > 0;
            G(opers->reg2.dst) = sum;
            PC += sizeof(instruction_t);
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
            G(opers->imm.dst) = sum;
            PC += sizeof(instruction_t);
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
            G(opers->reg2.dst) = sum;
            PC += sizeof(instruction_t);
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
            G(opers->imm.dst) = prod;
            PC += sizeof(instruction_t);
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
            G(opers->reg2.dst) = prod;
            PC += sizeof(instruction_t);
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
            G(opers->imm.dst) = prod;
            PC += sizeof(instruction_t);
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
            G(opers->reg2.dst) = prod;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        sub_imm:
        {
            auto diff = __builtin_subcll(G(opers->imm.dst),
                                         opers->imm.src,
                                         0,
                                         &carry_out);
            flags->z  = diff == 0;
            flags->c  = carry_out > 0;
            G(opers->imm.dst) = diff;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        sub_reg2:
        {
            auto diff = __builtin_subcll(G(opers->reg2.dst),
                                         G(opers->reg2.src),
                                         0,
                                         &carry_out);
            flags->z  = diff == 0;
            flags->c  = carry_out > 0;
            G(opers->reg2.dst) = diff;
            PC += sizeof(instruction_t);
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
            G(opers->imm.dst) = diff;
            PC += sizeof(instruction_t);
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
            G(opers->reg2.dst) = diff;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        div_imm:
        {
            auto quotient = opers->imm.src == 0 ? 0 : G(opers->imm.dst) / opers->imm.src;
            flags->v = false;
            flags->c = false;
            flags->z = quotient == 0;
            G(opers->imm.dst) = quotient;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        div_reg2:
        {
            auto src = G(opers->reg2.src);
            auto quotient = src == 0 ? 0 : G(opers->reg2.dst) / src;
            flags->v = false;
            flags->c = false;
            flags->z = quotient == 0;
            G(opers->reg2.dst) = quotient;
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            LR = PC + sizeof(instruction_t);
            PC += s64(opers->imm.src);
            EXEC_NEXT();
        }
        blr_reg1:
        {
            LR = PC + sizeof(instruction_t);
            PC += s64(G(opers->reg1.dst));
            EXEC_NEXT();
        }
        cmp_imm:
        {
            auto diff = __builtin_subcll(G(opers->imm.dst),
                                         opers->imm.src,
                                         0,
                                         &carry_out);
            flags->v  = false;
            flags->c  = carry_out > 0;
            flags->z  = diff == 0;
            flags->n  = s64(diff) < 0;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        cmp_reg2:
        {
            auto diff = __builtin_subcll(G(opers->reg2.dst),
                                         G(opers->reg2.src),
                                         0,
                                         &carry_out);
            flags->v  = false;
            flags->c  = carry_out > 0;
            flags->z  = diff == 0;
            flags->n  = s64(diff) < 0;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        cmps_imm:
        {
            s64 diff{};
            flags->v = __builtin_ssubll_overflow(G(opers->imm.dst),
                                                 opers->imm.src,
                                                 &diff);
            flags->c  = false;
            flags->z  = diff == 0;
            flags->n  = diff < 0;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        cmps_reg2:
        {
            s64 diff{};
            flags->v = __builtin_ssubll_overflow(G(opers->reg2.dst),
                                                 G(opers->reg2.src),
                                                 &diff);
            flags->c  = false;
            flags->z  = diff == 0;
            flags->n  = diff < 0;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        beq_imm:
        {
            PC += s64(flags->z ? opers->imm.src : 1);
            EXEC_NEXT();
        }
        beqs_imm:
        {
            PC += s64(flags->z ? opers->imm.src : 1);
            EXEC_NEXT();
        }
        bne_imm:
        {
            PC += s64(!flags->z ? opers->imm.src : 1);
            EXEC_NEXT();
        }
        bnes_imm:
        {
            PC += s64(!flags->z ? opers->imm.src : 1);
            EXEC_NEXT();
        }
        bl_imm:
        {
            PC += s64(!flags->c ? opers->imm.src : 1);
            EXEC_NEXT();
        }
        bls_imm:
        {
            PC += s64(flags->n != flags->v ? opers->imm.src : 1);
            EXEC_NEXT();
        }
        ble_imm:
        {
            PC += s64(!flags->c || flags->z ? opers->imm.src : 1);
            EXEC_NEXT();
        }
        bles_imm:
        {
            PC += s64(flags->z || flags->n != flags->v ? opers->imm.src : 1);
            EXEC_NEXT();
        }
        bg_imm:
        {
            PC += s64(flags->c && !flags->z ? opers->imm.src : 1);
            EXEC_NEXT();
        }
        bgs_imm:
        {
            PC += s64(!flags->z && flags->n == flags->v ? opers->imm.src : 1);
            EXEC_NEXT();
        }
        bge_imm:
        {
            PC += s64(flags->c ? opers->imm.src : 1);
            EXEC_NEXT();
        }
        bges_imm:
        {
            PC += s64(flags->n == flags->v ? opers->imm.src : 1);
            EXEC_NEXT();
        }
        seq_reg1:
        {
            G(opers->reg1.dst) = flags->z;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        seqs_reg1:
        {
            G(opers->reg1.dst) = flags->z;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        sne_reg1:
        {
            G(opers->reg1.dst) = !flags->z;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        snes_reg1:
        {
            G(opers->reg1.dst) = !flags->z;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        sl_reg1:
        {
            G(opers->reg1.dst) = !flags->c;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        sls_reg1:
        {
            G(opers->reg1.dst) = flags->n != flags->v;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        sle_reg1:
        {
            G(opers->reg1.dst) = !flags->c || flags->z;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        sles_reg1:
        {
            G(opers->reg1.dst) = flags->z || flags->n != flags->v;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        sg_reg1:
        {
            G(opers->reg1.dst) = flags->c && !flags->z;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        sgs_reg1:
        {
            G(opers->reg1.dst) = !flags->z || flags->n == flags->v;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        sge_reg1:
        {
            G(opers->reg1.dst) = flags->c;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        sges_reg1:
        {
            G(opers->reg1.dst) = flags->n == flags->v;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        ret_reg1:
        {
            PC = G(opers->reg1.dst);
            EXEC_NEXT();
        }
        mma_imm:
        {
            auto area = &vm.memory_map.entries[opers->imm.src];
            G(opers->imm.dst) = area->valid ? area->addr : 0;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        pop_reg2:
        {
            G(opers->reg2.dst) = H(G(opers->reg2.src));
            G(opers->reg2.src) += s32(opers->reg2.aux);
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        fix_imm2:
        {
            G(opers->imm.dst) = u64(make_fixnum(ctx, G(opers->imm.src)));
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        fix_reg2:
        {
            G(opers->reg2.dst) = u64(make_fixnum(ctx, G(opers->reg2.src)));
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        flo_imm2:
        {
            G(opers->imm.dst) = u64(make_flonum(ctx, f32(G(opers->imm.src))));
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        flo_reg2:
        {
            G(opers->reg2.dst) = u64(make_flonum(ctx, f32(G(opers->reg2.src))));
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        env_reg2:
        {
            auto curr_env = (obj_t*) H(G(opers->reg2.src));
            G(opers->reg2.dst) = u64(make_environment(ctx, curr_env));
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
                lst = cons(ctx, (obj_t*) H(base + offs), lst);
                offs -= sizeof(u64);
            }
            restore_gc(ctx, gc);
            push_gc(ctx, lst);
            G(opers->reg2_imm.b) = u64(lst);
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        error_reg2:
        {
            G(opers->reg2.dst) = u64(make_error(ctx,
                                                (obj_t*) G(opers->reg2.src),
                                                ctx->call_list));
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        qq_reg2:
        {
            auto v = quasiquote(ctx, (obj_t*) G(opers->reg2.src), env);
            flags->c = false;
            flags->z = !IS_NIL(v);
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg2.dst) = u64(v);
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        gc:
        {
            collect_garbage(ctx);
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        gc_reg1:
        {
            push_gc(ctx, (obj_t*) G(opers->reg1.dst));
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        gc_reg2:
        {
            G(opers->reg2.dst) = u64(pop_gc(ctx));
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        eval_reg2:
        {
            // XXX:
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        apply_reg2:
        {
            // XXX:
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        car_reg2:
        {
            G(opers->reg2.dst) = u64(CAR((obj_t*) G(opers->reg2.src)));
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        cdr_reg2:
        {
            G(opers->reg2.dst) = u64(CDR((obj_t*) G(opers->reg2.src)));
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        setcar_reg2:
        {
            SET_CAR((obj_t*) G(opers->reg2.dst), (obj_t*) G(opers->reg2.src));
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        setcdr_reg2:
        {
            SET_CDR((obj_t*) G(opers->reg2.dst), (obj_t*) G(opers->reg2.src));
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        get_imm:
        {
            auto key = (obj_t*) OBJ_AT(G(opers->imm.src));
            auto v   = get(ctx, key, env);
            flags->c = false;
            flags->z = !IS_NIL(v);
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->imm.dst) = u64(v);
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        get_reg2:
        {
            auto v = get(ctx, (obj_t*) G(opers->reg2.src), env);
            flags->c = false;
            flags->z = !IS_NIL(v);
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg2.dst) = u64(v);
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        set_imm:
        {
            set(ctx,
                (obj_t*) G(opers->imm.dst),
                OBJ_AT(G(opers->imm.src)),
                env);
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        set_reg2:
        {
            set(ctx,
                (obj_t*) G(opers->reg2.dst),
                (obj_t*) G(opers->reg2.src),
                env);
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        listp_reg2:
        {
            // FIXME
            auto v = (obj_t*) G(opers->reg2.src);
            flags->c = false;
            flags->z = TYPE(v) == obj_type_t::pair; // XXX: NOT CORRECT, fix!
            flags->n = false;
            flags->i = false;
            flags->v = false;
            G(opers->reg2.dst) = u64(flags->z ? ctx->true_ : ctx->false_);
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        ladd_reg2_imm:
        {
            auto     base = G(opers->reg2_imm.a);
            auto     size = s32(opers->reg2_imm.imm) / sizeof(u64);
            auto     offs = 0;
            flonum_t acc  = to_flonum((obj_t*) H(base + offs));
            auto gc = save_gc(ctx);
            base += sizeof(u64);
            for (u32 i = 0; i < size - 1; ++i) {
                acc += to_flonum((obj_t*) H(base + offs));
                offs += sizeof(u64);
            }
            restore_gc(ctx, gc);
            auto res = make_flonum(ctx, acc);
            push_gc(ctx, res);
            G(opers->reg2_imm.b) = u64(res);
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        lsub_reg2_imm:
        {
            auto     base = G(opers->reg2_imm.a);
            auto     size = s32(opers->reg2_imm.imm) / sizeof(u64);
            auto     offs = 0;
            flonum_t acc  = to_flonum((obj_t*) H(base + offs));
            auto gc = save_gc(ctx);
            base += sizeof(u64);
            for (u32 i = 0; i < size - 1; ++i) {
                acc -= to_flonum((obj_t*) H(base + offs));
                offs += sizeof(u64);
            }
            restore_gc(ctx, gc);
            auto res = make_flonum(ctx, acc);
            push_gc(ctx, res);
            G(opers->reg2_imm.b) = u64(res);
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        lmul_reg2_imm:
        {
            auto     base = G(opers->reg2_imm.a);
            auto     size = s32(opers->reg2_imm.imm) / sizeof(u64);
            auto     offs = 0;
            flonum_t acc  = to_flonum((obj_t*) H(base + offs));
            auto gc = save_gc(ctx);
            base += sizeof(u64);
            for (u32 i = 0; i < size - 1; ++i) {
                acc *= to_flonum((obj_t*) H(base + offs));
                offs += sizeof(u64);
            }
            restore_gc(ctx, gc);
            auto res = make_flonum(ctx, acc);
            push_gc(ctx, res);
            G(opers->reg2_imm.b) = u64(res);
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        ldiv_reg2_imm:
        {
            auto     base = G(opers->reg2_imm.a);
            auto     size = s32(opers->reg2_imm.imm) / sizeof(u64);
            auto     offs = 0;
            flonum_t acc  = to_flonum((obj_t*) H(base + offs));
            auto gc = save_gc(ctx);
            base += sizeof(u64);
            for (u32 i = 0; i < size - 1; ++i) {
                acc /= to_flonum((obj_t*) H(base + offs));
                offs += sizeof(u64);
            }
            restore_gc(ctx, gc);
            auto res = make_flonum(ctx, acc);
            push_gc(ctx, res);
            G(opers->reg2_imm.b) = u64(res);
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        lmod_reg2_imm:
        {
            auto     base = G(opers->reg2_imm.a);
            auto     size = s32(opers->reg2_imm.imm) / sizeof(u64);
            auto     offs = 0;
            fixnum_t acc  = to_fixnum((obj_t*) H(base + offs));
            auto gc = save_gc(ctx);
            base += sizeof(u64);
            for (u32 i = 0; i < size - 1; ++i) {
                acc %= to_fixnum((obj_t*) H(base + offs));
                offs += sizeof(u64);
            }
            restore_gc(ctx, gc);
            auto res = make_fixnum(ctx, acc);
            push_gc(ctx, res);
            G(opers->reg2_imm.b) = u64(res);
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        push_imm:
        {
            G(opers->imm.dst) += s8(opers->imm.aux);
            H(G(opers->imm.dst)) = opers->imm.src;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        push_reg2:
        {
            G(opers->reg2.dst) += s32(opers->reg2.aux);
            H(G(opers->reg2.dst)) = G(opers->reg2.src);
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        lea_imm:
        {
            G(opers->imm.dst) = PC + s32(opers->imm.src);
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        lea_offs:
        {
            G(opers->offset.dst) = G(opers->offset.src) + s32(opers->offset.offs);
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        move_imm:
        {
            G(opers->imm.dst) = opers->imm.src;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        move_reg2:
        {
            G(opers->reg2.dst) = G(opers->reg2.src);
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        load_reg2:
        {
            auto src = H(G(opers->reg2.src));
            G(opers->reg2.dst) = src;
            flags->v = false;
            flags->c = false;
            flags->z = src == 0;
            flags->n = s32(src) < 0;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        load_offset:
        {
            auto src = H(G(opers->offset.src) + opers->offset.offs);
            G(opers->offset.dst) = src;
            flags->v = false;
            flags->c = false;
            flags->z = src == 0;
            flags->n = s32(src) < 0;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        load_indexed:
        {
            auto src = H(G(opers->indexed.base) + G(opers->indexed.ndx) + opers->offset.offs);
            G(opers->indexed.dst) = src;
            flags->v = false;
            flags->c = false;
            flags->z = src == 0;
            flags->n = s32(src) < 0;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        store_reg2:
        {
            auto src = G(opers->reg2.src);
            H(G(opers->reg2.dst)) = src;
            flags->v = false;
            flags->c = false;
            flags->z = src == 0;
            flags->n = s32(src) < 0;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        store_offset:
        {
            auto src = G(opers->offset.src);
            H(G(opers->offset.dst) + opers->offset.offs) = src;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        store_indexed:
        {
            auto src = G(opers->indexed.dst);
            H(G(opers->indexed.base) + G(opers->indexed.ndx) + opers->indexed.offs) = src;
            flags->v = false;
            flags->c = false;
            flags->z = src == 0;
            flags->n = s32(src) < 0;
            PC += sizeof(instruction_t);
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
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        trap_reg2:
        {
            auto id       = G(opers->reg2.src);
            auto arg      = G(opers->reg2.dst);
            auto callback = hashtab::find(vm.traptab, u32(id));
            status = callback && callback(vm, arg) ? status_t::ok : status_t::fail;
            PC += sizeof(instruction_t);
            EXEC_NEXT();
        }
        error:
        {
            return status_t::fail;
        }
    }

    status_t init(vm_t& vm, alloc_t* alloc, u32 heap_size) {
        vm.alloc                = alloc;
        vm.heap                 = (u64*) memory::alloc(vm.alloc, heap_size, alignof(u64));
        vm.memory_map           = {};
        vm.memory_map.heap_size = heap_size / sizeof(u64);
        hashtab::init(vm.traptab, vm.alloc);
        return status_t::ok;
    }

    u0 memory_map(vm_t& vm, memory_area_t area, u8 reg, u32 size, b8 top) {
        auto entry = &vm.memory_map.entries[u32(area)];
        entry->offs  = 0;
        entry->reg   = reg;
        entry->top   = top;
        entry->size  = size;
        entry->valid = true;
    }

    const memory_map_entry_t* find_memory_map_entry(const vm_t& vm, u8 reg) {
        s32 idx = vm.memory_map.reg_to_entry[reg];
        if (idx == -1)
            return nullptr;
        return &vm.memory_map.entries[idx];
    }
}
