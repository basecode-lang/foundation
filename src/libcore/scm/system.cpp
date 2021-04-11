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

#include <basecode/core/scm/system.h>

namespace basecode::scm::system {
    using path_stack_t          = stack_t<const path_t*>;

    struct system_t final {
        alloc_t*                alloc;
        ctx_t*                  ctx;
        path_stack_t            path_stack;
        u32                     heap_size;
    };

    system_t                    g_scm_sys;

    u0 fini() {
        scm::free(g_scm_sys.ctx);
        memory::free(g_scm_sys.alloc, g_scm_sys.ctx);
        stack::free(g_scm_sys.path_stack);
    }

    ctx_t* global_ctx() {
        return g_scm_sys.ctx;
    }

    const path_t* current_eval_path() {
        return stack::top(g_scm_sys.path_stack);
    }

    status_t init(u32 heap_size, alloc_t* alloc) {
        g_scm_sys.alloc     = alloc;
        g_scm_sys.heap_size = heap_size;
        g_scm_sys.ctx       = (scm::ctx_t*) memory::alloc(g_scm_sys.alloc,
                                                          g_scm_sys.heap_size);
        scm::init(g_scm_sys.ctx, g_scm_sys.heap_size, g_scm_sys.alloc);
        stack::init(g_scm_sys.path_stack, g_scm_sys.alloc);
        kernel::create_common_types();
        return status_t::ok;
    }

    status_t eval(const path_t& path, obj_t** obj) {
        stack::push(g_scm_sys.path_stack, &path);
        buf_t buf{};
        buf::init(buf, g_scm_sys.alloc);
        auto status = buf::map_existing(buf, path);
        if (!OK(status))
            return status_t::bad_input;
        buf_crsr_t crsr{};
        buf::cursor::init(crsr, buf);
        defer(
            buf::cursor::free(crsr);
            buf::free(buf);
            stack::pop(g_scm_sys.path_stack));
        auto gc = save_gc(g_scm_sys.ctx);
        while (true) {
            auto expr = read(g_scm_sys.ctx, crsr);
            if (!expr) break;
            *obj = eval(g_scm_sys.ctx, expr);
            restore_gc(g_scm_sys.ctx, gc);
        }
        restore_gc(g_scm_sys.ctx, gc);
        return status_t::ok;
    }

    status_t eval(const u8* source, u32 len, obj_t** obj) {
        buf_t buf{};
        buf::init(buf, g_scm_sys.alloc);
        auto status = buf::load(buf, source, len);
        if (!OK(status))
            return status_t::bad_input;
        buf_crsr_t crsr{};
        buf::cursor::init(crsr, buf);
        auto gc = save_gc(g_scm_sys.ctx);
        defer(
            restore_gc(g_scm_sys.ctx, gc);
            buf::cursor::free(crsr);
            buf::free(buf));
        auto expr = read(g_scm_sys.ctx, crsr);
        if (!expr) {
            *obj = nil(g_scm_sys.ctx);
            return status_t::ok;
        }
        *obj = eval(g_scm_sys.ctx, expr);
        return status_t::ok;
    }
}
