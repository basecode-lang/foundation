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

#define REQ(name, type)         basecode::scm::kernel::proc_param_t{            \
    name##_ss,                                                                  \
    TYPE_DECL_REF(type)}
#define REST(name, type)        basecode::scm::kernel::proc_param_t{            \
    name##_ss,                                                                  \
    TYPE_DECL_REF(type),                                                        \
    .is_rest = true}
#define OPT(name, type, v)      basecode::scm::kernel::proc_param_t{            \
    name##_ss,                                                                  \
    TYPE_DECL_REF(type),                                                        \
    .default_value.qw = u64(v),                                                 \
    .has_default = true}
#define OVERLOAD(fn, type, ...)                                                 \
    proc_overload_t{(u0*) fn,                                                   \
    #fn##_ss,                                                                   \
    TYPE_DECL_REF(type),                                                        \
    u32(VA_COUNT(__VA_ARGS__)),                                                 \
    __VA_ARGS__ }
#define TYPE_DECL_KINDS         TYPE_DECL_KIND(b8)                              \
                                TYPE_DECL_KIND(u8)                              \
                                TYPE_DECL_KIND(u0)                              \
                                TYPE_DECL_KIND(u16)                             \
                                TYPE_DECL_KIND(u32)                             \
                                TYPE_DECL_KIND(f32)                             \
                                TYPE_DECL_KIND(obj_ptr)                         \
                                TYPE_DECL_KIND(list_ptr)                        \
                                TYPE_DECL_KIND(slice_ptr)

#define TYPE_DECL_REF(k)        (u32(basecode::scm::kernel::type_decl_kind_t::Kind_##k))

namespace basecode::scm::kernel {
    using type_decl_id          = u32;

    struct type_decl_t final {
        param_cls_t             cls;
        param_size_t            size;
        ffi_type_t              user;
    };

    enum class type_decl_kind_t : u32 {
#define TYPE_DECL_KIND(k)       Kind_##k,
        TYPE_DECL_KINDS
#undef TYPE_DECL
        Kind_Count
    };

    struct proc_param_t final {
        str::slice_t            name;
        type_decl_id            type;
        param_alias_t           default_value       {};
        u8                      is_rest:        1   {};
        u8                      has_default:    1   {};
        u8                      pad:            6   {};
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

    u0 create_common_types();

    u0 create_types(type_decl_t* decls, u32 size);

    u0 create_exports(scm::ctx_t* ctx, proc_export_t* exports);
}
