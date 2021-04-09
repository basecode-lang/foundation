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

#include <basecode/core/path.h>
#include <basecode/core/types.h>
#include <basecode/core/context.h>
#include <basecode/core/scm/scm.h>

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
            const u8*           ptr;
            f64                 real;
            u64                 integer;
            b8                  flag;
        }                       value;
    };
    static_assert(sizeof(cvar_t) <= 32, "cvar_t is now larger than 32 bytes!");

    namespace config {
        enum var_t : u8 {
            platform            = 10,
            build_type,
            test_runner,
            product_name,
            version_major,
            version_minor,
            version_revision,
        };

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
                          alloc_t* alloc = context::top()->alloc);
        }

        namespace cvar {
            u0 clear();

            status_t remove(u32 id);

            status_t get(u32 id, cvar_t** var);

            status_t add(u32 id, const s8* name, cvar_type_t type, s32 len = -1);

            status_t add(u32 id, const String_Concept auto& name, cvar_type_t type) {
                return add(id, (const s8*) name.data, type, name.length);
            }
        }
    }
}
