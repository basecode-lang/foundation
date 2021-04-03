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
                [truthy]    = "TRUTHY"_ss,
            };

            str::slice_t name(u8 op) {
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

        str::slice_t name(u8 reg) {
            return s_names[u32(reg)];
        }
    }

    namespace vm {
        namespace basic_block {
            u0 free(bb_t& bb) {
                array::free(bb.entries);
                array::free(bb.comments);
            }

            u0 dw(bb_t& bb, imm_t imm) {
                array::append(bb.entries, imm.lu);
            }

            u0 none(bb_t& bb, u8 opcode) {
                u64 buf{};
                auto inst = (instruction_t*) &buf;
                inst->data     = 0;
                inst->type     = opcode;
                inst->encoding = instruction::encoding::none;
                array::append(bb.entries, buf);
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

            u0 reg1(bb_t& bb, u8 opcode, u8 arg) {
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

            u0 apply_label(bb_t& bb, label_t label) {
                bb.label = label;
                hashtab::insert(bb.emitter->labels, label, &bb);
            }

            u0 imm1(bb_t& bb, u8 opcode, imm_t imm) {
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

            bb_t& ubuf(bb_t& bb, u8 addr_reg, u32 size) {
                for (u32 i = 0; i < size; ++i)
                    array::append(bb.entries, 0);
                auto& load_block = emitter::make_basic_block(*bb.emitter, bb_type_t::code);
                imm2(load_block, instruction::type::lea, emitter::imm(&bb), addr_reg);
                return load_block;
            }

            u0 init(bb_t& bb, emitter_t* e, bb_type_t type) {
                bb.next    = bb.prev = {};
                bb.addr    = 0;
                bb.type    = type;
                bb.label   = 0;
                bb.emitter = e;
                array::init(bb.entries, e->alloc);
                array::init(bb.comments, e->alloc);
            }

            u0 reg3(bb_t& bb, u8 opcode, u8 src, u8 dest1, u8 dest2) {
                u64 buf{};
                u64  data{};
                auto inst = (instruction_t*) &buf;
                auto operands = (operand_encoding_t*) &data;
                inst->type           = opcode;
                inst->encoding       = instruction::encoding::reg3;
                operands->reg3.src   = src;
                operands->reg3.dest1 = dest1;
                operands->reg3.dest2 = dest2;
                operands->reg3.pad   = 0;
                inst->data           = data;
                array::append(bb.entries, buf);
            }

            u0 offs(bb_t& bb, u8 opcode, s32 offset, u8 src, u8 dest) {
                u64 buf{};
                u64  data{};
                auto inst = (instruction_t*) &buf;
                auto operands = (operand_encoding_t*) &data;
                inst->type            = opcode;
                inst->encoding        = instruction::encoding::offset;
                operands->offset.offs = offset;
                operands->offset.src  = src;
                operands->offset.dest = dest;
                operands->offset.pad  = 0;
                inst->data            = data;
                array::append(bb.entries, buf);
            }

            bb_t& ibuf(bb_t& bb, u8 addr_reg, const imm_t* data, u32 size) {
                for (u32 i = 0; i < size; ++i)
                    array::append(bb.entries, data[i].lu);
                auto& load_block = emitter::make_basic_block(*bb.emitter, bb_type_t::code);
                imm2(load_block, instruction::type::lea, emitter::imm(&bb), addr_reg);
                return load_block;
            }

            u0 imm2(bb_t& bb, u8 opcode, imm_t imm, u8 dest, b8 is_signed) {
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

            u0 indx(bb_t& bb, u8 opcode, s32 offset, u8 base, u8 index, u8 dest) {
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

            u0 reg2(bb_t& bb, u8 opcode, u8 src, u8 dest, b8 is_signed, s32 aux) {
                u64 buf{};
                u64  data{};
                auto inst = (instruction_t*) &buf;
                auto operands = (operand_encoding_t*) &data;
                inst->type               = opcode;
                inst->is_signed          = is_signed;
                inst->encoding           = instruction::encoding::reg2;
                operands->reg2.src       = src;
                operands->reg2.dest      = dest;
                operands->reg2.pad       = 0;
                auto area = vm::find_memory_map_entry(*bb.emitter->vm, src);
                if (area && !aux) {
                    operands->reg2.aux = area->top ? -1 : 1;
                } else {
                    operands->reg2.aux = aux;
                }
                inst->data = data;
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
                register_alloc::free(e.gp);
            }

            u0 reset(emitter_t& e) {
                for (auto bb : e.blocks)
                    basic_block::free(*bb);
                stable_array::reset(e.blocks);
                str_array::reset(e.strings);
                hashtab::reset(e.labels);
                register_alloc::reset(e.gp);
            }

            u0 disassemble(emitter_t& e, bb_t& start_block) {
                for (auto bb : e.blocks) {
                    format::print("id: {}, prev: {}, next: {}\n",
                                  bb->id,
                                  bb->prev ? bb->prev->id : 0,
                                  bb->next ? bb->next->id : 0);
                }
                auto curr = &start_block;
                u64  addr = curr->addr;
                while (curr) {
                    for (auto str_id : curr->comments)
                        format::print("${:08X}: ; {}\n", addr, e.strings[str_id - 1]);
                    format::print("${:08X}: bb_{}:\n", addr, curr->id);
                    if (curr->label) {
                        format::print("${:08X}: {}:\n", addr, e.strings[curr->label - 1]);
                    }
                    if (curr->type == bb_type_t::data) {
                        for (auto encoded : curr->entries)
                            format::print("${:08X}:    .word {}\n", addr, encoded);
                    } else {
                        for (auto encoded : curr->entries) {
                            format::print("${:08X}:    ", addr);
                            auto inst     = (instruction_t*) &encoded;
                            u64  data     = inst->data;
                            auto operands = (operand_encoding_t*) &data;
                            format::print("{:<12} ", instruction::type::name(inst->type));
                            switch (inst->encoding) {
                                case instruction::encoding::imm: {
                                    switch (imm_type_t(operands->imm.type)) {
                                        case imm_type_t::obj:
                                            // XXX: format the scheme object
                                            format::print("{}", operands->imm.src);
                                            break;
                                        case imm_type_t::trap:
                                            format::print("{}", trap::name(operands->imm.src));
                                            break;
                                        case imm_type_t::value:
                                            format::print("{}", s32(operands->imm.src));
                                            break;
                                        case imm_type_t::label:
                                            break;
                                        case imm_type_t::block:
                                            format::print("bb_{}", operands->imm.src);
                                            break;
                                        case imm_type_t::boolean:
                                            format::print("{}", operands->imm.src ? "TRUE" : "FALSE");
                                            break;
                                    }
                                    if (operands->imm.dest > 0)
                                        format::print(", {}",
                                                      register_file::name(operands->imm.dest));
                                    break;
                                }
                                case instruction::encoding::reg1: {
                                    format::print("{}",
                                                  register_file::name(operands->reg1.dest));
                                    break;
                                }
                                case instruction::encoding::reg2: {
                                    format::print("{}, {}",
                                                  register_file::name(operands->reg2.src),
                                                  register_file::name(operands->reg2.dest));
                                    break;
                                }
                                case instruction::encoding::reg3: {
                                    format::print("{}, {}, {}",
                                                  register_file::name(operands->reg3.src),
                                                  register_file::name(operands->reg3.dest1),
                                                  register_file::name(operands->reg3.dest2));
                                    break;
                                }
                                case instruction::encoding::reg4: {
                                    format::print("{}, {}, {}",
                                                  register_file::name(operands->reg4.src),
                                                  register_file::name(operands->reg4.dest1),
                                                  register_file::name(operands->reg4.dest2),
                                                  register_file::name(operands->reg4.dest3));
                                    break;
                                }
                                case instruction::encoding::offset: {
                                    format::print("{}({}), {}",
                                                  s32(operands->offset.offs),
                                                  register_file::name(operands->offset.src),
                                                  register_file::name(operands->offset.dest));
                                    break;
                                }
                                case instruction::encoding::indexed: {
                                    format::print("{}({}, {}), {}",
                                                  s32(operands->indexed.offs),
                                                  register_file::name(operands->indexed.base),
                                                  register_file::name(operands->indexed.index),
                                                  register_file::name(operands->indexed.dest));
                                    break;
                                }
                                default: {
                                    break;
                                }
                            }
                            switch (inst->type) {
                                case instruction::type::br:
                                case instruction::type::blr:
                                case instruction::type::beq:
                                case instruction::type::bne:
                                case instruction::type::bl:
                                case instruction::type::ble:
                                case instruction::type::bg:
                                case instruction::type::bge: {
                                    format::print("\t\t\t\t; pc relative address: ${:08X}",
                                                  s32(operands->imm.src));
                                    break;
                                }
                                default: {
                                    break;
                                }
                            }
                            format::print("\n");
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
                register_alloc::init(e.gp, register_file::r0, register_file::r15);
            }
        }

        namespace bytecode {
            namespace op = instruction::type;
            namespace rf = register_file;

            bb_t& leave(bb_t& bb) {
                auto& entry_block = emitter::make_basic_block(*bb.emitter, bb_type_t::code);
               basic_block::succ(bb, entry_block);
                basic_block::note(entry_block, "** epilogue"_ss);
                basic_block::reg2(entry_block, op::move, rf::fp, rf::sp);
                basic_block::reg2(entry_block, op::pop, rf::sp, rf::lr);
                basic_block::reg1(entry_block, op::ret, rf::lr);
                return entry_block;
            }

            bb_t& enter(bb_t& bb, u32 locals) {
                auto& entry_block =  emitter::make_basic_block(*bb.emitter, bb_type_t::code);
                basic_block::note(entry_block, "** prologue"_ss);
                basic_block::succ(bb, entry_block);

                auto& exit_block  = emitter::make_basic_block(*bb.emitter, bb_type_t::code);
                basic_block::succ(entry_block, exit_block);
                basic_block::reg2(exit_block, op::push, rf::lr, rf::sp);
                basic_block::reg2(exit_block, op::move, rf::sp, rf::fp);
                if (locals > 0)
                    basic_block::imm2(exit_block, op::sub, emitter::imm(locals), rf::sp);

                return exit_block;
            }
        }

        namespace register_alloc {
            u0 free(register_alloc_t& reg_alloc) {
                memory::free(reg_alloc.alloc, reg_alloc.slots);
            }

            u0 reset(register_alloc_t& reg_alloc) {
                std::memset(reg_alloc.slots,
                            0,
                            sizeof(u8) * (reg_alloc.end - reg_alloc.start));
            }

            u0 release_all(register_alloc_t& reg_alloc) {
                memset(reg_alloc.slots, 0, reg_alloc.end - reg_alloc.start);
            }

            b8 release(register_alloc_t& reg_alloc, u8 reg) {
                const u32 i = reg - reg_alloc.start;
                if (!reg_alloc.slots[i])
                    return false;
                reg_alloc.slots[i] = 0;
                return true;
            }

            b8 reserve(register_alloc_t& reg_alloc, u8 reg) {
                const u32 i = reg - reg_alloc.start;
                if (reg_alloc.slots[i])
                    return false;
                reg_alloc.slots[i] = 1;
                return true;
            }

            b8 reserve_next(register_alloc_t& reg_alloc, u8& reg) {
                reg              = 0;
                const u32 length = reg_alloc.end - reg_alloc.start;
                for (u32  i      = 0; i < length; ++i) {
                    if (!reg_alloc.slots[i]) {
                        reg_alloc.slots[i] = 1;
                        reg = reg_alloc.start + i;
                        return true;
                    }
                }
                return false;
            }

            u0 init(register_alloc_t& reg_alloc, u32 start, u32 end, alloc_t* alloc) {
                reg_alloc.alloc = alloc;
                reg_alloc.start = start;
                reg_alloc.end   = end;
                reg_alloc.slots = (u8*) memory::alloc(reg_alloc.alloc, end - start);
                memset(reg_alloc.slots, 0, reg_alloc.end - reg_alloc.start);
            }
        }
    }
}
