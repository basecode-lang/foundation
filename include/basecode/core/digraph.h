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

#include <basecode/core/array.h>
#include <basecode/core/stack.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode {
    template <typename V>
    struct digraph_t final {
        struct node_t;
        struct edge_t;
        struct edge_pair_t;

        using Node              = node_t;
        using Edge              = edge_t;
        using Edge_Pair         = edge_pair_t;
        using Edge_Pair_Array   = array_t<Edge_Pair>;
        using Edge_Set          = array_t<Edge_Pair_Array>;
        using Value_Type        = V;
        using Node_Array        = array_t<Node*>;
        using Edge_Array        = array_t<Edge*>;
        using Node_Stack        = stack_t<Node*>;
        using Component_Array   = array_t<array_t<Node*>>;

        struct edge_pair_t final {
            const Edge*         edge;
            const Node*         node;
        };

        struct node_t final {
            Value_Type*         value;
            u32                 id;
            s32                 index;
            s32                 low_link;
            b8                  on_stack;

            inline b8 operator==(const node_t& rhs) const {
                return id == rhs.id;
            }
        };

        struct edge_t final {
            const Node*         src;
            const Node*         dst;
            s64                 wgt;
            s32                 type;

            inline b8 operator==(const edge_t& rhs) const {
                return src == rhs.src
                       && dst == rhs.dst
                       && type == rhs.type;
            }

            inline auto operator<=>(const edge_t& rhs) const {
                const auto eq = src == rhs.src
                                && dst == rhs.dst
                                && type == rhs.type;
                if (!eq)
                    return std::strong_ordering::less;
                return wgt <=> rhs.wgt;
            }
        };

        static constexpr u32    Node_Size    = sizeof(node_t);
        static constexpr u32    Edge_Size    = sizeof(edge_t);
        static constexpr u32    Node_Align   = alignof(node_t);
        static constexpr u32    Edge_Align   = alignof(edge_t);

        alloc_t*                alloc;
        alloc_t*                node_slab;
        alloc_t*                edge_slab;
        Node_Array              nodes;
        Edge_Array              edges;
        Edge_Set                outgoing;
        Edge_Set                incoming;
        u32                     size;
        u32                     id;
    };
    static_assert(sizeof(digraph_t<s32>) <= 128,
                  "digraph_t<T> is now larger than 128 bytes!");

    namespace digraph {
        enum class status_t {
            ok
        };

        template <Directed_Graph T,
                  typename Node = typename T::Node,
                  typename Edge_Pair_Array = typename T::Edge_Pair_Array>
        const Edge_Pair_Array& incoming_nodes(T& graph, Node* node);

        template <Directed_Graph T,
                  typename Node = typename T::Node,
                  typename Edge_Pair_Array = typename T::Edge_Pair_Array>
        const Edge_Pair_Array& outgoing_nodes(T& graph, Node* node);

        template <Directed_Graph T>
        u0 free(T& graph) {
            for (auto& nodes : graph.outgoing)
                array::free(nodes);
            for (auto& nodes : graph.incoming)
                array::free(nodes);
            array::free(graph.outgoing);
            array::free(graph.incoming);
            array::free(graph.edges);
            array::free(graph.nodes);
            memory::system::free(graph.edge_slab);
            memory::system::free(graph.node_slab);
            graph.id = graph.size = {};
        }

        template <Directed_Graph T>
        u0 reset(T& graph) {
            for (auto& nodes : graph.outgoing)
                array::free(nodes);
            for (auto& nodes : graph.incoming)
                array::free(nodes);
            array::reset(graph.outgoing);
            array::reset(graph.incoming);
            array::reset(graph.edges);
            array::reset(graph.nodes);
            memory::slab::reset(graph.edge_slab);
            memory::slab::reset(graph.node_slab);
            graph.id = graph.size = {};
        }

        template <Directed_Graph T,
                  typename Edge = typename T::Edge,
                  typename Node = typename T::Node>
        const Edge* make_edge(T& graph,
                              const Node* src,
                              const Node* dst,
                              s32 type = 0,
                              s64 wgt = 0) {
            if (src->id > graph.outgoing.size)
                array::resize(graph.outgoing, src->id);
            if (dst->id > graph.incoming.size)
                array::resize(graph.incoming, dst->id);
            auto& outgoing = graph.outgoing[src->id - 1];
            if (!outgoing.alloc)
                array::init(outgoing, graph.alloc);
            auto& incoming = graph.incoming[dst->id - 1];
            if (!incoming.alloc)
                array::init(incoming, graph.alloc);

            for (auto& pair : outgoing) {
                if (pair.node == dst
                &&  pair.edge->wgt == wgt
                &&  pair.edge->type == type) {
                    return pair.edge;
                }
            }

            auto edge = (Edge*) memory::alloc(graph.edge_slab);
            edge->src  = src;
            edge->dst  = dst;
            edge->wgt  = wgt;
            edge->type = type;
            array::append(graph.edges, edge);
            auto& og_pair = array::append(outgoing);
            og_pair.edge = edge;
            og_pair.node = dst;
            auto& ic_pair = array::append(incoming);
            ic_pair.edge = edge;
            ic_pair.node = src;
            return edge;
        }

        template <Directed_Graph T,
                  typename Edge = typename T::Edge,
                  typename Node = typename T::Node,
                  typename Node_Stack = typename T::Node_Stack,
                  typename Component_Array = typename T::Component_Array>
        u0 strongly_connected(T& graph,
                              Component_Array& comps,
                              Node_Stack& stack,
                              u32& index,
                              Node* v) {
            v->index    = index;
            v->low_link = index++;
            stack::push(stack, v);
            v->on_stack = true;

            const auto& succs = outgoing_nodes(graph, v);
            for (const auto& w : succs) {
                if (w.node->index == -1) {
                    strongly_connected(graph,
                                       comps,
                                       stack,
                                       index,
                                       const_cast<Node*>(w.node));
                    v->low_link = std::min(v->low_link, w.node->low_link);
                } else if (w.node->on_stack) {
                    v->low_link = std::min(v->low_link, w.node->index);
                }
            }

            if (v->low_link == v->index) {
                if (stack::empty(stack))
                    return;
                auto& comp = array::append(comps);
                array::init(comp, graph.alloc);
                array::reserve(comp, stack.size);
                Node* w{};
                while (w != v) {
                    w = stack::pop(stack);
                    w->on_stack = false;
                    array::append(comp, w);
                }
            }
        }

        template <Directed_Graph T,
                  typename Edge = typename T::Edge,
                  typename Node = typename T::Node,
                  typename Node_Stack = typename T::Node_Stack,
                  typename Component_Array = typename T::Component_Array>
        Component_Array strongly_connected(T& graph) {
            Node_Stack stack{};
            stack::init(stack, graph.alloc);

            Component_Array comps{};
            array::init(comps, graph.alloc);

            u32 index{};
            for (auto n : graph.nodes) {
                if (n->index == -1)
                    strongly_connected(graph, comps, stack, index, n);
            }

            stack::free(stack);
            return comps;
        }

        template <Directed_Graph T,
                  typename Node = typename T::Node,
                  typename Value_Type = typename T::Value_Type>
        Node* make_node(T& graph, const Value_Type& value) {
            auto node = (Node*) memory::alloc(graph.node_slab);
            node->id       = ++graph.id;
            node->index    = node->low_link = -1;
            node->value    = const_cast<Value_Type*>(&value);
            node->on_stack = false;
            array::append(graph.nodes, node);
            ++graph.size;
            return node;
        }

        template <Directed_Graph T,
                  typename Node,
                  typename Edge_Pair_Array>
        const Edge_Pair_Array& outgoing_nodes(T& graph, Node* node) {
            if (node->id > graph.outgoing.size)
                array::resize(graph.outgoing, node->id);
            auto& outgoing = graph.outgoing[node->id - 1];
            if (!outgoing.alloc)
                array::init(outgoing, graph.alloc);
            return outgoing;
        }

        template <Directed_Graph T,
                  typename Node,
                  typename Edge_Pair_Array>
        const Edge_Pair_Array& incoming_nodes(T& graph, Node* node) {
            if (node->id > graph.incoming.size)
                array::resize(graph.incoming, node->id);
            auto& incoming = graph.incoming[node->id - 1];
            if (!incoming.alloc)
                array::init(incoming, graph.alloc);
            return incoming;
        }

        template <Directed_Graph T>
        status_t init(T& graph, alloc_t* alloc = context::top()->alloc.main) {
            graph.alloc = alloc;
            graph.id    = {};
            array::init(graph.nodes, graph.alloc);
            array::init(graph.edges, graph.alloc);
            array::init(graph.outgoing, graph.alloc);
            array::init(graph.incoming, graph.alloc);

            slab_config_t node_cfg{};
            node_cfg.buf_size      = digraph_t<T>::Node_Size;
            node_cfg.buf_align     = digraph_t<T>::Node_Align;
            node_cfg.num_pages     = DEFAULT_NUM_PAGES;
            node_cfg.backing.alloc = graph.alloc;
            graph.node_slab = memory::system::make(&node_cfg);

            slab_config_t edge_cfg{};
            edge_cfg.buf_size      = digraph_t<T>::Edge_Size;
            edge_cfg.buf_align     = digraph_t<T>::Edge_Align;
            edge_cfg.num_pages     = DEFAULT_NUM_PAGES;
            edge_cfg.backing.alloc = graph.alloc;
            graph.edge_slab    = memory::system::make(&edge_cfg);

            return status_t::ok;
        }
    }
}
