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

#include <basecode/core/scm/bytecode.h>

namespace basecode::scm {
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

            u0 dw(bb_t& bb, imm_t imm) {
                array::append(bb.entries, imm.lu);
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

            u0 none(bb_t& bb, op_code_t opcode) {
                u64 buf{};
                auto inst = (instruction_t*) &buf;
                inst->data     = 0;
                inst->type     = opcode;
                inst->encoding = instruction::encoding::none;
                array::append(bb.entries, buf);
            }

            u0 apply_label(bb_t& bb, label_t label) {
                bb.label = label;
                hashtab::insert(bb.emitter->labels, label, &bb);
            }

            bb_t& make_succ(bb_t& bb, bb_type_t type) {
                auto& new_bb = emitter::make_basic_block(*bb.emitter, type);
                succ(bb, new_bb);
                return new_bb;
            }

            u0 reg1(bb_t& bb, op_code_t opcode, reg_t arg) {
                u64 buf{};
                u64 data{};
                auto inst     = (instruction_t*) &buf;
                auto operands = (operand_encoding_t*) &data;
                inst->type          = opcode;
                inst->encoding      = instruction::encoding::reg1;
                operands->reg1.dest = arg;
                operands->reg1.pad  = 0;
                inst->data          = data;
                array::append(bb.entries, buf);
            }

            bb_t& ubuf(bb_t& bb, reg_t addr_reg, u32 size) {
                for (u32 i = 0; i < size; ++i)
                    array::append(bb.entries, 0);
                auto& load_block = emitter::make_basic_block(*bb.emitter, bb_type_t::code);
                imm2(load_block, instruction::type::lea, emitter::imm(&bb), addr_reg);
                return load_block;
            }

            u0 imm1(bb_t& bb, op_code_t opcode, imm_t imm) {
                u64 buf{};
                u64 data{};
                auto inst = (instruction_t*) &buf;
                auto operands = (operand_encoding_t*) &data;
                inst->type     = opcode;
                inst->encoding = instruction::encoding::imm;
                switch (imm.type) {
                    case imm_type_t::block:
                        operands->imm.src = imm.b->id;
                        break;
                    case imm_type_t::obj:
                    case imm_type_t::trap:
                    case imm_type_t::value:
                    case imm_type_t::label:
                    case imm_type_t::boolean:
                        operands->imm.src = imm.s;
                        break;
                }
                operands->imm.dest = 0;
                operands->imm.type = u8(imm.type);
                operands->imm.size = u8(imm.size);
                operands->imm.aux  = 0;
                inst->data         = data;
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

            bb_t& ibuf(bb_t& bb, reg_t addr_reg, const imm_t* data, u32 size) {
                for (u32 i = 0; i < size; ++i)
                    array::append(bb.entries, data[i].lu);
                auto& load_block = emitter::make_basic_block(*bb.emitter, bb_type_t::code);
                imm2(load_block, instruction::type::lea, emitter::imm(&bb), addr_reg);
                return load_block;
            }

            u0 imm2(bb_t& bb, op_code_t opcode, imm_t imm, reg_t dest, b8 is_signed) {
                u64 buf{};
                u64  data{};
                auto inst = (instruction_t*) &buf;
                auto operands = (operand_encoding_t*) &data;
                inst->type      = opcode;
                inst->encoding  = instruction::encoding::imm;
                inst->is_signed = is_signed;
                switch (imm.type) {
                    case imm_type_t::block:
                        operands->imm.src = imm.b->id;
                        break;
                    case imm_type_t::obj:
                    case imm_type_t::trap:
                    case imm_type_t::value:
                    case imm_type_t::label:
                    case imm_type_t::boolean:
                        operands->imm.src = imm.s;
                        break;
                }
                operands->imm.dest = dest;
                operands->imm.type = u8(imm.type);
                operands->imm.size = u8(imm.size);
                auto area = vm::find_memory_map_entry(*bb.emitter->vm, dest);
                if (area) {
                    operands->imm.aux = area->top ? -1 : 1;
                } else {
                    operands->imm.aux = 0;
                }
                inst->data = data;
                array::append(bb.entries, buf);
            }

            u0 reg3(bb_t& bb, op_code_t opcode, reg_t src, reg_t dest1, reg_t dest2) {
                u64 buf{};
                u64  data{};
                auto inst = (instruction_t*) &buf;
                auto operands = (operand_encoding_t*) &data;
                inst->type         = opcode;
                inst->encoding     = instruction::encoding::reg3;
                operands->reg3.a   = src;
                operands->reg3.b   = dest1;
                operands->reg3.c   = dest2;
                operands->reg3.pad = 0;
                inst->data         = data;
                array::append(bb.entries, buf);
            }

            u0 offs(bb_t& bb, op_code_t opcode, s32 offset, reg_t src, reg_t dest, b8 mode) {
                u64 buf{};
                u64  data{};
                auto inst = (instruction_t*) &buf;
                auto operands = (operand_encoding_t*) &data;
                inst->type            = opcode;
                inst->encoding        = instruction::encoding::offset;
                operands->offset.offs = offset;
                operands->offset.src  = src;
                operands->offset.dest = dest;
                operands->offset.mode = mode;
                operands->offset.pad  = 0;
                inst->data            = data;
                array::append(bb.entries, buf);
            }

            u0 reg2(bb_t& bb, op_code_t opcode, reg_t src, reg_t dest, b8 is_signed, s32 aux) {
                u64 buf{};
                u64  data{};
                auto inst = (instruction_t*) &buf;
                auto operands = (operand_encoding_t*) &data;
                inst->type          = opcode;
                inst->is_signed     = is_signed;
                inst->encoding      = instruction::encoding::reg2;
                operands->reg2.src  = src;
                operands->reg2.dest = dest;
                operands->reg2.pad  = 0;
                auto area = vm::find_memory_map_entry(*bb.emitter->vm, src);
                if (area && !aux) {
                    operands->reg2.aux = area->top ? -1 : 1;
                } else {
                    operands->reg2.aux = aux;
                }
                inst->data = data;
                array::append(bb.entries, buf);
            }

            u0 reg2_imm(bb_t& bb, op_code_t opcode, reg_t a, reg_t b, imm_t imm, b8 is_signed) {
                u64 buf{};
                u64  data{};
                auto inst = (instruction_t*) &buf;
                auto operands = (operand_encoding_t*) &data;
                inst->type           = opcode;
                inst->is_signed      = is_signed;
                inst->encoding       = instruction::encoding::reg2_imm;
                operands->reg2_imm.a = a;
                operands->reg2_imm.b = b;
                switch (imm.type) {
                    case imm_type_t::block:
                        operands->reg2_imm.imm = imm.b->id;
                        break;
                    case imm_type_t::obj:
                    case imm_type_t::trap:
                    case imm_type_t::value:
                    case imm_type_t::label:
                    case imm_type_t::boolean:
                        operands->reg2_imm.imm = imm.s;
                        break;
                }
                operands->reg2_imm.type = u8(imm.type);
                operands->reg2_imm.size = u8(imm.size);
                inst->data = data;
                array::append(bb.entries, buf);
            }

            u0 indx(bb_t& bb, op_code_t opcode, s32 offset, reg_t base, reg_t index, reg_t dest) {
                u64 buf{};
                u64  data{};
                auto inst = (instruction_t*) &buf;
                auto operands = (operand_encoding_t*) &data;
                inst->type              = opcode;
                inst->encoding          = instruction::encoding::indexed;
                operands->indexed.offs  = offset;
                operands->indexed.base  = base;
                operands->indexed.index = index;
                operands->indexed.dest  = dest;
                operands->indexed.pad   = 0;
                inst->data              = data;
                array::append(bb.entries, buf);
            }
        }

        namespace emitter {
            u0 free(emitter_t& e) {
                for (auto bb : e.blocks)
                    basic_block::free(*bb);
                stable_array::free(e.blocks);
                str_array::free(e.strings);
                hashtab::free(e.labels);
            }

            u0 reset(emitter_t& e) {
                for (auto bb : e.blocks)
                    basic_block::free(*bb);
                stable_array::reset(e.blocks);
                str_array::reset(e.strings);
                hashtab::reset(e.labels);
                reg_alloc::reset(e.gp);
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

            static u0 format_imm_oper(str_buf_t& buf, imm_type_t type, s32 value) {
                switch (type) {
                    case imm_type_t::obj:
                        // XXX: format the scheme object
                        format::format_to(buf, "{}", value);
                        break;
                    case imm_type_t::trap:
                        format::format_to(buf, "{}", trap::name(value));
                        break;
                    case imm_type_t::value:
                    case imm_type_t::boolean:
                        format::format_to(buf, "{}", value);
                        break;
                    case imm_type_t::label:
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
                    for (auto str_id : curr->notes)
                        format::format_to(buf, "${:08X}: ; {}\n", addr, e.strings[str_id - 1]);
                    format::format_to(buf, "${:08X}: bb_{}:\n", addr, curr->id);
                    if (curr->label)
                        format::format_to(buf, "${:08X}: {}:\n", addr, e.strings[curr->label - 1]);
                    if (curr->prev)
                        format::format_to(buf, "${:08X}:    .pred bb_{}\n", addr, curr->prev->id);
                    if (curr->next)
                        format::format_to(buf, "${:08X}:    .succ bb_{}\n", addr, curr->next->id);
                    if (curr->type == bb_type_t::data) {
                        auto start_pos = buf.size();
                        for (s32 i = 0; i < curr->entries.size; ++i) {
                            format::format_to(buf, "${:08X}:    .word {}", addr, curr->entries[i]);
                            format_comments(buf,
                                            buf.size() - start_pos,
                                            hashtab::find(curr->comments, i + 1),
                                            e.strings,
                                            addr);
                        }
                    } else {
                        for (s32 i = 0; i < curr->entries.size; ++i) {
                            auto& encoded = curr->entries[i];
                            auto inst     = (instruction_t*) &encoded;
                            u64  data      = inst->data;
                            auto opers     = (operand_encoding_t*) &data;
                            auto start_pos = buf.size();
                            format::format_to(buf, "${:08X}:    {:<12}",
                                          addr,
                                          instruction::type::name(inst->type));
                            switch (inst->encoding) {
                                case instruction::encoding::imm: {
                                    format_imm_oper(buf, imm_type_t(opers->imm.type), opers->imm.src);
                                    if (opers->imm.dest > 0) {
                                        format::format_to(buf,
                                                          ", {}",
                                                          register_file::name(opers->imm.dest));
                                    }
                                    break;
                                }
                                case instruction::encoding::reg1: {
                                    format::format_to(buf,
                                                      "{}",
                                                      register_file::name(opers->reg1.dest));
                                    break;
                                }
                                case instruction::encoding::reg2: {
                                    format::format_to(buf,
                                                      "{}, {}",
                                                      register_file::name(opers->reg2.src),
                                                      register_file::name(opers->reg2.dest));
                                    break;
                                }
                                case instruction::encoding::reg2_imm: {
                                    format::format_to(buf,
                                                      "{}, {}, ",
                                                      register_file::name(opers->reg2_imm.a),
                                                      register_file::name(opers->reg2_imm.b));
                                    format_imm_oper(buf, imm_type_t(opers->reg2_imm.type), opers->reg2_imm.imm);
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
                                                          register_file::name(opers->offset.dest));
                                    } else {
                                        format::format_to(buf,
                                                          "{}({}), {}",
                                                          s32(opers->offset.offs),
                                                          register_file::name(opers->offset.src),
                                                          register_file::name(opers->offset.dest));
                                    }
                                    break;
                                }
                                case instruction::encoding::indexed: {
                                    format::format_to(buf,
                                                      "{}({}, {}), {}",
                                                      s32(opers->indexed.offs),
                                                      register_file::name(opers->indexed.base),
                                                      register_file::name(opers->indexed.index),
                                                      register_file::name(opers->indexed.dest));
                                    break;
                                }
                                default: {
                                    break;
                                }
                            }
                            format_comments(buf,
                                            buf.size() - start_pos,
                                            hashtab::find(curr->comments, i + 1),
                                            e.strings,
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
                                                    auto target_block = hashtab::find(e.labels, operands->imm.src);
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

            u0 init(emitter_t& e, vm_t* vm, u64 address, alloc_t* alloc) {
                e.alloc   = alloc;
                e.vm      = vm;
                e.address = address;
                stable_array::init(e.blocks, e.alloc);
                str_array::init(e.strings, e.alloc);
                hashtab::init(e.labels, e.alloc);
                reg_alloc::init(e.gp, register_file::r0, register_file::r15);
            }
        }

        namespace bytecode {
            namespace rf = register_file;
            namespace op = instruction::type;

            bb_t& leave(bb_t& bb) {
                auto& entry_block = emitter::make_basic_block(*bb.emitter);
                basic_block::succ(bb, entry_block);
                basic_block::reg2(entry_block, op::move, rf::fp, rf::sp);
                basic_block::comment(entry_block, "*** proc epilogue"_ss);
                basic_block::comment(entry_block, "**"_ss);
                basic_block::comment(entry_block, "*"_ss);
                basic_block::reg2(entry_block, op::pop, rf::sp, rf::lr);
                basic_block::reg1(entry_block, op::ret, rf::lr);
                return entry_block;
            }

            u0 save_protected(bb_t& bb) {
                for (reg_t r = rf::r0; r < rf::r15; ++r) {
                    if (vm::reg_alloc::is_protected(bb.emitter->gp, r))
                        vm::basic_block::reg2(bb, op::push, r, rf::sp);
                }
            }

            u0 restore_protected(bb_t& bb) {
                for (reg_t r = rf::r15; r >= rf::r0; --r) {
                    if (vm::reg_alloc::is_protected(bb.emitter->gp, r))
                        vm::basic_block::reg2(bb, op::pop, rf::sp, r);
                }
            }

            bb_t& enter(bb_t& bb, u32 locals) {
                auto& entry_block =  emitter::make_basic_block(*bb.emitter);
                basic_block::succ(bb, entry_block);

                auto& exit_block  = emitter::make_basic_block(*bb.emitter);
                basic_block::succ(entry_block, exit_block);
                basic_block::reg2(exit_block, op::push, rf::lr, rf::sp);
                basic_block::comment(exit_block, "*** proc prologue"_ss);
                basic_block::comment(exit_block, "**"_ss);
                basic_block::comment(exit_block, "*"_ss);
                basic_block::reg2(exit_block, op::move, rf::sp, rf::fp);
                if (locals > 0)
                    basic_block::imm2(exit_block, op::sub, emitter::imm(locals), rf::sp);

                return exit_block;
            }

            u0 free_stack(bb_t& bb, u32 words) {
                basic_block::imm2(bb, op::add, vm::emitter::imm(words * 8), rf::sp);
            }

            u0 lnot(bb_t& bb, reg_t target_reg) {
                basic_block::reg1(bb, op::lnot, target_reg);
            }

            u0 set(bb_t& bb, u32 idx, reg_t reg) {
                basic_block::imm2(bb, op::set, vm::emitter::imm(idx), reg);
            }

            u0 get(bb_t& bb, u32 idx, reg_t reg) {
                basic_block::imm2(bb, op::get, vm::emitter::imm(idx), reg);
            }

            u0 get(bb_t& bb, reg_t sym, reg_t reg) {
                basic_block::reg2(bb, op::get, sym, reg);
            }

            u0 set(bb_t& bb, reg_t sym, reg_t val) {
                basic_block::reg2(bb, op::set, val, sym);
            }

            u0 const_(bb_t& bb, u32 idx, reg_t reg) {
                basic_block::imm2(bb, op::const_, vm::emitter::imm(idx), reg);
            }

            u0 qt(bb_t& bb, u32 idx, reg_t target_reg) {
                auto reg = reg_alloc::reserve(bb.emitter->gp);
                const_(bb, idx, reg[0]);
                basic_block::reg2(bb, op::qt, reg[0], target_reg);
            }

            u0 qq(bb_t& bb, u32 idx, reg_t target_reg) {
                auto reg = reg_alloc::reserve(bb.emitter->gp);
                const_(bb, idx, reg[0]);
                basic_block::reg2(bb, op::qq, reg[0], target_reg);
            }

            u0 error(bb_t& bb, u32 idx, reg_t target_reg) {
                auto reg = reg_alloc::reserve(bb.emitter->gp);
                const_(bb, idx, reg[0]);
                basic_block::reg2(bb, op::error, reg[0], target_reg);
            }

            u0 car(bb_t& bb, reg_t val_reg, reg_t target_reg) {
                basic_block::reg2(bb, op::car, val_reg, target_reg);
            }

            u0 cdr(bb_t& bb, reg_t val_reg, reg_t target_reg) {
                basic_block::reg2(bb, op::cdr, val_reg, target_reg);
            }

            u0 lnot(bb_t& bb, reg_t val_reg, reg_t target_reg) {
                basic_block::reg2(bb, op::lnot, val_reg, target_reg);
            }

            u0 alloc_stack(bb_t& bb, u32 words, reg_t base_reg) {
                vm::basic_block::imm2(bb, op::sub, vm::emitter::imm(words * 8), rf::sp);
                vm::basic_block::offs(bb, op::lea, -(words * 8), rf::sp, base_reg);
            }

            u0 setcar(bb_t& bb, reg_t val_reg, reg_t target_reg) {
                basic_block::reg2(bb, op::setcar, val_reg, target_reg);
            }

            u0 setcdr(bb_t& bb, reg_t val_reg, reg_t target_reg) {
                basic_block::reg2(bb, op::setcdr, val_reg, target_reg);
            }

            u0 gt(bb_t& bb, reg_t lhs, reg_t rhs, reg_t target_reg) {
                vm::basic_block::reg2(bb, op::lcmp, lhs, rhs);
                vm::basic_block::comment(bb, "prim: gt"_ss);
                vm::basic_block::reg1(bb, op::sg, target_reg);
            }

            u0 lt(bb_t& bb, reg_t lhs, reg_t rhs, reg_t target_reg) {
                vm::basic_block::reg2(bb, op::lcmp, lhs, rhs);
                vm::basic_block::comment(bb, "prim: lt"_ss);
                vm::basic_block::reg1(bb, op::sl, target_reg);
            }

            u0 is(bb_t& bb, reg_t lhs, reg_t rhs, reg_t target_reg) {
                vm::basic_block::reg2(bb, op::lcmp, lhs, rhs);
                vm::basic_block::comment(bb, "prim: gte"_ss);
                vm::basic_block::reg1(bb, op::seq, target_reg);
            }

            u0 lte(bb_t& bb, reg_t lhs, reg_t rhs, reg_t target_reg) {
                vm::basic_block::reg2(bb, op::lcmp, lhs, rhs);
                vm::basic_block::comment(bb, "prim: lte"_ss);
                vm::basic_block::reg1(bb, op::sle, target_reg);
            }

            u0 gte(bb_t& bb, reg_t lhs, reg_t rhs, reg_t target_reg) {
                vm::basic_block::reg2(bb, op::lcmp, lhs, rhs);
                vm::basic_block::comment(bb, "prim: gte"_ss);
                vm::basic_block::reg1(bb, op::sge, target_reg);
            }

            u0 cons(bb_t& bb, reg_t car, reg_t cdr, reg_t target_reg) {
                vm::basic_block::reg3(bb, op::cons, car, cdr, target_reg);
            }

            bb_t& if_(bb_t& bb, bb_t& pred_block, bb_t& true_bb, bb_t& false_bb) {
                auto& exit_bb = emitter::make_basic_block(*bb.emitter);
                return exit_bb;
            }

            bb_t& list(bb_t& bb, reg_t lst_reg, reg_t base_reg, reg_t target_reg, u32 size) {
                auto reg = reg_alloc::reserve(bb.emitter->gp);
                auto& list_bb = basic_block::make_succ(bb);
                basic_block::reg2_imm(list_bb, op::list, target_reg, base_reg, emitter::imm(size * 8));
                basic_block::offs(list_bb, op::load, 0, base_reg, lst_reg);
                free_stack(list_bb, size);
                return list_bb;
            }

            bb_t& arith_op(bb_t& bb, op_code_t op_code, reg_t base_reg, reg_t target_reg, u32 size) {
                auto reg = reg_alloc::reserve(bb.emitter->gp);
                auto& arith_bb = basic_block::make_succ(bb);
                basic_block::reg2_imm(arith_bb, op_code, base_reg, target_reg, emitter::imm(size * 8));
                free_stack(arith_bb, size);
                return arith_bb;
            }
        }

        namespace reg_alloc {
            u0 reset(reg_alloc_t& alloc) {
                alloc.slots = {};
                alloc.prots = {};
            }

            reg_result_t reserve(reg_alloc_t& alloc, u32 count) {
                reg_result_t r(alloc);
                for (u32 i = 0; i < count; ++i) {
                    u32 msb_zeros = alloc.slots != 0 ? __builtin_clzll(alloc.slots) : 64;
                    if (!msb_zeros)
                        return r;
                    u32 idx = 64 - msb_zeros;
                    alloc.slots |= (1UL << idx);
                    r[r.count++] = alloc.start + idx;
                }
                return r;
            }

            b8 is_protected(reg_alloc_t& alloc, reg_t reg) {
                const auto mask = 1UL << reg;
                return (alloc.prots & mask) == mask;
            }

            u0 protect(reg_alloc_t& alloc, reg_t reg, b8 flag) {
                if (flag) {
                    alloc.prots |= (1UL << reg);
                } else {
                    alloc.prots &= ~(1UL << reg);
                }
            }

            u0 init(reg_alloc_t& alloc, reg_t start, reg_t end) {
                alloc.end   = end;
                alloc.start = start;
                alloc.slots = alloc.prots = {};
            }

            u0 release(reg_alloc_t& alloc, const reg_result_t& result) {
                for (u32 i = 0; i < result.count; ++i) {
                    const u32 idx = result[i] - alloc.start;
                    if (!(alloc.slots & (1UL << idx)))
                        continue;
                    const auto mask = ~(1UL << idx);
                    alloc.slots &= mask;
                    alloc.prots &= mask;
                }
            }
        }
    }
}
