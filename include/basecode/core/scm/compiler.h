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

        struct context_t final {
            bb_t*               bb;
            ctx_t*              ctx;
            obj_t*              obj;
            obj_t*              env;
            union {
                struct {
                    obj_t*      body;
                    obj_t*      params;
                    obj_t*      new_env;
                }               proc;
                struct {
                    obj_t*      form;
                    obj_t*      args;
                }               prim;
            }                   kind;
            label_t             label;
            reg_t               target;
            b8                  top_level;
        };

        context_t make_context(bb_t& bb,
                               ctx_t* ctx,
                               obj_t* obj,
                               obj_t* env,
                               reg_t target = 0,
                               b8 top_level = false);

        bb_t& compile(const context_t& c);

        namespace proc {
            bb_t& compile(const context_t& c);
        }

        namespace prim {
            bb_t& compile(const context_t& c);
        }
    }
}
