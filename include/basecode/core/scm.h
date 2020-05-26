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

#include <chibi/eval.h>
#include <basecode/core/path.h>
#include <basecode/core/defer.h>
#include <basecode/core/memory.h>

namespace basecode {
    struct scm_t final {
        alloc_t*                alloc;
        sexp                    heap;
        sexp                    env;
        sexp                    ctx;
    };
    static_assert(sizeof(scm_t) <= 32, "sizeof(scm_t) is now greater than 32 bytes!");

    namespace scm {
        enum class status_t : u8 {
            ok,
            error,
        };

        namespace system {
            status_t init();

            status_t fini();
        }

        u0 free(scm_t& scm);

        status_t load(scm_t& scm, const path_t& path);

        status_t global_set(scm_t& scm, sexp val, s32 slot);

        status_t global_ref(scm_t& scm, sexp* obj, s32 slot);

        status_t eval(scm_t& scm, sexp* obj, const String_Concept auto& str) {
            sexp_gc_var1(s);
            sexp_gc_preserve1(scm.ctx, s);
            defer(sexp_gc_release1(scm.ctx));
            s = sexp_read_from_string(scm.ctx, (const s8*) str.data, str.length);
            if (sexp_exceptionp(s)) {
                *obj = s;
                return status_t::error;
            }
            *obj = sexp_eval(scm.ctx, s, nullptr);
            return sexp_exceptionp(*obj) ? status_t::error : status_t::ok;
        }

        status_t intern(scm_t& scm, sexp* obj, const String_Concept auto& str) {
            *obj = sexp_intern(scm.ctx, (const s8*) str.data, str.length);
            return sexp_exceptionp(*obj) ? status_t::error : status_t::ok;
        }

        status_t init(scm_t& scm, alloc_t* alloc = context::top()->alloc, scm_t* parent = {});

        status_t env_define(scm_t& scm, sexp* obj, const String_Concept auto& str, sexp value) {
            sexp_gc_var1(s);
            sexp_gc_preserve1(scm.ctx, s);
            defer(sexp_gc_release1(scm.ctx));
            s = sexp_intern(scm.ctx, (const s8*) str.data, str.length);
            if (sexp_exceptionp(s)) {
                *obj = s;
                return status_t::error;
            }
            *obj = sexp_env_define(scm.ctx, scm.env, s, value);
            return sexp_exceptionp(*obj) ? status_t::error : status_t::ok;
        }

        status_t env_ref(scm_t& scm, sexp* obj, const String_Concept auto& str, sexp dft_val = {}) {
            sexp_gc_var1(s);
            sexp_gc_preserve1(scm.ctx, s);
            defer(sexp_gc_release1(scm.ctx));
            s = sexp_intern(scm.ctx, (const s8*) str.data, str.length);
            if (sexp_exceptionp(s)) {
                *obj = s;
                return status_t::error;
            }
            *obj = sexp_env_ref(scm.ctx, scm.env, s, dft_val);
            return sexp_exceptionp(*obj) ? status_t::error : status_t::ok;
        }
    }
}
