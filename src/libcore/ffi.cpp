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

#include <basecode/core/ffi.h>
#include <basecode/core/stable_array.h>

namespace basecode::ffi {
    struct lib_pair_t final {
        lib_t*                                  lib;
        u32                                     idx;
    };

    struct proto_pair_t final {
        proto_t*                                proto;
        u32                                     idx;
    };

    struct system_t final {
        alloc_t*                                alloc;
        stable_array_t<lib_t>                   libs;
        stable_array_t<param_t>                 params;
        stable_array_t<proto_t>                 protos;
        symtab_t<lib_pair_t>                    lib_map;
        symtab_t<proto_pair_t>                  proto_map;
    };

    static str::slice_t                         s_status_names[] = {
        "ok"_ss,
        "address null"_ss,
        "prototype null"_ss,
        "lib not loaded"_ss,
        "symbol not found"_ss,
        "invalid int size"_ss,
        "invalid float size"_ss,
        "load library failure"_ss,
        "struct by value not implemented"_ss,
    };

    system_t                                    g_ffi_system;

    u0 system::fini() {
        for (auto lib : g_ffi_system.libs)
            lib::unload(lib);
        for (auto param : g_ffi_system.params)
            array::free(param->members);
        for (auto proto : g_ffi_system.protos)
            array::free(proto->params);
        symtab::free(g_ffi_system.lib_map);
        symtab::free(g_ffi_system.proto_map);
        stable_array::free(g_ffi_system.libs);
        stable_array::free(g_ffi_system.protos);
        stable_array::free(g_ffi_system.params);
    }

    u0 free(ffi_t& ffi) {
        WITH_ALLOC(ffi.alloc, if (ffi.vm) dcFree(ffi.vm););
        ffi.vm        = {};
        ffi.heap_size = {};
    }

    u0 reset(ffi_t& ffi) {
        dcReset(ffi.vm);
    }

    u0 proto::free(proto_t* proto) {
        for (auto param : proto->params)
            param::free(param);
        array::free(proto->params);
    }

    u0 param::free(param_t* param) {
        if (!param)
            return;
        for (auto member_param : param->members)
            param::free(member_param);
        array::free(param->members);
    }

    status_t lib::unload(lib_t* lib) {
        if (!lib) return status_t::ok;
        symtab::free(lib->symbols);
        if (lib->handle) dlFreeLibrary(lib->handle);
        lib_pair_t pair{};
        const auto path_slice = slice::make(lib->path.str);
        if (symtab::find(g_ffi_system.lib_map, path_slice, pair)) {
            stable_array::erase(g_ffi_system.libs, pair.idx - 1);
            symtab::remove(g_ffi_system.lib_map, path_slice);
            path::free(lib->path);
            return status_t::ok;
        }
        return status_t::lib_not_loaded;
    }

    b8 proto::remove(str::slice_t symbol) {
        proto_pair_t pair{};
        if (symtab::find(g_ffi_system.proto_map, symbol, pair)) {
            proto::free(pair.proto);
            stable_array::erase(g_ffi_system.protos, pair.idx - 1);
            symtab::remove(g_ffi_system.proto_map, symbol);
            return true;
        }
        return false;
    }

    str::slice_t status_name(status_t status) {
        return s_status_names[(u32) status];
    }

    proto_t* proto::find(str::slice_t symbol) {
        proto_pair_t pair{};
        return symtab::find(g_ffi_system.proto_map, symbol, pair) ? pair.proto : nullptr;
    }

    proto_t* proto::make(str::slice_t symbol) {
        proto_pair_t pair{};
        if (symtab::find(g_ffi_system.proto_map, symbol, pair))
            return pair.proto;
        proto_pair_t* new_pair{};
        if (symtab::emplace(g_ffi_system.proto_map, symbol, &new_pair)) {
            auto proto = &stable_array::append(g_ffi_system.protos);
            new_pair->idx   = g_ffi_system.protos.size;
            new_pair->proto = proto;
            proto->lib      = {};
            proto->func     = {};
            proto->ret_type = {};
            proto->name     = symbol;
            proto->mode     = call_mode_t::system;
            array::init(proto->params, g_ffi_system.alloc);
            return proto;
        }
        return nullptr;
    }

    status_t lib::load(const path_t& path, lib_t** lib) {
        lib_pair_t pair{};
        const auto path_slice = slice::make(path);
        if (symtab::find(g_ffi_system.lib_map, path_slice, pair)) {
            *lib = pair.lib;
            return status_t::ok;
        }
        lib_pair_t* new_pair{};
        if (symtab::emplace(g_ffi_system.lib_map, path_slice, &new_pair)) {
            new_pair->lib         = &stable_array::append(g_ffi_system.libs);
            new_pair->idx         = g_ffi_system.libs.size;
            path::init(new_pair->lib->path, path_slice, g_ffi_system.alloc);
            new_pair->lib->alloc  = g_ffi_system.alloc;
            new_pair->lib->handle = dlLoadLibrary(str::c_str(const_cast<str_t&>(path.str)));
            if (!new_pair->lib->handle)
                return status_t::load_library_failure;
            symtab::init(new_pair->lib->symbols, new_pair->lib->alloc);
            *lib = new_pair->lib;
            return status_t::ok;
        }
        return status_t::load_library_failure;
    }

    status_t system::init(alloc_t* alloc, u8 num_pages) {
        g_ffi_system.alloc = alloc;
        stable_array::init(g_ffi_system.libs, g_ffi_system.alloc, num_pages);
        stable_array::init(g_ffi_system.params, g_ffi_system.alloc, num_pages);
        stable_array::init(g_ffi_system.protos, g_ffi_system.alloc, num_pages);
        symtab::init(g_ffi_system.lib_map, g_ffi_system.alloc);
        symtab::init(g_ffi_system.proto_map, g_ffi_system.alloc);
        return status_t::ok;
    }

    status_t push(ffi_t& ffi, const param_value_t& arg) {
        switch (arg.type.cls) {
            case param_cls_t::int_:
                switch (arg.type.size) {
                    case param_size_t::byte:    dcArgChar(ffi.vm, arg.alias.b);             break;
                    case param_size_t::word:    dcArgShort(ffi.vm, arg.alias.w);            break;
                    case param_size_t::dword:   dcArgInt(ffi.vm, arg.alias.dw);             break;
                    case param_size_t::qword:   dcArgLongLong(ffi.vm, arg.alias.qw);        break;
                    case param_size_t::none:    return status_t::invalid_int_size;
                }
                break;
            case param_cls_t::ptr:              dcArgPointer(ffi.vm, arg.alias.p);          break;
            case param_cls_t::struct_:          return status_t::struct_by_value_not_implemented;
            case param_cls_t::float_:
                switch (arg.type.size) {
                    case param_size_t::dword:   dcArgFloat(ffi.vm, arg.alias.fdw);          break;
                    case param_size_t::qword:   dcArgDouble(ffi.vm, arg.alias.fqw);         break;
                    default:                    return status_t::invalid_float_size;
                }
                break;
        }
        return status_t::ok;
    }

    u0 lib::syms(const lib_t* lib, symbol_array_t& syms) {
        return symtab::find_prefix(lib->symbols, syms);
    }

    status_t init(ffi_t& ffi, u32 heap_size, alloc_t* alloc) {
        ffi.alloc     = alloc;
        ffi.heap_size = heap_size;
        WITH_ALLOC(ffi.alloc, ffi.vm = dcNewCallVM(heap_size););
        return status_t::ok;
    }

    status_t lib::symaddr(lib_t* lib, str::slice_t name, u0** address) {
        if (!address) return status_t::address_null;
        if (symtab::find(lib->symbols, name, *address))
            return status_t::ok;
        WITH_SLICE_AS_CSTR(name, *address = dlFindSymbol(lib->handle, name););
        if (!(*address))
            return status_t::symbol_not_found;
        symtab::insert(lib->symbols, name, *address);
        return status_t::ok;
    }

    status_t call(ffi_t& ffi, const proto_t* proto, param_alias_t& ret) {
        switch (proto->mode) {
            case call_mode_t::system:           dcMode(ffi.vm, DC_CALL_C_DEFAULT);              break;
            case call_mode_t::variadic:         dcMode(ffi.vm, DC_CALL_C_ELLIPSIS);             break;
        }
        switch (proto->ret_type.cls) {
            case param_cls_t::ptr:              ret.p = dcCallPointer(ffi.vm, proto->func);     break;
            case param_cls_t::int_:
                switch (proto->ret_type.size) {
                    case param_size_t::none:    dcCallVoid(ffi.vm, proto->func); ret.qw = {};   break;
                    case param_size_t::byte:    ret.b = dcCallChar(ffi.vm, proto->func);        break;
                    case param_size_t::word:    ret.w = dcCallShort(ffi.vm, proto->func);       break;
                    case param_size_t::dword:   ret.dw = dcCallInt(ffi.vm, proto->func);        break;
                    case param_size_t::qword:   ret.qw = dcCallLongLong(ffi.vm, proto->func);   break;
                }
                break;
            case param_cls_t::float_:
                switch (proto->ret_type.size) {
                    case param_size_t::dword:   ret.fdw = dcCallFloat(ffi.vm, proto->func);     break;
                    case param_size_t::qword:   ret.fqw = dcCallDouble(ffi.vm, proto->func);    break;
                    default:                    return status_t::invalid_float_size;
                }
                break;
            case param_cls_t::struct_:          return status_t::struct_by_value_not_implemented;
        }
        return status_t::ok;
    }

    status_t proto::make(lib_t* lib, str::slice_t symbol, proto_t** proto) {
        if (!proto) return status_t::prototype_null;
        auto new_proto = proto::make(symbol);
        if (!new_proto->lib) {
            auto status = lib::symaddr(lib, symbol, &new_proto->func);
            if (!OK(status)) return status;
            new_proto->lib = lib;
        }
        *proto = new_proto;
        return status_t::ok;
    }

    param_t* param::make(str::slice_t name, param_type_t type, b8 rest, param_alias_t* dft_val) {
        auto param = &stable_array::append(g_ffi_system.params);
        param->pad        = {};
        param->name       = name;
        param->is_rest    = rest;
        param->value.type = type;
        if (dft_val) {
            param->has_dft     = true;
            param->value.alias = *dft_val;
        }
        array::init(param->members, g_ffi_system.alloc);
        return param;
    }
}
