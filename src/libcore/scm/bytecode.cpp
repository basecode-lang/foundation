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

#include <basecode/core/scm/compiler.h>
#include <basecode/core/scm/bytecode.h>

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
            u0 free(bb_t& bb) {
                for (const auto& pair : bb.comments)
                    array::free(const_cast<note_list_t&>(pair.value));
                array::free(bb.notes);
                array::free(bb.entries);
                hashtab::free(bb.comments);
            }

            u0 offs(bb_t& bb,
                    op_code_t opcode,
                    s32 offset,
                    reg_t src,
                    reg_t dst,
                    b8 mode) {
                u64 buf{};
                u64  data{};
                auto inst          = (instruction_t*) &buf;
                auto opers         = (operand_encoding_t*) &data;
                inst->type         = opcode;
                inst->encoding     = instruction::encoding::offset;
                opers->offset.dst  = dst;
                opers->offset.src  = src;
                opers->offset.pad  = 0;
                opers->offset.offs = offset;
                opers->offset.mode = mode;
                inst->data         = data;
                array::append(bb.entries, buf);
            }

            u0 reg3(bb_t& bb,
                    op_code_t opcode,
                    reg_t src,
                    reg_t dest1,
                    reg_t dest2) {
                u64 buf{};
                u64  data{};
                auto inst  = (instruction_t*) &buf;
                auto opers = (operand_encoding_t*) &data;
                inst->type      = opcode;
                inst->encoding  = instruction::encoding::reg3;
                opers->reg3.a   = src;
                opers->reg3.b   = dest1;
                opers->reg3.c   = dest2;
                opers->reg3.pad = 0;
                inst->data      = data;
                array::append(bb.entries, buf);
            }

            u0 reg2_imm(bb_t& bb,
                        op_code_t opcode,
                        reg_t a,
                        reg_t b,
                        imm_t imm,
                        b8 is_signed) {
                u64 buf{};
                u64  data{};
                auto inst  = (instruction_t*) &buf;
                auto opers = (operand_encoding_t*) &data;
                inst->type        = opcode;
                inst->is_signed   = is_signed;
                inst->encoding    = instruction::encoding::reg2_imm;
                opers->reg2_imm.a = a;
                opers->reg2_imm.b = b;
                switch (imm.type) {
                    case imm_type_t::block:
                        opers->reg2_imm.imm = imm.b->id;
                        break;
                    case imm_type_t::trap:
                    case imm_type_t::label:
                    case imm_type_t::value:
                        opers->reg2_imm.imm = imm.s;
                        break;
                }
                opers->reg2_imm.type = u8(imm.type);
                inst->data           = data;
                array::append(bb.entries, buf);
            }

            u0 dw(bb_t& bb, u64 data) {
                array::append(bb.entries, data);
            }

            u0 pred(bb_t& bb, bb_t& pred) {
                assert((!pred.next && !bb.prev) && (&bb != &pred));
                pred.next = &bb;
                bb.prev   = &pred;
            }

            u0 succ(bb_t& bb, bb_t& succ) {
                assert((!succ.prev && !bb.next) && (&bb != &succ));
                succ.prev = &bb;
                bb.next   = &succ;
            }

            bb_builder_t encode(bb_t* bb) {
                return bb_builder_t(bb);
            }

            bb_builder_t encode(bb_t& bb) {
                return bb_builder_t(bb);
            }

            reg_t* find_bind(bb_t& bb, obj_t* obj) {
                return hashtab::find(bb.emitter->regtab, u64(obj));
            }

            u0 apply_label(bb_t& bb, label_t label) {
                bb.label = label;
                hashtab::insert(bb.emitter->labtab, label, &bb);
            }

            bb_t& make_succ(bb_t& bb, bb_type_t type) {
                auto& new_bb = emitter::make_basic_block(*bb.emitter, type);
                succ(bb, new_bb);
                return new_bb;
            }

            u0 set_bind(bb_t& bb, reg_t reg, obj_t* obj) {
                hashtab::insert(bb.emitter->regtab, u64(obj), reg);
            }

            u0 imm1(bb_t& bb, op_code_t opcode, imm_t imm) {
                u64 buf{};
                u64 data{};
                auto inst      = (instruction_t*) &buf;
                auto opers     = (operand_encoding_t*) &data;
                inst->type     = opcode;
                inst->encoding = instruction::encoding::imm;
                switch (imm.type) {
                    case imm_type_t::block:
                        opers->imm.src = imm.b->id;
                        break;
                    case imm_type_t::trap:
                    case imm_type_t::label:
                    case imm_type_t::value:
                        opers->imm.src = imm.s;
                        break;
                }
                opers->imm.dst  = 0;
                opers->imm.aux  = 0;
                opers->imm.type = u8(imm.type);
                inst->data      = data;
                array::append(bb.entries, buf);
            }

            u0 init(bb_t& bb, emitter_t* e, bb_type_t type) {
                bb.next    = bb.prev = {};
                bb.addr    = 0;
                bb.type    = type;
                bb.label   = 0;
                bb.emitter = e;
                array::init(bb.notes, e->alloc);
                array::init(bb.entries, e->alloc);
                hashtab::init(bb.comments, e->alloc);
            }

            bb_builder_t::bb_builder_t(bb_t* bb) : _bb(bb),
                                                   _str(),
                                                   _imm(),
                                                   _reg(),
                                                   _imm1(this),
                                                   _imm2(this),
                                                   _reg1(this),
                                                   _reg2(this),
                                                   _none(this),
                                                   _offset(),
                                                   _aux(),
                                                   _op_code(),
                                                   _encoding(),
                                                   _mode(),
                                                   _is_signed() {
            }

            bb_builder_t::bb_builder_t(bb_t& bb) : _bb(&bb),
                                                   _str(),
                                                   _imm(),
                                                   _reg(),
                                                   _imm1(this),
                                                   _imm2(this),
                                                   _reg1(this),
                                                   _reg2(this),
                                                   _none(this),
                                                   _offset(),
                                                   _aux(),
                                                   _op_code(),
                                                   _encoding(),
                                                   _mode(),
                                                   _is_signed() {
            }

            status_t bb_builder_t::build() {
                u64 buf     {};
                u64 data    {};
                auto inst  = (instruction_t*) &buf;
                auto opers = (operand_encoding_t*) &data;
                inst->type      = _op_code;
                inst->is_signed = _is_signed;
                switch (_encoding) {
                    case instruction::encoding::none:
                        inst->data     = 0;
                        inst->encoding = instruction::encoding::none;
                        break;
                    case instruction::encoding::imm:
                        inst->encoding  = instruction::encoding::imm;
                        switch (_imm.type) {
                            case imm_type_t::block:
                                opers->imm.src = _imm.b->id;
                                break;
                            case imm_type_t::trap:
                            case imm_type_t::label:
                            case imm_type_t::value:
                                opers->imm.src = _imm.s;
                                break;
                        }
                        if (_reg[1].type == reg_oper_type_t::reg) {
                            opers->imm.dst = _reg[1].kind.r;
                            auto area = find_memory_map_entry(*_bb->emitter->vm, opers->imm.dst);
                            if (area) {
                                opers->imm.aux = area->top ? -1 : 1;
                            } else {
                                opers->imm.aux = 0;
                            }
                        } else {
                        }
                        opers->imm.mode = _mode;
                        opers->imm.type = u8(_imm.type);
                        break;
                    case instruction::encoding::offset:
                        inst->encoding = instruction::encoding::offset;
                        if (_reg[1].type == reg_oper_type_t::reg) {
                            opers->offset.dst = _reg[1].kind.r;
                        } else {
                        }
                        if (_reg[0].type == reg_oper_type_t::reg) {
                            opers->offset.src = _reg[0].kind.r;
                        } else {
                        }
                        opers->offset.pad  = 0;
                        opers->offset.offs = _offset;
                        opers->offset.mode = _mode;
                        break;
                    case instruction::encoding::indexed:
                        inst->encoding      = instruction::encoding::indexed;
                        opers->indexed.pad  = 0;
                        if (_reg[0].type == reg_oper_type_t::reg) {
                            opers->indexed.ndx = _reg[0].kind.r;
                        } else {
                        }
                        if (_reg[1].type == reg_oper_type_t::reg) {
                            opers->indexed.dst = _reg[1].kind.r;
                        } else {
                        }
                        if (_reg[2].type == reg_oper_type_t::reg) {
                            opers->indexed.base = _reg[2].kind.r;
                        } else {
                        }
                        opers->indexed.offs = _offset;
                        break;
                    case instruction::encoding::reg1:
                        inst->encoding  = instruction::encoding::reg1;
                        if (_reg[1].type == reg_oper_type_t::reg) {
                            opers->reg1.dst = _reg[1].kind.r;
                        } else {
                        }
                        opers->reg1.pad = 0;
                        break;
                    case instruction::encoding::reg2:
                        inst->encoding   = instruction::encoding::reg2;
                        if (_reg[1].type == reg_oper_type_t::reg) {
                            opers->reg2.dst = _reg[1].kind.r;
                        } else {
                        }
                        if (_reg[0].type == reg_oper_type_t::reg) {
                            opers->reg2.src = _reg[0].kind.r;
                            auto area = find_memory_map_entry(*_bb->emitter->vm, opers->reg2.src);
                            if (area && !_aux) {
                                opers->reg2.aux = area->top ? -1 : 1;
                            } else {
                                opers->reg2.aux = _aux;
                            }
                        } else {
                        }
                        opers->reg2.pad  = 0;
                        break;
                    case instruction::encoding::reg2_imm:
                        inst->encoding    = instruction::encoding::reg2_imm;
                        if (_reg[0].type == reg_oper_type_t::reg) {
                            opers->reg2_imm.a = _reg[0].kind.r;
                        } else {
                        }
                        if (_reg[1].type == reg_oper_type_t::reg) {
                            opers->reg2_imm.b = _reg[1].kind.r;
                        } else {
                        }
                        switch (_imm.type) {
                            case imm_type_t::block:
                                opers->reg2_imm.imm = _imm.b->id;
                                break;
                            case imm_type_t::trap:
                            case imm_type_t::label:
                            case imm_type_t::value:
                                opers->reg2_imm.imm = _imm.s;
                                break;
                        }
                        opers->reg2_imm.type = u8(_imm.type);
                        break;
                    case instruction::encoding::reg3:
                        inst->encoding  = instruction::encoding::reg3;
                        if (_reg[0].type == reg_oper_type_t::reg) {
                            opers->reg3.a = _reg[1].kind.r;
                        } else {
                        }
                        if (_reg[1].type == reg_oper_type_t::reg) {
                            opers->reg3.b = _reg[1].kind.r;
                        } else {
                        }
                        if (_reg[2].type == reg_oper_type_t::reg) {
                            opers->reg3.c = _reg[1].kind.r;
                        } else {
                        }
                        opers->reg3.pad = 0;
                        break;
                    default:
                        return status_t::fail;
                }
                inst->data = data;
                array::append(_bb->entries, buf);
                return status_t::ok;
            }

            imm1_builder_t& bb_builder_t::imm1() {
                _encoding = instruction::encoding::imm;
                return _imm1;
            }

            imm2_builder_t& bb_builder_t::imm2() {
                _encoding = instruction::encoding::imm;
                return _imm2;
            }

            reg1_builder_t& bb_builder_t::reg1() {
                _encoding = instruction::encoding::reg1;
                return _reg1;
            }

            reg2_builder_t& bb_builder_t::reg2() {
                _encoding = instruction::encoding::reg2;
                return _reg2;
            }

            none_builder_t& bb_builder_t::none() {
                _encoding = instruction::encoding::none;
                return _none;
            }

            bb_builder_t& none_builder_t::next() {
                return *_builder;
            }

            none_builder_t& none_builder_t::op(op_code_t op_code) {
                _builder->_op_code = op_code;
                return *this;
            }

            bb_builder_t& imm1_builder_t::next() {
                return *_builder;
            }

            imm1_builder_t& imm1_builder_t::value(s32 value) {
                _builder->_imm = emitter::imm(value);
                _builder->_is_signed = true;
                return *this;
            }

            imm1_builder_t& imm1_builder_t::value(u32 value) {
                _builder->_imm = emitter::imm(value);
                return *this;
            }

            imm1_builder_t& imm1_builder_t::value(bb_t* value) {
                _builder->_imm = emitter::imm(value);
                return *this;
            }

            bb_builder_t& imm2_builder_t::next() {
                return *_builder;
            }

            imm2_builder_t& imm2_builder_t::mode(b8 flag) {
                _builder->_mode = true;
                return *this;
            }

            imm2_builder_t& imm2_builder_t::src(reg_t reg) {
                _builder->_reg[1] = reg_oper_t{
                    .kind.r = reg,
                    .type = reg_oper_type_t::reg
                };
                _builder->_mode = true;
                return *this;
            }

            imm2_builder_t& imm2_builder_t::dst(reg_t reg) {
                _builder->_reg[1] = reg_oper_t{
                    .kind.r = reg,
                    .type = reg_oper_type_t::reg
                };
                return *this;
            }

            imm2_builder_t& imm2_builder_t::dst(var_t* var) {
                _builder->_reg[1] = reg_oper_t{
                    .kind.v = var,
                    .type = reg_oper_type_t::var
                };
                return *this;
            }

            imm2_builder_t& imm2_builder_t::src(var_t* var) {
                _builder->_reg[1] = reg_oper_t{
                    .kind.v = var,
                    .type = reg_oper_type_t::var
                };
                _builder->_mode = true;
                return *this;
            }

            imm1_builder_t& imm1_builder_t::op(op_code_t op) {
                _builder->_op_code = op;
                return *this;
            }

            imm2_builder_t& imm2_builder_t::value(s32 value) {
                _builder->_imm = emitter::imm(value);
                _builder->_is_signed = true;
                return *this;
            }

            imm2_builder_t& imm2_builder_t::value(u32 value) {
                _builder->_imm = emitter::imm(value);
                return *this;
            }

            imm2_builder_t& imm2_builder_t::op(op_code_t op) {
                _builder->_op_code = op;
                return *this;
            }

            imm1_builder_t& imm1_builder_t::is_signed(b8 flag) {
                _builder->_is_signed = flag;
                return *this;
            }

            imm2_builder_t& imm2_builder_t::value(bb_t* value) {
                _builder->_imm = emitter::imm(value);
                return *this;
            }

            imm2_builder_t& imm2_builder_t::is_signed(b8 flag) {
                _builder->_is_signed = flag;
                return *this;
            }

            bb_builder_t& reg1_builder_t::next() {
                return *_builder;
            }

            reg1_builder_t& reg1_builder_t::dst(reg_t reg) {
                _builder->_reg[1] = reg_oper_t{
                    .kind.r = reg,
                    .type = reg_oper_type_t::reg
                };
                return *this;
            }

            reg1_builder_t& reg1_builder_t::dst(var_t* var) {
                _builder->_reg[1] = reg_oper_t{
                    .kind.v = var,
                    .type = reg_oper_type_t::var
                };
                return *this;
            }

            reg1_builder_t& reg1_builder_t::op(op_code_t op_code) {
                _builder->_op_code = op_code;
                return *this;
            }

            bb_builder_t& reg2_builder_t::next() {
                return *_builder;
            }

            reg2_builder_t& reg2_builder_t::aux(s8 value) {
                _builder->_aux = value;
                return *this;
            }

            reg2_builder_t& reg2_builder_t::dst(reg_t reg) {
                _builder->_reg[1] = reg_oper_t{
                    .kind.r = reg,
                    .type = reg_oper_type_t::reg
                };
                return *this;
            }

            reg2_builder_t& reg2_builder_t::dst(var_t* var) {
                _builder->_reg[1] = reg_oper_t{
                    .kind.v = var,
                    .type = reg_oper_type_t::var
                };
                return *this;
            }

            reg2_builder_t& reg2_builder_t::src(reg_t reg) {
                _builder->_reg[0] = reg_oper_t{
                    .kind.r = reg,
                    .type = reg_oper_type_t::reg
                };
                return *this;
            }

            reg2_builder_t& reg2_builder_t::src(var_t* var) {
                _builder->_reg[0] = reg_oper_t{
                    .kind.v = var,
                    .type = reg_oper_type_t::var
                };
                return *this;
            }

            reg2_builder_t& reg2_builder_t::is_signed(b8 flag) {
                _builder->_is_signed = flag;
                return *this;
            }

            reg2_builder_t& reg2_builder_t::op(op_code_t op_code) {
                _builder->_op_code = op_code;
                return *this;
            }
        }

        namespace emitter {
            u0 free(emitter_t& e) {
                for (auto bb : e.blocks)
                    basic_block::free(*bb);
                symtab::free(e.vartab);
                hashtab::free(e.regtab);
                hashtab::free(e.labtab);
                str_array::free(e.strtab);
                stable_array::free(e.vars);
                stable_array::free(e.blocks);
            }

            u0 reset(emitter_t& e) {
                for (auto bb : e.blocks)
                    basic_block::free(*bb);
                reg_alloc::reset(e.gp);
                symtab::reset(e.vartab);
                hashtab::reset(e.regtab);
                hashtab::reset(e.labtab);
                str_array::reset(e.strtab);
                stable_array::reset(e.vars);
                stable_array::reset(e.blocks);
            }

            static u0 format_comments(str_buf_t& buf,
                                      u32 len,
                                      const note_list_t* notes,
                                      const str_array_t& strings,
                                      u64 addr) {
                if (!notes) {
                    format::format_to(buf, "\n");
                    return;
                }
                for (u32 i = 0; i < notes->size; ++i) {
                    if (i > 0) {
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
                                      strings[(*notes)[i] - 1]);
                }
            }

            static u0 format_imm_oper(str_buf_t& buf,
                                      imm_type_t type,
                                      s32 value) {
                switch (type) {
                    case imm_type_t::trap:
                        format::format_to(buf, "{}", trap::name(value));
                        break;
                    case imm_type_t::label:
                        break;
                    case imm_type_t::value:
                        format::format_to(buf, "{}", value);
                        break;
                    case imm_type_t::block:
                        format::format_to(buf, "bb_{}", value);
                        break;
                }
            }

            u0 disassemble(emitter_t& e, bb_t& start_block, str_buf_t& buf) {
                auto curr = &start_block;
                u64  addr = curr->addr;
                while (curr) {
                    for (auto str_id : curr->notes) {
                        format::format_to(buf,
                                          "${:08X}: ; {}\n",
                                          addr,
                                          e.strtab[str_id - 1]);
                    }
                    format::format_to(buf,
                                      "${:08X}: bb_{}:\n",
                                      addr,
                                      curr->id);
                    if (curr->label) {
                        format::format_to(buf,
                                          "${:08X}: {}:\n",
                                          addr,
                                          e.strtab[curr->label - 1]);
                    }
                    if (curr->prev) {
                        format::format_to(buf,
                                          "${:08X}:    .pred bb_{}\n",
                                          addr,
                                          curr->prev->id);
                    }
                    if (curr->next) {
                        format::format_to(buf,
                                          "${:08X}:    .succ bb_{}\n",
                                          addr,
                                          curr->next->id);
                    }
                    if (curr->type == bb_type_t::data) {
                        auto start_pos = buf.size();
                        for (s32 i = 0; i < curr->entries.size; ++i) {
                            format::format_to(buf,
                                              "${:08X}:    .word {}",
                                              addr,
                                              curr->entries[i]);
                            format_comments(buf,
                                            buf.size() - start_pos,
                                            hashtab::find(curr->comments, i),
                                            e.strtab,
                                            addr);
                        }
                    } else {
                        for (s32 i = 0; i < curr->entries.size; ++i) {
                            auto& encoded = curr->entries[i];
                            auto inst     = (instruction_t*) &encoded;
                            u64  data      = inst->data;
                            auto opers     = (operand_encoding_t*) &data;
                            auto start_pos = buf.size();
                            format::format_to(buf,
                                              "${:08X}:    {:<12}",
                                              addr,
                                              instruction::type::name(inst->type));
                            switch (inst->encoding) {
                                case instruction::encoding::imm: {
                                    if (opers->imm.mode && opers->imm.dst > 0) {
                                        format::format_to(buf,
                                                          "{}, ",
                                                          register_file::name(opers->imm.dst));
                                        format_imm_oper(buf,
                                                        imm_type_t(opers->imm.type),
                                                        opers->imm.src);
                                    } else {
                                        format_imm_oper(buf,
                                                        imm_type_t(opers->imm.type),
                                                        opers->imm.src);
                                        if (opers->imm.dst > 0) {
                                            format::format_to(buf,
                                                              ", {}",
                                                              register_file::name(opers->imm.dst));
                                        }
                                    }
                                    break;
                                }
                                case instruction::encoding::reg1: {
                                    format::format_to(buf,
                                                      "{}",
                                                      register_file::name(opers->reg1.dst));
                                    break;
                                }
                                case instruction::encoding::reg2: {
                                    format::format_to(buf,
                                                      "{}, {}",
                                                      register_file::name(opers->reg2.src),
                                                      register_file::name(opers->reg2.dst));
                                    break;
                                }
                                case instruction::encoding::reg2_imm: {
                                    format::format_to(buf,
                                                      "{}, {}, ",
                                                      register_file::name(opers->reg2_imm.a),
                                                      register_file::name(opers->reg2_imm.b));
                                    format_imm_oper(buf,
                                                    imm_type_t(opers->reg2_imm.type),
                                                    opers->reg2_imm.imm);
                                    break;
                                }
                                case instruction::encoding::reg3: {
                                    format::format_to(buf,
                                                      "{}, {}, {}",
                                                      register_file::name(opers->reg3.a),
                                                      register_file::name(opers->reg3.b),
                                                      register_file::name(opers->reg3.c));
                                    break;
                                }
                                case instruction::encoding::reg4: {
                                    format::format_to(buf,
                                                      "{}, {}, {}",
                                                      register_file::name(opers->reg4.a),
                                                      register_file::name(opers->reg4.b),
                                                      register_file::name(opers->reg4.c),
                                                      register_file::name(opers->reg4.d));
                                    break;
                                }
                                case instruction::encoding::offset: {
                                    if (opers->offset.mode) {
                                        format::format_to(buf,
                                                          "{}, {}({})",
                                                          register_file::name(opers->offset.src),
                                                          s32(opers->offset.offs),
                                                          register_file::name(opers->offset.dst));
                                    } else {
                                        format::format_to(buf,
                                                          "{}({}), {}",
                                                          s32(opers->offset.offs),
                                                          register_file::name(opers->offset.src),
                                                          register_file::name(opers->offset.dst));
                                    }
                                    break;
                                }
                                case instruction::encoding::indexed: {
                                    format::format_to(buf,
                                                      "{}({}, {}), {}",
                                                      s32(opers->indexed.offs),
                                                      register_file::name(opers->indexed.base),
                                                      register_file::name(opers->indexed.ndx),
                                                      register_file::name(opers->indexed.dst));
                                    break;
                                }
                                default: {
                                    break;
                                }
                            }
                            format_comments(buf,
                                            buf.size() - start_pos,
                                            hashtab::find(curr->comments, i),
                                            e.strtab,
                                            addr);
                            addr += sizeof(instruction_t);
                        }
                    }
                    curr = curr->next;
                }
            }

            status_t assemble(emitter_t& e, bb_t& start_block) {
                auto& vm = *e.vm;
                u64  addr = LP;
                auto curr = &start_block;

                // assign block addresses
                while (curr) {
                    curr->addr = addr;
                    addr += curr->entries.size * sizeof(instruction_t);
                    curr = curr->next;
                }

                // emit blocks to vm heap
                curr = &start_block;
                addr = LP;
                while (curr) {
                    if (curr->type == bb_type_t::data) {
                        for (auto encoded : curr->entries)
                            G(LP++) = encoded;
                    } else {
                        for (auto& encoded : curr->entries) {
                            auto inst     = (instruction_t*) &encoded;
                            u64  data     = inst->data;
                            auto operands = (operand_encoding_t*) &data;
                            switch (inst->type) {
                                case instruction::type::br:
                                case instruction::type::blr:
                                case instruction::type::beq:
                                case instruction::type::bne:
                                case instruction::type::bl:
                                case instruction::type::ble:
                                case instruction::type::bg:
                                case instruction::type::bge: {
                                    switch (inst->encoding) {
                                        case instruction::encoding::imm:
                                            switch (imm_type_t(operands->imm.type)) {
                                                case imm_type_t::block: {
                                                    auto target_block = &e.blocks[operands->imm.src - 1];
                                                    operands->imm.src = s32(target_block->addr - addr);
                                                    operands->imm.type = u8(imm_type_t::value);
                                                    break;
                                                }
                                                case imm_type_t::label: {
                                                    auto target_block = hashtab::find(e.labtab, operands->imm.src);
                                                    if (!target_block)
                                                        return status_t::unresolved_label;
                                                    operands->imm.src = s32(target_block->addr - addr);
                                                    operands->imm.type = u8(imm_type_t::value);
                                                    break;
                                                }
                                                default:
                                                    break;
                                            }
                                            break;
                                        default:
                                            break;
                                    }
                                    break;
                                }
                                default: {
                                    break;
                                }
                            }
                            inst->data = data;
                            H(addr)    = encoded;
                            addr += sizeof(instruction_t);
                        }
                    }
                    curr = curr->next;
                }
                LP = addr;
                return status_t::ok;
            }

            bb_t& make_basic_block(emitter_t& e, bb_type_t type) {
                auto& bb = stable_array::append(e.blocks);
                bb.id = e.blocks.size;
                basic_block::init(bb, &e, type);
                return bb;
            }

            u0 init(emitter_t& e, vm_t* vm, u64 addr, alloc_t* alloc) {
                e.vm    = vm;
                e.addr  = addr;
                e.alloc = alloc;
                hashtab::init(e.labtab, e.alloc);
                hashtab::init(e.regtab, e.alloc);
                str_array::init(e.strtab, e.alloc);
                stable_array::init(e.vars, e.alloc);
                stable_array::init(e.blocks, e.alloc);
                reg_alloc::init(e.gp, register_file::r0, register_file::r15);
            }
        }

        namespace bytecode {
            namespace rf = register_file;
            namespace op = instruction::type;

            bb_t& leave(bb_t& bb) {
                auto& entry_block = emitter::make_basic_block(*bb.emitter);
                basic_block::encode(entry_block)
                    .comment("*** proc epilogue"_ss)
                    .comment("**"_ss)
                    .comment("*"_ss)
                    .pred(bb)
                    .reg2()
                    .op(op::move)
                    .src(rf::fp)
                    .dst(rf::sp)
                    .next()
                    .build();
                basic_block::encode(entry_block)
                    .reg2()
                    .op(op::pop)
                    .src(rf::sp)
                    .dst(rf::lr)
                    .next()
                    .build();
                basic_block::encode(entry_block)
                    .reg1()
                    .op(op::ret)
                    .dst(rf::lr)
                    .next()
                    .build();
                return entry_block;
            }

            bb_t& arith_op(bb_t& bb,
                           op_code_t op_code,
                           reg_t base_reg,
                           reg_t target_reg,
                           u32 size) {
                auto reg = reg_alloc::reserve(bb.emitter->gp);
                auto& arith_bb = basic_block::make_succ(bb);
                basic_block::reg2_imm(arith_bb,
                                      op_code,
                                      base_reg,
                                      target_reg,
                                      emitter::imm(size * 8));
                free_stack(arith_bb, size);
                return arith_bb;
            }

            compile_result_t fn(compiler_t& comp,
                                const context_t& c,
                                obj_t* form,
                                obj_t* args) {
                auto& vm = *comp.vm;
                auto ctx        = c.ctx;
                b8   is_mac     = PRIM(form) == prim_type_t::mac;
                auto proc       = make_proc(c.ctx,
                                            CAR(args),
                                            CDR(args),
                                            c.env,
                                            is_mac);
                auto idx        = OBJ_IDX(proc);
                auto target_reg = compiler::reserve_reg(comp, c);
                G(target_reg) = idx;
                if (!c.top_level)
                    const_(*c.bb, idx, target_reg);
                return {c.bb, target_reg, true};
            }

            bb_t& enter(bb_t& bb, u32 locals) {
                auto& entry_block = emitter::make_basic_block(*bb.emitter);
                basic_block::encode(entry_block)
                    .pred(bb)
                    .comment("*** proc prologue"_ss)
                    .comment("**"_ss)
                    .comment("*"_ss)
                    .reg2()
                    .op(op::push)
                    .src(rf::lr)
                    .dst(rf::sp)
                    .next()
                    .build();
                basic_block::encode(entry_block)
                    .reg2()
                    .op(op::move)
                    .src(rf::sp)
                    .dst(rf::fp)
                    .next()
                    .build();
                auto& exit_block = emitter::make_basic_block(*bb.emitter);
                auto exit_encoder = basic_block::encode(exit_block);
                exit_encoder.pred(entry_block);
                if (locals > 0) {
                    exit_encoder
                        .imm2()
                        .value(locals)
                        .dst(rf::sp)
                        .op(op::sub)
                        .next()
                        .build();
                }
                return exit_block;
            }

            u0 free_stack(bb_t& bb, u32 words) {
                basic_block::encode(bb)
                    .imm2()
                    .op(op::add)
                    .value(words * 8)
                    .dst(rf::sp)
                    .next()
                    .build();
            }

            u0 todo(bb_t& bb, str::slice_t msg) {
                basic_block::encode(bb)
                    .comment(msg)
                    .none()
                    .op(op::nop)
                    .next()
                    .build();
            }

            u0 set(bb_t& bb, u32 idx, reg_t reg) {
                basic_block::encode(bb)
                    .imm2()
                    .value(idx)
                    .op(op::set)
                    .dst(reg)
                    .mode(true)
                    .next()
                    .build();
            }

            u0 get(bb_t& bb, u32 idx, reg_t reg) {
                basic_block::encode(bb)
                    .imm2()
                    .value(idx)
                    .op(op::get)
                    .dst(reg)
                    .next()
                    .build();
            }

            compile_result_t inline_(compiler_t& comp,
                                     const context_t& c,
                                     obj_t* sym,
                                     obj_t* form,
                                     obj_t* args) {
                todo(*c.bb, "XXX: macro call"_ss);
                return {c.bb, 0};
            }

            u0 set(bb_t& bb, reg_t sym, reg_t val) {
                basic_block::encode(bb)
                    .reg2()
                    .src(val)
                    .dst(sym)
                    .op(op::set)
                    .next()
                    .build();
            }

            u0 get(bb_t& bb, reg_t sym, reg_t reg) {
                basic_block::encode(bb)
                    .reg2()
                    .src(sym)
                    .dst(reg)
                    .op(op::get)
                    .next()
                    .build();
            }

            u0 pop_env(compiler_t& comp, bb_t& bb) {
                auto reg = reg_alloc::reserve(bb.emitter->gp);
                basic_block::encode(bb)
                    .comment("drop apply env"_ss)
                    .reg2()
                    .op(op::pop)
                    .src(rf::ep)
                    .dst(reg[0])
                    .next()
                    .build();
            }

            u0 push_env(compiler_t& comp, bb_t& bb) {
                auto reg = reg_alloc::reserve(bb.emitter->gp);
                save_protected(bb, comp);
                basic_block::encode(bb)
                    .reg2()
                    .op(op::env)
                    .src(rf::ep)
                    .dst(reg[0])
                    .next()
                    .build();
                basic_block::encode(bb)
                    .reg2()
                    .op(op::push)
                    .src(reg[0])
                    .dst(rf::ep)
                    .next()
                    .build();
            }

            u0 const_(bb_t& bb, u32 idx, reg_t reg) {
                basic_block::encode(bb)
                    .imm2()
                    .op(op::const_)
                    .value(idx)
                    .dst(reg)
                    .next()
                    .build();
            }

            compile_result_t prim(compiler_t& comp,
                                  const context_t& c,
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
                    case prim_type_t::mac:              return fn(comp, c, form, args);
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
                auto rhs_res    = compiler::compile(comp, oc);
                auto target_reg = compiler::reserve_reg(comp, c);

                switch (type) {
                    case prim_type_t::is:
                        basic_block::encode(rhs_res.bb)
                            .comment("prim: is"_ss)
                            .reg2()
                            .op(op::lcmp)
                            .src(lhs_res.reg)
                            .dst(rhs_res.reg)
                            .next()
                            .build();
                        basic_block::encode(rhs_res.bb)
                            .reg1()
                            .op(op::seq)
                            .dst(target_reg)
                            .next()
                            .build();
                        break;

                    case prim_type_t::lt:
                        basic_block::encode(rhs_res.bb)
                            .comment("prim: lt"_ss)
                            .reg2()
                            .op(op::lcmp)
                            .src(lhs_res.reg)
                            .dst(rhs_res.reg)
                            .next()
                            .build();
                        basic_block::encode(rhs_res.bb)
                            .reg1()
                            .op(op::sl)
                            .dst(target_reg)
                            .next()
                            .build();
                        break;

                    case prim_type_t::gt:
                        basic_block::encode(rhs_res.bb)
                            .comment("prim: gt"_ss)
                            .reg2()
                            .op(op::lcmp)
                            .src(lhs_res.reg)
                            .dst(rhs_res.reg)
                            .next()
                            .build();
                        basic_block::encode(rhs_res.bb)
                            .reg1()
                            .op(op::sg)
                            .dst(target_reg)
                            .next()
                            .build();
                        break;

                    case prim_type_t::lte:
                        basic_block::encode(rhs_res.bb)
                            .comment("prim: lte"_ss)
                            .reg2()
                            .op(op::lcmp)
                            .src(lhs_res.reg)
                            .dst(rhs_res.reg)
                            .next()
                            .build();
                        basic_block::encode(rhs_res.bb)
                            .reg1()
                            .op(op::sle)
                            .dst(target_reg)
                            .next()
                            .build();
                        break;

                    case prim_type_t::gte:
                        basic_block::encode(rhs_res.bb)
                            .comment("prim: gte"_ss)
                            .reg2()
                            .op(op::lcmp)
                            .src(lhs_res.reg)
                            .dst(rhs_res.reg)
                            .next()
                            .build();
                        basic_block::encode(rhs_res.bb)
                            .reg1()
                            .op(op::sge)
                            .dst(target_reg)
                            .next()
                            .build();
                        break;

                    default:
                        error(ctx, "unknown compare prim");
                }

                compiler::release_result(comp, lhs_res);
                compiler::release_result(comp, rhs_res);

                return {c.bb, target_reg, true};
            }

            compile_result_t arith_op(compiler_t& comp,
                                      const context_t& c,
                                      op_code_t op_code,
                                      obj_t* args) {
                auto ctx = c.ctx;
                auto reg = reg_alloc::reserve(comp.emitter.gp);
                reg_alloc::protect(comp.emitter.gp, reg[0], true);
                u32 size = length(ctx, args);
                alloc_stack(*c.bb, size, reg[0]);
                s32  offs = 0;
                auto ac   = c;
                while (!IS_NIL(args)) {
                    ac.obj = CAR(args);
                    auto res = compiler::compile(comp, ac);
                    ac.bb = res.bb;
                    basic_block::offs(*res.bb,
                                      op::store,
                                      offs,
                                      res.reg,
                                      reg[0],
                                      true);
                    args = CDR(args);
                    offs += 8;
                    compiler::release_result(comp, res);
                }
                auto target_reg = compiler::reserve_reg(comp, ac);
                auto& arith_bb = basic_block::make_succ(*c.bb);
                basic_block::reg2_imm(arith_bb,
                                      op_code,
                                      reg[0],
                                      target_reg,
                                      emitter::imm(size * 8));
                free_stack(arith_bb, size);
                return {&arith_bb, target_reg, true};
            }

            compile_result_t call_back(compiler_t& comp,
                                       const context_t& c,
                                       obj_t* sym,
                                       obj_t* form,
                                       obj_t* args) {
                todo(*c.bb, "XXX: cfunc call"_ss);
                return {c.bb, 0};
            }

            u0 error(bb_t& bb, u32 idx, reg_t target_reg) {
                auto reg = reg_alloc::reserve(bb.emitter->gp);
                const_(bb, idx, reg[0]);
                basic_block::encode(bb)
                    .reg2()
                    .op(op::error)
                    .src(reg[0])
                    .dst(target_reg)
                    .next()
                    .build();
            }

            u0 save_protected(bb_t& bb, compiler_t& comp) {
                for (reg_t r = rf::r0; r < rf::r15; ++r) {
                    if (reg_alloc::is_protected(comp.emitter.gp, r)) {
                        basic_block::encode(bb)
                            .reg2()
                            .op(op::push)
                            .src(r)
                            .dst(rf::sp)
                            .next()
                            .build();
                        comp.prot[comp.prot_count++] = r;
                        reg_alloc::release_one(comp.emitter.gp, r);
                    }
                }
            }

            u0 restore_protected(bb_t& bb, compiler_t& comp) {
                for (s32 i = comp.prot_count - 1; i >= 0; --i) {
                    reg_t r = comp.prot[i];
                    reg_alloc::reserve_and_protect_reg(comp.emitter.gp, r);
                    basic_block::encode(bb)
                        .reg2()
                        .op(op::pop)
                        .src(rf::sp)
                        .dst(r)
                        .next()
                        .build();
                    comp.prot[i] = 0;
                }
                comp.prot_count = 0;
            }

            u0 alloc_stack(bb_t& bb, u32 words, reg_t base_reg) {
                basic_block::encode(bb)
                    .imm2()
                    .op(op::sub)
                    .value(words * 8)
                    .dst(rf::sp)
                    .next()
                    .build();
                basic_block::offs(bb,
                                  op::lea,
                                  -(words * 8),
                                  rf::sp,
                                  base_reg);
            }

            compile_result_t lookup(compiler_t& comp, const context_t& c) {
                auto ctx = c.ctx;
                auto target_reg = basic_block::find_bind(*c.bb, c.obj);
                if (!target_reg) {
                    auto reg = compiler::reserve_reg(comp, c);
                    get(*c.bb, (u32) OBJ_IDX(c.obj), reg);
                    basic_block::encode(c.bb)
                        .comment(format::format("symbol: {}", scm::to_string(c.ctx, c.obj)));
                    basic_block::set_bind(*c.bb, reg, c.obj);
                    return {c.bb, reg, false};
                }
                return {c.bb, *target_reg, false};
            }

            compile_result_t self_eval(compiler_t& comp, const context_t& c) {
                auto ctx = c.ctx;
                auto target_reg = basic_block::find_bind(*c.bb, c.obj);
                if (!target_reg) {
                    auto reg = compiler::reserve_reg(comp, c);
                    const_(*c.bb, OBJ_IDX(c.obj), reg);
                    basic_block::encode(c.bb)
                        .comment(format::format("literal: {}", scm::to_string(c.ctx, c.obj)));
                    basic_block::set_bind(*c.bb, reg, c.obj);
                    return {c.bb, reg, false};
                }
                return {c.bb, *target_reg, false};
            }

            compile_result_t qt(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto target_reg = compiler::reserve_reg(comp, c);
                auto reg = reg_alloc::reserve(comp.emitter.gp);
                const_(*c.bb, OBJ_IDX(CAR(args)), reg[0]);
                basic_block::encode(c.bb)
                    .reg2()
                    .op(op::qt)
                    .src(reg[0])
                    .dst(target_reg)
                    .next()
                    .build();
                return {c.bb, target_reg, true};
            }

            compile_result_t qq(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto target_reg = compiler::reserve_reg(comp, c);
                auto reg = reg_alloc::reserve(comp.emitter.gp);
                const_(*c.bb, OBJ_IDX(CAR(args)), reg[0]);
                basic_block::encode(c.bb)
                    .reg2()
                    .op(op::qq)
                    .src(reg[0])
                    .dst(target_reg)
                    .next()
                    .build();
                return {c.bb, target_reg, true};
            }

            compile_result_t uq(compiler_t& comp, const context_t& c, obj_t* args) {
                UNUSED(args);
                scm::error(c.ctx, "unquote is not valid in this context.");
                return {c.bb, 0};
            }

            compile_result_t car(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto target_reg = compiler::reserve_reg(comp, c);
                auto cc = c;
                cc.obj = CAR(args);
                auto res = compiler::compile(comp, cc);
                basic_block::encode(res.bb)
                    .reg2()
                    .op(op::car)
                    .src(res.reg)
                    .dst(target_reg)
                    .next()
                    .build();
                compiler::release_result(comp, res);
                return {res.bb, target_reg, true};
            }

            compile_result_t cdr(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto target_reg = compiler::reserve_reg(comp, c);
                auto cc = c;
                cc.obj = CAR(args);
                auto res = compiler::compile(comp, cc);
                basic_block::encode(res.bb)
                    .reg2()
                    .op(op::cdr)
                    .src(res.reg)
                    .dst(target_reg)
                    .next()
                    .build();
                compiler::release_result(comp, res);
                return {res.bb, target_reg, true};
            }

            compile_result_t or_(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto& exit_bb   = emitter::make_basic_block(comp.emitter);
                auto target_reg = compiler::reserve_reg(comp, c);
                auto oc         = c;
                while (!IS_NIL(args)) {
                    oc.obj = CAR(args);
                    if (IS_NIL(CDR(args)))
                        oc.target = &comp.ret_reg;
                    auto res = compiler::compile(comp, oc);
                    oc.bb = res.bb;
                    basic_block::encode(oc.bb)
                        .reg1()
                        .op(op::truep)
                        .dst(res.reg)
                        .next()
                        .build();
                    basic_block::encode(oc.bb)
                        .imm1()
                        .op(op::beq)
                        .value(&exit_bb)
                        .next()
                        .build();
                    compiler::release_result(comp, res);
                    args = CDR(args);
                }
                basic_block::encode(oc.bb)
                    .succ(exit_bb);
                return {&exit_bb, target_reg};
            }

            compile_result_t do_(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto dc = c;
                while (!IS_NIL(args)) {
                    dc.obj   = CAR(args);
                    auto res = compiler::compile(comp, dc);
                    dc.bb = res.bb;
                    args = CDR(args);
                    compiler::release_result(comp, res);
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

                auto& true_bb  = basic_block::make_succ(*c.bb);
                auto& false_bb = emitter::make_basic_block(*c.bb->emitter);
                auto& exit_bb  = emitter::make_basic_block(*c.bb->emitter);

                auto ic  = c;
                auto reg = reg_alloc::reserve(comp.emitter.gp);

                ic.obj = CAR(args);
                auto pred_res = compiler::compile(comp, ic);
                basic_block::encode(c.bb)
                    .reg1()
                    .op(op::truep)
                    .dst(pred_res.reg)
                    .next()
                    .build();
                basic_block::encode(c.bb)
                    .imm1()
                    .value(&false_bb)
                    .op(op::bne)
                    .next()
                    .build();
                compiler::release_result(comp, pred_res);

                ic.bb     = &true_bb;
                ic.obj    = CADR(args);
                ic.target = &comp.ret_reg;
                auto true_res = compiler::compile(comp, ic);
                basic_block::encode(true_res.bb)
                    .succ(false_bb)
                    .imm1()
                    .op(op::br)
                    .value(&exit_bb)
                    .next()
                    .build();
                compiler::release_result(comp, true_res);

                ic.bb     = &false_bb;
                ic.obj    = CADDR(args);
                ic.target = &comp.ret_reg;
                auto false_res = compiler::compile(comp, ic);
                basic_block::encode(false_res.bb)
                    .succ(exit_bb);
                compiler::release_result(comp, false_res);

                return {&exit_bb, 0};
            }

            compile_result_t cons(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto target_reg = compiler::reserve_reg(comp, c);
                auto cc = c;
                cc.obj = CAR(args);
                auto lhs_res = compiler::compile(comp, cc);
                cc.bb  = lhs_res.bb;
                cc.obj = CADR(args);
                auto rhs_res = compiler::compile(comp, cc);
                basic_block::reg3(*rhs_res.bb, op::cons, lhs_res.reg, rhs_res.reg, target_reg);
                compiler::release_result(comp, lhs_res);
                compiler::release_result(comp, rhs_res);
                return {rhs_res.bb, target_reg, true};
            }

            compile_result_t atom(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto target_reg = compiler::reserve_reg(comp, c);
                auto ac = c;
                ac.obj = CAR(args);
                auto res = compiler::compile(comp, ac);
                basic_block::encode(res.bb)
                    .reg2()
                    .op(op::atomp)
                    .src(res.reg)
                    .dst(target_reg)
                    .next()
                    .build();
                compiler::release_result(comp, res);
                return {res.bb, target_reg, true};
            }

            compile_result_t eval(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto target_reg = compiler::reserve_reg(comp, c);
                auto ec = c;
                ec.obj = CAR(args);
                auto res = compiler::compile(comp, ec);
                basic_block::encode(res.bb)
                    .reg2()
                    .op(op::eval)
                    .src(res.reg)
                    .dst(target_reg)
                    .next()
                    .build();
                compiler::release_result(comp, res);
                return {res.bb, target_reg, true};
            }

            compile_result_t list(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx  = c.ctx;
                auto reg  = reg_alloc::reserve(comp.emitter.gp);
                u32  size = length(ctx, args);
                alloc_stack(*c.bb, size, reg[0]);
                s32  offs = 0;
                auto lc   = c;
                while (!IS_NIL(args)) {
                    lc.obj   = CAR(args);
                    auto res = compiler::compile(comp, lc);
                    lc.bb = res.bb;
                    basic_block::offs(*res.bb,
                                      op::store,
                                      offs,
                                      res.reg,
                                      reg[0],
                                      true);
                    args = CDR(args);
                    offs += 8;
                    compiler::release_result(comp, res);
                }
                auto target_reg = compiler::reserve_reg(comp, lc);
                auto& list_bb = basic_block::make_succ(*c.bb);
                basic_block::reg2_imm(list_bb,
                                      op::list,
                                      reg[0],
                                      target_reg,
                                      emitter::imm(size * 8));
                free_stack(list_bb, size);
                return {&list_bb, target_reg, true};
            }

            compile_result_t and_(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto& exit_bb   = emitter::make_basic_block(comp.emitter);
                auto target_reg = compiler::reserve_reg(comp, c);
                auto oc         = c;
                while (!IS_NIL(args)) {
                    oc.obj = CAR(args);
                    if (IS_NIL(CDR(args)))
                        oc.target = &comp.ret_reg;
                    auto res = compiler::compile(comp, oc);
                    oc.bb = res.bb;
                    basic_block::encode(oc.bb)
                        .reg1()
                        .op(op::truep)
                        .dst(res.reg)
                        .next()
                        .build();
                    basic_block::encode(oc.bb)
                        .imm1()
                        .op(op::bne)
                        .value(&exit_bb)
                        .next()
                        .build();
                    compiler::release_result(comp, res);
                    args = CDR(args);
                }
                basic_block::encode(oc.bb)
                    .succ(exit_bb);
                return {&exit_bb, target_reg};
            }

            compile_result_t not_(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto nc = c;
                nc.obj = CAR(args);
                auto res = compiler::compile(comp, nc);
                basic_block::encode(res.bb)
                    .reg1()
                    .op(op::lnot)
                    .dst(res.reg)
                    .next()
                    .build();
                compiler::release_result(comp, res);
                return res;
            }

            compile_result_t error(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto target_reg = compiler::reserve_reg(comp, c);
                error(*c.bb, OBJ_IDX(args), target_reg);
                basic_block::encode(c.bb)
                    .comment(format::format("literal: {}",
                                            to_string(c.ctx, args)),
                             c.bb->entries.size - 1);
                return {c.bb, target_reg, true};
            }

            compile_result_t print(compiler_t& comp, const context_t& c, obj_t* args) {
                UNUSED(args);
                return {c.bb, 0};
            }

            compile_result_t while_(compiler_t& comp, const context_t& c, obj_t* args) {
                UNUSED(args);
                return {c.bb, 0};
            }

            compile_result_t let_set(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto& vm = *comp.vm;
                auto key = CAR(args);
                u32  idx = OBJ_IDX(key);
                auto vc  = c;
                vc.obj    = CADR(args);
                vc.target = &comp.ret_reg;
                vc.label  = emitter::make_label(
                    comp.emitter,
                    *string::interned::get_slice(STRING_ID(OBJ_AT(idx))));
                auto res = compiler::compile(comp, vc);
                if (c.top_level) {
                    auto value = OBJ_AT(G(res.reg));
                    set(ctx, (obj_t*) OBJ_AT(idx), value, c.env);
                    compiler::release_result(comp, res);
                    if (TYPE(value) == obj_type_t::func
                    ||  TYPE(value) == obj_type_t::macro) {
                        auto proc = (proc_t*) NATIVE_PTR(CDR(value));
                        if (!proc->is_compiled) {
                            auto pc = c;
                            pc.label     = vc.label;
                            pc.top_level = false;
                            return comp_proc(comp, pc, proc);
                        }
                    }
                } else {
                    set(*res.bb, idx, res.reg);
                    basic_block::encode(res.bb)
                        .comment(format::format("symbol: {}",
                                                printable_t{c.ctx, key, true}));
                    compiler::release_result(comp, res);
                }
                return res;
            }

            compile_result_t set_cdr(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto sc = c;
                sc.obj = CADR(args);
                auto rhs_res = compiler::compile(comp, sc);
                sc.bb  = rhs_res.bb;
                sc.obj = CAR(args);
                auto lhs_res = compiler::compile(comp, sc);
                basic_block::encode(lhs_res.bb)
                    .reg2()
                    .op(op::setcdr)
                    .src(lhs_res.reg)
                    .dst(rhs_res.reg)
                    .next()
                    .build();
                compiler::release_result(comp, lhs_res);
                compiler::release_result(comp, rhs_res);
                return {lhs_res.bb, 0};
            }

            compile_result_t set_car(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto sc = c;
                sc.obj = CADR(args);
                auto rhs_res = compiler::compile(comp, sc);
                sc.bb  = rhs_res.bb;
                sc.obj = CAR(args);
                auto lhs_res = compiler::compile(comp, sc);
                basic_block::encode(lhs_res.bb)
                    .reg2()
                    .op(op::setcar)
                    .src(lhs_res.reg)
                    .dst(rhs_res.reg)
                    .next()
                    .build();
                compiler::release_result(comp, lhs_res);
                compiler::release_result(comp, rhs_res);
                return {lhs_res.bb, 0};
            }

            compile_result_t ffi(compiler_t& comp, const context_t& c, obj_t* sym, obj_t* form, obj_t* args) {
                todo(*c.bb, "XXX: ffi call"_ss);
                return {c.bb, 0};
            }

            compile_result_t apply(compiler_t& comp, const context_t& c, obj_t* sym, obj_t* form, obj_t* args) {
                auto ctx = c.ctx;
                auto proc = (proc_t*) NATIVE_PTR(CDR(form));
                push_env(comp, *c.bb);
                {
                    auto keys = proc->params;
                    auto vals = args;
                    auto vc   = c;
                    vc.target = {};
                    while (!IS_NIL(keys)) {
                        if (TYPE(keys) != obj_type_t::pair) {
                            vc.obj = vals;
                            auto res = compiler::compile(comp, vc);
                            vc.bb  = res.bb;
                            set(*vc.bb,
                                (u32) OBJ_IDX(keys),
                                res.reg);
                            compiler::release_result(comp, res);
                            break;
                        } else {
                            vc.obj = CAR(vals);
                            auto res = compiler::compile(comp, vc);
                            vc.bb  = res.bb;
                            set(*vc.bb,
                                (u32) OBJ_IDX(CAR(keys)),
                                res.reg);
                            keys = CDR(keys);
                            vals = CDR(vals);
                            compiler::release_result(comp, res);
                        }
                    }
                }
                assert(proc->addr.bb);
                basic_block::imm1(
                    *c.bb,
                    op::blr,
                    proc->is_assembled ? emitter::imm(proc->addr.abs) : emitter::imm(proc->addr.bb));
                basic_block::comment(
                    *c.bb,
                    format::format("call: {}",
                                   printable_t{c.ctx, sym, true}));
                pop_env(comp, *c.bb);
                return {c.bb, 0};
            }

            compile_result_t comp_proc(compiler_t& comp, const context_t& c, proc_t* proc) {
                auto ctx  = c.ctx;
                auto& proc_bb = basic_block::make_succ(*c.bb);
                proc->addr.bb = &proc_bb;
                str_t note_str;
                if (c.label) {
                    basic_block::apply_label(proc_bb, c.label);
                    note_str = format::format(
                        "proc: ({} (fn {} {}))",
                        emitter::get_string(comp.emitter, c.label),
                        printable_t{c.ctx, proc->params, true},
                        printable_t{c.ctx, proc->body, true});
                } else {
                    note_str = format::format(
                        "proc: (_ (fn {} {}))",
                        printable_t{c.ctx, proc->params, true},
                        printable_t{c.ctx, proc->body, true});
                }
                basic_block::note(proc_bb, note_str);
                auto body = proc->body;
                auto bc   = c;
                bc.bb     = &enter(proc_bb, 0);
                while (!IS_NIL(body)) {
                    bc.obj = CAR(body);
                    auto res = compiler::compile(comp, bc);
                    bc.bb = res.bb;
                    body = CDR(body);
                    compiler::release_result(comp, res);
                }
                proc->is_compiled = true;
                return {&leave(*bc.bb), 0};
            }
        }
    }
}
