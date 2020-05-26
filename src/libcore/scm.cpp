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

#include <chibi/gc_heap.h>
#include <basecode/core/scm.h>

namespace basecode::scm {
    namespace system {
        status_t init() {
            sexp_scheme_init();
            return status_t::ok;
        }

        status_t fini() {
            return status_t::ok;
        }
    }

    u0 free(scm_t& scm) {
        sexp_destroy_context(scm.ctx);
        scm.ctx = scm.heap = scm.env = {};
    }

    status_t global_set(scm_t& scm, sexp val, s32 slot) {
        sexp_global(scm.ctx, slot) = val;
        return status_t::ok;
    }

    status_t global_ref(scm_t& scm, sexp* obj, s32 slot) {
        *obj = sexp_global(scm.ctx, slot);
        return status_t::ok;
    }

    status_t load(scm_t& scm, sexp* obj, const path_t& path) {
        sexp_gc_var1(s);
        sexp_gc_preserve1(scm.ctx, s);
        s = sexp_c_string(scm.ctx, (const s8*) path.str.data, path.str.length);
        *obj = sexp_load(scm.ctx, s, nullptr);
        sexp_gc_release1(scm.ctx);
        return sexp_exceptionp(*obj) ? status_t::error : status_t::ok;
    }

    status_t init(scm_t& scm, alloc_t* alloc, scm_t* parent) {
        scm.alloc = alloc;
        scm.heap  = sexp_make_context(alloc, parent ? parent->heap : nullptr, 0, 0);
        scm.ctx   = sexp_make_eval_context(alloc, scm.ctx, nullptr, nullptr, 0, 0);
        scm.env   = sexp_load_standard_env(scm.ctx, sexp_context_env(scm.ctx), SEXP_SEVEN);
        sexp_load_standard_ports(scm.ctx, scm.env, stdin, stdout, stderr, 1);
        return status_t::ok;
    }
}
