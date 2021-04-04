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

#pragma once

#include <basecode/core/scm/vm.h>
#include <basecode/core/str_array.h>
#include <basecode/core/stable_array.h>

#define H(a)                    (vm.heap[(a / 8)])
#define G(n)                    (vm.heap[(vm.memory_map.heap_size - 1) - (n)])
#define M                       G(basecode::scm::register_file::m)
#define F                       G(basecode::scm::register_file::f)
#define PC                      G(basecode::scm::register_file::pc)
#define EP                      G(basecode::scm::register_file::ep)
#define HP                      G(basecode::scm::register_file::hp)
#define DP                      G(basecode::scm::register_file::dp)
#define LP                      G(basecode::scm::register_file::lp)
#define SP                      G(basecode::scm::register_file::sp)
#define LR                      G(basecode::scm::register_file::lr)
#define R(n)                    G(basecode::scm::register_file::r0 + (n))

namespace basecode::scm {
    using reg_t                 = u8;
    using op_code_t             = u8;

    namespace register_file {
        constexpr reg_t pc      = 0;
        constexpr reg_t ep      = 1;
        constexpr reg_t dp      = 2;
        constexpr reg_t hp      = 3;
        constexpr reg_t fp      = 4;
        constexpr reg_t sp      = 5;   // code stack ptr
        constexpr reg_t lp      = 6;   // code load ptr
        constexpr reg_t m       = 7;   // mode
        constexpr reg_t f       = 8;   // flags register
        constexpr reg_t lr      = 9;   // link register
        constexpr reg_t r0      = 10;
        constexpr reg_t r1      = 11;
        constexpr reg_t r2      = 12;
        constexpr reg_t r3      = 13;
        constexpr reg_t r4      = 14;
        constexpr reg_t r5      = 15;
        constexpr reg_t r6      = 16;
        constexpr reg_t r7      = 17;
        constexpr reg_t r8      = 18;
        constexpr reg_t r9      = 19;
        constexpr reg_t r10     = 20;
        constexpr reg_t r11     = 21;
        constexpr reg_t r12     = 22;
        constexpr reg_t r13     = 23;
        constexpr reg_t r14     = 24;
        constexpr reg_t r15     = 25;

        str::slice_t name(reg_t reg);
    }

    namespace instruction {
        namespace type {
            constexpr op_code_t nop     = 0;
            constexpr op_code_t add     = 1;
            constexpr op_code_t mul     = 2;
            constexpr op_code_t sub     = 3;
            constexpr op_code_t div     = 4;
            constexpr op_code_t pow     = 5;
            constexpr op_code_t mod     = 6;
            constexpr op_code_t neg     = 7;
            constexpr op_code_t not_    = 8;
            constexpr op_code_t shl     = 9;
            constexpr op_code_t shr     = 10;
            constexpr op_code_t or_     = 11;
            constexpr op_code_t and_    = 12;
            constexpr op_code_t xor_    = 13;
            constexpr op_code_t br      = 14;
            constexpr op_code_t blr     = 15;
            constexpr op_code_t cmp     = 16;
            constexpr op_code_t beq     = 17;
            constexpr op_code_t bne     = 18;
            constexpr op_code_t bl      = 19;
            constexpr op_code_t ble     = 20;
            constexpr op_code_t bg      = 21;
            constexpr op_code_t bge     = 22;
            constexpr op_code_t seq     = 23;
            constexpr op_code_t sne     = 24;
            constexpr op_code_t sl      = 25;
            constexpr op_code_t sle     = 26;
            constexpr op_code_t sg      = 27;
            constexpr op_code_t sge     = 28;
            constexpr op_code_t ret     = 29;
            constexpr op_code_t mma     = 30;
            constexpr op_code_t pop     = 31;
            constexpr op_code_t get     = 32;
            constexpr op_code_t set     = 33;
            constexpr op_code_t push    = 34;
            constexpr op_code_t move    = 35;
            constexpr op_code_t load    = 36;
            constexpr op_code_t store   = 37;
            constexpr op_code_t exit    = 38;
            constexpr op_code_t trap    = 39;
            constexpr op_code_t lea     = 40;
            constexpr op_code_t bra     = 41;
            constexpr op_code_t car     = 42;
            constexpr op_code_t cdr     = 43;
            constexpr op_code_t setcar  = 44;
            constexpr op_code_t setcdr  = 45;
            constexpr op_code_t fix     = 46;
            constexpr op_code_t flo     = 47;
            constexpr op_code_t cons    = 48;
            constexpr op_code_t env     = 49;
            constexpr op_code_t type    = 50;
            constexpr op_code_t list    = 51;
            constexpr op_code_t eval    = 52;
            constexpr op_code_t error   = 53;
            constexpr op_code_t write   = 54;
            constexpr op_code_t qt      = 55;
            constexpr op_code_t qq      = 56;
            constexpr op_code_t gc      = 57;
            constexpr op_code_t apply   = 58;
            constexpr op_code_t const_  = 59;
            constexpr op_code_t ladd    = 60;
            constexpr op_code_t lsub    = 61;
            constexpr op_code_t lmul    = 62;
            constexpr op_code_t ldiv    = 63;
            constexpr op_code_t lmod    = 64;
            constexpr op_code_t lnot    = 65;
            constexpr op_code_t pairp   = 66;
            constexpr op_code_t listp   = 67;
            constexpr op_code_t symp    = 68;
            constexpr op_code_t atomp   = 69;
            constexpr op_code_t truep   = 70;
            constexpr op_code_t falsep  = 71;
            constexpr op_code_t lcmp    = 72;

            str::slice_t name(op_code_t op);
        }

        namespace encoding {
            constexpr u8 none           = 0;
            constexpr u8 imm            = 1;
            constexpr u8 reg1           = 2;
            constexpr u8 reg2           = 3;
            constexpr u8 reg3           = 4;
            constexpr u8 reg4           = 5;
            constexpr u8 offset         = 6;
            constexpr u8 indexed        = 7;
            constexpr u8 reg2_imm       = 8;
        }
    }

    union operand_encoding_t final {
        struct {
            u64                 src:        32;
            u64                 dest:       8;
            u64                 type:       4;
            u64                 size:       3;
            u64                 aux:        4;
        }                       imm;
        struct {
            u64                 dest:       8;
            u64                 pad:        43;
        }                       reg1;
        struct {
            u64                 src:        8;
            u64                 dest:       8;
            u64                 aux:        32;
            u64                 pad:        3;
        }                       reg2;
        struct {
            u64                 a:          8;
            u64                 b:          8;
            u64                 imm:        28;
            u64                 size:       3;
            u64                 type:       4;
        }                       reg2_imm;
        struct {
            u64                 a:          8;
            u64                 b:          8;
            u64                 c:          8;
            u64                 pad:        27;
        }                       reg3;
        struct {
            u64                 a:          8;
            u64                 b:          8;
            u64                 c:          8;
            u64                 d:          8;
            u64                 pad:        19;
        }                       reg4;
        struct {
            u64                 offs:       32;
            u64                 src:        8;
            u64                 dest:       8;
            u64                 mode:       1;
            u64                 pad:        2;
        }                       offset;
        struct {
            u64                 offs:       24;
            u64                 base:       8;
            u64                 index:      8;
            u64                 dest:       8;
            u64                 mode:       1;
            u64                 pad:        2;
        }                       indexed;
    };

    struct instruction_t final {
        u64                     type:       8;
        u64                     is_signed:  1;
        u64                     encoding:   4;
        u64                     data:       51;
    };
    static_assert(sizeof(instruction_t) <= 8, "instruction_t is now greater than 8 bytes!");

    struct bb_t;
    struct emitter_t;

    using label_t               = u32;
    using bb_list_t             = stable_array_t<bb_t>;
    using label_map_t           = hashtab_t<u32, bb_t*>;
    using note_list_t           = array_t<u32>;
    using encoded_list_t        = array_t<u64>;
    using comment_table_t       = hashtab_t<u32, note_list_t>;

    enum class imm_size_t : u8 {
        signed_word,
        unsigned_word,
        signed_half_word,
        unsigned_half_word,
    };

    enum class imm_type_t : u8 {
        obj,
        trap,
        value,
        label,
        block,
        boolean
    };

    struct imm_t final {
        union {
            s32                 s;
            u32                 u;
            s64                 ls;
            u64                 lu;
            bb_t*               b;
        };
        imm_type_t              type;
        imm_size_t              size;
    };

    enum class bb_type_t : u8 {
        code,
        data,
        empty
    };

    struct bb_t final {
        emitter_t*              emitter;
        bb_t*                   prev;
        bb_t*                   next;
        u64                     addr;
        note_list_t             notes;
        encoded_list_t          entries;
        comment_table_t         comments;
        label_t                 label;
        u32                     id;
        bb_type_t               type;
    };

    struct reg_alloc_t final {
        u64                     slots;
        reg_t                   start;
        reg_t                   end;
    };

    struct reg_result_t;

    struct emitter_t final {
        alloc_t*                alloc;
        vm_t*                   vm;
        u64                     address;
        bb_list_t               blocks;
        str_array_t             strings;
        label_map_t             labels;
        reg_alloc_t             gp;
    };

    namespace vm {
        namespace basic_block {
            u0 free(bb_t& bb);

            u0 dw(bb_t& bb, imm_t imm);

            u0 pred(bb_t& bb, bb_t& pred);

            u0 succ(bb_t& bb, bb_t& succ);

            template <String_Concept T>
            u0 note(bb_t& bb, const T& value) {
                str_array::append(bb.emitter->strings, value);
                array::append(bb.notes, bb.emitter->strings.size);
            }

            u0 none(bb_t& bb, op_code_t opcode);

            u0 apply_label(bb_t& bb, label_t label);

            u0 reg1(bb_t& bb, op_code_t opcode, reg_t arg);

            u0 imm1(bb_t& bb, op_code_t opcode, imm_t imm);

            bb_t& ubuf(bb_t& bb, reg_t addr_reg, u32 size);

            u0 init(bb_t& bb, emitter_t* e, bb_type_t type);

            template <String_Concept T>
            u0 comment(bb_t& bb, const T& value, s32 line = -1) {
                line = line == -1 ? bb.entries.size : line;
                auto notes = hashtab::find(bb.comments, line);
                if (!notes) {
                    notes = hashtab::emplace(bb.comments, line);
                    array::init(*notes, bb.emitter->alloc);
                }
                str_array::append(bb.emitter->strings, value);
                array::append(*notes, bb.emitter->strings.size);
            }

            bb_t& make_succ(bb_t& bb, bb_type_t type = bb_type_t::code);

            bb_t& ibuf(bb_t& bb, u8 addr_reg, const imm_t* data, u32 size);

            u0 reg3(bb_t& bb, op_code_t opcode, reg_t src, reg_t dest1, reg_t dest2);

            u0 imm2(bb_t& bb, op_code_t opcode, imm_t src, reg_t dest, b8 is_signed = false);

            u0 indx(bb_t& bb, op_code_t opcode, s32 offset, reg_t base, reg_t index, reg_t dest);

            u0 offs(bb_t& bb, op_code_t opcode, s32 offset, reg_t src, reg_t dest, b8 mode = false);

            u0 reg2_imm(bb_t& bb, op_code_t opcode, reg_t a, reg_t b, imm_t imm, b8 is_signed = false);

            u0 reg2(bb_t& bb, op_code_t opcode, reg_t src, reg_t dest, b8 is_signed = false, s32 aux = 0);
        }

        namespace emitter {
            u0 free(emitter_t& e);

            u0 reset(emitter_t& e);

            constexpr imm_t imm(bb_t* bb) {
                return imm_t{
                    .b = bb,
                    .type = imm_type_t::block,
                    .size = imm_size_t::unsigned_half_word
                };
            }

            template <String_Concept T>
            label_t make_label(emitter_t& e, const T& name) {
                str_array::append(e.strings, name);
                return e.strings.size;
            }

            status_t assemble(emitter_t& e, bb_t& start_block);

            u0 disassemble(emitter_t& e, bb_t& start_block, str_buf_t& buf);

            constexpr imm_t imm(s32 v, imm_type_t type = imm_type_t::value) {
                return imm_t{
                    .s = v,
                    .type = type,
                    .size = imm_size_t::signed_half_word
                };
            }

            constexpr imm_t imm(u32 v, imm_type_t type = imm_type_t::value) {
                return imm_t{
                    .u = v,
                    .type = type,
                    .size = imm_size_t::unsigned_half_word
                };
            }

            constexpr imm_t imm(u64 v, imm_type_t type = imm_type_t::value) {
                return imm_t{
                    .lu = v,
                    .type = type,
                    .size = imm_size_t::unsigned_half_word
                };
            }

            bb_t& make_basic_block(emitter_t& e, bb_type_t type = bb_type_t::code);

            u0 init(emitter_t& e, vm_t* vm, u64 address, alloc_t* alloc = context::top()->alloc);
        }

        namespace bytecode {
            bb_t& leave(bb_t& bb);

            bb_t& enter(bb_t& bb, u32 locals);

            u0 free_stack(bb_t& bb, u32 words);

            u0 get(bb_t& bb, u32 idx, reg_t reg);

            u0 set(bb_t& bb, u32 idx, reg_t reg);

            u0 set(bb_t& bb, reg_t sym, reg_t val);

            u0 const_(bb_t& bb, u32 idx, reg_t reg);

            u0 qt(bb_t& bb, u32 idx, reg_t target_reg);

            u0 qq(bb_t& bb, u32 idx, reg_t target_reg);

            u0 error(bb_t& bb, u32 idx, reg_t target_reg);

            u0 car(bb_t& bb, reg_t val_reg, reg_t target_reg);

            u0 cdr(bb_t& bb, reg_t val_reg, reg_t target_reg);

            u0 not_(bb_t& bb, reg_t val_reg, reg_t target_reg);

            u0 alloc_stack(bb_t& bb, u32 words, reg_t base_reg);

            u0 setcar(bb_t& bb, reg_t val_reg, reg_t target_reg);

            u0 setcdr(bb_t& bb, reg_t val_reg, reg_t target_reg);

            u0 is(bb_t& bb, reg_t lhs, reg_t rhs, reg_t target_reg);

            u0 lt(bb_t& bb, reg_t lhs, reg_t rhs, reg_t target_reg);

            u0 gt(bb_t& bb, reg_t lhs, reg_t rhs, reg_t target_reg);

            u0 lte(bb_t& bb, reg_t lhs, reg_t rhs, reg_t target_reg);

            u0 gte(bb_t& bb, reg_t lhs, reg_t rhs, reg_t target_reg);

            u0 cons(bb_t& bb, reg_t car, reg_t cdr, reg_t target_reg);

            bb_t& list(bb_t& bb, reg_t lst_reg, reg_t base_reg, reg_t target_reg, u32 size);

            bb_t& arith_op(bb_t& bb, op_code_t op_code, reg_t base_reg, reg_t target_reg, u32 size);
        }

        namespace reg_alloc {
            u0 reset(reg_alloc_t& alloc);

            u0 init(reg_alloc_t& alloc, reg_t start, reg_t end);

            reg_result_t reserve(reg_alloc_t& alloc, u32 count = 1);

            u0 release(reg_alloc_t& alloc, const reg_result_t& result);
        }
    }

    struct reg_result_t final {
        u32                     count;

        ~reg_result_t() {
            vm::reg_alloc::release(*alloc, *this);
        }

        reg_t& operator[](u32 idx) {
            return regs[idx];
        };

        const reg_t operator[](u32 idx) const {
            return regs[idx];
        };

        reg_result_t(reg_alloc_t& a) : count(0), alloc(&a), regs() {
        }

    private:
        reg_alloc_t*            alloc;
        reg_t                   regs[16];
    };
}
