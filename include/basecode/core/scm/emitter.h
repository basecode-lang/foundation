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

        inline var_version_t* read(emitter_t& e,
                                   var_version_t* curr,
                                   u32 inst_id) {
            assert(curr && "virtual_var: cannot read a null var_version_t!");
            if (curr->accesses.size == 0) {
                auto& ac = array::append(curr->accesses);
                ac.type    = var_access_type_t::def;
                ac.inst_id = inst_id;
            }
            auto& ac = array::append(curr->accesses);
            ac.type    = var_access_type_t::read;
            ac.inst_id = inst_id;
            return curr;
        }

        inline var_version_t* write(emitter_t& e,
                                    var_version_t* curr,
                                    u32 inst_id) {
            auto var = curr->var;

            if (curr->accesses.size == 0) {
                auto& ac = array::append(curr->accesses);
                ac.type    = var_access_type_t::def;
                ac.inst_id = inst_id;
            }

            auto new_version = &stable_array::append(e.versions);
            array::init(new_version->accesses, e.alloc);
            var->current = new_version;
            new_version->var    = var;
            new_version->reg    = vm::register_file::none;
            new_version->node   = digraph::make_node(e.var_graph, *new_version);
            new_version->number = curr->number + 1;

            auto& ac = array::append(new_version->accesses);
            ac.type    = var_access_type_t::write;
            ac.inst_id = inst_id;
            digraph::make_edge(e.var_graph, curr->node, new_version->node);
            return new_version;
        }

        template <String_Concept T>
        var_version_t* get(emitter_t& e, const T& name) {
            var_t* var;
            symtab::find(e.vartab, name, var);
            return var ? var->current : nullptr;
        }

        template <String_Concept T>
        var_version_t* declare(emitter_t& e, const T& name) {
            var_t* var{};
            if (symtab::find(e.vartab, name, var))
                assert(false && "virtual_var: ssa var can only be declared once!");
            var = &stable_array::append(e.vars);
            var->symbol  = name;
            auto first = &stable_array::append(e.versions);
            array::init(first->accesses, e.alloc);
            first->var    = var;
            first->reg    = vm::register_file::none;
            first->node   = digraph::make_node(e.var_graph, *first);
            first->number = 1;
            var->first    = var->current = first;
            symtab::insert(e.vartab, name, var);
            return var->first;
        }

        inline u0 format_to(fmt_ctx_t& ctx, const var_version_t* version) {
            auto buf = ctx.out();
            if (version->reg != vm::register_file::none) {
                fmt::format_to(buf,
                               "{} <",
                               vm::register_file::name(version->reg));
            }
            fmt::format_to(buf, "{}", version->var->symbol);
            fmt::format_to(buf, "@{}", version->number);
            if (version->reg != vm::register_file::none)
                fmt::format_to(buf, ">");
        }
    }
}

FORMAT_TYPE(basecode::scm::var_version_t,
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
