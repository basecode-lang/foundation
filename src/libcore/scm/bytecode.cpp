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
            u0 imm2(bb_t& bb,
                    op_code_t opcode,
                    imm_t imm,
                    reg_t dst,
                    b8 is_signed,
                    b8 mode) {
                u64 buf{};
                u64  data{};
                auto inst       = (instruction_t*) &buf;
                auto opers      = (operand_encoding_t*) &data;
                inst->type      = opcode;
                inst->encoding  = instruction::encoding::imm;
                inst->is_signed = is_signed;
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
                opers->imm.dst  = dst;
                opers->imm.mode = mode;
                opers->imm.type = u8(imm.type);
                auto area = find_memory_map_entry(*bb.emitter->vm, dst);
                if (area) {
                    opers->imm.aux = area->top ? -1 : 1;
                } else {
                    opers->imm.aux = 0;
                }
                inst->data = data;
                array::append(bb.entries, buf);
            }

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

            u0 reg2(bb_t& bb,
                    op_code_t opcode,
                    reg_t src,
                    reg_t dst,
                    b8 is_signed,
                    s32 aux) {
                u64 buf{};
                u64  data{};
                auto inst        = (instruction_t*) &buf;
                auto opers       = (operand_encoding_t*) &data;
                inst->type       = opcode;
                inst->is_signed  = is_signed;
                inst->encoding   = instruction::encoding::reg2;
                opers->reg2.dst  = dst;
                opers->reg2.src  = src;
                opers->reg2.pad  = 0;
                auto area = find_memory_map_entry(*bb.emitter->vm, src);
                if (area && !aux) {
                    opers->reg2.aux = area->top ? -1 : 1;
                } else {
                    opers->reg2.aux = aux;
                }
                inst->data = data;
                array::append(bb.entries, buf);
            }

            [[maybe_unused]] u0 indx(bb_t& bb,
                    op_code_t opcode,
                    s32 offset,
                    reg_t base,
                    reg_t ndx,
                    reg_t dst) {
                u64 buf{};
                u64  data{};
                auto inst  = (instruction_t*) &buf;
                auto opers = (operand_encoding_t*) &data;
                inst->type          = opcode;
                inst->encoding      = instruction::encoding::indexed;
                opers->indexed.ndx  = ndx;
                opers->indexed.dst  = dst;
                opers->indexed.pad  = 0;
                opers->indexed.offs = offset;
                opers->indexed.base = base;
                inst->data          = data;
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

            u0 none(bb_t& bb, op_code_t opcode) {
                u64 buf{};
                auto inst = (instruction_t*) &buf;
                inst->data     = 0;
                inst->type     = opcode;
                inst->encoding = instruction::encoding::none;
                array::append(bb.entries, buf);
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

            u0 reg1(bb_t& bb, op_code_t opcode, reg_t arg) {
                u64 buf{};
                u64 data{};
                auto inst  = (instruction_t*) &buf;
                auto opers = (operand_encoding_t*) &data;
                inst->type      = opcode;
                inst->encoding  = instruction::encoding::reg1;
                opers->reg1.dst = arg;
                opers->reg1.pad = 0;
                inst->data      = data;
                array::append(bb.entries, buf);
            }

            bb_t& ubuf(bb_t& bb, reg_t addr_reg, u32 size) {
                for (u32 i = 0; i < size; ++i)
                    array::append(bb.entries, 0);
                auto& load_block = emitter::make_basic_block(*bb.emitter);
                imm2(load_block,
                     instruction::type::lea, emitter::imm(&bb),
                     addr_reg);
                return load_block;
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

            bb_t& ibuf(bb_t& bb, reg_t addr_reg, const u64* data, u32 size) {
                for (u32 i = 0; i < size; ++i)
                    array::append(bb.entries, data[i]);
                auto& load_block = emitter::make_basic_block(*bb.emitter);
                imm2(load_block,
                     instruction::type::lea,
                     emitter::imm(&bb),
                     addr_reg);
                return load_block;
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
                                            hashtab::find(curr->comments, i + 1),
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
                                            hashtab::find(curr->comments, i + 1),
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
                basic_block::succ(bb, entry_block);
                basic_block::reg2(entry_block, op::move, rf::fp, rf::sp);
                basic_block::comment(entry_block, "*** proc epilogue"_ss);
                basic_block::comment(entry_block, "**"_ss);
                basic_block::comment(entry_block, "*"_ss);
                basic_block::reg2(entry_block, op::pop, rf::sp, rf::lr);
                basic_block::reg1(entry_block, op::ret, rf::lr);
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
                auto& entry_block =  emitter::make_basic_block(*bb.emitter);
                basic_block::succ(bb, entry_block);

                auto& exit_block  = emitter::make_basic_block(*bb.emitter);
                basic_block::succ(entry_block, exit_block);
                basic_block::reg2(exit_block, op::push, rf::lr, rf::sp);
                basic_block::comment(exit_block, "*** proc prologue"_ss);
                basic_block::comment(exit_block, "**"_ss);
                basic_block::comment(exit_block, "*"_ss);
                basic_block::reg2(exit_block, op::move, rf::sp, rf::fp);
                if (locals > 0) {
                    basic_block::imm2(exit_block,
                                      op::sub,
                                      emitter::imm(locals),
                                      rf::sp);
                }

                return exit_block;
            }

            u0 free_stack(bb_t& bb, u32 words) {
                basic_block::imm2(bb,
                                  op::add,
                                  emitter::imm(words * 8),
                                  rf::sp);
            }

            u0 todo(bb_t& bb, str::slice_t msg) {
                basic_block::none(bb, op::nop);
                basic_block::comment(bb, msg);
            }

            u0 set(bb_t& bb, u32 idx, reg_t reg) {
                basic_block::imm2(bb,
                                  op::set,
                                  emitter::imm(idx),
                                  reg,
                                  false,
                                  true);
            }

            u0 get(bb_t& bb, u32 idx, reg_t reg) {
                basic_block::imm2(bb, op::get, emitter::imm(idx), reg);
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
                basic_block::reg2(bb, op::set, val, sym);
            }

            u0 get(bb_t& bb, reg_t sym, reg_t reg) {
                basic_block::reg2(bb, op::get, sym, reg);
            }

            u0 pop_env(compiler_t& comp, bb_t& bb) {
                auto reg = reg_alloc::reserve(bb.emitter->gp);
                basic_block::reg2(bb, op::pop, rf::ep, reg[0]);
                basic_block::comment(bb, "drop apply env"_ss);
            }

            u0 push_env(compiler_t& comp, bb_t& bb) {
                auto reg = reg_alloc::reserve(bb.emitter->gp);
                save_protected(bb, comp);
                basic_block::reg2(bb, op::env, rf::ep, reg[0]);
                basic_block::reg2(bb, op::push, reg[0], rf::ep);
            }

            u0 const_(bb_t& bb, u32 idx, reg_t reg) {
                basic_block::imm2(bb, op::const_, emitter::imm(idx), reg);
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
                        basic_block::reg2(*rhs_res.bb, op::lcmp, lhs_res.reg, rhs_res.reg);
                        basic_block::comment(*rhs_res.bb, "prim: is"_ss);
                        basic_block::reg1(*rhs_res.bb, op::seq, target_reg);
                        break;

                    case prim_type_t::lt:
                        basic_block::reg2(*rhs_res.bb, op::lcmp, lhs_res.reg, rhs_res.reg);
                        basic_block::comment(*rhs_res.bb, "prim: lt"_ss);
                        basic_block::reg1(*rhs_res.bb, op::sl, target_reg);
                        break;

                    case prim_type_t::gt:
                        basic_block::reg2(*rhs_res.bb, op::lcmp, lhs_res.reg, rhs_res.reg);
                        basic_block::comment(*rhs_res.bb, "prim: gt"_ss);
                        basic_block::reg1(*rhs_res.bb, op::sg, target_reg);
                        break;

                    case prim_type_t::lte:
                        basic_block::reg2(*rhs_res.bb, op::lcmp, lhs_res.reg, rhs_res.reg);
                        basic_block::comment(*rhs_res.bb, "prim: lte"_ss);
                        basic_block::reg1(*rhs_res.bb, op::sle, target_reg);
                        break;

                    case prim_type_t::gte:
                        basic_block::reg2(*rhs_res.bb, op::lcmp, lhs_res.reg, rhs_res.reg);
                        basic_block::comment(*rhs_res.bb, "prim: gte"_ss);
                        basic_block::reg1(*rhs_res.bb, op::sge, target_reg);
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
                basic_block::reg2(bb, op::error, reg[0], target_reg);
            }

            u0 save_protected(bb_t& bb, compiler_t& comp) {
                for (reg_t r = rf::r0; r < rf::r15; ++r) {
                    if (reg_alloc::is_protected(comp.emitter.gp, r)) {
                        basic_block::reg2(bb, op::push, r, rf::sp);
                        comp.prot[comp.prot_count++] = r;
                        reg_alloc::release_one(comp.emitter.gp, r);
                    }
                }
            }

            u0 restore_protected(bb_t& bb, compiler_t& comp) {
                for (s32 i = comp.prot_count - 1; i >= 0; --i) {
                    reg_t r = comp.prot[i];
                    reg_alloc::reserve_and_protect_reg(comp.emitter.gp, r);
                    basic_block::reg2(bb, op::pop, rf::sp, r);
                    comp.prot[i] = 0;
                }
                comp.prot_count = 0;
            }

            u0 alloc_stack(bb_t& bb, u32 words, reg_t base_reg) {
                basic_block::imm2(bb,
                                  op::sub,
                                  emitter::imm(words * 8),
                                  rf::sp);
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
                    basic_block::comment(
                        *c.bb,
                        format::format("symbol: {}", scm::to_string(c.ctx, c.obj)));
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
                    basic_block::comment(
                        *c.bb,
                        format::format("literal: {}", scm::to_string(c.ctx, c.obj)));
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
                basic_block::reg2(*c.bb, op::qt, reg[0], target_reg);
                return {c.bb, target_reg, true};
            }

            compile_result_t qq(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto target_reg = compiler::reserve_reg(comp, c);
                auto reg = reg_alloc::reserve(comp.emitter.gp);
                const_(*c.bb, OBJ_IDX(CAR(args)), reg[0]);
                basic_block::reg2(*c.bb, op::qq, reg[0], target_reg);
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
                basic_block::reg2(*res.bb, op::car, res.reg, target_reg);
                compiler::release_result(comp, res);
                return {res.bb, target_reg, true};
            }

            compile_result_t cdr(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto target_reg = compiler::reserve_reg(comp, c);
                auto cc = c;
                cc.obj = CAR(args);
                auto res = compiler::compile(comp, cc);
                basic_block::reg2(*res.bb, op::cdr, res.reg, target_reg);
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
                    basic_block::reg1(*oc.bb, op::truep, res.reg);
                    basic_block::imm1(*oc.bb, op::beq, emitter::imm(&exit_bb));
                    compiler::release_result(comp, res);
                    args = CDR(args);
                }
                basic_block::succ(*oc.bb, exit_bb);
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
                basic_block::reg1(*c.bb, op::truep, pred_res.reg);
                basic_block::imm1(*c.bb, op::bne, emitter::imm(&false_bb));
                compiler::release_result(comp, pred_res);

                ic.bb     = &true_bb;
                ic.obj    = CADR(args);
                ic.target = &comp.ret_reg;
                auto true_res = compiler::compile(comp, ic);
                basic_block::imm1(*true_res.bb, op::br, emitter::imm(&exit_bb));
                basic_block::succ(*true_res.bb, false_bb);
                compiler::release_result(comp, true_res);

                ic.bb     = &false_bb;
                ic.obj    = CADDR(args);
                ic.target = &comp.ret_reg;
                auto false_res = compiler::compile(comp, ic);
                basic_block::succ(*false_res.bb, exit_bb);
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
                basic_block::reg2(*res.bb, op::atomp, res.reg, target_reg);
                compiler::release_result(comp, res);
                return {res.bb, target_reg, true};
            }

            compile_result_t eval(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto target_reg = compiler::reserve_reg(comp, c);
                auto ec = c;
                ec.obj = CAR(args);
                auto res = compiler::compile(comp, ec);
                basic_block::reg2(*res.bb, op::eval, res.reg, target_reg);
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
                    basic_block::reg1(*oc.bb, op::truep, res.reg);
                    basic_block::imm1(*oc.bb, op::bne, emitter::imm(&exit_bb));
                    compiler::release_result(comp, res);
                    args = CDR(args);
                }
                basic_block::succ(*oc.bb, exit_bb);
                return {&exit_bb, target_reg};
            }

            compile_result_t not_(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto nc = c;
                nc.obj = CAR(args);
                auto res = compiler::compile(comp, nc);
                basic_block::reg1(*res.bb, op::lnot, res.reg);
                compiler::release_result(comp, res);
                return res;
            }

            compile_result_t error(compiler_t& comp, const context_t& c, obj_t* args) {
                auto ctx = c.ctx;
                auto target_reg = compiler::reserve_reg(comp, c);
                error(*c.bb, OBJ_IDX(args), target_reg);
                basic_block::comment(
                    *c.bb,
                    format::format("literal: {}",
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
                    basic_block::comment(
                        *res.bb,
                        format::format("symbol: {}",
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
                basic_block::reg2(*lhs_res.bb, op::setcdr, lhs_res.reg, rhs_res.reg);
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
                basic_block::reg2(*lhs_res.bb, op::setcar, lhs_res.reg, rhs_res.reg);
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
