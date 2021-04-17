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

namespace basecode::scm::emitter {
    u0 init(emitter_t& e,
            vm_t* vm,
            alloc_t* alloc = context::top()->alloc);

    u0 free(emitter_t& e);

    u0 reset(emitter_t& e);

    template <typename T>
    status_t encode_imm(emitter_t& e,
                        u64 addr,
                        const operand_t& oper,
                        T& field) {
        switch (oper.type) {
            case operand_type_t::trap:
            case operand_type_t::value: {
                field = oper.kind.s;
                break;
            }
            case operand_type_t::block: {
                auto target_block = &e.blocks[oper.kind.branch.bb->id - 1];
                field = s32(target_block->addr - addr);
                break;
            }
            default:
                return status_t::fail;
        }
        return status_t::ok;
    }

    status_t allocate_registers(emitter_t& e);

    u0 format_liveliness_intervals(emitter_t& e);

    status_t find_liveliness_intervals(emitter_t& e);

    status_t create_dot(emitter_t& e, const path_t& path);

    template <String_Concept T>
    bb_t& make_basic_block(emitter_t& e,
                           const T& name,
                           bb_t* prev,
                           bb_type_t type = bb_type_t::code) {
        str_array::append(e.strtab, name);
        auto& bb = stable_array::append(e.blocks);
        bb.id     = e.blocks.size;
        bb.node   = digraph::make_node(e.bb_graph, bb);
        bb.prev   = {};
        bb.next   = {};
        bb.addr   = 0;
        bb.emit   = &e;
        bb.type   = type;
        bb.notes  = {};
        bb.insts  = {};
        bb.params = {};
        bb.str_id = e.strtab.size;
        if (prev) {
            prev->next = &bb;
            bb.prev    = prev;
            digraph::make_edge(e.bb_graph, prev->node, bb.node);
        }
        return bb;
    }

    u32 assembled_size_bytes(emitter_t& e, bb_t& start_block);

    inst_t* make_instruction(emitter_t& e, bb_t& bb, u8 encoding);

    status_t assemble(emitter_t& e, bb_t& start_block, u64* heap);

    inline status_t encode_reg(const operand_t& oper, u8& field) {
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

    u0 disassemble(emitter_t& e, bb_t& start_block, str_buf_t& buf);

    status_t encode_inst(emitter_t& e, const inst_t& inst, u64* heap);

    namespace virtual_var {
        namespace access_type {
            inline str::slice_t name(var_access_type_t type) {
                switch (type) {
                    case var_access_type_t::def:    return "def"_ss;
                    case var_access_type_t::none:   return "none"_ss;
                    case var_access_type_t::read:   return "read"_ss;
                    case var_access_type_t::write:  return "write"_ss;
                }
            }
        }

        template <String_Concept T>
        var_t* get(emitter_t& e, const T& name) {
            var_t* var{};
            if (!symtab::find(e.vartab, name, var))
                return nullptr;
            return var;
        }

        template <String_Concept T>
        var_t* declare(emitter_t& e, const T& name) {
            var_t* var{};
            if (symtab::find(e.vartab, name, var))
                assert(false && "virtual_var: ssa var can only be declared once!");
            auto rc = string::interned::fold_for_result(name);
            if (!OK(rc.status))
                return nullptr;
            var = &stable_array::append(e.vars);
            array::init(var->accesses, e.alloc);
            var->node     = digraph::make_node(e.var_graph, *var);
            var->active   = false;
            var->symbol   = rc.id;
            var->version  = 1;
            var->spilled  = false;
            var->incubate = true;
            var->reg      = vm::register_file::none;
            symtab::insert(e.vartab, name, var);
            return var;
        }

        inline u0 format_to(fmt_ctx_t& ctx, const var_t* var) {
            auto buf = ctx.out();
            if (var->reg != vm::register_file::none) {
                fmt::format_to(buf,
                               "{} <",
                               vm::register_file::name(var->reg));
            }
            fmt::format_to(buf,
                           "{}",
                           *string::interned::get_slice(var->symbol));
            if (!var->incubate)
                fmt::format_to(buf, "@{}", var->version);
            if (var->reg != vm::register_file::none)
                fmt::format_to(buf, ">");
        }

        inline var_t* latest(emitter_t& e, intern_id symbol) {
            auto key = *string::interned::get_slice(symbol);
            var_t* var{};
            if (!symtab::find(e.vartab, key, var))
                assert(false && "virtual_var: latest should always find something!");
            return var;
        }

        inline var_t* read(emitter_t& e, var_t* var, u32 inst_id) {
            assert(var && "virtual_var: cannot read a null var_t!");
            assert(!var->incubate && "virtual_var: cannot read an incubated var_t!");
            auto& ac = array::append(var->accesses);
            ac.type    = var_access_type_t::read;
            ac.inst_id = inst_id;
            return var;
        }

        template <String_Concept T>
        var_t* read(emitter_t& e, const T& name, u32 inst_id) {
            var_t* var{};
            if (!symtab::find(e.vartab, name, var))
                assert(false && "virtual_var: cannot read an undeclared var_t!");
            return read(e, var, inst_id);
        }

        inline var_t* write(emitter_t& e, var_t* prev, u32 inst_id) {
            const auto curr = latest(e, prev->symbol);
            auto var = &stable_array::append(e.vars);
            array::init(var->accesses, e.alloc);
            if (prev->incubate) {
                if (curr->incubate) {
                    auto& ac      = array::append(var->accesses);
                    ac.type       = var_access_type_t::def;
                    ac.inst_id    = inst_id;
                    var->version  = prev->version;
                } else {
                    prev = curr;
                }
            }
            if (!prev->incubate) {
                var->version = curr->version + 1;
                auto& ac = array::append(var->accesses);
                ac.type    = var_access_type_t::write;
                ac.inst_id = inst_id;
            }
            var->symbol  = prev->symbol;
            var->active  = prev->active;
            var->spilled = prev->spilled;
            var->incubate = false;
            symtab::set(e.vartab,
                        *string::interned::get_slice(var->symbol),
                        var);
            var->node = digraph::make_node(e.var_graph, *var);
            digraph::make_edge(e.var_graph, prev->node, var->node);
            return var;
        }

        template <String_Concept T>
        var_t* write(emitter_t& e, const T& name, u32 inst_id) {
            var_t* var{};
            if (!symtab::find(e.vartab, name, var))
                assert(false && "virtual_var: cannot write an undeclared var_t!");
            return write(e, var, inst_id);
        }
    }
}

FORMAT_TYPE(basecode::scm::var_t,
            basecode::scm::emitter::virtual_var::format_to(ctx, &data));

FORMAT_TYPE(
    basecode::scm::operand_t,
    {
        switch (data.type) {
            case basecode::scm::operand_type_t::none:
                break;
            case basecode::scm::operand_type_t::value:
                format_to(ctx.out(), "{}", data.kind.s);
                break;
            case basecode::scm::operand_type_t::block:
                format_to(ctx.out(), "{}", *data.kind.branch.bb);
                break;
            case basecode::scm::operand_type_t::reg:
                format_to(
                    ctx.out(),
                    "{}",
                    basecode::scm::vm::register_file::name(data.kind.reg));
                break;
            case basecode::scm::operand_type_t::var:
                if (data.kind.var)
                    format_to(ctx.out(), "{}", *data.kind.var);
                else
                    format_to(ctx.out(), "__nil__@0");
                break;
            case basecode::scm::operand_type_t::trap:
                format_to(
                    ctx.out(),
                    "{} ({})",
                    basecode::scm::vm::trap::name(data.kind.u),
                    data.kind.u);
                break;
        }
    });
