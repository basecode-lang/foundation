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

#include <basecode/core/graphviz/gv.h>
#include <basecode/core/scm/compiler.h>
#include <basecode/core/scm/bytecode.h>


#define ENCODE_REG(o, f)        SAFE_SCOPE(                                     \
    u8  __tmp{};                                                                \
    auto __status = encode_reg((o), __tmp);                                     \
    if (!OK(__status))                                                          \
        return __status;                                                        \
    (f) = __tmp;)
#define ENCODE_IMM(o, f)        SAFE_SCOPE(                                     \
    u32 __tmp{};                                                                \
    auto __status = encode_imm(e, addr, (o), __tmp);                            \
    if (!OK(__status))                                                          \
        return __status;                                                        \
    (f) = __tmp;)

namespace basecode::scm {
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
                [gc]        = "GC"_ss,
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
            [ep]    = "EP"_ss,
            [dp]    = "DP"_ss,
            [hp]    = "HP"_ss,
            [sp]    = "SP"_ss,
            [fp]    = "FP"_ss,
            [lp]    = "LP"_ss,
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

    namespace vm {
        namespace basic_block {
            bb_builder_t encode(bb_t* bb) {
                return bb_builder_t(bb);
            }

            bb_builder_t encode(bb_t& bb) {
                return bb_builder_t(bb);
            }

            bb_builder_t::bb_builder_t(bb_t* bb) : _bb(bb),
                                                   _em(bb->emit),
                                                   _inst(),
                                                   _imm1(this),
                                                   _imm2(this),
                                                   _reg1(this),
                                                   _reg2(this),
                                                   _none(this),
                                                   _offs(this),
                                                   _reg3(this),
                                                   _reg2_imm(this) {
            }

            bb_builder_t::bb_builder_t(bb_t& bb) : _bb(&bb),
                                                   _em(bb.emit),
                                                   _inst(),
                                                   _imm1(this),
                                                   _imm2(this),
                                                   _reg1(this),
                                                   _reg2(this),
                                                   _none(this),
                                                   _offs(this),
                                                   _reg3(this),
                                                   _reg2_imm(this) {
            }

            imm1_builder_t& bb_builder_t::imm1() {
                _inst = &array::append(_em->insts);
                _inst->id       = _em->insts.size;
                _inst->block_id = _bb->id;
                _inst->encoding = instruction::encoding::imm;
                if (_bb->insts.eidx == 0) {
                    _bb->insts.sidx = _em->insts.size - 1;
                    _bb->insts.eidx = _bb->insts.sidx + 1;
                } else {
                    _bb->insts.eidx = _em->insts.size;
                }
                _imm1.reset();
                return _imm1;
            }

            imm2_builder_t& bb_builder_t::imm2() {
                _inst = &array::append(_em->insts);
                _inst->id       = _em->insts.size;
                _inst->block_id = _bb->id;
                _inst->encoding = instruction::encoding::imm;
                if (_bb->insts.eidx == 0) {
                    _bb->insts.sidx = _em->insts.size - 1;
                    _bb->insts.eidx = _bb->insts.sidx + 1;
                } else {
                    _bb->insts.eidx = _em->insts.size;
                }
                _imm2.reset();
                return _imm2;
            }

            reg1_builder_t& bb_builder_t::reg1() {
                _inst = &array::append(_em->insts);
                _inst->id       = _em->insts.size;
                _inst->block_id = _bb->id;
                _inst->encoding = instruction::encoding::reg1;
                if (_bb->insts.eidx == 0) {
                    _bb->insts.sidx = _em->insts.size - 1;
                    _bb->insts.eidx = _bb->insts.sidx + 1;
                } else {
                    _bb->insts.eidx = _em->insts.size;
                }
                _reg1.reset();
                return _reg1;
            }

            reg2_builder_t& bb_builder_t::reg2() {
                _inst = &array::append(_em->insts);
                _inst->id       = _em->insts.size;
                _inst->block_id = _bb->id;
                _inst->encoding = instruction::encoding::reg2;
                if (_bb->insts.eidx == 0) {
                    _bb->insts.sidx = _em->insts.size - 1;
                    _bb->insts.eidx = _bb->insts.sidx + 1;
                } else {
                    _bb->insts.eidx = _em->insts.size;
                }
                _reg2.reset();
                return _reg2;
            }

            none_builder_t& bb_builder_t::none() {
                _inst = &array::append(_em->insts);
                _inst->id       = _em->insts.size;
                _inst->block_id = _bb->id;
                _inst->encoding = instruction::encoding::none;
                if (_bb->insts.eidx == 0) {
                    _bb->insts.sidx = _em->insts.size - 1;
                    _bb->insts.eidx = _bb->insts.sidx + 1;
                } else {
                    _bb->insts.eidx = _em->insts.size;
                }
                _none.reset();
                return _none;
            }

            reg3_builder_t& bb_builder_t::reg3() {
                _inst = &array::append(_em->insts);
                _inst->id       = _em->insts.size;
                _inst->block_id = _bb->id;
                _inst->encoding = instruction::encoding::reg3;
                if (_bb->insts.eidx == 0) {
                    _bb->insts.sidx = _em->insts.size - 1;
                    _bb->insts.eidx = _bb->insts.sidx + 1;
                } else {
                    _bb->insts.eidx = _em->insts.size;
                }
                _reg3.reset();
                return _reg3;
            }

            offs_builder_t& bb_builder_t::offs() {
                _inst = &array::append(_em->insts);
                _inst->id       = _em->insts.size;
                _inst->block_id = _bb->id;
                _inst->encoding = instruction::encoding::offset;
                if (_bb->insts.eidx == 0) {
                    _bb->insts.sidx = _em->insts.size - 1;
                    _bb->insts.eidx = _bb->insts.sidx + 1;
                } else {
                    _bb->insts.eidx = _em->insts.size;
                }
                _offs.reset();
                return _offs;
            }

            reg2_imm_builder_t& bb_builder_t::reg2_imm() {
                _inst = &array::append(_em->insts);
                _inst->id       = _em->insts.size;
                _inst->block_id = _bb->id;
                _inst->encoding = instruction::encoding::reg2_imm;
                if (_bb->insts.eidx == 0) {
                    _bb->insts.sidx = _em->insts.size - 1;
                    _bb->insts.eidx = _bb->insts.sidx + 1;
                } else {
                    _bb->insts.eidx = _em->insts.size;
                }
                _reg2_imm.reset();
                return _reg2_imm;
            }

            // --------------------------------------------

            bb_builder_t& none_builder_t::build() {
                _builder->_inst = {};
                return *_builder;
            }

            none_builder_t& none_builder_t::op(op_code_t op_code) {
                _builder->_inst->type = op_code;
                return *this;
            }

            // --------------------------------------------

            bb_builder_t& imm1_builder_t::build() {
                switch (_builder->_inst->type) {
                    case instruction::type::br:
                    case instruction::type::bl:
                    case instruction::type::bg:
                    case instruction::type::ble:
                    case instruction::type::bge:
                    case instruction::type::beq:
                    case instruction::type::bne:
                    case instruction::type::blr:
                    case instruction::type::bra: {
                        auto& imm = _builder->_inst->operands[0];
                        digraph::make_edge(_builder->_em->bb_graph,
                                           _builder->_bb->node,
                                           imm.kind.bb->node);
                        break;
                    }
                    default:
                        break;
                }

                _builder->_inst = {};
                return *_builder;
            }

            imm1_builder_t& imm1_builder_t::value(s32 value) {
                _builder->_inst->operands[0] = _builder->operand(value);
                _builder->_inst->is_signed = true;
                return *this;
            }

            imm1_builder_t& imm1_builder_t::value(u32 value) {
                _builder->_inst->operands[0] = _builder->operand(value);
                return *this;
            }

            imm1_builder_t& imm1_builder_t::value(bb_t* value) {
                _builder->_inst->operands[0] = _builder->operand(value);
                return *this;
            }

            imm1_builder_t& imm1_builder_t::is_signed(b8 flag) {
                _builder->_inst->is_signed = flag;
                return *this;
            }

            imm1_builder_t& imm1_builder_t::op(op_code_t op_code) {
                _builder->_inst->type = op_code;
                return *this;
            }

            // --------------------------------------------

            bb_builder_t& imm2_builder_t::build() {
                if (_dst) {
                    if (_builder->_inst->mode) {
                        emitter::virtual_var::read(*_builder->_em,
                                                   *_dst,
                                                   _builder->_inst->id);
                    } else {
                        *_dst = emitter::virtual_var::write(*_builder->_em,
                                                            *_dst,
                                                            _builder->_inst->id);
                    }
                    _builder->_inst->operands[1] = _builder->operand(*_dst);
                }
                _builder->_inst = {};
                return *_builder;
            }

            imm2_builder_t& imm2_builder_t::mode(b8 flag) {
                _builder->_inst->mode = flag;
                return *this;
            }

            imm2_builder_t& imm2_builder_t::dst(reg_t reg) {
                _builder->_inst->operands[1] = _builder->operand(reg);
                return *this;
            }

            imm2_builder_t& imm2_builder_t::src(s32 value) {
                _builder->_inst->operands[0] = _builder->operand(value);
                _builder->_inst->is_signed = true;
                return *this;
            }

            imm2_builder_t& imm2_builder_t::src(u32 value) {
                _builder->_inst->operands[0] = _builder->operand(value);
                return *this;
            }

            imm2_builder_t& imm2_builder_t::dst(var_t** var) {
                _dst = var;
                return *this;
            }

            imm2_builder_t& imm2_builder_t::src(bb_t* value) {
                _builder->_inst->operands[0] = _builder->operand(value);
                return *this;
            }

            imm2_builder_t& imm2_builder_t::is_signed(b8 flag) {
                _builder->_inst->is_signed = flag;
                return *this;
            }

            imm2_builder_t& imm2_builder_t::op(op_code_t op_code) {
                _builder->_inst->type = op_code;
                return *this;
            }

            // --------------------------------------------

            bb_builder_t& reg1_builder_t::build() {
                if (_dst) {
                    *_dst = emitter::virtual_var::write(*_builder->_em,
                                                        *_dst,
                                                        _builder->_inst->id);
                    _builder->_inst->operands[0] = _builder->operand(*_dst);
                }
                _builder->_inst = {};
                return *_builder;
            }

            reg1_builder_t& reg1_builder_t::dst(reg_t reg) {
                _builder->_inst->operands[0] = _builder->operand(reg);
                return *this;
            }

            reg1_builder_t& reg1_builder_t::dst(var_t** var) {
                _dst = var;
                return *this;
            }

            reg1_builder_t& reg1_builder_t::op(op_code_t op_code) {
                _builder->_inst->type = op_code;
                return *this;
            }

            // --------------------------------------------

            bb_builder_t& reg2_builder_t::build() {
                if (_src) {
                    emitter::virtual_var::read(*_builder->_em,
                                               *_src,
                                               _builder->_inst->id);
                    _builder->_inst->operands[0] = _builder->operand(*_src);
                }
                if (_dst) {
                    switch (_builder->_inst->type) {
                        case instruction::type::cmp:
                        case instruction::type::lcmp:
                        case instruction::type::setcar:
                        case instruction::type::setcdr:
                            emitter::virtual_var::read(*_builder->_em,
                                                       *_dst,
                                                       _builder->_inst->id);
                            break;
                        default:
                            *_dst = emitter::virtual_var::write(*_builder->_em,
                                                                *_dst,
                                                                _builder->_inst->id);
                            break;
                    }
                    _builder->_inst->operands[1] = _builder->operand(*_dst);
                }
                _builder->_inst = {};
                return *_builder;
            }

            reg2_builder_t& reg2_builder_t::aux(s8 value) {
                _builder->_inst->aux = value;
                return *this;
            }

            reg2_builder_t& reg2_builder_t::src(reg_t reg) {
                _builder->_inst->operands[0] = _builder->operand(reg);
                return *this;
            }

            reg2_builder_t& reg2_builder_t::dst(reg_t reg) {
                _builder->_inst->operands[1] = _builder->operand(reg);
                return *this;
            }

            reg2_builder_t& reg2_builder_t::src(var_t** var) {
                _src = var;
                return *this;
            }

            reg2_builder_t& reg2_builder_t::dst(var_t** var) {
                _dst = var;
                return *this;
            }

            reg2_builder_t& reg2_builder_t::is_signed(b8 flag) {
                _builder->_inst->is_signed = flag;
                return *this;
            }

            reg2_builder_t& reg2_builder_t::op(op_code_t op_code) {
                _builder->_inst->type = op_code;
                return *this;
            }

            // --------------------------------------------

            bb_builder_t& reg2_imm_builder_t::build() {
                if (_src) {
                    emitter::virtual_var::read(*_builder->_em,
                                               *_src,
                                               _builder->_inst->id);
                    _builder->_inst->operands[0] = _builder->operand(*_src);
                }
                if (_dst) {
                    *_dst = emitter::virtual_var::write(*_builder->_em,
                                                        *_dst,
                                                        _builder->_inst->id);
                    _builder->_inst->operands[1] = _builder->operand(*_dst);
                }
                _builder->_inst = {};
                return *_builder;
            }

            reg2_imm_builder_t& reg2_imm_builder_t::src(reg_t reg) {
                _builder->_inst->operands[0] = _builder->operand(reg);
                return *this;
            }

            reg2_imm_builder_t& reg2_imm_builder_t::dst(reg_t reg) {
                _builder->_inst->operands[1] = _builder->operand(reg);
                return *this;
            }

            reg2_imm_builder_t& reg2_imm_builder_t::src(var_t** var) {
                _src = var;
                return *this;
            }

            reg2_imm_builder_t& reg2_imm_builder_t::dst(var_t** var) {
                _dst = var;
                return *this;
            }

            reg2_imm_builder_t& reg2_imm_builder_t::is_signed(b8 flag) {
                _builder->_inst->is_signed = flag;
                return *this;
            }

            reg2_imm_builder_t& reg2_imm_builder_t::value(s32 value) {
                _builder->_inst->operands[2] = _builder->operand(value);
                return *this;
            }

            reg2_imm_builder_t& reg2_imm_builder_t::value(u32 value) {
                _builder->_inst->operands[2] = _builder->operand(value);
                return *this;
            }

            reg2_imm_builder_t& reg2_imm_builder_t::value(bb_t* value) {
                _builder->_inst->operands[2] = _builder->operand(value);
                return *this;
            }

            reg2_imm_builder_t& reg2_imm_builder_t::op(op_code_t op_code) {
                _builder->_inst->type = op_code;
                return *this;
            }

            // --------------------------------------------

            bb_builder_t& offs_builder_t::build() {
                if (_src) {
                    switch (_builder->_inst->type) {
                        case instruction::type::lea:
                        case instruction::type::load:
                        case instruction::type::store:
                            emitter::virtual_var::read(*_builder->_em,
                                                       *_src,
                                                       _builder->_inst->id);
                            break;
                    }
                    _builder->_inst->operands[0] = _builder->operand(*_src);
                }
                if (_dst) {
                    switch (_builder->_inst->type) {
                        case instruction::type::lea:
                        case instruction::type::load:
                            *_dst = emitter::virtual_var::write(*_builder->_em,
                                                                *_dst,
                                                                _builder->_inst->id);
                            break;
                        case instruction::type::store:
                            emitter::virtual_var::read(*_builder->_em,
                                                       *_dst,
                                                       _builder->_inst->id);
                            break;
                    }
                    _builder->_inst->operands[1] = _builder->operand(*_dst);
                }
                _builder->_inst = {};
                return *_builder;
            }

            offs_builder_t& offs_builder_t::mode(b8 flag) {
                _builder->_inst->mode = flag;
                return *this;
            }

            offs_builder_t& offs_builder_t::src(reg_t reg) {
                _builder->_inst->operands[0] = _builder->operand(reg);
                return *this;
            }

            offs_builder_t& offs_builder_t::dst(reg_t reg) {
                _builder->_inst->operands[1] = _builder->operand(reg);
                return *this;
            }

            offs_builder_t& offs_builder_t::src(var_t** var) {
                _src = var;
                return *this;
            }

            offs_builder_t& offs_builder_t::dst(var_t** var) {
                _dst = var;
                return *this;
            }

            offs_builder_t& offs_builder_t::offset(s32 value) {
                _builder->_inst->operands[2] = _builder->operand(value);
                return *this;
            }

            offs_builder_t& offs_builder_t::op(op_code_t op_code) {
                _builder->_inst->type = op_code;
                return *this;
            }

            // --------------------------------------------

            bb_builder_t& reg3_builder_t::build() {
                if (_a) {
                    emitter::virtual_var::read(*_builder->_em,
                                               *_a,
                                               _builder->_inst->id);
                    _builder->_inst->operands[0] = _builder->operand(*_a);
                }
                if (_b) {
                    emitter::virtual_var::read(*_builder->_em,
                                               *_b,
                                               _builder->_inst->id);
                    _builder->_inst->operands[1] = _builder->operand(*_b);
                }
                if (_c) {
                    *_c = emitter::virtual_var::write(*_builder->_em,
                                                      *_c,
                                                      _builder->_inst->id);
                    _builder->_inst->operands[2] = _builder->operand(*_c);
                }
                _builder->_inst = {};
                return *_builder;
            }

            reg3_builder_t& reg3_builder_t::a(reg_t reg) {
                _builder->_inst->operands[0] = _builder->operand(reg);
                return *this;
            }

            reg3_builder_t& reg3_builder_t::b(reg_t reg) {
                _builder->_inst->operands[1] = _builder->operand(reg);
                return *this;
            }

            reg3_builder_t& reg3_builder_t::c(reg_t reg) {
                _builder->_inst->operands[2] = _builder->operand(reg);
                return *this;
            }

            reg3_builder_t& reg3_builder_t::a(var_t** var) {
                _a = var;
                return *this;
            }

            reg3_builder_t& reg3_builder_t::b(var_t** var) {
                _b = var;
                return *this;
            }

            reg3_builder_t& reg3_builder_t::c(var_t** var) {
                _c = var;
                return *this;
            }

            reg3_builder_t& reg3_builder_t::op(op_code_t op_code) {
                _builder->_inst->type = op_code;
                return *this;
            }
        }

        namespace emitter {
            u0 free(emitter_t& e) {
                for (auto var : e.vars)
                    array::free(var->accesses);
                array::free(e.insts);
                symtab::free(e.vartab);
                array::free(e.comments);
                digraph::free(e.bb_graph);
                str_array::free(e.strtab);
                digraph::free(e.var_graph);
                stable_array::free(e.vars);
                stable_array::free(e.blocks);
            }

            u0 reset(emitter_t& e) {
                array::reset(e.insts);
                symtab::reset(e.vartab);
                array::reset(e.comments);
                digraph::reset(e.bb_graph);
                str_array::reset(e.strtab);
                digraph::reset(e.var_graph);
                stable_array::reset(e.vars);
                stable_array::reset(e.blocks);
            }

            static u0 format_comments(str_buf_t& buf,
                                      u32 len,
                                      const comment_array_t& comments,
                                      const str_array_t& strings,
                                      comment_type_t type,
                                      u32 block_id,
                                      u32 sidx,
                                      u32 eidx,
                                      s32 line,
                                      u64 addr) {
                u32 j{};
                for (u32 i = sidx; i < eidx; ++i) {
                    const auto& c = comments[i];
                    if (c.type != type
                    ||  c.block_id != block_id) {
                        continue;
                    }
                    const auto& str = strings[c.id - 1];
                    switch (type) {
                        case comment_type_t::note:
                        case comment_type_t::line: {
                            format::format_to(buf, "${:08X}: ; {}\n", addr, str);
                            ++j;
                            break;
                        }
                        case comment_type_t::margin: {
                            if (c.line != line)
                                break;
                            if (j > 0) {
                                format::format_to(buf,
                                                  "${:08X}:{:<{}}",
                                                  addr,
                                                  " ",
                                                  54 - len);
                            }
                            format::format_to(buf,
                                              "{:<{}}; {}\n",
                                              " ",
                                              52 - len,
                                              str);
                            ++j;
                            break;
                        }
                    }
                }
                if (!j && type != comment_type_t::note)
                    format::format_to(buf, "\n");
            }

            static u0 format_edges(str_buf_t& buf, bb_t* block, u64 addr) {
                bb_digraph_t::Node_Array nodes{};
                array::init(nodes, block->emit->alloc);
                defer(array::free(nodes));

                digraph::incoming_nodes(block->emit->bb_graph, block->node, nodes);
                if (nodes.size > 0) {
                    format::format_to(buf, "${:08X}:    .preds      ", addr);
                    for (u32 i = 0; i < nodes.size; ++i) {
                        if (i > 0) format::format_to(buf, ", ");
                        format::format_to(buf, "{}", *(nodes[i]->value));
                    }
                    format::format_to(buf, "\n");
                }

                array::reset(nodes);
                digraph::outgoing_nodes(block->emit->bb_graph, block->node, nodes);
                if (nodes.size > 0) {
                    format::format_to(buf, "${:08X}:    .succs      ", addr);
                    for (u32 i = 0; i < nodes.size; ++i) {
                        if (i > 0) format::format_to(buf, ", ");
                        format::format_to(buf, "{}", *(nodes[i]->value));
                    }
                    format::format_to(buf, "\n");
                }
            }

            u0 disassemble(emitter_t& e, bb_t& start_block, str_buf_t& buf) {
                auto curr = &start_block;
                u64  addr = curr->addr;
                u32  line{};
                while (curr) {
                    format_comments(buf,
                                    0,
                                    e.comments,
                                    e.strtab,
                                    comment_type_t::note,
                                    curr->id,
                                    curr->notes.sidx,
                                    curr->notes.eidx,
                                    -1,
                                    addr);
                    format::format_to(buf, "${:08X}: {}:\n", addr, *curr);
                    format_edges(buf, curr, addr);
                    for (s32 i = curr->insts.sidx; i < curr->insts.eidx; ++i) {
                        const auto& inst = e.insts[i];
                        if (inst.block_id != curr->id)
                            continue;
                        auto start_pos = buf.size();
                        format::format_to(
                            buf,
                            "${:08X}:    {:<12}",
                            addr,
                            instruction::type::name(inst.type));
                        switch (inst.encoding) {
                            case instruction::encoding::imm: {
                                if (inst.mode
                                &&  inst.operands[1].type != operand_type_t::none) {
                                    format::format_to(
                                        buf,
                                        "{}, ",
                                        inst.operands[1]);
                                    format::format_to(
                                        buf,
                                        "{}",
                                        inst.operands[0]);
                                } else {
                                    format::format_to(
                                        buf,
                                        "{}",
                                        inst.operands[0]);
                                    if (inst.operands[1].type != operand_type_t::none) {
                                        format::format_to(
                                            buf,
                                            ", {}",
                                            inst.operands[1]);
                                    }
                                }
                                break;
                            }
                            case instruction::encoding::reg1: {
                                format::format_to(
                                    buf,
                                    "{}",
                                    inst.operands[0]);
                                break;
                            }
                            case instruction::encoding::reg2: {
                                format::format_to(
                                    buf,
                                    "{}, {}",
                                    inst.operands[0],
                                    inst.operands[1]);
                                break;
                            }
                            case instruction::encoding::reg2_imm: {
                                format::format_to(
                                    buf,
                                    "{}, {}, {}",
                                    inst.operands[0],
                                    inst.operands[1],
                                    inst.operands[2]);
                                break;
                            }
                            case instruction::encoding::reg3: {
                                format::format_to(
                                    buf,
                                    "{}, {}, {}",
                                    inst.operands[0],
                                    inst.operands[1],
                                    inst.operands[2]);
                                break;
                            }
                            case instruction::encoding::reg4: {
                                format::format_to(
                                    buf,
                                    "{}, {}, {}, {}",
                                    inst.operands[0],
                                    inst.operands[1],
                                    inst.operands[2],
                                    inst.operands[3]);
                                break;
                            }
                            case instruction::encoding::offset: {
                                if (inst.mode) {
                                    format::format_to(
                                        buf,
                                        "{}, {}({})",
                                        inst.operands[0],
                                        inst.operands[2],
                                        inst.operands[1]);
                                } else {
                                    format::format_to(
                                        buf,
                                        "{}({}), {}",
                                        inst.operands[2],
                                        inst.operands[0],
                                        inst.operands[1]);
                                }
                                break;
                            }
                            case instruction::encoding::indexed: {
                                format::format_to(
                                    buf,
                                    "{}({}, {}), {}",
                                    inst.operands[3],
                                    inst.operands[0],
                                    inst.operands[1],
                                    inst.operands[2]);
                                break;
                            }
                            default: {
                                break;
                            }
                        }
                        format_comments(buf,
                                        buf.size() - start_pos,
                                        e.comments,
                                        e.strtab,
                                        comment_type_t::margin,
                                        curr->id,
                                        curr->notes.sidx,
                                        curr->notes.eidx,
                                        line,
                                        addr);
                        addr += sizeof(encoded_inst_t);
                        ++line;
                    }
                    line = {};
                    curr = curr->next;
                }
            }

            template <typename T>
            status_t encode_imm(emitter_t& e,
                                u64 addr,
                                const operand_t& oper,
                                T& field) {
                switch (oper.type) {
                    case operand_type_t::trap:
                    case operand_type_t::value: {
                        field = oper.kind.s;
                        break;
                    }
                    case operand_type_t::block: {
                        auto target_block = &e.blocks[oper.kind.bb->id - 1];
                        field = s32(target_block->addr - addr);
                        break;
                    }
                    default:
                        return status_t::fail;
                }
                return status_t::ok;
            }

            static status_t encode_reg(const operand_t& oper, u8& field) {
                switch (oper.type) {
                    case operand_type_t::reg: {
                        field = oper.kind.reg;
                        break;
                    }
                    case operand_type_t::var: {
                        break;
                    }
                    default:
                        return status_t::fail;
                }
                return status_t::ok;
            }

            status_t encode_inst(emitter_t& e, const inst_t& inst, u64 addr) {
                u64 buf{};
                u64 data{};
                auto& vm = *e.vm;
                auto encoded = (encoded_inst_t*) &buf;
                auto opers   = (encoded_operand_t*) &data;
                encoded->type      = inst.type;
                encoded->is_signed = inst.is_signed;
                switch (inst.encoding) {
                    case instruction::encoding::none: {
                        encoded->data     = 0;
                        encoded->encoding = instruction::encoding::none;
                        break;
                    }
                    case instruction::encoding::imm: {
                        encoded->encoding  = instruction::encoding::imm;
                        auto& src = inst.operands[0];
                        auto& dst = inst.operands[1];
                        ENCODE_IMM(src, opers->imm.src);
                        ENCODE_REG(dst, opers->imm.dst);
                        auto area = find_mem_map_entry(vm, opers->imm.dst);
                        if (area) {
                            opers->imm.aux = area->top ? -1 : 1;
                        } else {
                            opers->imm.aux = 0;
                        }
                        break;
                    }
                    case instruction::encoding::offset: {
                        encoded->encoding = instruction::encoding::offset;
                        auto& src  = inst.operands[0];
                        auto& dst  = inst.operands[1];
                        auto& offs = inst.operands[2];
                        ENCODE_REG(src, opers->offset.src);
                        ENCODE_REG(dst, opers->offset.dst);
                        opers->offset.pad  = 0;
                        opers->offset.offs = offs.kind.s;
                        break;
                    }
                    case instruction::encoding::indexed: {
                        encoded->encoding = instruction::encoding::indexed;
                        opers->indexed.pad  = 0;
                        auto& offs = inst.operands[3];
                        auto& base = inst.operands[0];
                        auto& ndx  = inst.operands[1];
                        auto& dst  = inst.operands[2];
                        ENCODE_REG(base, opers->indexed.base);
                        ENCODE_REG(ndx,  opers->indexed.ndx);
                        ENCODE_REG(dst,  opers->indexed.dst);
                        ENCODE_IMM(offs, opers->indexed.offs);
                        break;
                    }
                    case instruction::encoding::reg1: {
                        encoded->encoding = instruction::encoding::reg1;
                        opers->reg1.pad   = 0;
                        auto& dst = inst.operands[0];
                        ENCODE_REG(dst, opers->reg1.dst);
                        break;
                    }
                    case instruction::encoding::reg2: {
                        encoded->encoding = instruction::encoding::reg2;
                        opers->reg2.pad   = 0;
                        auto& src = inst.operands[0];
                        auto& dst = inst.operands[1];
                        ENCODE_REG(src, opers->reg2.src);
                        ENCODE_REG(dst, opers->reg2.dst);
                        auto area = find_mem_map_entry(vm, opers->reg2.src);
                        if (area && !inst.aux) {
                            opers->reg2.aux = area->top ? -1 : 1;
                        } else {
                            opers->reg2.aux = inst.aux;
                        }
                        break;
                    }
                    case instruction::encoding::reg2_imm: {
                        encoded->encoding = instruction::encoding::reg2_imm;
                        auto& src = inst.operands[0];
                        auto& dst = inst.operands[1];
                        auto& imm = inst.operands[2];
                        ENCODE_REG(src, opers->reg2_imm.a);
                        ENCODE_REG(dst, opers->reg2_imm.b);
                        ENCODE_IMM(imm, opers->reg2_imm.imm);
                        break;
                    }
                    case instruction::encoding::reg3: {
                        encoded->encoding = instruction::encoding::reg3;
                        opers->reg3.pad   = 0;
                        auto& a = inst.operands[0];
                        auto& b = inst.operands[1];
                        auto& c = inst.operands[2];
                        ENCODE_REG(a, opers->reg3.a);
                        ENCODE_REG(b, opers->reg3.b);
                        ENCODE_REG(c, opers->reg3.c);
                        break;
                    }
                    default:
                        return status_t::fail;
                }
                encoded->data = data;
                H(addr)       = buf;
                return status_t::ok;
            }

            status_t create_dot(emitter_t& e, const path_t& path) {
                using namespace graphviz;

                graph_t bb_sg{};
                graph::init(bb_sg, graph_type_t::directed, "bb"_ss);

                graph_t var_sg{};
                graph::init(var_sg, graph_type_t::directed, "vvar"_ss);

                graph_t g{};
                graph::init(g, graph_type_t::directed, "compiler"_ss);
                graph::node_sep(g, 1);
                graph::add_cluster_subgraph(g, &bb_sg);
                graph::add_cluster_subgraph(g, &var_sg);
                defer(
                    graph::free(g);
                    graph::free(bb_sg);
                    graph::free(var_sg));

                u32 block_node_ids[e.bb_graph.size];
                {
                    bb_digraph_t::Node_Array nodes{};
                    array::init(nodes, e.alloc);
                    defer(array::free(nodes));

                    for (auto bb_node : e.bb_graph.nodes) {
                        auto block_node = graph::make_node(bb_sg);
                        block_node_ids[bb_node->id - 1] = block_node->id;
                        node::label(*block_node, format::format(
                            "block: {}",
                            *(bb_node->value)));
                        node::shape(*block_node, shape_t::record);
                        node::style(*block_node, node_style_t::filled);
                        node::fill_color(*block_node, color_t::aliceblue);
                    }

                    for (auto bb_node : e.bb_graph.nodes) {
                        array::reset(nodes);
                        digraph::incoming_nodes(e.bb_graph, bb_node, nodes);
                        for (auto incoming : nodes) {
                            auto edge = graph::make_edge(bb_sg);
                            edge->first  = block_node_ids[incoming->id - 1];
                            edge->second = block_node_ids[bb_node->id - 1];
                            edge::label(*edge, "pred"_ss);
                            edge::style(*edge, edge_style_t::dotted);
                            edge::dir(*edge, dir_type_t::back);
                        }

                        array::reset(nodes);
                        digraph::outgoing_nodes(e.bb_graph, bb_node, nodes);
                        for (auto outgoing : nodes) {
                            auto edge = graph::make_edge(bb_sg);
                            edge->first  = block_node_ids[bb_node->id - 1];
                            edge->second = block_node_ids[outgoing->id - 1];
                            edge::label(*edge, "succ"_ss);
                            edge::dir(*edge, dir_type_t::forward);
                        }

                        if (bb_node->value->next) {
                            auto straight_edge = graph::make_edge(bb_sg);
                            straight_edge->first  = block_node_ids[bb_node->id - 1];
                            straight_edge->second = block_node_ids[bb_node->value->next->node->id - 1];
                            edge::label(*straight_edge, "next"_ss);
                            edge::dir(*straight_edge, dir_type_t::forward);
                            edge::color(*straight_edge, color_t::lightblue);
                            edge::style(*straight_edge, edge_style_t::dashed);
                        }
                    }
                }

                {
                    u32 var_node_ids[e.var_graph.size];
                    var_digraph_t::Node_Array nodes{};
                    array::init(nodes, e.alloc);
                    defer(array::free(nodes));

                    str_t str{};
                    str::init(str, e.alloc);
                    for (auto vv_node : e.var_graph.nodes) {
                        auto var_node = graph::make_node(var_sg);
                        var_node_ids[vv_node->id - 1] = var_node->id;
                        {
                            str::reset(str); {
                                str_buf_t buf{&str};
                                auto var = vv_node->value;
                                format::format_to(buf, "{}", *var);
//                                if (var->accesses.size > 0) {
//                                    format::format_to(buf, "\n");
//                                    for (u32 i = 0; i < var->accesses.size; ++i) {
//                                        if (i > 0) format::format_to(buf, "\n");
//                                        const auto& ac = var->accesses[i];
//                                        format::format_to(buf,
//                                                       "{}({})",
//                                                       virtual_var::access_type::name(ac.type),
//                                                       ac.inst_id);
//                                    }
//                                }
                            }
                        }
                        node::label(*var_node, str);
                        node::shape(*var_node, shape_t::component);
                        node::style(*var_node, node_style_t::filled);
                        node::fill_color(*var_node, color_t::lavender);
                    }

                    for (auto vv_node : e.var_graph.nodes) {
                        array::reset(nodes);
                        digraph::outgoing_nodes(e.var_graph, vv_node, nodes);
                        for (auto outgoing : nodes) {
                            auto edge = graph::make_edge(var_sg);
                            edge->first  = var_node_ids[vv_node->id - 1];
                            edge->second = var_node_ids[outgoing->id - 1];
//                            edge::tail_label(*edge, "next"_ss);
                            edge::dir(*edge, dir_type_t::both);
                            edge::arrow_tail(*edge, arrow_type_t::dot);
                            edge::arrow_head(*edge, arrow_type_t::normal);
                        }
//                        auto var = vv_node->value;
//                        for (const auto& ac : var->accesses) {
//                            const auto& inst = e.insts[ac.inst_id];
//                            auto edge = graph::make_edge(g);
//                            edge->first  = block_node_ids[inst.block_id - 1];
//                            edge->second = var_node_ids[vv_node->id - 1];
//                            edge::head_label(*edge, virtual_var::access_type::name(ac.type));
//                            edge::color(*edge, color_t::pink);
//                            edge::dir(*edge, dir_type_t::forward);
//                        }
                    }
                }


                buf_t buf{};
                buf.mode = buf_mode_t::alloc;
                buf::init(buf);
                defer(buf::free(buf));
                {
                    auto status = graphviz::graph::serialize(g, buf);
                    if (!OK(status))
                        return status_t::fail;
                }
                {
                    auto status = buf::save(buf, path);
                    if (!OK(status))
                        return status_t::fail;
                }

                return status_t::ok;
            }

            status_t assemble(emitter_t& e, bb_t& start_block) {
                auto& vm = *e.vm;
                u64  addr = LP;
                auto curr = &start_block;

                // assign block addresses
                while (curr) {
                    curr->addr = addr;
                    addr += curr->insts.size() * sizeof(encoded_inst_t);
                    curr = curr->next;
                }

                // emit blocks to vm heap
                curr = &start_block;
                addr = LP;
                while (curr) {
                    for (u32 i = curr->insts.sidx; i < curr->insts.eidx; ++i) {
                        const auto& inst = e.insts[i];
                        if (inst.block_id != curr->id)
                            continue;
                        auto status = encode_inst(e, inst, addr);
                        if (!OK(status))
                            return status;
                        addr += sizeof(encoded_inst_t);
                    }
                    curr = curr->next;
                }
                LP = addr;
                return status_t::ok;
            }

            u0 init(emitter_t& e, vm_t* vm, u64 addr, alloc_t* alloc) {
                e.vm    = vm;
                e.addr  = addr;
                e.alloc = alloc;
                array::init(e.insts, e.alloc);
                symtab::init(e.vartab, e.alloc);
                array::init(e.comments, e.alloc);
                digraph::init(e.bb_graph, e.alloc);
                str_array::init(e.strtab, e.alloc);
                digraph::init(e.var_graph, e.alloc);
                stable_array::init(e.vars, e.alloc);
                stable_array::init(e.blocks, e.alloc);
            }
        }

        namespace bytecode {
            namespace rf = register_file;
            namespace op = instruction::type;

            bb_t& leave(bb_t& bb) {
                auto& entry_block = emitter::make_basic_block(*bb.emit,
                                                              "proc_epilogue"_ss,
                                                              &bb);
                auto& exit_block = emitter::make_basic_block(*bb.emit,
                                                             "proc_exit"_ss,
                                                             &entry_block);
                basic_block::encode(bb)
                    .imm1()
                        .op(op::br)
                        .value(&entry_block)
                        .build();
                basic_block::encode(entry_block)
                    .comment("*"_ss)
                    .comment("**"_ss)
                    .comment("*** proc epilogue"_ss)
                    .reg2()
                        .op(op::move)
                        .src(rf::fp)
                        .dst(rf::sp)
                        .build()
                    .reg2()
                        .op(op::pop)
                        .src(rf::sp)
                        .dst(rf::lr)
                        .build()
                    .reg1()
                        .op(op::ret)
                        .dst(rf::lr)
                        .build();
                return exit_block;
            }

            compile_result_t fn(compiler_t& comp,
                                const context_t& c,
                                obj_t* sym,
                                obj_t* form,
                                obj_t* args) {
                auto& vm = *comp.vm;
                auto ctx        = c.ctx;
                b8   is_mac     = PRIM(form) == prim_type_t::mac;
                auto proc       = make_proc(c.ctx, sym, CAR(args), CDR(args), is_mac);
                auto idx        = OBJ_IDX(proc);
                auto target_reg = rf::m;       //FIXME
                G(target_reg) = idx;
                if (!c.top_level) {
                    basic_block::encode(c.bb)
                        .comment(format::format("literal: {}", scm::to_string(c.ctx, c.obj)))
                        .imm2()
                            .op(op::const_)
                            .src(u32(idx))
                            .dst(target_reg)
                            .build();
                }
                return {c.bb, 0};
            }

            bb_t& enter(bb_t& bb, u32 locals) {
                auto& entry_block = emitter::make_basic_block(*bb.emit, "proc_prologue"_ss, &bb);
                auto& exit_block  = emitter::make_basic_block(*bb.emit, "proc_begin"_ss, &entry_block);
                basic_block::encode(bb)
                    .imm1()
                        .op(op::br)
                        .value(&entry_block)
                        .build();
                basic_block::encode(entry_block)
                    .comment("*** proc prologue"_ss)
                    .comment("**"_ss)
                    .comment("*"_ss)
                    .reg2()
                        .op(op::push)
                        .src(rf::lr)
                        .dst(rf::sp)
                        .build()
                    .reg2()
                        .op(op::move)
                        .src(rf::sp)
                        .dst(rf::fp)
                        .build()
                    .imm1()
                        .op(op::br)
                        .value(&exit_block)
                        .build();
                if (locals > 0) {
                    basic_block::encode(exit_block)
                        .imm2()
                            .src(locals)
                            .dst(rf::sp)
                            .op(op::sub)
                            .build();
                }
                return exit_block;
            }

            compile_result_t ffi(compiler_t& comp,
                                 const context_t& c,
                                 obj_t* sym,
                                 obj_t* form,
                                 obj_t* args) {
                todo(*c.bb, "XXX: ffi call"_ss);
                return {c.bb, {}};
            }

            u0 free_stack(bb_t& bb, u32 words) {
                basic_block::encode(bb)
                    .imm2()
                        .op(op::add)
                        .src(words * 8)
                        .dst(rf::sp)
                        .build();
            }

            u0 todo(bb_t& bb, str::slice_t msg) {
                basic_block::encode(bb)
                    .comment(msg)
                    .none()
                        .op(op::nop)
                        .build();
            }

            compile_result_t apply(compiler_t& comp,
                                   const context_t& c,
                                   obj_t* sym,
                                   obj_t* form,
                                   obj_t* args) {
                auto ctx  = c.ctx;
                auto proc = PROC(form);
                auto tmp       = emitter::virtual_var::get(comp.emit, "_"_ss);
                auto base_addr = emitter::virtual_var::get(comp.emit, "base"_ss);
                auto& apply_bb = emitter::make_basic_block(comp.emit, "apply"_ss, c.bb);
                basic_block::encode(apply_bb)
                    .reg2()
                        .op(op::env)
                        .src(rf::ep)
                        .dst(&tmp)
                        .build()
                    .reg2()
                        .op(op::push)
                        .src(&tmp)
                        .dst(rf::ep)
                        .build();
                alloc_stack(apply_bb, 1, &base_addr);
                auto keys = proc->params;
                auto vals = args;
                auto vc = c;
                vc.bb = &apply_bb;
                while (!IS_NIL(keys)) {
                    auto key  = TYPE(keys) == obj_type_t::pair ? CAR(keys) : keys;
                    auto name = *string::interned::get_slice(STRING_ID(key));
                    if (TYPE(keys) != obj_type_t::pair) {
                        vc.obj = vals;
                        auto comp_res = compiler::compile(comp, vc);
                        vc.bb = comp_res.bb;
                        basic_block::encode(vc.bb)
                            .comment(format::format("symbol: {}", name))
                            .imm2()
                                .src(u32(OBJ_IDX(keys)))
                                .op(op::set)
                                .dst(&comp_res.var)
                                .mode(true)
                                .build();
                        break;
                    } else {
                        vc.obj = CAR(vals);
                        auto comp_res = compiler::compile(comp, vc);
                        vc.bb = comp_res.bb;
                        basic_block::encode(vc.bb)
                            .comment(format::format("symbol: {}", name))
                            .imm2()
                                .src(u32(OBJ_IDX(key)))
                                .op(op::set)
                                .dst(&comp_res.var)
                                .mode(true)
                                .build();
                        keys = CDR(keys);
                        vals = CDR(vals);
                    }
                }

                basic_block::encode(vc.bb)
                    .comment(format::format(
                        "call: {}",
                        printable_t{c.ctx, sym, true}))
                    .imm1()
                        .op(op::blr)
                        .value(proc->addr.bb)
                        .build();

                auto res = emitter::virtual_var::get(comp.emit, "res"_ss);
                auto& cleanup_bb = emitter::make_basic_block(comp.emit,
                                                             "apply_cleanup"_ss,
                                                             vc.bb);
                basic_block::encode(cleanup_bb)
                    .comment("load return value from proc call"_ss)
                    .offs()
                        .op(op::load)
                        .src(&base_addr)
                        .dst(&res)
                        .offset(0)
                        .build();
                free_stack(cleanup_bb, 1);
                basic_block::encode(cleanup_bb)
                    .comment("drop apply env"_ss)
                    .reg2()
                        .op(op::pop)
                        .src(rf::ep)
                        .dst(&tmp)
                        .build();
                return {&cleanup_bb, res};
            }

            compile_result_t inline_(compiler_t& comp,
                                     const context_t& c,
                                     obj_t* sym,
                                     obj_t* form,
                                     obj_t* args) {
                UNUSED(sym);
                UNUSED(form);
                UNUSED(args);
                todo(*c.bb, "XXX: macro call"_ss);
                return {c.bb, 0};
            }

            compile_result_t comp_proc(compiler_t& comp,
                                       const context_t& c,
                                       proc_t* proc) {
                auto ctx = c.ctx;
                auto& e = *c.bb->emit;

                auto params = proc->params;
                while (!IS_NIL(params)) {
                    auto name = *string::interned::get_slice(STRING_ID(CAR(params)));
                    emitter::virtual_var::declare(e, name);
                    params = CDR(params);
                }

                auto name = *string::interned::get_slice(STRING_ID(proc->sym));
                auto& proc_bb = emitter::make_basic_block(*c.bb->emit, name, c.bb);
                proc->addr.bb = &proc_bb;

                basic_block::encode(proc_bb)
                    .note("-------------------------------------------------------------------"_ss)
                    .note(format::format("procedure: {}", name))
                    .note(format::format("(fn {} {})",
                                         printable_t{c.ctx, proc->params, true},
                                         printable_t{c.ctx, CAR(proc->body), true}))
                    .note("-------------------------------------------------------------------"_ss);

                auto body = proc->body;
                auto bc   = c;
                bc.bb = &enter(proc_bb, 0);
                while (!IS_NIL(body)) {
                    bc.obj = CAR(body);
                    auto comp_res = compiler::compile(comp, bc);
                    bc.bb = comp_res.bb;
                    body = CDR(body);
                }

                auto res = emitter::virtual_var::get(comp.emit, "res"_ss);
                auto base = emitter::virtual_var::get(comp.emit, "base"_ss);
                basic_block::encode(bc.bb)
                    .offs()
                        .op(op::store)
                        .src(&res)
                        .dst(&base)
                        .offset(0)
                        .mode(true)
                        .build();

                proc->is_compiled = true;
                return {&leave(*bc.bb), res};
            }

            compile_result_t prim(compiler_t& comp,
                                  const context_t& c,
                                  obj_t* sym,
                                  obj_t* form,
                                  obj_t* args,
                                  prim_type_t type) {
                switch (type) {
                    case prim_type_t::is:
                    case prim_type_t::gt:
                    case prim_type_t::lt:
                    case prim_type_t::lte:
                    case prim_type_t::gte:              return cmp_op(comp, c, type, args);
                    case prim_type_t::add:              return arith_op(comp, c, op::ladd, args);
                    case prim_type_t::sub:              return arith_op(comp, c, op::lsub, args);
                    case prim_type_t::mul:              return arith_op(comp, c, op::lmul, args);
                    case prim_type_t::div:              return arith_op(comp, c, op::ldiv, args);
                    case prim_type_t::mod:              return arith_op(comp, c, op::lmod, args);
                    case prim_type_t::let:
                    case prim_type_t::set:              return let_set(comp, c, args);
                    case prim_type_t::fn:
                    case prim_type_t::mac:              return fn(comp, c, c.ctx->nil, form, args);
                    case prim_type_t::if_:              return if_(comp, c, args);
                    case prim_type_t::do_:              return do_(comp, c, args);
                    case prim_type_t::or_:              return or_(comp, c, args);
                    case prim_type_t::and_:             return and_(comp, c, args);
                    case prim_type_t::not_:             return not_(comp, c, args);
                    case prim_type_t::car:              return car(comp, c, args);
                    case prim_type_t::cdr:              return cdr(comp, c, args);
                    case prim_type_t::cons:             return cons(comp, c, args);
                    case prim_type_t::atom:             return atom(comp, c, args);
                    case prim_type_t::eval:             return eval(comp, c, args);
                    case prim_type_t::list:             return list(comp, c, args);
                    case prim_type_t::while_:           return while_(comp, c, args);
                    case prim_type_t::setcar:           return set_car(comp, c, args);
                    case prim_type_t::setcdr:           return set_cdr(comp, c, args);
                    case prim_type_t::error:            return error(comp, c, args);
                    case prim_type_t::print:            return print(comp, c, args);
                    case prim_type_t::format:           return format(comp, c, args);
                    case prim_type_t::quote:            return qt(comp, c, args);
                    case prim_type_t::unquote:          return uq(comp, c, args);
                    case prim_type_t::quasiquote:       return qq(comp, c, args);
                    case prim_type_t::unquote_splicing: return uqs(comp, c, args);
                    default:                            return {c.bb, 0};
                }
            }

            compile_result_t cmp_op(compiler_t& comp,
                                    const context_t& c,
                                    prim_type_t type,
                                    obj_t* args) {
                auto ctx  = c.ctx;
                auto oc = c;
                oc.obj = CAR(args);
                auto lhs_res = compiler::compile(comp, oc);
                oc.bb  = lhs_res.bb;
                oc.obj = CADR(args);
                auto rhs_res = compiler::compile(comp, oc);
                auto res     = emitter::virtual_var::get(comp.emit, "res"_ss);

                switch (type) {
                    case prim_type_t::is:
                        basic_block::encode(rhs_res.bb)
                            .comment("prim: is"_ss)
                            .reg2()
                                .op(op::lcmp)
                                .src(&lhs_res.var)
                                .dst(&rhs_res.var)
                                .build()
                            .reg1()
                                .op(op::seq)
                                .dst(&res)
                                .build();
                        break;

                    case prim_type_t::lt:
                        basic_block::encode(rhs_res.bb)
                            .comment("prim: lt"_ss)
                            .reg2()
                                .op(op::lcmp)
                                .src(&lhs_res.var)
                                .dst(&rhs_res.var)
                                .build()
                            .reg1()
                                .op(op::sl)
                                .dst(&res)
                                .build();
                        break;

                    case prim_type_t::gt:
                        basic_block::encode(rhs_res.bb)
                            .comment("prim: gt"_ss)
                            .reg2()
                                .op(op::lcmp)
                                .src(&lhs_res.var)
                                .dst(&rhs_res.var)
                                .build()
                            .reg1()
                                .op(op::sg)
                                .dst(&res)
                                .build();
                        break;

                    case prim_type_t::lte:
                        basic_block::encode(rhs_res.bb)
                            .comment("prim: lte"_ss)
                            .reg2()
                                .op(op::lcmp)
                                .src(&lhs_res.var)
                                .dst(&rhs_res.var)
                                .build()
                            .reg1()
                                .op(op::sle)
                                .dst(&res)
                                .build();
                        break;

                    case prim_type_t::gte:
                        basic_block::encode(rhs_res.bb)
                            .comment("prim: gte"_ss)
                            .reg2()
                                .op(op::lcmp)
                                .src(&lhs_res.var)
                                .dst(&rhs_res.var)
                                .build()
                            .reg1()
                                .op(op::sge)
                                .dst(&res)
                                .build();
                        break;

                    default:
                        error(ctx, "unknown compare prim");
                }

                return {c.bb, res};
            }

            compile_result_t arith_op(compiler_t& comp,
                                      const context_t& c,
                                      op_code_t op_code,
                                      obj_t* args) {
                auto ctx       = c.ctx;
                u32  size      = length(ctx, args);
                auto res       = emitter::virtual_var::get(comp.emit, "res"_ss);
                auto base_addr = emitter::virtual_var::get(comp.emit, "base"_ss);
                alloc_stack(*c.bb, size, &base_addr);
                s32  offs = 0;
                auto ac   = c;
                while (!IS_NIL(args)) {
                    ac.obj = CAR(args);
                    auto comp_res = compiler::compile(comp, ac);
                    ac.bb = comp_res.bb;
                    basic_block::encode(ac.bb)
                        .offs()
                            .op(op::store)
                            .offset(offs)
                            .src(&comp_res.var)
                            .dst(&base_addr)
                            .mode(true)
                            .build();
                    args = CDR(args);
                    offs += 8;
                }
                basic_block::encode(*ac.bb)
                    .reg2_imm()
                        .op(op_code)
                        .src(&base_addr)
                        .dst(&res)
                        .value(size * 8)
                        .build();
                free_stack(*ac.bb, size);
                return {ac.bb, res};
            }

            compile_result_t call_back(compiler_t& comp,
                                       const context_t& c,
                                       obj_t* sym,
                                       obj_t* form,
                                       obj_t* args) {
                UNUSED(sym);
                UNUSED(form);
                UNUSED(args);
                todo(*c.bb, "XXX: cfunc call"_ss);
                return {c.bb, 0};
            }

            u0 alloc_stack(bb_t& bb, u32 words, var_t** base_addr) {
                basic_block::encode(bb)
                    .imm2()
                        .op(op::sub)
                        .src(words * 8)
                        .dst(rf::sp)
                        .build()
                    .offs()
                        .op(op::lea)
                        .offset(-(words * 8))
                        .dst(base_addr)
                        .src(rf::sp)
                        .build();
            }

            compile_result_t lookup(compiler_t& comp, const context_t& c) {
                auto ctx = c.ctx;
                auto sym = scm::to_string(c.ctx, c.obj);
                auto res = emitter::virtual_var::get(*c.bb->emit, sym);
                basic_block::encode(c.bb)
                    .comment(format::format("symbol: {}", sym))
                    .imm2()
                        .src((u32) OBJ_IDX(c.obj))
                        .op(op::get)
                        .dst(&res)
                        .build();
                return {c.bb, res};
            }

            compile_result_t self_eval(compiler_t& comp, const context_t& c) {
                auto ctx = c.ctx;
                auto lit = emitter::virtual_var::get(comp.emit, "lit"_ss);
                basic_block::encode(c.bb)
                    .comment(format::format(
                        "literal: {}",
                        scm::to_string(c.ctx, c.obj)))
                    .imm2()
                        .op(op::const_)
                        .src(u32(OBJ_IDX(c.obj)))
                        .dst(&lit)
                        .build();
                return {c.bb, lit};
            }

            compile_result_t qt(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto tmp = emitter::virtual_var::get(comp.emit, "_"_ss);
                auto res = emitter::virtual_var::get(comp.emit, "res"_ss);
                basic_block::encode(c.bb)
                    .comment(format::format(
                        "literal: {}",
                        scm::to_string(c.ctx, c.obj)))
                    .imm2()
                        .op(op::const_)
                        .src(u32(OBJ_IDX(CAR(args))))
                        .dst(&tmp)
                        .build()
                    .reg2()
                        .op(op::qt)
                        .src(&tmp)
                        .dst(&res)
                        .build();
                return {c.bb, res};
            }

            compile_result_t qq(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto tmp = emitter::virtual_var::get(comp.emit, "_"_ss);
                auto res = emitter::virtual_var::get(comp.emit, "res"_ss);
                basic_block::encode(c.bb)
                    .comment(format::format(
                        "literal: {}",
                        scm::to_string(c.ctx, c.obj)))
                    .imm2()
                        .op(op::const_)
                        .src(u32(OBJ_IDX(CAR(args))))
                        .dst(&tmp)
                        .build()
                    .reg2()
                        .op(op::qq)
                        .src(&tmp)
                        .dst(&res)
                        .build();
                return {c.bb, res};
            }

            compile_result_t uq(compiler_t& comp, const context_t& c, obj_t* args) {
                UNUSED(args);
                scm::error(c.ctx, "unquote is not valid in this context.");
                return {c.bb, 0};
            }

            compile_result_t car(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto res = emitter::virtual_var::get(comp.emit, "res"_ss);
                auto cc  = c;
                cc.obj = CAR(args);
                auto comp_res = compiler::compile(comp, cc);
                basic_block::encode(comp_res.bb)
                    .reg2()
                        .op(op::car)
                        .src(&comp_res.var)
                        .dst(&res)
                        .build();
                return {comp_res.bb, res};
            }

            compile_result_t cdr(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto res = emitter::virtual_var::get(comp.emit, "res"_ss);
                auto cc  = c;
                cc.obj = CAR(args);
                auto comp_res = compiler::compile(comp, cc);
                basic_block::encode(comp_res.bb)
                    .reg2()
                        .op(op::cdr)
                        .src(&comp_res.var)
                        .dst(&res)
                        .build();
                return {comp_res.bb, res};
            }

            compile_result_t or_(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto& exit_bb = emitter::make_basic_block(comp.emit,
                                                          "or_exit"_ss,
                                                          c.bb);
                auto res = emitter::virtual_var::get(comp.emit, "res"_ss);
                auto tmp = emitter::virtual_var::get(comp.emit, "_"_ss);
                auto oc  = c;
                while (!IS_NIL(args)) {
                    oc.obj        = CAR(args);
                    auto comp_res = compiler::compile(comp, oc);
                    oc.bb = comp_res.bb;
                    basic_block::encode(oc.bb)
                        .reg2()
                            .op(op::truep)
                            .src(&comp_res.var)
                            .dst(&tmp)
                            .build()
                        .imm1()
                            .op(op::beq)
                            .value(&exit_bb)
                            .build();
                    args = CDR(args);
                }
                basic_block::encode(oc.bb)
                    .imm1()
                        .op(op::br)
                        .value(&exit_bb)
                        .build();
                return {&exit_bb, res};
            }

            compile_result_t do_(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto dc = c;
                while (!IS_NIL(args)) {
                    dc.obj   = CAR(args);
                    auto res = compiler::compile(comp, dc);
                    dc.bb = res.bb;
                    args = CDR(args);
                }
                return {dc.bb, 0};
            }

            compile_result_t uqs(compiler_t& comp, const context_t& c, obj_t* args) {
                UNUSED(args);
                scm::error(c.ctx, "unquote-splicing is not valid in this context.");
                return {c.bb, 0};
            }

            compile_result_t if_(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;

                auto& true_bb  = emitter::make_basic_block(*c.bb->emit,
                                                           "if_true"_ss,
                                                           c.bb);
                auto& false_bb = emitter::make_basic_block(*c.bb->emit,
                                                           "if_false"_ss,
                                                           &true_bb);
                auto& exit_bb  = emitter::make_basic_block(*c.bb->emit,
                                                           "if_exit"_ss,
                                                           &false_bb);

                auto tmp = emitter::virtual_var::get(comp.emit, "_"_ss);
                auto res = emitter::virtual_var::get(comp.emit, "res"_ss);

                auto ic = c;
                ic.obj = CAR(args);
                auto pred_res = compiler::compile(comp, ic);
                basic_block::encode(c.bb)
                    .next(true_bb)
                    .reg2()
                        .op(op::truep)
                        .src(&pred_res.var)
                        .dst(&tmp)
                        .build()
                    .imm1()
                        .op(op::bne)
                        .value(&false_bb)
                        .build();

                ic.bb  = &true_bb;
                ic.obj = CADR(args);
                auto true_res = compiler::compile(comp, ic);
                basic_block::encode(true_res.bb)
                    .next(false_bb)
                    .reg2()
                        .op(op::move)
                        .src(&true_res.var)
                        .dst(&res)
                        .build()
                    .imm1()
                        .op(op::br)
                        .value(&exit_bb)
                        .build();

                ic.bb  = &false_bb;
                ic.obj = CADDR(args);
                auto false_res = compiler::compile(comp, ic);
                basic_block::encode(false_res.bb)
                    .next(exit_bb)
                    .reg2()
                        .op(op::move)
                        .src(&false_res.var)
                        .dst(&res)
                        .build();

                return {&exit_bb, res};
            }

            compile_result_t cons(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto res = emitter::virtual_var::get(comp.emit, "res"_ss);
                auto cc  = c;
                cc.obj = CAR(args);
                auto lhs_res = compiler::compile(comp, cc);
                cc.bb  = lhs_res.bb;
                cc.obj = CADR(args);
                auto rhs_res = compiler::compile(comp, cc);
                basic_block::encode(rhs_res.bb)
                    .reg3()
                        .op(op::cons)
                        .a(&lhs_res.var)
                        .b(&rhs_res.var)
                        .c(&res)
                        .build();
                return {rhs_res.bb, res};
            }

            compile_result_t atom(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto res = emitter::virtual_var::get(comp.emit, "res"_ss);
                auto ac  = c;
                ac.obj = CAR(args);
                auto comp_res = compiler::compile(comp, ac);
                basic_block::encode(comp_res.bb)
                    .reg2()
                        .op(op::atomp)
                        .src(&comp_res.var)
                        .dst(&res)
                        .build();
                return {comp_res.bb, res};
            }

            compile_result_t eval(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto res = emitter::virtual_var::get(comp.emit, "res"_ss);
                auto ec  = c;
                ec.obj = CAR(args);
                auto comp_res = compiler::compile(comp, ec);
                basic_block::encode(comp_res.bb)
                    .reg2()
                        .op(op::eval)
                        .src(&comp_res.var)
                        .dst(&res)
                        .build();
                return {comp_res.bb, res};
            }

            compile_result_t list(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx       = c.ctx;
                u32  size      = length(ctx, args);
                auto base_addr = emitter::virtual_var::get(comp.emit, "base"_ss);
                alloc_stack(*c.bb, size, &base_addr);
                s32  offs = 0;
                auto lc   = c;
                while (!IS_NIL(args)) {
                    lc.obj   = CAR(args);
                    auto res = compiler::compile(comp, lc);
                    lc.bb = res.bb;
                    basic_block::encode(res.bb)
                        .offs()
                            .op(op::store)
                            .offset(offs)
                            .src(&res.var)
                            .dst(&base_addr)
                            .mode(true)
                            .build();
                    args = CDR(args);
                    offs += 8;
                }
                auto res  = emitter::virtual_var::get(comp.emit, "res"_ss);
                auto& list_bb = emitter::make_basic_block(*c.bb->emit,
                                                          "make_list"_ss,
                                                          lc.bb);
                basic_block::encode(list_bb)
                    .reg2_imm()
                        .op(op::list)
                        .src(&base_addr)
                        .dst(&res)
                        .value(size * 8)
                        .build();
                free_stack(list_bb, size);
                return {&list_bb, res};
            }

            compile_result_t and_(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto& exit_bb = emitter::make_basic_block(comp.emit,
                                                          "and_exit"_ss,
                                                          c.bb);
                auto tmp = emitter::virtual_var::get(comp.emit, "_"_ss);
                auto res = emitter::virtual_var::get(comp.emit, "res"_ss);
                auto oc  = c;
                while (!IS_NIL(args)) {
                    oc.obj        = CAR(args);
                    auto comp_res = compiler::compile(comp, oc);
                    oc.bb = comp_res.bb;
                    basic_block::encode(oc.bb)
                        .reg2()
                            .op(op::move)
                            .src(&comp_res.var)
                            .dst(&res)
                            .build()
                        .reg2()
                            .op(op::truep)
                            .src(&res)
                            .dst(&tmp)
                            .build()
                        .imm1()
                            .op(op::bne)
                            .value(&exit_bb)
                            .build();
                    args = CDR(args);
                }
                return {&exit_bb, res};
            }

            compile_result_t not_(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto nc  = c;
                nc.obj = CAR(args);
                auto res      = emitter::virtual_var::get(comp.emit, "res"_ss);
                auto comp_res = compiler::compile(comp, nc);
                basic_block::encode(comp_res.bb)
                    .reg2()
                        .op(op::lnot)
                        .src(&comp_res.var)
                        .dst(&res)
                        .build();
                return {comp_res.bb, res};
            }

            compile_result_t error(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto res = emitter::virtual_var::get(comp.emit, "res"_ss);
                auto tmp = emitter::virtual_var::get(comp.emit, "_"_ss);
                basic_block::encode(c.bb)
                    .comment(format::format(
                        "literal: {}",
                        scm::to_string(c.ctx, c.obj)))
                    .imm2()
                        .op(op::const_)
                        .src(u32(OBJ_IDX(args)))
                        .dst(&tmp)
                        .build()
                    .reg2()
                        .op(op::error)
                        .src(&tmp)
                        .dst(&res)
                        .build();
                basic_block::encode(c.bb)
                    .comment(format::format("literal: {}",
                                            to_string(c.ctx, args)),
                             c.bb->insts.size() - 1);
                return {c.bb, res};
            }

            compile_result_t print(compiler_t& comp, const context_t& c, obj_t* args) {
                UNUSED(comp);
                UNUSED(args);
                return {c.bb, 0};
            }

            compile_result_t format(compiler_t& comp, const context_t& c, obj_t* args) {
                UNUSED(comp);
                UNUSED(args);
                return compile_result_t();
            }

            compile_result_t while_(compiler_t& comp, const context_t& c, obj_t* args) {
                UNUSED(comp);
                UNUSED(args);
                return {c.bb, 0};
            }

            compile_result_t let_set(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto& vm = *comp.vm;
                auto key      = CAR(args);
                u32  idx      = OBJ_IDX(key);
                auto key_name = *string::interned::get_slice(STRING_ID(OBJ_AT(idx)));
                auto res      = emitter::virtual_var::declare(comp.emit, key_name);
                auto vc       = c;
                vc.obj = CADR(args);
                auto comp_res = compiler::compile(comp, vc);
                if (c.top_level) {
                    auto value = OBJ_AT(G(rf::m));  // FIXME: this is a temporary hack
                    set(ctx, key, value);
                    if (TYPE(value) == obj_type_t::func
                    ||  TYPE(value) == obj_type_t::macro) {
                        auto proc = PROC(value);
                        if (!proc->is_compiled) {
                            auto pc = c;
                            pc.top_level = false;
                            proc->sym    = key;
                            return comp_proc(comp, pc, proc);
                        }
                    }
                }
                basic_block::encode(comp_res.bb)
                    .comment(format::format(
                        "symbol: {}",
                        printable_t{c.ctx, key, true}))
                    .imm2()
                        .op(op::set)
                        .src(u32(idx))
                        .dst(&comp_res.var)
                        .mode(true)
                        .build()
                    .reg2()
                        .op(op::move)
                        .src(&comp_res.var)
                        .dst(&res)
                        .build();
                return {comp_res.bb, res};
            }

            compile_result_t set_cdr(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto sc  = c;
                sc.obj = CADR(args);
                auto rhs_res = compiler::compile(comp, sc);
                sc.bb  = rhs_res.bb;
                sc.obj = CAR(args);
                auto lhs_res = compiler::compile(comp, sc);
                basic_block::encode(lhs_res.bb)
                    .reg2()
                        .op(op::setcdr)
                        .src(&lhs_res.var)
                        .dst(&rhs_res.var)
                        .build();
                return {lhs_res.bb, {}};
            }

            compile_result_t set_car(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto sc  = c;
                sc.obj = CADR(args);
                auto rhs_res = compiler::compile(comp, sc);
                sc.bb  = rhs_res.bb;
                sc.obj = CAR(args);
                auto lhs_res = compiler::compile(comp, sc);
                basic_block::encode(lhs_res.bb)
                    .reg2()
                        .op(op::setcar)
                        .src(&lhs_res.var)
                        .dst(&rhs_res.var)
                        .build();
                return {lhs_res.bb, {}};
            }
        }
    }
}
