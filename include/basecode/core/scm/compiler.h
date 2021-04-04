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

namespace basecode {
    struct bb_t;

    namespace scm {
        struct ctx_t;
        struct obj_t;
        struct env_t;
        struct proc_t;

        enum class context_kind_t : u8 {
            none,
            proc,
            prim,
        };

        struct context_t final {
            bb_t*               bb;
            ctx_t*              ctx;
            obj_t*              obj;
            obj_t*              env;
            union {
                proc_t*         proc;
                struct {
                    obj_t*      form;
                    obj_t*      args;
                }               prim;
            }                   kind;
            label_t             label;
            context_kind_t      type;
            b8                  top_level;
        };

        struct compile_result_t final {
            bb_t*               bb;
            reg_t               reg;
            b8                  should_release;
        };

        context_t make_context(bb_t& bb,
                               ctx_t* ctx,
                               obj_t* obj,
                               obj_t* env,
                               b8 top_level = false);

        compile_result_t compile(const context_t& c);

        inline u0 release_result(reg_alloc_t& alloc,
                                 const compile_result_t& r) {
            if (r.should_release && r.reg != 0)
                vm::reg_alloc::release_one(alloc, r.reg);
        }

        namespace proc {
            compile_result_t apply(const context_t& c);

            compile_result_t compile(const context_t& c);
        }

        namespace prim {
            compile_result_t compile(const context_t& c);
        }
    }
}
