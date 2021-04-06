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

#include <basecode/core/array.h>
#include <basecode/core/stack.h>
#include <basecode/core/memory.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode {
    template <typename T>
    concept Directed_Graph = requires(const T& t) {
        typename                T::Node;
        typename                T::Edge;
        typename                T::Value_Type;
        typename                T::Node_Array;
        typename                T::Edge_Array;
        typename                T::Node_Stack;
        typename                T::Component_Array;

        {t.alloc}               -> same_as<alloc_t*>;
        {t.node_slab}           -> same_as<alloc_t*>;
        {t.edge_slab}           -> same_as<alloc_t*>;
        {t.nodes}               -> same_as<typename T::Node_Array>;
        {t.edges}               -> same_as<typename T::Edge_Array>;
        {t.size}                -> same_as<u32>;
        {t.id}                  -> same_as<u32>;
    };

    template <typename V>
    struct digraph_t final {
        struct node_t;
        struct edge_t;

        using Node              = node_t;
        using Edge              = edge_t;
        using Value_Type        = V;
        using Node_Array        = array_t<Node*>;
        using Edge_Array        = array_t<Edge*>;
        using Node_Stack        = stack_t<Node*>;
        using Component_Array   = array_t<array_t<Node*>>;

        struct node_t final {
            Value_Type*         value;
            u32                 id;
            s32                 index;
            s32                 low_link;
            b8                  on_stack;
        };

        struct edge_t final {
            const Node*         src;
            const Node*         dst;
            f64                 w;
            s32                 type;
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
        u32                     size;
        u32                     id;
    };

    namespace digraph {
        enum class status_t {
            ok
        };

        template <Directed_Graph T,
                  typename Node = typename T::Node,
                  typename Node_Array = typename T::Node_Array>
        u0 outgoing_nodes(const T& graph, Node* node, Node_Array& nodes);

        template <Directed_Graph T>
        u0 free(T& graph) {
            memory::system::free(graph.edge_slab);
            memory::system::free(graph.node_slab);
            array::free(graph.edges);
            array::free(graph.nodes);
            graph.id = graph.size = {};
        }

        template <Directed_Graph T>
        u0 reset(T& graph) {
            array::reset(graph.edges);
            array::reset(graph.nodes);
            memory::slab::reset(graph.edge_slab);
            memory::slab::reset(graph.node_slab);
            graph.id = graph.size = {};
        }

        template <Directed_Graph T,
                  typename Edge = typename T::Edge,
                  typename Node = typename T::Node,
                  typename Node_Array = typename T::Node_Array,
                  typename Node_Stack = typename T::Node_Stack,
                  typename Component_Array = typename T::Component_Array>
        u0 strongly_connected(const T& graph,
                              Component_Array& comps,
                              Node_Stack& stack,
                              u32& index,
                              Node* v) {
            v->index    = index;
            v->low_link = index++;
            stack::push(stack, v);
            v->on_stack = true;

            Node_Array succs{};
            array::init(succs, graph.alloc);
            defer(array::free(succs));

            outgoing_nodes(graph, v, succs);
            for (auto w : succs) {
                if (w->index == -1) {
                    strongly_connected(graph, comps, stack, index, w);
                    v->low_link = std::min(v->low_link, w->low_link);
                } else if (w->on_stack) {
                    v->low_link = std::min(v->low_link, w->index);
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
        Component_Array strongly_connected(const T& graph) {
            Node_Stack stack{};
            stack::init(stack, graph.alloc);

            Component_Array comps{};
            array::init(comps, graph.alloc);

            u32 index{};
            for (auto n : graph.nodes) {
                if (n->index == -1) {
                    strongly_connected(graph, comps, stack, index, n);
                }
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

        template <Directed_Graph T, typename Node, typename Node_Array>
        u0 outgoing_nodes(const T& graph, Node* node, Node_Array& nodes) {
            for (auto e : graph.edges) {
                if (e->src != node) continue;
                array::append(nodes, const_cast<Node*>(e->dst));
            }
        }

        template <Directed_Graph T>
        status_t init(T& graph, alloc_t* alloc = context::top()->alloc) {
            graph.alloc = alloc;
            array::init(graph.nodes, graph.alloc);
            array::init(graph.edges, graph.alloc);

            slab_config_t node_cfg{};
            node_cfg.backing   = graph.alloc;
            node_cfg.buf_size  = digraph_t<T>::Node_Size;
            node_cfg.buf_align = digraph_t<T>::Node_Align;
            graph.node_slab = memory::system::make(alloc_type_t::slab, &node_cfg);

            slab_config_t edge_cfg{};
            edge_cfg.backing   = graph.alloc;
            edge_cfg.buf_size  = digraph_t<T>::Edge_Size;
            edge_cfg.buf_align = digraph_t<T>::Edge_Align;
            graph.edge_slab    = memory::system::make(alloc_type_t::slab, &edge_cfg);

            return status_t::ok;
        }

        template <Directed_Graph T,
                  typename Edge = typename T::Edge,
                  typename Node = typename T::Node>
        Edge* make_edge(T& graph, const Node* src, const Node* dst, s32 type = 0, f64 weight = 1.0) {
            auto edge = (Edge*) memory::alloc(graph.edge_slab);
            edge->src  = src;
            edge->dst  = dst;
            edge->w    = weight;
            edge->type = type;
            array::append(graph.edges, edge);
            return edge;
        }
    }
}

namespace basecode::hash {
    template <Directed_Graph T,
              typename Node = typename T::Node>
    inline u64 hash64(const Node& key) {
        return murmur::hash64(key.id, sizeof(u32));
    }
}
