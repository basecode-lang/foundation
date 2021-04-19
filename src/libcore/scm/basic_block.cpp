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

#include <basecode/core/scm/emitter.h>
#include <basecode/core/scm/basic_block.h>

namespace basecode::scm::basic_block {
    namespace op = basecode::scm::vm::instruction;
    namespace rf = basecode::scm::vm::register_file;

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
        _inst = emitter::make_instruction(*_em,
                                          *_bb,
                                          op::encoding::imm);
        _imm1.reset();
        return _imm1;
    }

    imm2_builder_t& bb_builder_t::imm2() {
        _inst = emitter::make_instruction(*_em,
                                          *_bb,
                                          op::encoding::imm);
        _imm2.reset();
        return _imm2;
    }

    reg1_builder_t& bb_builder_t::reg1() {
        _inst = emitter::make_instruction(*_em,
                                          *_bb,
                                          op::encoding::reg1);
        _reg1.reset();
        return _reg1;
    }

    reg2_builder_t& bb_builder_t::reg2() {
        _inst = emitter::make_instruction(*_em,
                                          *_bb,
                                          op::encoding::reg2);
        _reg2.reset();
        return _reg2;
    }

    none_builder_t& bb_builder_t::none() {
        _inst = emitter::make_instruction(*_em,
                                          *_bb,
                                          op::encoding::none);
        _none.reset();
        return _none;
    }

    reg3_builder_t& bb_builder_t::reg3() {
        _inst = emitter::make_instruction(*_em,
                                          *_bb,
                                          op::encoding::reg3);
        _reg3.reset();
        return _reg3;
    }

    offs_builder_t& bb_builder_t::offs() {
        _inst = emitter::make_instruction(*_em,
                                          *_bb,
                                          op::encoding::offset);
        return _offs;
    }

    reg2_imm_builder_t& bb_builder_t::reg2_imm() {
        _inst = emitter::make_instruction(*_em,
                                          *_bb,
                                          op::encoding::reg2_imm);
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
            case op::type::br:
            case op::type::bl:
            case op::type::bg:
            case op::type::ble:
            case op::type::bge:
            case op::type::beq:
            case op::type::bne:
            case op::type::blr:
            case op::type::bra: {
                auto& imm = _builder->_inst->operands[0];
                digraph::make_edge(_builder->_em->bb_graph,
                                   _builder->_bb->node,
                                   imm.kind.branch.bb->node);
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

    imm2_builder_t& imm2_builder_t::dst(var_version_t** var) {
        _dst = var;
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

    reg1_builder_t& reg1_builder_t::op(op_code_t op_code) {
        _builder->_inst->type = op_code;
        return *this;
    }

    reg1_builder_t& reg1_builder_t::dst(var_version_t** var) {
        _dst = var;
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
                case op::type::cmp:
                case op::type::lcmp:
                case op::type::setcar:
                case op::type::setcdr:
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

    reg2_builder_t& reg2_builder_t::is_signed(b8 flag) {
        _builder->_inst->is_signed = flag;
        return *this;
    }

    reg2_builder_t& reg2_builder_t::op(op_code_t op_code) {
        _builder->_inst->type = op_code;
        return *this;
    }

    reg2_builder_t& reg2_builder_t::src(var_version_t** var) {
        _src = var;
        return *this;
    }

    reg2_builder_t& reg2_builder_t::dst(var_version_t** var) {
        _dst = var;
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

    reg2_imm_builder_t& reg2_imm_builder_t::src(var_version_t** var) {
        _src = var;
        return *this;
    }

    reg2_imm_builder_t& reg2_imm_builder_t::dst(var_version_t** var) {
        _dst = var;
        return *this;
    }

    // --------------------------------------------

    bb_builder_t& offs_builder_t::build() {
        if (_src) {
            switch (_builder->_inst->type) {
                case op::type::lea:
                case op::type::load:
                case op::type::store:
                    emitter::virtual_var::read(*_builder->_em,
                                               *_src,
                                               _builder->_inst->id);
                    break;
            }
            _builder->_inst->operands[0] = _builder->operand(*_src);
        }
        if (_dst) {
            switch (_builder->_inst->type) {
                case op::type::lea:
                case op::type::load:
                    *_dst = emitter::virtual_var::write(*_builder->_em,
                                                        *_dst,
                                                        _builder->_inst->id);
                    break;
                case op::type::store:
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

    offs_builder_t& offs_builder_t::offset(s32 value) {
        _builder->_inst->operands[2] = _builder->operand(value);
        return *this;
    }

    offs_builder_t& offs_builder_t::op(op_code_t op_code) {
        _builder->_inst->type = op_code;
        return *this;
    }

    offs_builder_t& offs_builder_t::src(var_version_t** var) {
        _src = var;
        return *this;
    }

    offs_builder_t& offs_builder_t::dst(var_version_t** var) {
        _dst = var;
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

    reg3_builder_t& reg3_builder_t::op(op_code_t op_code) {
        _builder->_inst->type = op_code;
        return *this;
    }

    reg3_builder_t& reg3_builder_t::a(var_version_t** var) {
        _a = var;
        return *this;
    }

    reg3_builder_t& reg3_builder_t::b(var_version_t** var) {
        _b = var;
        return *this;
    }

    reg3_builder_t& reg3_builder_t::c(var_version_t** var) {
        _c = var;
        return *this;
    }
}
