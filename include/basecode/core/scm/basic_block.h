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

namespace basecode::scm::basic_block {
    class bb_builder_t;

    class reg1_builder_t final {
        var_version_t**         _dst;
        bb_builder_t*           _builder;
    public:
        reg1_builder_t(bb_builder_t* builder) : _dst(),
                                                _builder(builder) {};

        u0 reset() { _dst = {}; }

        bb_builder_t& build();

        reg1_builder_t& dst(reg_t reg);

        reg1_builder_t& op(op_code_t op_code);

        reg1_builder_t& dst(var_version_t** var);
    };

    class reg2_builder_t final {
        var_version_t**         _src;
        var_version_t**         _dst;
        bb_builder_t*           _builder;

    public:
        reg2_builder_t(bb_builder_t* builder) : _src(),
                                                _dst(),
                                                _builder(builder) {};

        u0 reset() { _src = _dst = {}; }

        bb_builder_t& build();

        reg2_builder_t& aux(s8 value);

        reg2_builder_t& dst(reg_t reg);

        reg2_builder_t& src(reg_t reg);

        reg2_builder_t& is_signed(b8 flag);

        reg2_builder_t& op(op_code_t op_code);

        reg2_builder_t& dst(var_version_t** var);

        reg2_builder_t& src(var_version_t** var);
    };

    class reg3_builder_t final {
        var_version_t**         _a;
        var_version_t**         _b;
        var_version_t**         _c;
        bb_builder_t*           _builder;
    public:
        reg3_builder_t(bb_builder_t* builder) : _a(),
                                                _b(),
                                                _c(),
                                                _builder(builder) {};

        u0 reset() { _a = _b = _c = {}; }

        bb_builder_t& build();

        reg3_builder_t& a(reg_t reg);

        reg3_builder_t& b(reg_t reg);

        reg3_builder_t& c(reg_t reg);

        reg3_builder_t& op(op_code_t op_code);

        reg3_builder_t& a(var_version_t** var);

        reg3_builder_t& b(var_version_t** var);

        reg3_builder_t& c(var_version_t** var);
    };

    class offs_builder_t final {
        var_version_t**         _src;
        var_version_t**         _dst;
        bb_builder_t*           _builder;
    public:
        offs_builder_t(bb_builder_t* builder) : _src(),
                                                _dst(),
                                                _builder(builder) {};

        u0 reset() { _src = _dst = {}; }

        bb_builder_t& build();

        offs_builder_t& mode(b8 flag);

        offs_builder_t& dst(reg_t reg);

        offs_builder_t& src(reg_t reg);

        offs_builder_t& offset(s32 value);

        offs_builder_t& op(op_code_t op_code);

        offs_builder_t& dst(var_version_t** var);

        offs_builder_t& src(var_version_t** var);
    };

    class reg2_imm_builder_t final {
        var_version_t**         _src;
        var_version_t**         _dst;
        bb_builder_t*           _builder;
    public:
        reg2_imm_builder_t(bb_builder_t* builder) : _src(),
                                                    _dst(),
                                                    _builder(builder) {};

        u0 reset() { _src = _dst = {}; }

        bb_builder_t& build();

        reg2_imm_builder_t& dst(reg_t reg);

        reg2_imm_builder_t& src(reg_t reg);

        reg2_imm_builder_t& value(s32 value);

        reg2_imm_builder_t& value(u32 value);

        reg2_imm_builder_t& is_signed(b8 flag);

        reg2_imm_builder_t& value(bb_t* value);

        reg2_imm_builder_t& op(op_code_t op_code);

        reg2_imm_builder_t& dst(var_version_t** var);

        reg2_imm_builder_t& src(var_version_t** var);
    };

    class none_builder_t final {
        bb_builder_t*   _builder;
    public:
        none_builder_t(bb_builder_t* builder) : _builder(builder) {};

        u0 reset() {}

        bb_builder_t& build();

        none_builder_t& op(op_code_t op_code);
    };

    class imm1_builder_t final {
        bb_builder_t*   _builder;
    public:
        imm1_builder_t(bb_builder_t* builder) : _builder(builder) {};

        u0 reset() {}

        bb_builder_t& build();

        imm1_builder_t& value(s32 value);

        imm1_builder_t& value(u32 value);

        imm1_builder_t& value(bb_t* value);

        imm1_builder_t& is_signed(b8 flag);

        imm1_builder_t& op(op_code_t op_code);
    };

    class imm2_builder_t final {
        var_version_t**         _dst;
        bb_builder_t*           _builder;
    public:
        imm2_builder_t(bb_builder_t* builder) : _dst(),
                                                _builder(builder) {};

        u0 reset() { _dst = {}; }

        bb_builder_t& build();

        imm2_builder_t& mode(b8 flag);

        imm2_builder_t& dst(reg_t reg);

        imm2_builder_t& src(s32 value);

        imm2_builder_t& src(u32 value);

        imm2_builder_t& src(bb_t* value);

        imm2_builder_t& is_signed(b8 flag);

        imm2_builder_t& op(op_code_t op_code);

        imm2_builder_t& dst(var_version_t** var);
    };

    class bb_builder_t final {
        bb_t*                   _bb;
        emitter_t*              _em;
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

        operand_t operand(bb_t* bb) {
            return operand_t{
                .kind.branch.bb = bb,
                .type = operand_type_t::block
            };
        }

        operand_t operand(bb_t& bb) {
            return operand_t{
                .kind.branch.bb = &bb,
                .type = operand_type_t::block
            };
        }

        operand_t operand(s32 value) {
            return operand_t{
                .kind.s = value,
                .type = operand_type_t::value
            };
        }

        operand_t operand(reg_t reg) {
            return operand_t{
                .kind.reg = reg,
                .type = operand_type_t::reg
            };
        }

        operand_t operand(var_version_t* var) {
            return operand_t{
                .kind.var = var,
                .type = operand_type_t::var
            };
        }

        directive_t& make_directive(directive_type_t type) {
            auto& directive = array::append(_em->directives);
            directive.type     = type;
            directive.line     = _em->insts.size;
            directive.block_id = _bb->id;
            if (_bb->directives.eidx == 0) {
                _bb->directives.sidx = _em->directives.size - 1;
                _bb->directives.eidx = _bb->directives.sidx + 1;
            } else {
                _bb->directives.eidx = _em->directives.size;
            }
            return directive;
        }

        operand_t operand(u32 value,
                          operand_type_t type = operand_type_t::value) {
            operand_t oper{};
            oper.type = type;
            switch (oper.type) {
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

        bb_builder_t& code() {
            make_directive(directive_type_t::code);
            return *this;
        }

        bb_builder_t& next(bb_t& next) {
            next.prev = _bb;
            _bb->next = &next;
            digraph::make_edge(_em->bb_graph, _bb->node, next.node);
            return *this;
        }

        template <String_Concept T>
        bb_builder_t& note(const T& value) {
            auto& strtab = _bb->emit->strtab;
            str_array::append(strtab, value);
            auto& c = array::append(_em->comments);
            c.id       = strtab.size;
            c.line     = 0;
            c.type     = comment_type_t::note;
            c.block_id = _bb->id;
            if (_bb->notes.eidx == 0) {
                _bb->notes.sidx = _em->comments.size - 1;
                _bb->notes.eidx = _bb->notes.sidx + 1;
            } else {
                _bb->notes.eidx = _em->comments.size;
            }
            return *this;
        }

        bb_builder_t& area(mem_area_type_t area) {
            auto& directive = make_directive(directive_type_t::area);
            directive.kind.area = area;
            return *this;
        }

        bb_builder_t& db(std::initializer_list<u8> bytes) {
            auto& directive = make_directive(directive_type_t::db);
            array::init(directive.kind.data, _em->alloc);
            for (auto byte : bytes)
                array::append(directive.kind.data, byte);
            return *this;
        }

        bb_builder_t& dw(std::initializer_list<u16> words) {
            auto& directive = make_directive(directive_type_t::db);
            array::init(directive.kind.data, _em->alloc);
            for (auto word : words)
                array::append(directive.kind.data, word);
            return *this;
        }

        bb_builder_t& dd(std::initializer_list<u32> dwords) {
            auto& directive = make_directive(directive_type_t::dd);
            array::init(directive.kind.data, _em->alloc);
            for (auto dword : dwords)
                array::append(directive.kind.data, dword);
            return *this;
        }

        bb_builder_t& dq(std::initializer_list<u64> qwords) {
            auto& directive = make_directive(directive_type_t::dq);
            array::init(directive.kind.data, _em->alloc);
            for (auto qword : qwords)
                array::append(directive.kind.data, qword);
            return *this;
        }

        template <String_Concept T>
        bb_builder_t& comment(const T& value, s32 line = -1) {
            auto& strtab = _em->strtab;
            line = (line == -1 ?
                    std::max<u32>(_bb->insts.eidx - _bb->insts.sidx, 0) : line);
            str_array::append(strtab, value);
            auto& c = array::append(_em->comments);
            c.id       = strtab.size;
            c.line     = line;
            c.type     = comment_type_t::margin;
            c.block_id = _bb->id;
            if (_bb->notes.eidx == 0) {
                _bb->notes.sidx = _em->comments.size - 1;
                _bb->notes.eidx = _bb->notes.sidx + 1;
            } else {
                _bb->notes.eidx = _em->comments.size;
            }
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

    bb_builder_t encode(bb_t* bb);

    bb_builder_t encode(bb_t& bb);
}
