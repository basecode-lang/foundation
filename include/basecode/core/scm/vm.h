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

#pragma once

#include <basecode/core/scm/types.h>

#define G(n)                    (vm.reg_file->operator[]((n)))
#define HS(a)                   (vm.heap->operator[]((a) / sizeof(u64)))
#define HU(a)                   (*(reinterpret_cast<u64*>(a)))
#define M                       G(basecode::scm::vm::register_file::m)
#define F                       G(basecode::scm::vm::register_file::f)
#define PC                      G(basecode::scm::vm::register_file::pc)
#define GP                      G(basecode::scm::vm::register_file::gp)
#define EP                      G(basecode::scm::vm::register_file::ep)
#define HP                      G(basecode::scm::vm::register_file::hp)
#define DP                      G(basecode::scm::vm::register_file::dp)
#define SP                      G(basecode::scm::vm::register_file::sp)
#define LR                      G(basecode::scm::vm::register_file::lr)
#define R(n)                    G(basecode::scm::vm::register_file::r0 + (n))

namespace basecode::scm::vm {
    namespace trap {
        constexpr u8 hash       = 1;
        constexpr u8 functor    = 2;

        str::slice_t name(u8 type);
    }

    namespace register_file {
        constexpr reg_t none            = 0;
        constexpr reg_t pc              = 1;
        constexpr reg_t ep              = 2;    // env pointer
        constexpr reg_t cp              = 3;    // continuation pointer
        constexpr reg_t lp              = 4;    // literal frame pointer
        constexpr reg_t gp              = 5;    // gc pointer
        constexpr reg_t dp              = 6;    // data stack pointer
        constexpr reg_t hp              = 7;    // heap stack pointer
        constexpr reg_t fp              = 8;    // frame pointer
        constexpr reg_t sp              = 9;    // code stack ptr
        constexpr reg_t m               = 10;   // mode
        constexpr reg_t f               = 11;   // flags register
        constexpr reg_t lr              = 12;   // link register
        constexpr reg_t r0              = 13;
        constexpr reg_t r1              = 14;
        constexpr reg_t r2              = 15;
        constexpr reg_t r3              = 16;
        constexpr reg_t r4              = 17;
        constexpr reg_t r5              = 18;
        constexpr reg_t r6              = 19;
        constexpr reg_t r7              = 20;
        constexpr reg_t r8              = 21;
        constexpr reg_t r9              = 22;
        constexpr reg_t r10             = 23;
        constexpr reg_t r11             = 24;
        constexpr reg_t r12             = 25;
        constexpr reg_t r13             = 26;
        constexpr reg_t r14             = 27;
        constexpr reg_t r15             = 28;
        constexpr reg_t max             = 29;

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
            constexpr op_code_t b       = 41;
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
            constexpr op_code_t collect = 57;
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
            constexpr op_code_t clc     = 73;
            constexpr op_code_t sec     = 74;
            constexpr op_code_t read    = 75;
            constexpr op_code_t define  = 76;

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

    namespace mem_area {
        status_t init(mem_area_t& area,
                      vm_t* vm,
                      u32 id,
                      mem_area_type_t type,
                      reg_t reg,
                      alloc_t* alloc,
                      u32 min_capacity,
                      b8 top);

        template <typename T>
        T top(mem_area_t& area) {
            auto& vm = *area.vm;
            if (area.size == 0)
                return T{};
            if (area.reg != register_file::none) {
                return T(HU(G(area.reg)));
            } else {
                return (T) (area.top ? area[area.capacity - area.size - 1] :
                            area[area.size - 1]);
            }
        }

        template <typename T>
        T pop(mem_area_t& area) {
            auto& vm = *area.vm;
            if (area.size > 0) {
                u64 tos{};
                if (area.reg != register_file::none) {
                    tos = HU(G(area.reg));
                    if (area.top) {
                        G(area.reg) += sizeof(u64);
                    } else {
                        G(area.reg) -= sizeof(u64);
                    }
                } else {
                    tos = area.top ? area[area.capacity - area.size - 1] :
                          area[area.size - 1];
                }
                --area.size;
                return T(tos);
            }
            return {};
        }

        u0 free(mem_area_t& area);

        template <typename T>
        u0 push(mem_area_t& area, T value) {
            auto& vm = *area.vm;
            if (area.size + 1 > area.capacity)
                grow(area);
            if (area.top) {
                if (area.reg != register_file::none) {
                    G(area.reg) -= sizeof(u64);
                    HU(G(area.reg)) = u64(value);
                } else {
                    area[area.capacity - area.size - 1] = u64(value);
                }
            } else {
                if (area.reg != register_file::none) {
                    HU(G(area.reg)) = u64(value);
                    G(area.reg) += sizeof(u64);
                } else {
                    area[area.size] = u64(value);
                }
            }
            ++area.size;
        }

        inline u64 base_addr(mem_area_t& area) {
            return area.top ? u64(area.data)
                              + ((area.capacity - area.size) * sizeof(u64)) :
                   u64(area.data) + (area.size * sizeof(u64));
        }

        u0 resize(mem_area_t& area, u32 new_size);

        str::slice_t type_name(mem_area_type_t type);

        u0 reserve(mem_area_t& area, u32 new_capacity);

        u0 reset(mem_area_t& area, b8 zero_mem = false);

        u0 grow(mem_area_t& area, u32 new_capacity = 16);

        u0 shrink_to_size(mem_area_t& area, u32 new_size);
    }

    u0 free(vm_t& vm);

    u0 reset(vm_t& vm);

    mem_area_t* add_mem_area(vm_t& vm,
                             mem_area_type_t type,
                             reg_t reg,
                             alloc_t* alloc,
                             u32 min_capacity = 0,
                             b8 top = false);

    b8 remove_mem_area(vm_t& vm, mem_area_t* area);

    mem_area_t* get_mem_area_by_reg(vm_t& vm, reg_t reg);

    status_t step(vm_t& vm, ctx_t* ctx, s32 cycles = -1);

    mem_area_t* get_mem_area_by_type(vm_t& vm, mem_area_type_t type);

    status_t init(vm_t& vm, alloc_t* alloc = context::top()->alloc.main);
}
