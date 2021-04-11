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

#include <basecode/core/scm/scm.h>

namespace basecode::scm::kernel {
    using type_decl_id          = u32;

    struct type_decl_t final {
        param_cls_t             cls;
        param_size_t            size;
        ffi_type_t              user;
    };

    struct proc_param_t final {
        str::slice_t            name;
        type_decl_id            type;
        param_alias_t           default_value       {};
        u8                      is_rest:        1   {};
        u8                      has_default:    1   {};
        u8                      pad:            7   {};
    };

    struct proc_overload_t final {
        u0*                     func;
        str::slice_t            name;
        type_decl_id            ret_type;
        u32                     num_params      {};
        proc_param_t            params[16]      {};
    };

    struct proc_export_t final {
        str::slice_t            name;
        u32                     num_overloads   {};
        proc_overload_t         overloads[16]   {};
    };

    namespace type_decl {
        constexpr u32 b8_       = 0;
        constexpr u32 u8_       = 1;
        constexpr u32 u0_       = 2;
        constexpr u32 u32_      = 3;
        constexpr u32 f32_      = 4;
        constexpr u32 obj_ptr   = 5;
        constexpr u32 list_ptr  = 6;
        constexpr u32 slice_ptr = 7;
    }

    u0 create_common_types();

    u0 create_types(type_decl_t* decls, u32 size);

    u0 create_exports(scm::ctx_t* ctx, proc_export_t* exports);
}
