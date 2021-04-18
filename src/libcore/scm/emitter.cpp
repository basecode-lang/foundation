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

#include <basecode/core/scm/emitter.h>
#include <basecode/core/scm/register_pool.h>

#define ENCODE_REG(o, f)        SAFE_SCOPE(                                     \
    u8  __tmp{};                                                                \
    auto __status = encode_reg((o), __tmp);                                     \
    if (!OK(__status))                                                          \
        return __status;                                                        \
    (f) = __tmp;)
#define ENCODE_IMM(o, f)        SAFE_SCOPE(                                     \
    u32 __tmp{};                                                                \
    auto __status = encode_imm(e, addr, (o), __tmp);                            \
    if (!OK(__status))                                                          \
        return __status;                                                        \
    (f) = __tmp;)

namespace basecode::scm::emitter {
    static u0 swap_ranges(liveliness_range_t* lhs,
                          liveliness_range_t* rhs);

    static u0 set_interval(emitter_t& e,
                           u32 block_id,
                           liveliness_range_t* new_range);

    static u0 create_ranges(emitter_t& e, var_version_t* version);

    [[maybe_unused]] static u0 create_split_ranges(emitter_t& e,
                                                   var_version_t* version);

    u0 free(emitter_t& e) {
        for (auto version : e.versions)
            array::free(version->accesses);
        array::free(e.insts);
        symtab::free(e.vartab);
        array::free(e.comments);
        array::free(e.intervals);
        digraph::free(e.bb_graph);
        str_array::free(e.strtab);
        digraph::free(e.var_graph);
        stable_array::free(e.vars);
        stable_array::free(e.blocks);
        stable_array::free(e.ranges);
        stable_array::free(e.versions);
    }

    u0 reset(emitter_t& e) {
        for (auto version : e.versions)
            array::free(version->accesses);
        array::reset(e.insts);
        symtab::reset(e.vartab);
        array::reset(e.comments);
        array::reset(e.intervals);
        digraph::reset(e.bb_graph);
        str_array::reset(e.strtab);
        digraph::reset(e.var_graph);
        stable_array::reset(e.vars);
        stable_array::reset(e.blocks);
        stable_array::reset(e.ranges);
        stable_array::reset(e.versions);
    }

    static u0 format_comments(str_buf_t& buf,
                              u32 len,
                              const comment_array_t& comments,
                              const str_array_t& strings,
                              comment_type_t type,
                              u32 block_id,
                              u32 sidx,
                              u32 eidx,
                              s32 line,
                              u64 addr) {
        u32 j{};
        for (u32 i = sidx; i < eidx; ++i) {
            const auto& c = comments[i];
            if (c.type != type
                ||  c.block_id != block_id) {
                continue;
            }
            const auto& str = strings[c.id - 1];
            str::each_line(str, [&](str::slice_t this_line) -> b8 {
                switch (type) {
                    case comment_type_t::note:
                    case comment_type_t::line: {
                        format::format_to(buf,
                                          "${:08X}: ; {}\n",
                                          addr,
                                          this_line);
                        ++j;
                        break;
                    }
                    case comment_type_t::margin: {
                        if (c.line != line)
                            break;
                        if (j > 0) {
                            format::format_to(buf,
                                              "${:08X}:{:<{}}",
                                              addr,
                                              " ",
                                              54 - std::min<s32>(len, 54));
                        }
                        format::format_to(buf,
                                          "{:<{}}; {}\n",
                                          " ",
                                          62 - std::min<s32>(len, 62),
                                          this_line);
                        ++j;
                        break;
                    }
                }
                return true;
            });
        }
        if (!j && type != comment_type_t::note)
            format::format_to(buf, "\n");
    }

    status_t allocate_registers(emitter_t& e) {
        reg_pool_t       pool       {};
        interval_array_t active     {};
        interval_array_t spilled    {};

        reg_pool::init(pool,
                       vm::register_file::r0,
                       vm::register_file::r15);
        array::init(active, e.alloc);
        array::init(spilled, e.alloc);
        array::resize(active, pool.size);
        array::resize(spilled, pool.size);
        defer(array::free(active);
              array::free(spilled));

        for (u32 i = 0; i < e.blocks.size; ++i) {
            auto interval = e.intervals[i];
            if (!interval)
                continue;
            while (interval) {
                // if there are live intervals in active, we need to check
                // their expiry against our current *interval*.  if expired,
                // we remove them from active and release their register in
                // the pool.
                u64 mask = pool.slots;
                while (mask) {
                    const auto bit = __builtin_ffs(mask) - 1;
                    mask &= ~(1UL << bit);
                    auto a = active[bit];
                    if (!a || a->end > interval->start)
                        continue;
                    reg_pool::release(pool, a->reg);
                    active[bit] = {};
                }

                // if there are free registers in the pool, we assign one
                // to *interval* and move the *interval* to active.
                if (reg_pool::has_free(pool)) {
                    auto reg = reg_pool::retain(pool);
                    interval->reg          = reg;
                    interval->version->reg = reg;
                    active[reg - pool.start] = interval;
                } else {
                    // if there are *no* free registers in the pool, we
                    // spill the range with the largest end point amongst
                    // those in active (the range that lives the longest).
                    // if something from active is spilled, we reclaim
                    // the register and assign it to our current *interval*
                }
                interval = interval->next;
            }
        }

        return status_t::fail;
    }

    u0 init(emitter_t& e, vm_t* vm, alloc_t* alloc) {
        e.vm    = vm;
        e.alloc = alloc;
        array::init(e.insts, e.alloc);
        symtab::init(e.vartab, e.alloc);
        array::init(e.comments, e.alloc);
        array::init(e.intervals, e.alloc);
        digraph::init(e.bb_graph, e.alloc);
        str_array::init(e.strtab, e.alloc);
        digraph::init(e.var_graph, e.alloc);
        stable_array::init(e.vars, e.alloc);
        stable_array::init(e.blocks, e.alloc);
        stable_array::init(e.ranges, e.alloc);
        stable_array::init(e.versions, e.alloc);
    }

    u0 format_liveliness_intervals(emitter_t& e) {
        for (u32 i = 0; i < e.blocks.size; ++i) {
            format::print("block {:03}: ", i + 1);
            auto range = e.intervals[i];
            if (!range) {
                format::print("none\n");
                continue;
            }
            format::print("{}@{}({})=[{:>3}, {:>3}]",
                          range->version->var->symbol,
                          range->version->number,
                          vm::register_file::name(range->reg),
                          range->start,
                          range->end);
            range = range->next;
            while (range) {
                format::print(" -> {}@{}({})=[{:>3}, {:>3}]",
                              range->version->var->symbol,
                              range->version->number,
                              vm::register_file::name(range->reg),
                              range->start,
                              range->end);
                range = range->next;
            }
            format::print("\n");
        }
    }

    static u0 swap_ranges(liveliness_range_t* lhs,
                          liveliness_range_t* rhs) {
        if (lhs->next) {
            if (*rhs < *lhs->next) {
                rhs->next = lhs->next;
                lhs->next = rhs;
            } else {
                swap_ranges(lhs->next, rhs);
            }
        } else {
            lhs->next = rhs;
        }
    }

    status_t find_liveliness_intervals(emitter_t& e) {
        assoc_array_t<var_t*> pairs{};
        assoc_array::init(pairs, e.alloc);
        defer(assoc_array::free(pairs));
        symtab::find_prefix(e.vartab, pairs);
        array::resize(e.intervals, e.blocks.size);
        for (u32 i = 0; i < pairs.size; ++i)
            create_ranges(e, pairs[i].value->first);
        return status_t::ok;
    }

    static u0 set_interval(emitter_t& e,
                           u32 block_id,
                           liveliness_range_t* new_range) {
        auto range = e.intervals[block_id - 1];
        if (!range) {
            e.intervals[block_id - 1] = new_range;
        } else {
            if (*new_range < *range) {
                new_range->next = range;
                e.intervals[block_id - 1] = new_range;
            } else {
                swap_ranges(range, new_range);
            }
        }
    }

    status_t create_dot(emitter_t& e, const path_t& path) {
        using namespace graphviz;

        graph_t bb_sg{};
        graph::init(bb_sg,
                    graph_type_t::directed,
                    "bb"_ss);
        graph::label(bb_sg, "basic blocks"_ss);

        graph_t var_sg{};
        graph::init(var_sg,
                    graph_type_t::directed,
                    "vvar"_ss);
        graph::label(var_sg, "virtual variables"_ss);

        graph_t g{};
        graph::init(g,
                    graph_type_t::directed,
                    "compiler"_ss);
        graph::node_sep(g, 1);
        graph::label(g, "IR data structures"_ss);
        graph::add_cluster_subgraph(g, &bb_sg);
        graph::add_cluster_subgraph(g, &var_sg);
        defer(
            graph::free(g);
            graph::free(bb_sg);
            graph::free(var_sg));

        u32 block_node_ids[e.bb_graph.size];
        {
            bb_digraph_t::Node_Array nodes{};
            array::init(nodes, e.alloc);
            defer(array::free(nodes));

            for (auto bb_node : e.bb_graph.nodes) {
                auto block_node = graph::make_node(bb_sg);
                block_node_ids[bb_node->id - 1] = block_node->id;
                node::label(*block_node, format::format(
                    "block: {}",
                    *(bb_node->value)));
                node::shape(*block_node, shape_t::record);
                node::style(*block_node, node_style_t::filled);
                node::fill_color(*block_node, color_t::aliceblue);
            }

            for (auto bb_node : e.bb_graph.nodes) {
                array::reset(nodes);
                digraph::incoming_nodes(e.bb_graph, bb_node, nodes);
                for (auto incoming : nodes) {
                    auto edge = graph::make_edge(bb_sg);
                    edge->first  = graph::node_ref(
                        &bb_sg,
                        block_node_ids[incoming->id - 1]);
                    edge->second = graph::node_ref(
                        &bb_sg,
                        block_node_ids[bb_node->id - 1]);
                    edge::label(*edge, "pred"_ss);
                    edge::style(*edge, edge_style_t::dotted);
                    edge::dir(*edge, dir_type_t::back);
                }

                array::reset(nodes);
                digraph::outgoing_nodes(e.bb_graph, bb_node, nodes);
                for (auto outgoing : nodes) {
                    auto edge = graph::make_edge(bb_sg);
                    edge->first  = graph::node_ref(
                        &bb_sg,
                        block_node_ids[bb_node->id - 1]);
                    edge->second = graph::node_ref(
                        &bb_sg,
                        block_node_ids[outgoing->id - 1]);
                    edge::label(*edge, "succ"_ss);
                    edge::dir(*edge, dir_type_t::forward);
                }

                if (bb_node->value->next) {
                    auto straight_edge = graph::make_edge(bb_sg);
                    straight_edge->first = graph::node_ref(
                        &bb_sg,
                        block_node_ids[bb_node->id - 1]);
                    straight_edge->second = graph::node_ref(
                        &bb_sg,
                        block_node_ids[bb_node->value->next->node->id - 1]);
                    edge::label(*straight_edge, "next"_ss);
                    edge::dir(*straight_edge, dir_type_t::forward);
                    edge::color(*straight_edge, color_t::blue);
                    edge::style(*straight_edge, edge_style_t::dashed);
                }
            }
        }

        {
            u32 var_node_ids[e.var_graph.size];
            var_digraph_t::Node_Array nodes{};
            array::init(nodes, e.alloc);
            defer(array::free(nodes));

            str_t str{};
            str::init(str, e.alloc);
            for (auto vv_node : e.var_graph.nodes) {
                auto var_node = graph::make_node(var_sg);
                var_node_ids[vv_node->id - 1] = var_node->id;
                {
                    str::reset(str); {
                        str_buf_t buf{&str};
                        auto var = vv_node->value;
                        format::format_to(buf, "{}", *var);
                    }
                }
                node::label(*var_node, str);
                node::shape(*var_node, shape_t::component);
                node::style(*var_node, node_style_t::filled);
                node::fill_color(*var_node, color_t::lavender);
            }

            for (auto vv_node : e.var_graph.nodes) {
                array::reset(nodes);
                digraph::outgoing_nodes(e.var_graph, vv_node, nodes);
                for (auto outgoing : nodes) {
                    auto edge = graph::make_edge(var_sg);
                    edge->first  = graph::node_ref(
                        &var_sg,
                        var_node_ids[vv_node->id - 1]);
                    edge->second = graph::node_ref(
                        &var_sg,
                        var_node_ids[outgoing->id - 1]);
                    edge::label(*edge, "next"_ss);
                    edge::dir(*edge, dir_type_t::both);
                    edge::arrow_tail(*edge, arrow_type_t::dot);
                    edge::arrow_head(*edge, arrow_type_t::normal);
                }
                auto var = vv_node->value;
                for (const auto& ac : var->accesses) {
                    if (ac.type != var_access_type_t::def)
                        continue;
                    const auto& inst = e.insts[ac.inst_id];
                    auto edge = graph::make_edge(g);
                    edge->first  = graph::node_ref(
                        &bb_sg,
                        block_node_ids[inst.block_id - 1]);
                    edge->second = graph::node_ref(
                        &var_sg,
                        var_node_ids[vv_node->id - 1]);
                    edge::label(*edge, virtual_var::access_type::name(ac.type));
                    edge::color(*edge, color_t::darkgreen);
                    edge::dir(*edge, dir_type_t::forward);
                }
            }
        }


        buf_t buf{};
        buf.mode = buf_mode_t::alloc;
        buf::init(buf);
        defer(buf::free(buf));
        {
            auto status = graphviz::graph::serialize(g, buf);
            if (!OK(status))
                return status_t::fail;
        }
        {
            auto status = buf::save(buf, path);
            if (!OK(status))
                return status_t::fail;
        }

        return status_t::ok;
    }

    static u0 create_ranges(emitter_t& e, var_version_t* version) {
        if (version->accesses.size > 0) {
            auto start = array::front(version->accesses)->inst_id;
            auto end   = array::back(version->accesses)->inst_id;

            const auto& start_inst = e.insts[start - 1];
            auto new_range = &stable_array::append(e.ranges);
            new_range->id      = e.ranges.size;
            new_range->end     = end - 1;
            new_range->reg     = vm::register_file::none;
            new_range->next    = {};
            new_range->start   = start - 1;
            new_range->version = version;
            set_interval(e, start_inst.block_id, new_range);
        }

        var_digraph_t::Node_Array nodes{};
        array::init(nodes, e.alloc);
        defer(array::free(nodes));

        digraph::outgoing_nodes(e.var_graph, version->node, nodes);
        for (const auto& node : nodes)
            create_ranges(e, node->value);
    }

    static u0 create_split_ranges(emitter_t& e, var_version_t* version) {
        var_digraph_t::Node_Array nodes{};
        array::init(nodes, e.alloc);
        defer(array::free(nodes));

        u32 end     {};
        u32 start   {};
        u32 block_id{};
        for (const auto& ac : version->accesses) {
            const auto& inst = e.insts[ac.inst_id - 1];
            if (block_id && block_id != inst.block_id) {
                auto new_range = &stable_array::append(e.ranges);
                new_range->end     = end - 1;
                new_range->next    = {};
                new_range->start   = start - 1;
                new_range->version = version;
                set_interval(e, block_id, new_range);
                start    = inst.id;
                end      = start;
                block_id = inst.block_id;
            }
            if (!block_id)
                block_id = inst.block_id;
            if (!start) {
                start = inst.id;
                end   = start;
            } else {
                end = inst.id;
            }
        }
        if (start && end && block_id) {
            auto new_range = &stable_array::append(e.ranges);
            new_range->end     = end - 1;
            new_range->next    = {};
            new_range->start   = start - 1;
            new_range->version = version;
            set_interval(e, block_id, new_range);
        }

        digraph::outgoing_nodes(e.var_graph, version->node, nodes);
        for (const auto& node : nodes)
            create_split_ranges(e, node->value);
    }

    u32 assembled_size_bytes(emitter_t& e, bb_t& start_block) {
        auto curr = &start_block;
        u64  size = 0;
        while (curr) {
            size += curr->insts.size() * sizeof(encoded_inst_t);
            curr = curr->next;
        }
        return size;
    }

    inst_t* make_instruction(emitter_t& e, bb_t& bb, u8 encoding) {
        auto inst = &array::append(e.insts);
        inst->id        = e.insts.size;
        inst->aux       = 0;
        inst->pad       = 0;
        inst->mode      = false;
        inst->block_id  = bb.id;
        inst->encoding  = encoding;
        inst->is_signed = false;
        if (bb.insts.eidx == 0) {
            bb.insts.sidx = e.insts.size - 1;
            bb.insts.eidx = bb.insts.sidx + 1;
        } else {
            bb.insts.eidx = e.insts.size;
        }
        return inst;
    }

    status_t assemble(emitter_t& e, bb_t& start_block, u64* heap) {
        auto curr = &start_block;

        // assign block addresses
        u64 addr = u64(heap);
        while (curr) {
            curr->addr = addr;
            addr += curr->insts.size() * sizeof(encoded_inst_t);
            curr = curr->next;
        }

        // emit blocks to vm heap
        curr = &start_block;
        while (curr) {
            for (u32 i = curr->insts.sidx; i < curr->insts.eidx; ++i) {
                const auto& inst = e.insts[i];
                if (inst.block_id != curr->id)
                    continue;
                auto status = encode_inst(e, inst, heap);
                if (!OK(status))
                    return status;
                ++heap;
            }
            curr = curr->next;
        }
        return status_t::ok;
    }

    static u0 format_edges(str_buf_t& buf, bb_t* block, u64 addr) {
        bb_digraph_t::Node_Array nodes{};
        array::init(nodes, block->emit->alloc);
        defer(array::free(nodes));

        digraph::incoming_nodes(block->emit->bb_graph,
                                block->node,
                                nodes);
        if (nodes.size > 0) {
            format::format_to(buf,
                              "${:08X}:    .preds      ",
                              addr);
            for (u32 i = 0; i < nodes.size; ++i) {
                if (i > 0) format::format_to(buf, ", ");
                format::format_to(buf,
                                  "{}",
                                  *(nodes[i]->value));
            }
            format::format_to(buf, "\n");
        }

        array::reset(nodes);
        digraph::outgoing_nodes(block->emit->bb_graph,
                                block->node,
                                nodes);
        if (nodes.size > 0) {
            format::format_to(buf,
                              "${:08X}:    .succs      ",
                              addr);
            for (u32 i = 0; i < nodes.size; ++i) {
                if (i > 0) format::format_to(buf, ", ");
                format::format_to(buf,
                                  "{}",
                                  *(nodes[i]->value));
            }
            format::format_to(buf, "\n");
        }
    }

    static u0 format_params(str_buf_t& buf, bb_t* block, u64 addr) {
        if (block->params.size == 0)
            return;
        format::format_to(buf,
                          "${:08X}:    .params      ",
                          addr);
        for (u32 i = 0; i < block->params.size; ++i) {
            if (i > 0) format::format_to(buf, ", ");
            format::format_to(buf,
                              "{}",
                              *(block->params.vars[i]));
        }
        format::format_to(buf, "\n");
    }

    u0 disassemble(emitter_t& e, bb_t& start_block, str_buf_t& buf) {
        auto curr = &start_block;
        u64  addr = curr->addr;
        u32  line{};
        while (curr) {
            format_comments(buf,
                            0,
                            e.comments,
                            e.strtab,
                            comment_type_t::note,
                            curr->id,
                            curr->notes.sidx,
                            curr->notes.eidx,
                            -1,
                            addr);
            format::format_to(buf,
                              "${:08X}: {}:\n",
                              addr,
                              *curr);
            format_params(buf, curr, addr);
            format_edges(buf, curr, addr);
            for (s32 i = curr->insts.sidx; i < curr->insts.eidx; ++i) {
                const auto& inst = e.insts[i];
                if (inst.block_id != curr->id)
                    continue;
                auto start_pos = buf.size();
                format::format_to(
                    buf,
                    "${:08X}:    {:<12}",
                    addr,
                    vm::instruction::type::name(inst.type));
                switch (inst.encoding) {
                    case vm::instruction::encoding::imm: {
                        if (inst.mode
                        &&  inst.operands[1].type != operand_type_t::none) {
                            format::format_to(
                                buf,
                                "{}, ",
                                inst.operands[1]);
                            format::format_to(
                                buf,
                                "{}",
                                inst.operands[0]);
                        } else {
                            format::format_to(
                                buf,
                                "{}",
                                inst.operands[0]);
                            if (inst.operands[1].type != operand_type_t::none) {
                                format::format_to(
                                    buf,
                                    ", {}",
                                    inst.operands[1]);
                            }
                        }
                        break;
                    }
                    case vm::instruction::encoding::reg1: {
                        format::format_to(
                            buf,
                            "{}",
                            inst.operands[0]);
                        break;
                    }
                    case vm::instruction::encoding::reg2: {
                        format::format_to(
                            buf,
                            "{}, {}",
                            inst.operands[0],
                            inst.operands[1]);
                        break;
                    }
                    case vm::instruction::encoding::reg2_imm: {
                        format::format_to(
                            buf,
                            "{}, {}, {}",
                            inst.operands[0],
                            inst.operands[1],
                            inst.operands[2]);
                        break;
                    }
                    case vm::instruction::encoding::reg3: {
                        format::format_to(
                            buf,
                            "{}, {}, {}",
                            inst.operands[0],
                            inst.operands[1],
                            inst.operands[2]);
                        break;
                    }
                    case vm::instruction::encoding::reg4: {
                        format::format_to(
                            buf,
                            "{}, {}, {}, {}",
                            inst.operands[0],
                            inst.operands[1],
                            inst.operands[2],
                            inst.operands[3]);
                        break;
                    }
                    case vm::instruction::encoding::offset: {
                        if (inst.mode) {
                            format::format_to(
                                buf,
                                "{}, {}({})",
                                inst.operands[0],
                                inst.operands[2],
                                inst.operands[1]);
                        } else {
                            format::format_to(
                                buf,
                                "{}({}), {}",
                                inst.operands[2],
                                inst.operands[0],
                                inst.operands[1]);
                        }
                        break;
                    }
                    case vm::instruction::encoding::indexed: {
                        format::format_to(
                            buf,
                            "{}({}, {}), {}",
                            inst.operands[3],
                            inst.operands[0],
                            inst.operands[1],
                            inst.operands[2]);
                        break;
                    }
                    default: {
                        break;
                    }
                }
                format_comments(buf,
                                buf.size() - start_pos,
                                e.comments,
                                e.strtab,
                                comment_type_t::margin,
                                curr->id,
                                curr->notes.sidx,
                                curr->notes.eidx,
                                line,
                                addr);
                addr += sizeof(encoded_inst_t);
                ++line;
            }
            line = {};
            curr = curr->next;
        }
    }

    status_t encode_inst(emitter_t& e, const inst_t& inst, u64* heap) {
        u64 buf{};
        u64 data{};
        auto& vm = *e.vm;
        const u64 addr    = u64(heap);
        auto      encoded = (encoded_inst_t*) &buf;
        auto      opers   = (encoded_operand_t*) &data;
        encoded->type      = inst.type;
        encoded->is_signed = inst.is_signed;
        switch (inst.encoding) {
            case vm::instruction::encoding::none: {
                encoded->data     = 0;
                encoded->encoding = vm::instruction::encoding::none;
                break;
            }
            case vm::instruction::encoding::imm: {
                encoded->encoding  = vm::instruction::encoding::imm;
                auto& src = inst.operands[0];
                auto& dst = inst.operands[1];
                ENCODE_IMM(src, opers->imm.src);
                if (dst.type == operand_type_t::reg) {
                    ENCODE_REG(dst, opers->imm.dst);
                    auto area = vm::get_mem_area_by_reg(vm, opers->imm.dst);
                    if (area) {
                        opers->imm.aux = area->top ? -sizeof(u64) : sizeof(u64);
                    } else {
                        opers->imm.aux = 0;
                    }
                } else {
                    opers->imm.dst = 0;
                }
                break;
            }
            case vm::instruction::encoding::offset: {
                encoded->encoding = vm::instruction::encoding::offset;
                auto& src  = inst.operands[0];
                auto& dst  = inst.operands[1];
                auto& offs = inst.operands[2];
                ENCODE_REG(src, opers->offset.src);
                ENCODE_REG(dst, opers->offset.dst);
                opers->offset.pad  = 0;
                opers->offset.offs = offs.kind.s;
                break;
            }
            case vm::instruction::encoding::indexed: {
                encoded->encoding = vm::instruction::encoding::indexed;
                opers->indexed.pad  = 0;
                auto& offs = inst.operands[3];
                auto& base = inst.operands[0];
                auto& ndx  = inst.operands[1];
                auto& dst  = inst.operands[2];
                ENCODE_REG(base, opers->indexed.base);
                ENCODE_REG(ndx,  opers->indexed.ndx);
                ENCODE_REG(dst,  opers->indexed.dst);
                ENCODE_IMM(offs, opers->indexed.offs);
                break;
            }
            case vm::instruction::encoding::reg1: {
                encoded->encoding = vm::instruction::encoding::reg1;
                opers->reg1.pad   = 0;
                auto& dst = inst.operands[0];
                ENCODE_REG(dst, opers->reg1.dst);
                break;
            }
            case vm::instruction::encoding::reg2: {
                encoded->encoding = vm::instruction::encoding::reg2;
                opers->reg2.pad   = 0;
                auto& src = inst.operands[0];
                auto& dst = inst.operands[1];
                ENCODE_REG(src, opers->reg2.src);
                ENCODE_REG(dst, opers->reg2.dst);
                auto area = vm::get_mem_area_by_reg(vm, opers->reg2.src);
                if (area && !inst.aux) {
                    opers->reg2.aux = area->top ? -sizeof(u64) : sizeof(u64);
                } else {
                    opers->reg2.aux = inst.aux;
                }
                break;
            }
            case vm::instruction::encoding::reg2_imm: {
                encoded->encoding = vm::instruction::encoding::reg2_imm;
                auto& src = inst.operands[0];
                auto& dst = inst.operands[1];
                auto& imm = inst.operands[2];
                ENCODE_REG(src, opers->reg2_imm.a);
                ENCODE_REG(dst, opers->reg2_imm.b);
                ENCODE_IMM(imm, opers->reg2_imm.imm);
                break;
            }
            case vm::instruction::encoding::reg3: {
                encoded->encoding = vm::instruction::encoding::reg3;
                opers->reg3.pad   = 0;
                auto& a = inst.operands[0];
                auto& b = inst.operands[1];
                auto& c = inst.operands[2];
                ENCODE_REG(a, opers->reg3.a);
                ENCODE_REG(b, opers->reg3.b);
                ENCODE_REG(c, opers->reg3.c);
                break;
            }
            default:
                return status_t::fail;
        }
        encoded->data = data;
        *heap         = buf;
        return status_t::ok;
    }
}

