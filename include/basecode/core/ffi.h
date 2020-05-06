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

#include <dyncall/dyncall.h>
#include <dynload/dynload.h>
#include <basecode/core/path.h>
#include <basecode/core/symtab.h>

namespace basecode {
    struct lib_t final {
        alloc_t*                alloc;
        DLLib*                  handle;
        symtab_t<u0*>           symbols;
        str::slice_t            path;
    };
    using symbol_array_t        = array_t<symtab_pair_t<u0*>>;

    enum class call_mode_t : u8 {
        system                  = 1,
        variadic,
    };

    enum class param_cls_t : u8 {
        ptr                     = 1,
        int_,
        float_,
        struct_,
    };

    enum class param_size_t : u8 {
        none,
        byte,
        word,
        dword,
        qword,
    };

    struct param_type_t final {
        param_cls_t             cls;
        param_size_t            size;
    };

    union param_alias_t final {
        u0*                     p;
        u8                      b;
        u16                     w;
        u32                     dw;
        u64                     qw;
        f32                     fdw;
        f64                     fqw;
    };

    struct param_value_t final {
        param_alias_t           alias;
        param_type_t            type;
    };

    struct param_t;
    using param_list_t          = array_t<param_t*>;

    struct param_t final {
        param_value_t           value;
        param_list_t            members;
        str::slice_t            name;
        u8                      has_dft:    1;
        u8                      is_rest:    1;
        u8                      pad:        6;
    };

    struct proto_t final {
        lib_t*                  lib;
        u0*                     func;
        str::slice_t            name;
        param_list_t            params;
        param_type_t            ret_type;
        call_mode_t             mode;
    };

    struct ffi_t final {
        alloc_t*                alloc;
        DCCallVM*               vm;
        u32                     heap_size;
    };

    namespace ffi {
        enum class status_t : u8 {
            ok,
            address_null,
            prototype_null,
            lib_not_loaded,
            symbol_not_found,
            invalid_int_size,
            invalid_float_size,
            load_library_failure,
            struct_by_value_not_implemented,
        };

        namespace lib {
            status_t unload(lib_t* lib);

            symbol_array_t syms(const lib_t* lib);

            status_t load(const path_t& path, lib_t** lib);

            status_t symaddr(lib_t* lib, str::slice_t name, u0** address);
        }

        namespace system {
            u0 fini();

            status_t init(alloc_t* alloc = context::top()->alloc, u8 num_pages = 1);
        }

        namespace param {
            inline param_type_t make_type(param_cls_t cls, param_size_t size) {
                return param_type_t{.cls = cls, .size = size};
            }

            param_t* make(str::slice_t name, param_type_t type, b8 rest = {}, param_alias_t* dft_val = {});
        }

        namespace proto {
            b8 remove(str::slice_t symbol);

            proto_t* find(str::slice_t symbol);

            proto_t* make(str::slice_t symbol);

            inline u0 append(proto_t* proto, param_t* param) {
                array::append(proto->params, param);
            }

            status_t make(lib_t* lib, str::slice_t symbol, proto_t** proto);
        }

        u0 free(ffi_t& ffi);

        u0 reset(ffi_t& ffi);

        str::slice_t status_name(status_t status);

        template <typename T> param_value_t arg(T value) {
            param_value_t arg{};
            if constexpr (std::is_pointer_v<T>) {
                arg.type.cls  = param_cls_t::ptr;
                arg.type.size = param_size_t::qword;
                arg.alias.p   = value;
            } else if constexpr (std::is_same_v<T, typeof(bool)>) {
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
            } else if constexpr (std::is_same_v<T, typeof(bool)>) {
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
            } else if constexpr (std::is_aggregate_v<T>) {
            }
        }

        status_t call(ffi_t& ffi, const proto_t* proto, param_alias_t& ret);

        status_t init(ffi_t& ffi, u32 heap_size = 2 * 1024, alloc_t* alloc = context::top()->alloc);
    }
}
