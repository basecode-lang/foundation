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

#include <basecode/core/scm/kernel.h>

#define TYPE_DECLS  TYPE_DECL(u0, param_cls_t::void_, param_size_t::byte)       \
                    TYPE_DECL_USER(b8,                                          \
                                   param_cls_t::int_,                           \
                                   param_size_t::byte,                          \
                                   scm::ffi_type_t::boolean)                    \
                    TYPE_DECL(u8,                                               \
                              param_cls_t::int_,                                \
                              param_size_t::byte)                               \
                    TYPE_DECL(u16,                                              \
                              param_cls_t::int_,                                \
                              param_size_t::word)                               \
                    TYPE_DECL(u32,                                              \
                              param_cls_t::int_,                                \
                              param_size_t::dword)                              \
                    TYPE_DECL(f32,                                              \
                              param_cls_t::float_,                              \
                              param_size_t::dword)                              \
                    TYPE_DECL_USER(obj_ptr,                                     \
                                   param_cls_t::ptr,                            \
                                   param_size_t::qword,                         \
                                   scm::ffi_type_t::object)                     \
                    TYPE_DECL_USER(list_ptr,                                    \
                                   param_cls_t::ptr,                            \
                                   param_size_t::qword,                         \
                                   scm::ffi_type_t::list)                       \
                    TYPE_DECL_USER(slice_ptr,                                   \
                                   param_cls_t::ptr,                            \
                                   param_size_t::qword,                         \
                                   scm::ffi_type_t::string)
#define TYPE_DECL(k, cls, size) TYPE_DECL_USER(k,                               \
                                               cls,                             \
                                               size,                            \
                                               scm::ffi_type_t::none)

namespace basecode::scm::kernel {
    static type_decl_t s_common_types[] = {
#define TYPE_DECL_USER(k, cls, size, user)  [TYPE_DECL_REF(k)] = {(cls), (size), (user)},
        TYPE_DECLS
#undef TYPE_DECL_USER
    };

    static param_type_t s_types[TYPE_DECL_REF(Count)] = {};

    u0 create_common_types() {
        scm::kernel::create_types(s_common_types, TYPE_DECL_REF(Count));
    }

    u0 create_types(type_decl_t* decls, u32 size) {
        for (u32 i = 0; i < size; ++i) {
            const auto& decl = decls[i];
            s_types[i] = ffi::param::make_type(decl.cls,
                                               decl.size,
                                               u8(decl.user));
        }
    }

    u0 create_exports(scm::ctx_t* ctx, proc_export_t* exports) {
        u32 i{};
        while (true) {
            const auto& exp = exports[i];
            if (slice::empty(exp.name))
                break;
            auto proto = ffi::proto::make(exp.name);
            for (u32 j = 0; j < exp.num_overloads; ++j) {
                const auto& ol_decl = exp.overloads[j];
                auto ol = ffi::overload::make(ol_decl.name,
                                              s_types[ol_decl.ret_type],
                                              ol_decl.func);
                for (u32 k = 0; k < ol_decl.num_params; ++k) {
                    const auto& param_decl = ol_decl.params[k];
                    auto param = ffi::param::make(param_decl.name,
                                                  s_types[param_decl.type]);
                    if (param_decl.is_rest) {
                        param->is_rest = true;
                    } else if (param_decl.has_default) {
                        param->has_dft = true;
                        param->value.alias = param_decl.default_value;
                    }
                    ffi::overload::append(ol, param);
                }
                ffi::proto::append(proto, ol);
            }
            scm::define(ctx,
                        scm::make_symbol(ctx, proto->name),
                        scm::make_ffi(ctx, proto));
            ++i;
        }
    }
}
