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
#include <basecode/core/scm/types.h>
#include <basecode/core/str_array.h>
#include <basecode/core/stable_array.h>

#define H(a)                    (vm[((a) / 8)])
#define G(n)                    (vm[(vm.memory_map.heap_size - 1) - (n)])
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
    namespace trap {
        constexpr u8 hash       = 1;
        constexpr u8 functor    = 2;

        str::slice_t name(u8 type);
    }

    namespace register_file {
        constexpr reg_t pc              = 0;
        constexpr reg_t ep              = 1;
        constexpr reg_t dp              = 2;
        constexpr reg_t hp              = 3;
        constexpr reg_t fp              = 4;
        constexpr reg_t sp              = 5;   // code stack ptr
        constexpr reg_t lp              = 6;   // code load ptr
        constexpr reg_t m               = 7;   // mode
        constexpr reg_t f               = 8;   // flags register
        constexpr reg_t lr              = 9;   // link register
        constexpr reg_t r0              = 10;
        constexpr reg_t r1              = 11;
        constexpr reg_t r2              = 12;
        constexpr reg_t r3              = 13;
        constexpr reg_t r4              = 14;
        constexpr reg_t r5              = 15;
        constexpr reg_t r6              = 16;
        constexpr reg_t r7              = 17;
        constexpr reg_t r8              = 18;
        constexpr reg_t r9              = 19;
        constexpr reg_t r10             = 20;
        constexpr reg_t r11             = 21;
        constexpr reg_t r12             = 22;
        constexpr reg_t r13             = 23;
        constexpr reg_t r14             = 24;
        constexpr reg_t r15             = 25;

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

    namespace vm {
        namespace basic_block {
            class bb_builder_t;

            u0 free(bb_t& bb);

            bb_builder_t encode(bb_t* bb);

            bb_builder_t encode(bb_t& bb);

            u0 init(bb_t& bb, emitter_t* e, bb_type_t type);
        }

        namespace emitter {
            u0 init(emitter_t& e,
                    vm_t* vm,
                    u64 addr,
                    alloc_t* alloc = context::top()->alloc);

            u0 free(emitter_t& e);

            u0 reset(emitter_t& e);

            template <String_Concept T>
            var_t* make_var(emitter_t& e, const T& name) {
                assoc_array_t<var_t*> pairs{};
                assoc_array::init(pairs, e.alloc);
                defer(assoc_array::free(pairs));
                symtab::find_prefix(e.vartab, pairs, name);
                if (pairs.size > 0)
                    return nullptr;
                auto rc = string::interned::fold_for_result(name);
                if (!OK(rc.status))
                    return nullptr;
                auto var = &stable_array::append(e.vars);
                var->pred    = var->succ = {};
                var->symbol  = rc.id;
                var->version = 1;
                symtab::insert(e.vartab,
                               format::format("{}", *var),
                               var);
                return var;
            }

            template <String_Concept T>
            label_t make_label(emitter_t& e, const T& name) {
                str_array::append(e.strtab, name);
                return e.strtab.size;
            }

            status_t assemble(emitter_t& e, bb_t& start_block);

            inline var_t* make_var(emitter_t& e, var_t* pred) {
                auto var = &stable_array::append(e.vars);
                var->succ    = {};
                var->pred    = pred;
                var->symbol  = pred->symbol;
                var->version = pred->version + 1;
                symtab::insert(e.vartab,
                               format::format("{}", *var),
                               var);
                pred->succ = var;
                return var;
            }

            status_t encode_inst(vm_t& vm, const inst_t& inst, u64 addr);

            inline str::slice_t get_string(emitter_t& e, label_t label) {
                return e.strtab[label - 1];
            }

            u0 disassemble(emitter_t& e, bb_t& start_block, str_buf_t& buf);

            bb_t& make_basic_block(emitter_t& e, bb_type_t type = bb_type_t::code);
        }

        namespace bytecode {
            bb_t& leave(bb_t& bb);

            bb_t& enter(bb_t& bb, u32 locals);

            u0 free_stack(bb_t& bb, u32 words);

            u0 todo(bb_t& bb, str::slice_t msg);

            u0 pop_env(compiler_t& comp, bb_t& bb);

            u0 push_env(compiler_t& comp, bb_t& bb);

            compile_result_t prim(compiler_t& comp,
                                  const context_t& c,
                                  obj_t* form,
                                  obj_t* args,
                                  prim_type_t type);

            compile_result_t cmp_op(compiler_t& comp,
                                    const context_t& c,
                                    prim_type_t type,
                                    obj_t* args);

            compile_result_t arith_op(compiler_t& comp,
                                      const context_t& c,
                                      op_code_t op_code,
                                      obj_t* args);

            u0 save_protected(bb_t& bb, compiler_t& comp);

            u0 restore_protected(bb_t& bb, compiler_t& comp);

            u0 alloc_stack(bb_t& bb, u32 words, reg_t base_reg);

            compile_result_t lookup(compiler_t& comp, const context_t& c);

            compile_result_t self_eval(compiler_t& comp, const context_t& c);

            compile_result_t qt(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t qq(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t uq(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t uqs(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t car(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t cdr(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t or_(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t do_(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t if_(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t atom(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t eval(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t list(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t cons(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t and_(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t not_(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t error(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t print(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t while_(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t set_car(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t set_cdr(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t let_set(compiler_t& comp, const context_t& c, obj_t* args);

            compile_result_t comp_proc(compiler_t& comp, const context_t& c, proc_t* proc);

            compile_result_t fn(compiler_t& comp, const context_t& c, obj_t* form, obj_t* args);

            compile_result_t ffi(compiler_t& comp, const context_t& c, obj_t* sym, obj_t* form, obj_t* args);

            compile_result_t apply(compiler_t& comp, const context_t& c, obj_t* sym, obj_t* form, obj_t* args);

            compile_result_t inline_(compiler_t& comp, const context_t& c, obj_t* sym, obj_t* form, obj_t* args);

            compile_result_t call_back(compiler_t& comp, const context_t& c, obj_t* sym, obj_t* form, obj_t* args);
        }

        namespace reg_alloc {
            inline u0 release(reg_alloc_t& alloc, const reg_result_t& result);
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

    namespace vm::reg_alloc {
        inline u0 reset(reg_alloc_t& alloc) {
            alloc.slots = {};
            alloc.prots = {};
        }

        inline reg_t reserve_one(reg_alloc_t& alloc) {
            u32 msb_zeros = alloc.slots != 0 ? __builtin_clzll(alloc.slots) : 64;
            if (!msb_zeros)
                return 0;
            u32 idx = 64 - msb_zeros;
            alloc.slots |= (1UL << idx);
            return alloc.start + idx;
        }

        inline u0 release_one(reg_alloc_t& alloc, reg_t reg) {
            const u32 idx = reg - alloc.start;
            if (!(alloc.slots & (1UL << idx)))
                return;
            const auto mask = ~(1UL << idx);
            alloc.slots &= mask;
            alloc.prots &= mask;
        }

        inline b8 is_protected(reg_alloc_t& alloc, reg_t reg) {
            const auto mask = 1UL << reg;
            return (alloc.prots & mask) == mask;
        }

        inline u0 protect(reg_alloc_t& alloc, reg_t reg, b8 flag) {
            if (flag) {
                alloc.prots |= (1UL << reg);
            } else {
                alloc.prots &= ~(1UL << reg);
            }
        }

        inline u0 init(reg_alloc_t& alloc, reg_t start, reg_t end) {
            alloc.end   = end;
            alloc.start = start;
            alloc.slots = alloc.prots = {};
        }

        inline reg_result_t reserve(reg_alloc_t& alloc, u32 count = 1) {
            reg_result_t r(alloc);
            for (u32 i = 0; i < count; ++i)
                r[r.count++] = reserve_one(alloc);
            return r;
        }

        inline u0 reserve_and_protect_reg(reg_alloc_t& alloc, reg_t reg) {
            const u32 idx = reg - alloc.start;
            const auto mask = (1UL << idx);
            alloc.slots |= mask;
            alloc.prots |= mask;
        }

        inline u0 release(reg_alloc_t& alloc, const reg_result_t& result) {
            for (u32 i = 0; i < result.count; ++i)
                release_one(alloc, result[i]);
        }
    }

    namespace vm::basic_block {
        class reg1_builder_t final {
            bb_builder_t*   _builder;
        public:
            reg1_builder_t(bb_builder_t* builder) : _builder(builder) {};

            bb_builder_t& build();

            reg1_builder_t& dst(reg_t reg);

            reg1_builder_t& dst(var_t* var);

            reg1_builder_t& op(op_code_t op_code);
        };

        class reg2_builder_t final {
            bb_builder_t*   _builder;
        public:
            reg2_builder_t(bb_builder_t* builder) : _builder(builder) {};

            bb_builder_t& build();

            reg2_builder_t& aux(s8 value);

            reg2_builder_t& dst(reg_t reg);

            reg2_builder_t& dst(var_t* var);

            reg2_builder_t& src(reg_t reg);

            reg2_builder_t& src(var_t* var);

            reg2_builder_t& is_signed(b8 flag);

            reg2_builder_t& op(op_code_t op_code);
        };

        class reg3_builder_t final {
            bb_builder_t*   _builder;
        public:
            reg3_builder_t(bb_builder_t* builder) : _builder(builder) {};

            bb_builder_t& build();

            reg3_builder_t& a(reg_t reg);

            reg3_builder_t& a(var_t* var);

            reg3_builder_t& b(reg_t reg);

            reg3_builder_t& b(var_t* var);

            reg3_builder_t& c(reg_t reg);

            reg3_builder_t& c(var_t* var);

            reg3_builder_t& op(op_code_t op_code);
        };

        class offs_builder_t final {
            bb_builder_t*   _builder;
        public:
            offs_builder_t(bb_builder_t* builder) : _builder(builder) {};

            bb_builder_t& build();

            offs_builder_t& mode(b8 flag);

            offs_builder_t& dst(reg_t reg);

            offs_builder_t& src(reg_t reg);

            offs_builder_t& dst(var_t* var);

            offs_builder_t& src(var_t* var);

            offs_builder_t& offset(s32 value);

            offs_builder_t& op(op_code_t op_code);
        };

        class reg2_imm_builder_t final {
            bb_builder_t*   _builder;
        public:
            reg2_imm_builder_t(bb_builder_t* builder) : _builder(builder) {};

            bb_builder_t& build();

            reg2_imm_builder_t& dst(reg_t reg);

            reg2_imm_builder_t& dst(var_t* var);

            reg2_imm_builder_t& src(reg_t reg);

            reg2_imm_builder_t& src(var_t* var);

            reg2_imm_builder_t& is_signed(b8 flag);

            reg2_imm_builder_t& value(s32 value);

            reg2_imm_builder_t& value(u32 value);

            reg2_imm_builder_t& value(bb_t* value);

            reg2_imm_builder_t& op(op_code_t op_code);
        };

        class none_builder_t final {
            bb_builder_t*   _builder;
        public:
            none_builder_t(bb_builder_t* builder) : _builder(builder) {};

            bb_builder_t& build();

            none_builder_t& op(op_code_t op_code);
        };

        class imm1_builder_t final {
            bb_builder_t*   _builder;
        public:
            imm1_builder_t(bb_builder_t* builder) : _builder(builder) {};

            bb_builder_t& build();

            imm1_builder_t& value(s32 value);

            imm1_builder_t& value(u32 value);

            imm1_builder_t& value(bb_t* value);

            imm1_builder_t& is_signed(b8 flag);

            imm1_builder_t& op(op_code_t op_code);
        };

        class imm2_builder_t final {
            bb_builder_t*   _builder;
        public:
            imm2_builder_t(bb_builder_t* builder) : _builder(builder) {};

            bb_builder_t& build();

            imm2_builder_t& mode(b8 flag);

            imm2_builder_t& dst(reg_t reg);

            imm2_builder_t& dst(var_t* var);

            imm2_builder_t& src(s32 value);

            imm2_builder_t& src(u32 value);

            imm2_builder_t& src(bb_t* value);

            imm2_builder_t& is_signed(b8 flag);

            imm2_builder_t& op(op_code_t op_code);
        };

        class bb_builder_t final {
            bb_t*                   _bb;
            [[maybe_unused]] emitter_t*              _em;
            inst_t*                 _inst;
            imm1_builder_t          _imm1;
            imm2_builder_t          _imm2;
            reg1_builder_t          _reg1;
            reg2_builder_t          _reg2;
            none_builder_t          _none;
            offs_builder_t          _offs;
            reg3_builder_t          _reg3;
            reg2_imm_builder_t      _reg2_imm;

            friend class imm1_builder_t;
            friend class imm2_builder_t;
            friend class reg1_builder_t;
            friend class reg2_builder_t;
            friend class reg3_builder_t;
            friend class offs_builder_t;
            friend class none_builder_t;
            friend class reg2_imm_builder_t;

            operand_t operand(s32 value) {
                return operand_t{
                    .kind.s = value,
                    .type = operand_type_t::value
                };
            }

            operand_t operand(bb_t* bb) {
                return operand_t{
                    .kind.bb = bb,
                    .type = operand_type_t::block
                };
            }

            operand_t operand(bb_t& bb) {
                return operand_t{
                    .kind.bb = &bb,
                    .type = operand_type_t::block
                };
            }

            operand_t operand(reg_t reg) {
                return operand_t{
                    .kind.reg = reg,
                    .type = operand_type_t::reg
                };
            }

            operand_t operand(var_t* var) {
                return operand_t{
                    .kind.var = var,
                    .type = operand_type_t::var
                };
            }

            operand_t operand(u32 value,
                              operand_type_t type = operand_type_t::value) {
                operand_t oper{};
                oper.type = type;
                switch (oper.type) {
                    case operand_type_t::label:
                        oper.kind.label = value;
                        break;
                    case operand_type_t::trap:
                    case operand_type_t::value:
                        oper.kind.u = value;
                        break;
                    default:
                        oper.type = operand_type_t::value;
                        oper.kind.u = value;
                        break;
                }
                return oper;
            }

        public:
            bb_builder_t(bb_t* bb);

            bb_builder_t(bb_t& bb);

            bb_builder_t& label(label_t label) {
                _bb->label = label;
                hashtab::insert(_bb->emitter->labtab, label, _bb);
                return *this;
            }

            bb_builder_t& pred(bb_t& prev) {
                assert((!prev.next && !_bb->prev) && (_bb != &prev));
                prev.next = _bb;
                _bb->prev = &prev;
                return *this;
            }

            bb_builder_t& succ(bb_t& next) {
                assert((!next.prev && !_bb->next) && (_bb != &next));
                next.prev = _bb;
                _bb->next = &next;
                return *this;
            }

            template <String_Concept T>
            bb_builder_t& note(const T& value) {
                auto& strtab = _bb->emitter->strtab;
                str_array::append(strtab, value);
                array::append(_bb->notes, strtab.size);
                return *this;
            }

            template <String_Concept T>
            bb_builder_t& comment(const T& value, s32 line = -1) {
                auto& strtab = _bb->emitter->strtab;
                line = (line == -1 ? _bb->insts.size : line);
                auto notes = hashtab::find(_bb->comments, line);
                if (!notes) {
                    notes = hashtab::emplace(_bb->comments, line);
                    array::init(*notes, _bb->emitter->alloc);
                }
                str_array::append(strtab, value);
                array::append(*notes, strtab.size);
                return *this;
            }

            imm1_builder_t& imm1();

            imm2_builder_t& imm2();

            reg1_builder_t& reg1();

            reg2_builder_t& reg2();

            none_builder_t& none();

            offs_builder_t& offs();

            reg3_builder_t& reg3();

            reg2_imm_builder_t& reg2_imm();
        };
    }
}

FORMAT_TYPE(
    basecode::scm::operand_t,
    {
    switch (data.type) {
        case basecode::scm::operand_type_t::none:
            break;
        case basecode::scm::operand_type_t::label:
            format_to(
                ctx.out(),
                "{}",
                *basecode::string::interned::get_slice(data.kind.u));
            break;
        case basecode::scm::operand_type_t::value:
            format_to(ctx.out(), "{}", data.kind.s);
            break;
        case basecode::scm::operand_type_t::block:
            format_to(ctx.out(), "bb_{}", data.kind.bb->id);
            break;
        case basecode::scm::operand_type_t::reg:
            format_to(
                ctx.out(),
                "{}",
                basecode::scm::register_file::name(data.kind.reg));
            break;
        case basecode::scm::operand_type_t::var:
            format_to(ctx.out(), "{}", *data.kind.var);
            break;
        case basecode::scm::operand_type_t::trap:
            format_to(
                ctx.out(),
                "{} ({})",
                basecode::scm::trap::name(data.kind.u),
                data.kind.u);
            break;
        }
    });
