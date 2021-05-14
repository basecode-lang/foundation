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

#include <basecode/core/types.h>
#include <dyncall/dyncall.h>
#include <dynload/dynload.h>

namespace basecode::ffi {
    namespace lib {
        status_t unload(lib_t* lib);

        status_t load(const path_t& path, lib_t** lib);

        u0 syms(const lib_t* lib, symbol_array_t& syms);

        status_t symaddr(lib_t* lib, str::slice_t name, u0** address);
    }

    namespace system {
        u0 fini();

        status_t init(alloc_t* alloc = context::top()->alloc.main,
                      u8 num_pages = DEFAULT_NUM_PAGES);
    }

    namespace param {
        u0 free(param_t* param);

        param_t* make(str::slice_t name,
                      param_type_t type,
                      b8 rest = {},
                      param_alias_t* dft_val = {});

        inline param_type_t make_type(param_cls_t cls,
                                      param_size_t size,
                                      u8 user = 0) {
            return param_type_t{.cls = cls, .size = size, .user = user};
        }
    }

    namespace proto {
        u0 free(proto_t* proto);

        b8 remove(str::slice_t symbol);

        proto_t* find(str::slice_t symbol);

        status_t append(proto_t* proto, overload_t* ol);

        proto_t* make(str::slice_t symbol, lib_t* lib = {});

        overload_t* match_signature(proto_t* proto, const str_t& candidate);
    }

    namespace overload {
        u0 free(overload_t* ol);

        b8 remove(str::slice_t symbol);

        overload_t* make(str::slice_t symbol,
                         param_type_t ret_type,
                         u0* func = {});

        overload_t* find(str::slice_t symbol);

        status_t append(overload_t* ol, param_t* param);
    }

    u0 free(ffi_t& ffi);

    u0 reset(ffi_t& ffi);

    template <typename T> param_value_t arg(T value) {
        param_value_t arg{};
        if constexpr (std::is_pointer_v<T>) {
            arg.type.cls  = param_cls_t::ptr;
            arg.type.size = param_size_t::qword;
            arg.alias.p   = value;
        } else if constexpr (std::is_same_v<T, bool>) {
            arg.type.cls  = param_cls_t::int_;
            arg.type.size = param_size_t::dword;
            arg.alias.dw  = value;
        } else if constexpr (std::is_integral_v<T>) {
            arg.type.cls = param_cls_t::int_;
            if constexpr (sizeof(T) == 1) {
                arg.type.size = param_size_t::byte;
                arg.alias.b   = value;
            } else if constexpr (sizeof(T) == 2) {
                arg.type.size = param_size_t::word;
                arg.alias.w   = value;
            } else if constexpr (sizeof(T) == 4) {
                arg.type.size = param_size_t::dword;
                arg.alias.dw  = value;
            } else if constexpr (sizeof(T) == 8) {
                arg.type.size = param_size_t::qword;
                arg.alias.qw  = value;
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            arg.type.cls = param_cls_t::float_;
            if constexpr (sizeof(T) == 4) {
                arg.type.size = param_size_t::dword;
                arg.alias.fdw = value;
            } else if constexpr (sizeof(T) == 8) {
                arg.type.size = param_size_t::qword;
                arg.alias.fqw = value;
            }
        } else if constexpr (std::is_aggregate_v<T>) {
            arg.type.cls  = param_cls_t::struct_;
            arg.type.size = param_size_t::none;
            arg.alias.p   = (u0*) &value;
        }
        return arg;
    }

    status_t push(ffi_t& ffi, const param_value_t& arg);

    template <typename T> u0 push(ffi_t& ffi, T value) {
        if constexpr (std::is_pointer_v<T>) {
            dcArgPointer(ffi.vm, value);
        } else if constexpr (std::is_same_v<T, bool>) {
            dcArgBool(ffi.vm, value);
        } else if constexpr (std::is_integral_v<T>) {
            if constexpr (sizeof(T) == 1) {
                dcArgChar(ffi.vm, value);
            } else if constexpr (sizeof(T) == 2) {
                dcArgShort(ffi.vm, value);
            } else if constexpr (sizeof(T) == 4) {
                dcArgInt(ffi.vm, value);
            } else if constexpr (sizeof(T) == 8) {
                dcArgLongLong(ffi.vm, value);
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            if constexpr (sizeof(T) == 4) {
                dcArgFloat(ffi.vm, value);
            } else if constexpr (sizeof(T) == 8) {
                dcArgDouble(ffi.vm, value);
            }
        }
    }

    status_t init(ffi_t& ffi,
                  u32 heap_size = KB(2),
                  alloc_t* alloc = context::top()->alloc.main);

    status_t call(ffi_t& ffi, const overload_t* ol, param_alias_t& ret);
}
