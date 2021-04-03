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
#define R(n)                    G(basecode::scm::register_file:r0 + (n))

namespace basecode::scm {
    namespace register_file {
        constexpr u8 pc         = 0;
        constexpr u8 ep         = 1;
        constexpr u8 dp         = 2;
        constexpr u8 hp         = 3;
        constexpr u8 fp         = 4;
        constexpr u8 sp         = 6;   // code stack ptr
        constexpr u8 lp         = 7;   // code load ptr
        constexpr u8 m          = 8;   // mode
        constexpr u8 f          = 9;   // flags register
        constexpr u8 lr         = 10;  // link register
        constexpr u8 r0         = 11;
        constexpr u8 r1         = 12;
        constexpr u8 r2         = 13;
        constexpr u8 r3         = 14;
        constexpr u8 r4         = 15;
        constexpr u8 r5         = 16;
        constexpr u8 r6         = 17;
        constexpr u8 r7         = 18;
        constexpr u8 r8         = 19;
        constexpr u8 r9         = 20;
        constexpr u8 r10        = 21;
        constexpr u8 r11        = 22;
        constexpr u8 r12        = 23;
        constexpr u8 r13        = 24;
        constexpr u8 r14        = 25;
        constexpr u8 r15        = 26;

        str::slice_t name(u8 reg);
    }

    namespace instruction {
        namespace type {
            constexpr u8 nop    = 0;
            constexpr u8 add    = 1;
            constexpr u8 mul    = 2;
            constexpr u8 sub    = 3;
            constexpr u8 div    = 4;
            constexpr u8 pow    = 5;
            constexpr u8 mod    = 6;
            constexpr u8 neg    = 7;
            constexpr u8 not_   = 8;
            constexpr u8 shl    = 9;
            constexpr u8 shr    = 10;
            constexpr u8 or_    = 11;
            constexpr u8 and_   = 12;
            constexpr u8 xor_   = 13;
            constexpr u8 br     = 14;
            constexpr u8 blr    = 15;
            constexpr u8 cmp    = 16;
            constexpr u8 beq    = 17;
            constexpr u8 bne    = 18;
            constexpr u8 bl     = 19;
            constexpr u8 ble    = 20;
            constexpr u8 bg     = 21;
            constexpr u8 bge    = 22;
            constexpr u8 seq    = 23;
            constexpr u8 sne    = 24;
            constexpr u8 sl     = 25;
            constexpr u8 sle    = 26;
            constexpr u8 sg     = 27;
            constexpr u8 sge    = 28;
            constexpr u8 ret    = 29;
            constexpr u8 mma    = 30;
            constexpr u8 pop    = 31;
            constexpr u8 get    = 32;
            constexpr u8 set    = 33;
            constexpr u8 push   = 34;
            constexpr u8 move   = 35;
            constexpr u8 load   = 36;
            constexpr u8 store  = 37;
            constexpr u8 exit   = 38;
            constexpr u8 trap   = 39;
            constexpr u8 lea    = 40;
            constexpr u8 bra    = 41;
            constexpr u8 car    = 42;
            constexpr u8 cdr    = 43;
            constexpr u8 setcar = 44;
            constexpr u8 setcdr = 45;
            constexpr u8 fix    = 46;
            constexpr u8 flo    = 47;
            constexpr u8 cons   = 48;
            constexpr u8 env    = 49;
            constexpr u8 type   = 50;
            constexpr u8 list   = 51;
            constexpr u8 eval   = 52;
            constexpr u8 error  = 53;
            constexpr u8 write  = 54;
            constexpr u8 qt     = 55;
            constexpr u8 qq     = 56;
            constexpr u8 gc     = 57;
            constexpr u8 apply  = 58;
            constexpr u8 const_ = 59;
            constexpr u8 ladd   = 60;
            constexpr u8 lsub   = 61;
            constexpr u8 lmul   = 62;
            constexpr u8 ldiv   = 63;
            constexpr u8 lmod   = 64;
            constexpr u8 truthy = 65;

            str::slice_t name(u8 op);
        }

        namespace encoding {
            constexpr u8 none   = 0;
            constexpr u8 imm    = 1;
            constexpr u8 reg1   = 2;
            constexpr u8 reg2   = 3;
            constexpr u8 reg3   = 4;
            constexpr u8 reg4   = 5;
            constexpr u8 offset = 6;
            constexpr u8 indexed= 7;
        }
    }

    union operand_encoding_t final {
        struct {
            u64                 src:        32;
            u64                 dest:       8;
            u64                 type:       4;
            u64                 size:       3;
            u64                 aux:        6;
        }                       imm;
        struct {
            u64                 dest:       8;
            u64                 pad:        45;
        }                       reg1;
        struct {
            u64                 src:        8;
            u64                 dest:       8;
            u64                 aux:        32;
            u64                 pad:        5;
        }                       reg2;
        struct {
            u64                 src:        8;
            u64                 dest1:      8;
            u64                 dest2:      8;
            u64                 pad:        29;
        }                       reg3;
        struct {
            u64                 src:        8;
            u64                 dest1:      8;
            u64                 dest2:      8;
            u64                 dest3:      8;
            u64                 pad:        21;
        }                       reg4;
        struct {
            u64                 offs:       32;
            u64                 src:        8;
            u64                 dest:       8;
            u64                 pad:        5;
        }                       offset;
        struct {
            u64                 offs:       24;
            u64                 base:       8;
            u64                 index:      8;
            u64                 dest:       8;
            u64                 pad:        5;
        }                       indexed;
    };

    struct instruction_t final {
        u64                     type:       7;
        u64                     is_signed:  1;
        u64                     encoding:   3;
        u64                     data:       53;
    };
    static_assert(sizeof(instruction_t) <= 8, "instruction_t is now greater than 8 bytes!");

    struct bb_t;
    struct emitter_t;

    using label_t               = u32;
    using bb_list_t             = stable_array_t<bb_t>;
    using label_map_t           = hashtab_t<u32, bb_t*>;
    using encoded_list_t        = array_t<u64>;
    using comment_list_t        = array_t<u32>;

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
        encoded_list_t          entries;
        comment_list_t          comments;
        label_t                 label;
        u32                     id;
        bb_type_t               type;
    };

    struct register_alloc_t final {
        alloc_t*                alloc;
        u8*                     slots;
        u32                     start;
        u32                     end;
    };

    struct emitter_t final {
        alloc_t*                alloc;
        vm_t*                   vm;
        u64                     address;
        bb_list_t               blocks;
        str_array_t             strings;
        label_map_t             labels;
        register_alloc_t        gp;
    };

    namespace vm {
        namespace basic_block {
            u0 free(bb_t& bb);

            u0 dw(bb_t& bb, imm_t imm);

            u0 none(bb_t& bb, u8 opcode);

            u0 pred(bb_t& bb, bb_t& pred);

            u0 succ(bb_t& bb, bb_t& succ);

            u0 reg1(bb_t& bb, u8 opcode, u8 arg);

            u0 imm1(bb_t& bb, u8 opcode, imm_t imm);

            u0 apply_label(bb_t& bb, label_t label);

            bb_t& ubuf(bb_t& bb, u8 addr_reg, u32 size);

            u0 init(bb_t& bb, emitter_t* e, bb_type_t type);

            u0 note(bb_t& bb, const String_Concept auto& value) {
                str_array::append(bb.emitter->strings, value);
                array::append(bb.comments, bb.emitter->strings.size);
            }

            u0 reg3(bb_t& bb, u8 opcode, u8 src, u8 dest1, u8 dest2);

            u0 offs(bb_t& bb, u8 opcode, s32 offset, u8 src, u8 dest);

            bb_t& ibuf(bb_t& bb, u8 addr_reg, const imm_t* data, u32 size);

            u0 indx(bb_t& bb, u8 opcode, s32 offset, u8 base, u8 index, u8 dest);

            u0 imm2(bb_t& bb, u8 opcode, imm_t src, u8 dest, b8 is_signed = false);

            u0 reg2(bb_t& bb, u8 opcode, u8 src, u8 dest, b8 is_signed = false, s32 aux = 0);
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

            u0 disassemble(emitter_t& e, bb_t& start_block);

            template <String_Concept T>
            label_t make_label(emitter_t& e, const T& name) {
                str_array::append(e.strings, name);
                return e.strings.size;
            }

            status_t assemble(emitter_t& e, bb_t& start_block);

            bb_t& make_basic_block(emitter_t& e, bb_type_t type);

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

            u0 init(emitter_t& e, vm_t* vm, u64 address, alloc_t* alloc = context::top()->alloc);
        }

        namespace bytecode {
            bb_t& leave(bb_t& e);

            bb_t& enter(bb_t& e, u32 locals);
        }

        namespace register_alloc {
            u0 free(register_alloc_t& reg_alloc);

            u0 reset(register_alloc_t& reg_alloc);

            u0 release_all(register_alloc_t& reg_alloc);

            b8 release(register_alloc_t& reg_alloc, u8 reg);

            b8 reserve(register_alloc_t& reg_alloc, u8 reg = 0);

            b8 reserve_next(register_alloc_t& reg_alloc, u8& reg);

            u0 init(register_alloc_t& reg_alloc, u32 start, u32 end, alloc_t* alloc = context::top()->alloc);
        }
    }
}
