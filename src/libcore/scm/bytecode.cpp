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
                    array::free(const_cast<note_array_t&>(pair.value));
                array::free(bb.notes);
                array::free(bb.insts);
                hashtab::free(bb.comments);
            }

            bb_builder_t encode(bb_t* bb) {
                return bb_builder_t(bb);
            }

            bb_builder_t encode(bb_t& bb) {
                return bb_builder_t(bb);
            }

            u0 init(bb_t& bb, emitter_t* e, bb_type_t type) {
                bb.next    = bb.prev = {};
                bb.addr    = 0;
                bb.type    = type;
                bb.label   = 0;
                bb.emitter = e;
                array::init(bb.notes, e->alloc);
                array::init(bb.insts, e->alloc);
                hashtab::init(bb.comments, e->alloc);
            }

            bb_builder_t::bb_builder_t(bb_t* bb) : _bb(bb),
                                                   _em(bb->emitter),
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
                                                   _em(bb.emitter),
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
                _inst = &array::append(_bb->insts);
                _inst->encoding = instruction::encoding::imm;
                return _imm1;
            }

            imm2_builder_t& bb_builder_t::imm2() {
                _inst = &array::append(_bb->insts);
                _inst->encoding = instruction::encoding::imm;
                return _imm2;
            }

            reg1_builder_t& bb_builder_t::reg1() {
                _inst = &array::append(_bb->insts);
                _inst->encoding = instruction::encoding::reg1;
                return _reg1;
            }

            reg2_builder_t& bb_builder_t::reg2() {
                _inst = &array::append(_bb->insts);
                _inst->encoding = instruction::encoding::reg2;
                return _reg2;
            }

            none_builder_t& bb_builder_t::none() {
                _inst = &array::append(_bb->insts);
                _inst->encoding = instruction::encoding::none;
                return _none;
            }

            reg3_builder_t& bb_builder_t::reg3() {
                _inst = &array::append(_bb->insts);
                _inst->encoding = instruction::encoding::reg3;
                return _reg3;
            }

            offs_builder_t& bb_builder_t::offs() {
                _inst = &array::append(_bb->insts);
                _inst->encoding = instruction::encoding::offset;
                return _offs;
            }

            reg2_imm_builder_t& bb_builder_t::reg2_imm() {
                _inst = &array::append(_bb->insts);
                _inst->encoding = instruction::encoding::reg2_imm;
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
                _builder->_inst = {};
                return *_builder;
            }

            imm2_builder_t& imm2_builder_t::mode(b8 flag) {
                _builder->_inst->mode = true;
                return *this;
            }

            imm2_builder_t& imm2_builder_t::dst(reg_t reg) {
                _builder->_inst->operands[1] = _builder->operand(reg);
                return *this;
            }

            imm2_builder_t& imm2_builder_t::dst(var_t* var) {
                _builder->_inst->operands[1] = _builder->operand(var);
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

            // --------------------------------------------

            bb_builder_t& reg1_builder_t::build() {
                _builder->_inst = {};
                return *_builder;
            }

            reg1_builder_t& reg1_builder_t::dst(reg_t reg) {
                _builder->_inst->operands[0] = _builder->operand(reg);
                return *this;
            }

            reg1_builder_t& reg1_builder_t::dst(var_t* var) {
                _builder->_inst->operands[0] = _builder->operand(var);
                return *this;
            }

            reg1_builder_t& reg1_builder_t::op(op_code_t op_code) {
                _builder->_inst->type = op_code;
                return *this;
            }

            // --------------------------------------------

            bb_builder_t& reg2_builder_t::build() {
                _builder->_inst = {};
                return *_builder;
            }

            reg2_builder_t& reg2_builder_t::aux(s8 value) {
                _builder->_inst->aux = value;
                return *this;
            }

            reg2_builder_t& reg2_builder_t::dst(reg_t reg) {
                _builder->_inst->operands[1] = _builder->operand(reg);
                return *this;
            }

            reg2_builder_t& reg2_builder_t::dst(var_t* var) {
                _builder->_inst->operands[1] = _builder->operand(var);
                return *this;
            }

            reg2_builder_t& reg2_builder_t::src(reg_t reg) {
                _builder->_inst->operands[0] = _builder->operand(reg);
                return *this;
            }

            reg2_builder_t& reg2_builder_t::src(var_t* var) {
                _builder->_inst->operands[0] = _builder->operand(var);
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
                _builder->_inst = {};
                return *_builder;
            }

            reg2_imm_builder_t& reg2_imm_builder_t::dst(reg_t reg) {
                _builder->_inst->operands[1] = _builder->operand(reg);
                return *this;
            }

            reg2_imm_builder_t& reg2_imm_builder_t::dst(var_t* var) {
                _builder->_inst->operands[1] = _builder->operand(var);
                return *this;
            }

            reg2_imm_builder_t& reg2_imm_builder_t::src(reg_t reg) {
                _builder->_inst->operands[0] = _builder->operand(reg);
                return *this;
            }

            reg2_imm_builder_t& reg2_imm_builder_t::src(var_t* var) {
                _builder->_inst->operands[0] = _builder->operand(var);
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
                _builder->_inst = {};
                return *_builder;
            }

            offs_builder_t& offs_builder_t::mode(b8 flag) {
                _builder->_inst->mode = flag;
                return *this;
            }

            offs_builder_t& offs_builder_t::dst(reg_t reg) {
                _builder->_inst->operands[1] = _builder->operand(reg);
                return *this;
            }

            offs_builder_t& offs_builder_t::src(reg_t reg) {
                _builder->_inst->operands[0] = _builder->operand(reg);
                return *this;
            }

            offs_builder_t& offs_builder_t::dst(var_t* var) {
                _builder->_inst->operands[1] = _builder->operand(var);
                return *this;
            }

            offs_builder_t& offs_builder_t::src(var_t* var) {
                _builder->_inst->operands[0] = _builder->operand(var);
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
                _builder->_inst = {};
                return *_builder;
            }

            reg3_builder_t& reg3_builder_t::a(reg_t reg) {
                _builder->_inst->operands[0] = _builder->operand(reg);
                return *this;
            }

            reg3_builder_t& reg3_builder_t::a(var_t* var) {
                _builder->_inst->operands[0] = _builder->operand(var);
                return *this;
            }

            reg3_builder_t& reg3_builder_t::b(reg_t reg) {
                _builder->_inst->operands[1] = _builder->operand(reg);
                return *this;
            }

            reg3_builder_t& reg3_builder_t::b(var_t* var) {
                _builder->_inst->operands[1] = _builder->operand(var);
                return *this;
            }

            reg3_builder_t& reg3_builder_t::c(reg_t reg) {
                _builder->_inst->operands[2] = _builder->operand(reg);
                return *this;
            }

            reg3_builder_t& reg3_builder_t::c(var_t* var) {
                _builder->_inst->operands[2] = _builder->operand(var);
                return *this;
            }

            reg3_builder_t& reg3_builder_t::op(op_code_t op_code) {
                _builder->_inst->type = op_code;
                return *this;
            }
        }

        namespace emitter {
            u0 free(emitter_t& e) {
                for (auto bb : e.blocks)
                    basic_block::free(*bb);
                symtab::free(e.vartab);
                hashtab::free(e.labtab);
                str_array::free(e.strtab);
                stable_array::free(e.vars);
                stable_array::free(e.blocks);
            }

            u0 reset(emitter_t& e) {
                for (auto bb : e.blocks)
                    basic_block::free(*bb);
                symtab::reset(e.vartab);
                hashtab::reset(e.labtab);
                str_array::reset(e.strtab);
                stable_array::reset(e.vars);
                stable_array::reset(e.blocks);
            }

            static u0 format_comments(str_buf_t& buf,
                                      u32 len,
                                      const note_array_t* notes,
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

            u0 disassemble(emitter_t& e, bb_t& start_block, str_buf_t& buf) {
                auto curr = &start_block;
                u64  addr = curr->addr;
                while (curr) {
                    for (auto str_id : curr->notes) {
                        format::format_to(
                            buf,
                            "${:08X}: ; {}\n",
                            addr,
                            e.strtab[str_id - 1]);
                    }
                    format::format_to(
                        buf,
                        "${:08X}: bb_{}:\n",
                        addr,
                        curr->id);
                    if (curr->label) {
                        format::format_to(
                            buf,
                            "${:08X}: {}:\n",
                            addr,
                            e.strtab[curr->label - 1]);
                    }
                    if (curr->prev) {
                        format::format_to(
                            buf,
                            "${:08X}:    .pred bb_{}\n",
                            addr,
                            curr->prev->id);
                    }
                    if (curr->next) {
                        format::format_to(
                            buf,
                            "${:08X}:    .succ bb_{}\n",
                            addr,
                            curr->next->id);
                    }
                    for (s32 i = 0; i < curr->insts.size; ++i) {
                        const auto& inst = curr->insts[i];
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
                                        hashtab::find(curr->comments, i),
                                        e.strtab,
                                        addr);
                        addr += sizeof(encoded_inst_t);
                    }
                    curr = curr->next;
                }
            }

            template <typename T>
            status_t encode_imm(emitter_t& e,
                                u64 addr,
                                const operand_t& oper,
                                T& field) {
                switch (oper.type) {
                    case operand_type_t::block: {
                        auto target_block = &e.blocks[oper.kind.bb->id - 1];
                        field = s32(target_block->addr - addr);
                        break;
                    }
                    case operand_type_t::label: {
                        auto target_block  = hashtab::find(e.labtab, oper.kind.u);
                        if (!target_block)
                            return status_t::unresolved_label;
                        field = s32(target_block->addr - addr);
                        break;
                    }
                    case operand_type_t::trap:
                    case operand_type_t::value:
                        field = oper.kind.s;
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
                u64 buf     {};
                u64 data    {};
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
                        auto area = find_memory_map_entry(vm, opers->imm.dst);
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
                        auto area = find_memory_map_entry(vm, opers->reg2.src);
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

            status_t assemble(emitter_t& e, bb_t& start_block) {
                auto& vm = *e.vm;
                u64  addr = LP;
                auto curr = &start_block;

                // assign block addresses
                while (curr) {
                    curr->addr = addr;
                    addr += curr->insts.size * sizeof(encoded_inst_t);
                    curr = curr->next;
                }

                // emit blocks to vm heap
                curr = &start_block;
                addr = LP;
                while (curr) {
                    for (const auto& inst : curr->insts) {
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
                symtab::init(e.vartab, e.alloc);
                hashtab::init(e.labtab, e.alloc);
                str_array::init(e.strtab, e.alloc);
                stable_array::init(e.vars, e.alloc);
                stable_array::init(e.blocks, e.alloc);
            }
        }

        namespace bytecode {
            namespace rf = register_file;
            namespace op = instruction::type;

            bb_t& leave(bb_t& bb) {
                auto& entry_block = emitter::make_basic_block(*bb.emitter);
                basic_block::encode(entry_block)
                    .comment("*"_ss)
                    .comment("**"_ss)
                    .comment("*** proc epilogue"_ss)
                    .pred(bb)
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
                return entry_block;
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
                        .build()
                    .reg2()
                        .op(op::move)
                        .src(rf::sp)
                        .dst(rf::fp)
                        .build();
                auto& exit_block = emitter::make_basic_block(*bb.emitter);
                auto exit_encoder = basic_block::encode(exit_block);
                exit_encoder.pred(entry_block);
                if (locals > 0) {
                    exit_encoder
                        .imm2()
                            .src(locals)
                            .dst(rf::sp)
                            .op(op::sub)
                            .build();
                }
                return exit_block;
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

            u0 pop_env(compiler_t& comp, bb_t& bb) {
                auto var = emitter::fill_var(comp.emitter, "dummy"_ss);
                basic_block::encode(bb)
                    .comment("drop apply env"_ss)
                    .reg2()
                        .op(op::pop)
                        .src(rf::ep)
                        .dst(var)
                        .build();
            }

            u0 push_env(compiler_t& comp, bb_t& bb) {
                auto var = emitter::fill_var(comp.emitter, "env"_ss);
                basic_block::encode(bb)
                    .reg2()
                        .op(op::env)
                        .src(rf::ep)
                        .dst(var)
                        .build()
                    .reg2()
                        .op(op::push)
                        .src(var)
                        .dst(rf::ep)
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
                auto rhs_res = compiler::compile(comp, oc);
                auto var = emitter::fill_var(comp.emitter, "cmp"_ss);

                switch (type) {
                    case prim_type_t::is:
                        basic_block::encode(rhs_res.bb)
                            .comment("prim: is"_ss)
                            .reg2()
                                .op(op::lcmp)
                                .src(lhs_res.var)
                                .dst(rhs_res.var)
                                .build()
                            .reg1()
                                .op(op::seq)
                                .dst(var)
                                .build();
                        break;

                    case prim_type_t::lt:
                        basic_block::encode(rhs_res.bb)
                            .comment("prim: lt"_ss)
                            .reg2()
                                .op(op::lcmp)
                                .src(lhs_res.var)
                                .dst(rhs_res.var)
                                .build()
                            .reg1()
                                .op(op::sl)
                                .dst(var)
                                .build();
                        break;

                    case prim_type_t::gt:
                        basic_block::encode(rhs_res.bb)
                            .comment("prim: gt"_ss)
                            .reg2()
                                .op(op::lcmp)
                                .src(lhs_res.var)
                                .dst(rhs_res.var)
                                .build()
                            .reg1()
                                .op(op::sg)
                                .dst(var)
                                .build();
                        break;

                    case prim_type_t::lte:
                        basic_block::encode(rhs_res.bb)
                            .comment("prim: lte"_ss)
                            .reg2()
                                .op(op::lcmp)
                                .src(lhs_res.var)
                                .dst(rhs_res.var)
                                .build()
                            .reg1()
                                .op(op::sle)
                                .dst(var)
                                .build();
                        break;

                    case prim_type_t::gte:
                        basic_block::encode(rhs_res.bb)
                            .comment("prim: gte"_ss)
                            .reg2()
                                .op(op::lcmp)
                                .src(lhs_res.var)
                                .dst(rhs_res.var)
                                .build()
                            .reg1()
                                .op(op::sge)
                                .dst(var)
                                .build();
                        break;

                    default:
                        error(ctx, "unknown compare prim");
                }

                return {c.bb, var};
            }

            compile_result_t arith_op(compiler_t& comp,
                                      const context_t& c,
                                      op_code_t op_code,
                                      obj_t* args) {
                auto ctx = c.ctx;
                u32 size = length(ctx, args);
                auto base_addr = emitter::fill_var(comp.emitter, "base_addr"_ss);
                alloc_stack(*c.bb, size, base_addr);
                s32  offs = 0;
                auto ac   = c;
                while (!IS_NIL(args)) {
                    ac.obj = CAR(args);
                    auto res = compiler::compile(comp, ac);
                    ac.bb = res.bb;
                    basic_block::encode(res.bb)
                        .offs()
                            .op(op::store)
                            .offset(offs)
                            .src(res.var)
                            .dst(base_addr)
                            .mode(true)
                            .build();
                    args = CDR(args);
                    offs += 8;
                }
                auto arith_res = emitter::fill_var(comp.emitter, "arith_res"_ss);
                auto& arith_bb = emitter::make_basic_block(*c.bb->emitter);
                basic_block::encode(arith_bb)
                    .reg2_imm()
                        .op(op_code)
                        .src(base_addr)
                        .dst(arith_res)
                        .value(size * 8)
                        .build();
                free_stack(arith_bb, size);
                emitter::spill_var(comp.emitter, "base_addr"_ss);
                return {&arith_bb, arith_res};
            }

            compile_result_t call_back(compiler_t& comp,
                                       const context_t& c,
                                       obj_t* sym,
                                       obj_t* form,
                                       obj_t* args) {
                todo(*c.bb, "XXX: cfunc call"_ss);
                return {c.bb, 0};
            }

            u0 alloc_stack(bb_t& bb, u32 words, var_t* base_addr) {
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
                auto var = emitter::fill_var(*c.bb->emitter, sym);
                basic_block::encode(c.bb)
                    .comment(format::format("symbol: {}", sym))
                    .imm2()
                        .src((u32) OBJ_IDX(c.obj))
                        .op(op::get)
                        .dst(var)
                        .build();
                return {c.bb, var};
            }

            compile_result_t self_eval(compiler_t& comp, const context_t& c) {
                auto ctx = c.ctx;
                auto var = emitter::fill_var(comp.emitter, "lit"_ss);
                basic_block::encode(c.bb)
                    .comment(format::format("literal: {}", scm::to_string(c.ctx, c.obj)))
                    .imm2()
                        .op(op::const_)
                        .src(u32(OBJ_IDX(c.obj)))
                        .dst(var)
                        .build();
                return {c.bb, var};
            }

            compile_result_t qt(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto qt_res = emitter::fill_var(comp.emitter, "qt_res"_ss);
                auto tmp = emitter::fill_var(comp.emitter, "tmp"_ss);
                basic_block::encode(c.bb)
                    .comment(format::format("literal: {}", scm::to_string(c.ctx, c.obj)))
                    .imm2()
                        .op(op::const_)
                        .src(u32(OBJ_IDX(CAR(args))))
                        .dst(tmp)
                        .build()
                    .reg2()
                        .op(op::qt)
                        .src(tmp)
                        .dst(qt_res)
                        .build();
                emitter::spill_var(comp.emitter, "tmp"_ss);
                return {c.bb, qt_res};
            }

            compile_result_t qq(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto qq_res = emitter::fill_var(comp.emitter, "qq_res"_ss);
                auto tmp = emitter::fill_var(comp.emitter, "tmp"_ss);
                basic_block::encode(c.bb)
                    .comment(format::format("literal: {}", scm::to_string(c.ctx, c.obj)))
                    .imm2()
                        .op(op::const_)
                        .src(u32(OBJ_IDX(CAR(args))))
                        .dst(tmp)
                        .build()
                    .reg2()
                        .op(op::qq)
                        .src(tmp)
                        .dst(qq_res)
                        .build();
                emitter::spill_var(comp.emitter, "tmp"_ss);
                return {c.bb, qq_res};
            }

            compile_result_t uq(compiler_t& comp, const context_t& c, obj_t* args) {
                UNUSED(args);
                scm::error(c.ctx, "unquote is not valid in this context.");
                return {c.bb, 0};
            }

            compile_result_t car(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto car_res = emitter::fill_var(comp.emitter, "car_res"_ss);
                auto cc = c;
                cc.obj = CAR(args);
                auto res = compiler::compile(comp, cc);
                basic_block::encode(res.bb)
                    .reg2()
                        .op(op::car)
                        .src(res.var)
                        .dst(car_res)
                        .build();
                return {res.bb, car_res};
            }

            compile_result_t cdr(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto cdr_res = emitter::fill_var(comp.emitter, "cdr_res"_ss);
                auto cc = c;
                cc.obj = CAR(args);
                auto res = compiler::compile(comp, cc);
                basic_block::encode(res.bb)
                    .reg2()
                        .op(op::cdr)
                        .src(res.var)
                        .dst(cdr_res)
                        .build();
                return {res.bb, cdr_res};
            }

            compile_result_t or_(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto& exit_bb = emitter::make_basic_block(comp.emitter);
                auto oc = c;
                while (!IS_NIL(args)) {
                    oc.obj = CAR(args);
                    if (IS_NIL(CDR(args)))
                        oc.target = &comp.ret_reg;
                    auto res = compiler::compile(comp, oc);
                    auto res_var = emitter::fill_var(comp.emitter, res.var);
                    oc.bb = res.bb;
                    basic_block::encode(oc.bb)
                        .reg1()
                            .op(op::truep)
                            .dst(res_var)
                            .build()
                        .imm1()
                            .op(op::beq)
                            .value(&exit_bb)
                            .build();
                    args = CDR(args);
                }
                basic_block::encode(oc.bb)
                    .succ(exit_bb);
                return {&exit_bb, 0};
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

                auto& true_bb  = emitter::make_basic_block(*c.bb->emitter);
                auto& false_bb = emitter::make_basic_block(*c.bb->emitter);
                auto& exit_bb  = emitter::make_basic_block(*c.bb->emitter);

                auto ic  = c;
                ic.obj = CAR(args);

                auto pred_res = compiler::compile(comp, ic);
                auto pred_var = emitter::fill_var(comp.emitter, pred_res.var);
                basic_block::encode(c.bb)
                    .succ(true_bb)
                    .reg1()
                        .op(op::truep)
                        .dst(pred_var)
                        .build()
                    .imm1()
                        .value(&false_bb)
                        .op(op::bne)
                        .build();

                basic_block::encode(true_bb).succ(false_bb);

                ic.bb     = &true_bb;
                ic.obj    = CADR(args);
                ic.target = &comp.ret_reg;
                auto true_res = compiler::compile(comp, ic);
                basic_block::encode(true_res.bb)
                    .imm1()
                        .op(op::br)
                        .value(&exit_bb)
                        .build();

                ic.bb     = &false_bb;
                ic.obj    = CADDR(args);
                ic.target = &comp.ret_reg;
                auto false_res = compiler::compile(comp, ic);
                basic_block::encode(false_res.bb)
                    .succ(exit_bb);

                return {&exit_bb, 0};
            }

            compile_result_t cons(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto cons_res = emitter::fill_var(comp.emitter, "cons_res"_ss);
                auto cc = c;
                cc.obj = CAR(args);
                auto lhs_res = compiler::compile(comp, cc);
                cc.bb  = lhs_res.bb;
                cc.obj = CADR(args);
                auto rhs_res = compiler::compile(comp, cc);
                basic_block::encode(rhs_res.bb)
                    .reg3()
                    .op(op::cons)
                        .a(lhs_res.var)
                        .b(rhs_res.var)
                        .c(cons_res)
                        .build();
                return {rhs_res.bb, cons_res};
            }

            compile_result_t atom(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto atom_res = emitter::fill_var(comp.emitter, "atom_res"_ss);
                auto ac = c;
                ac.obj = CAR(args);
                auto res = compiler::compile(comp, ac);
                basic_block::encode(res.bb)
                    .reg2()
                        .op(op::atomp)
                        .src(res.var)
                        .dst(atom_res)
                        .build();
                return {res.bb, atom_res};
            }

            compile_result_t eval(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto eval_res = emitter::fill_var(comp.emitter, "eval_res"_ss);
                auto ec = c;
                ec.obj = CAR(args);
                auto res = compiler::compile(comp, ec);
                basic_block::encode(res.bb)
                    .reg2()
                        .op(op::eval)
                        .src(res.var)
                        .dst(eval_res)
                        .build();
                return {res.bb, eval_res};
            }

            compile_result_t list(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx  = c.ctx;
                u32  size = length(ctx, args);
                auto base_addr = emitter::fill_var(comp.emitter, "base_addr"_ss);
                alloc_stack(*c.bb, size, base_addr);
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
                            .src(res.var)
                            .dst(base_addr)
                            .mode(true)
                            .build();
                    args = CDR(args);
                    offs += 8;
                }
                auto list_res = emitter::fill_var(comp.emitter, "list_res"_ss);
                auto& list_bb = emitter::make_basic_block(*c.bb->emitter);
                basic_block::encode(list_bb)
                    .pred(*c.bb)
                    .reg2_imm()
                        .op(op::list)
                        .src(base_addr)
                        .dst(list_res)
                        .value(size * 8)
                        .build();
                free_stack(list_bb, size);
                emitter::spill_var(comp.emitter, "base_addr"_ss);
                return {&list_bb, list_res};
            }

            compile_result_t and_(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto& exit_bb   = emitter::make_basic_block(comp.emitter);
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
                            .dst(res.var)
                            .build()
                        .imm1()
                            .op(op::bne)
                            .value(&exit_bb)
                            .build();
                    args = CDR(args);
                }
                basic_block::encode(oc.bb)
                    .succ(exit_bb);
                return {&exit_bb, 0};
            }

            compile_result_t not_(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto nc = c;
                nc.obj = CAR(args);
                auto res = compiler::compile(comp, nc);
                auto not_var = emitter::fill_var(comp.emitter, res.var);
                basic_block::encode(res.bb)
                    .reg1()
                        .op(op::lnot)
                        .dst(not_var)
                        .build();
                return {res.bb, not_var};
            }

            compile_result_t error(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto error_res = emitter::fill_var(comp.emitter, "error_res"_ss);
                auto tmp = emitter::fill_var(comp.emitter, "tmp"_ss);
                basic_block::encode(c.bb)
                    .comment(format::format("literal: {}", scm::to_string(c.ctx, c.obj)))
                    .imm2()
                        .op(op::const_)
                        .src(u32(OBJ_IDX(args)))
                        .dst(tmp)
                        .build()
                    .reg2()
                        .op(op::error)
                        .src(tmp)
                        .dst(error_res)
                        .build();
                basic_block::encode(c.bb)
                    .comment(format::format("literal: {}",
                                            to_string(c.ctx, args)),
                             c.bb->insts.size - 1);
                return {c.bb, error_res};
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
                    auto value = OBJ_AT(G(rf::m));  // FIXME: this is a temporary hack
                    set(ctx, (obj_t*) OBJ_AT(idx), value, c.env);
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
                    basic_block::encode(res.bb)
                        .imm2()
                            .src(u32(idx))
                            .op(op::set)
                            .dst(res.var)
                            .mode(true)
                            .build();
                    basic_block::encode(res.bb)
                        .comment(format::format("symbol: {}",
                                                printable_t{c.ctx, key, true}));
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
                        .src(lhs_res.var)
                        .dst(rhs_res.var)
                        .build();
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
                        .src(lhs_res.var)
                        .dst(rhs_res.var)
                        .build();
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
                            basic_block::encode(vc.bb)
                                .imm2()
                                    .src(u32(OBJ_IDX(keys)))
                                    .op(op::set)
                                    .dst(res.var)
                                    .mode(true)
                                    .build();
                            break;
                        } else {
                            vc.obj = CAR(vals);
                            auto res = compiler::compile(comp, vc);
                            vc.bb  = res.bb;
                            basic_block::encode(vc.bb)
                                .imm2()
                                    .src(u32(OBJ_IDX(keys)))
                                    .op(op::set)
                                    .dst(res.var)
                                    .mode(true)
                                    .build();
                            keys = CDR(keys);
                            vals = CDR(vals);
                        }
                    }
                }
                assert(proc->addr.bb);
                if (proc->is_assembled) {
                    basic_block::encode(c.bb)
                        .comment(format::format(
                            "call: {}",
                            printable_t{c.ctx, sym, true}))
                        .imm1()
                            .op(op::blr)
                            .value(proc->addr.abs)
                            .build();
                } else {
                    basic_block::encode(c.bb)
                        .comment(format::format(
                            "call: {}",
                            printable_t{c.ctx, sym, true}))
                        .imm1()
                            .op(op::blr)
                            .value(proc->addr.bb)
                            .build();
                }
                pop_env(comp, *c.bb);
                return {c.bb, emitter::fill_var(comp.emitter, "__ret__"_ss)};
            }

            compile_result_t comp_proc(compiler_t& comp, const context_t& c, proc_t* proc) {
                auto ctx  = c.ctx;
                auto& e   = *c.bb->emitter;

                auto ret_var = emitter::make_var(e, "__ret__"_ss);
                UNUSED(ret_var);
                auto params = proc->params;
                while (!IS_NIL(params)) {
                    emitter::make_var(e, *string::interned::get_slice(STRING_ID(CAR(params))));
                    params = CDR(params);
                }

                auto& proc_bb = emitter::make_basic_block(*c.bb->emitter);
                proc->addr.bb = &proc_bb;

                auto proc_encoder = basic_block::encode(proc_bb);
                proc_encoder.pred(*c.bb);

                str_t note_str;
                if (c.label) {
                    proc_encoder.label(c.label);
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
                proc_encoder.note(note_str);

                auto body = proc->body;
                auto bc   = c;
                bc.bb     = &enter(proc_bb, 0);
                while (!IS_NIL(body)) {
                    bc.obj = CAR(body);
                    auto res = compiler::compile(comp, bc);
                    bc.bb = res.bb;
                    body = CDR(body);
                }
                proc->is_compiled = true;
                return {&leave(*bc.bb), 0};
            }
        }
    }
}
