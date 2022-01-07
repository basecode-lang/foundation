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

#include <basecode/core/scm/scm.h>
#include <basecode/core/string.h>
#include <basecode/core/context.h>

namespace basecode {
    enum class cvar_type_t : u8 {
        none,
        flag,
        real,
        integer,
        string,
        pointer,
    };

    struct config_settings_t final {
        scm::ctx_t*             ctx             {};
        str::slice_t            build_type      {};
        str::slice_t            product_name    {};
        struct {
            u32                 major           {};
            u32                 minor           {};
            u32                 revision        {};
        }                       version         {};
        b8                      test_runner     {};
    };

    struct cvar_t final {
        str::slice_t            name;
        u32                     id;
        cvar_type_t             type;
        union {
            u0*                 ptr;
            f64                 real;
            u64                 integer;
            b8                  flag;
        }                       value;
    };
    static_assert(sizeof(cvar_t) <= 32, "cvar_t is now larger than 32 bytes!");

    namespace config {
        enum class status_t : u8 {
            ok                  = 0,
            bad_input,
            duplicate_cvar,
            cvar_not_found,
            cvar_id_out_of_range,
        };

        namespace system {
            u0 fini();

            scm::ctx_t* context();

            status_t init(const config_settings_t& settings,
                          alloc_t* alloc = context::top()->alloc.main);
        }

        namespace cvar {
            u0 clear();

            status_t remove(str::slice_t name);

            inline u0 set(cvar_t* var, b8 value) {
                var->value.flag = value;
            }

            u0 set(cvar_t* var, scm::obj_t* value);

            inline u0 set(cvar_t* var, u64 value) {
                var->value.integer = value;
            }

            inline u0 set(cvar_t* var, u32 value) {
                var->value.integer = value;
            }

            inline u0 set(cvar_t* var, f32 value) {
                var->value.real = value;
            }

            inline u0 set(cvar_t* var, f64 value) {
                var->value.real = value;
            }

            status_t get(str::slice_t name, cvar_t** var);

            inline u0 set(cvar_t* var, str::slice_t value) {
                auto rc = string::interned::fold_for_result(value);
                var->value.integer = rc.id;
            }

            status_t add(str::slice_t name, cvar_type_t type, cvar_t** var);

            status_t add(const String_Concept auto& name,
                         cvar_type_t type,
                         cvar_t** var) {
                return add(slice::make(name), type, var);
            }
        }
    }
}
