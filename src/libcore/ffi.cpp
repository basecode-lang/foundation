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
    struct lib_pair_t;
    struct proto_pair_t;
    struct overload_pair_t;

    using lib_slab_t            = stable_array_t<lib_t>;
    using lib_symtab_t          = symtab_t<lib_pair_t>;
    using proto_symtab_t        = symtab_t<proto_pair_t>;
    using param_slab_t          = stable_array_t<param_t>;
    using proto_slab_t          = stable_array_t<proto_t>;
    using overload_slab_t       = stable_array_t<overload_t>;
    using overload_symtab_t     = symtab_t<overload_pair_t>;

    struct lib_pair_t final {
        lib_t*                  lib;
        u32                     idx;
    };

    struct overload_pair_t final {
        overload_t*             overload;
        u32                     idx;
    };

    struct proto_pair_t final {
        proto_t*                proto;
        u32                     idx;
    };

    struct system_t final {
        alloc_t*                alloc;
        lib_symtab_t            libs;
        proto_symtab_t          protos;
        overload_symtab_t       overloads;
        lib_slab_t              lib_slab;
        param_slab_t            param_slab;
        proto_slab_t            proto_slab;
        overload_slab_t         overload_slab;
    };

    system_t                    g_ffi_system;

    namespace lib {
        status_t unload(lib_t* lib) {
            if (!lib)
                return status_t::ok;
            symtab::free(lib->symbols);
            if (lib->handle)
                dlFreeLibrary(lib->handle);
            lib_pair_t pair{};
            const auto path_slice = slice::make(lib->path.str);
            if (symtab::find(g_ffi_system.libs, path_slice, pair)) {
                stable_array::erase(g_ffi_system.lib_slab, pair.idx - 1);
                symtab::remove(g_ffi_system.libs, path_slice);
                path::free(lib->path);
                return status_t::ok;
            }
            return status_t::lib_not_loaded;
        }

        status_t load(const path_t& path, lib_t** lib) {
            lib_pair_t pair{};
            const auto path_slice = slice::make(path);
            if (symtab::find(g_ffi_system.libs, path_slice, pair)) {
                *lib = pair.lib;
                return status_t::ok;
            }
            lib_pair_t* new_pair{};
            if (symtab::emplace(g_ffi_system.libs, path_slice, &new_pair)) {
                new_pair->lib = &stable_array::append(g_ffi_system.lib_slab);
                new_pair->idx = g_ffi_system.libs.size;
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

        u0 syms(const lib_t* lib, symbol_array_t& syms) {
            return symtab::find_prefix(lib->symbols, syms);
        }

        status_t symaddr(lib_t* lib, str::slice_t name, u0** address) {
            if (!address) return status_t::address_null;
            if (symtab::find(lib->symbols, name, *address))
                return status_t::ok;
            WITH_SLICE_AS_CSTR(name, *address = dlFindSymbol(lib->handle, name););
            if (!(*address))
                return status_t::symbol_not_found;
            symtab::insert(lib->symbols, name, *address);
            return status_t::ok;
        }
    }

    namespace param {
        u0 free(param_t* param) {
            if (!param)
                return;
            for (auto member_param : param->members)
                param::free(member_param);
            array::free(param->members);
        }

        param_t* make(str::slice_t name, param_type_t type, b8 rest, param_alias_t* dft_val) {
            auto param = &stable_array::append(g_ffi_system.param_slab);
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

    namespace proto {
        u0 free(proto_t* proto) {
            assoc_array_t<overload_t*> pairs{};
            assoc_array::init(pairs, g_ffi_system.alloc);
            symtab::find_prefix(proto->overloads, pairs);
            for (u32 i = 0; i < pairs.size; ++i)
                overload::free(pairs[i].value);
            symtab::free(proto->overloads);
            assoc_array::free(pairs);
        }

        b8 remove(str::slice_t symbol) {
            proto_pair_t pair{};
            if (symtab::find(g_ffi_system.protos, symbol, pair)) {
                proto::free(pair.proto);
                stable_array::erase(g_ffi_system.proto_slab, pair.idx - 1);
                symtab::remove(g_ffi_system.protos, symbol);
                return true;
            }
            return false;
        }

        proto_t* find(str::slice_t symbol) {
            proto_pair_t pair{};
            return symtab::find(g_ffi_system.protos, symbol, pair) ? pair.proto : nullptr;
        }

        proto_t* make(str::slice_t symbol, lib_t* lib) {
            proto_pair_t pair{};
            if (symtab::find(g_ffi_system.protos, symbol, pair))
                return pair.proto;
            proto_pair_t* new_pair{};
            if (symtab::emplace(g_ffi_system.protos, symbol, &new_pair)) {
                auto proto = &stable_array::append(g_ffi_system.proto_slab);
                new_pair->idx   = g_ffi_system.proto_slab.size;
                new_pair->proto = proto;
                proto->lib      = lib;
                proto->name     = symbol;
                proto->min_req  = {};
                proto->max_req  = {};
                symtab::init(proto->overloads, g_ffi_system.alloc);
                return proto;
            }
            return nullptr;
        }

        status_t append(proto_t* proto, overload_t* ol) {
            if (proto->lib && !ol->func) {
                auto status = lib::symaddr(proto->lib, ol->name, &ol->func);
                if (!OK(status))
                    return status;
            }
            ol->proto = proto;
            if (ol->key_len == 0)
                ol->key[ol->key_len++] = u8(param_cls_t::void_);
            auto key = slice::make(ol->key, ol->key_len);
            if (!symtab::insert(proto->overloads, key, ol))
                return status_t::duplicate_overload;
            if (ol->params.size > proto->min_req)
                proto->min_req = ol->params.size;
            if (ol->params.size > proto->max_req)
                proto->max_req = ol->params.size;
            return status_t::ok;
        }

        overload_t* match_signature(proto_t* proto, const u8* buf, u32 len) {
            assoc_array_t<overload_t*> pairs{};
            assoc_array::init(pairs, g_ffi_system.alloc);
            defer(assoc_array::free(pairs));

            auto prefix = slice::make(buf, len);
        retry:
            symtab::find_prefix(proto->overloads, pairs, prefix);

            if (pairs.size == 1)
                return pairs[0].value;
            else if (pairs.size > 1) {
                for (u32 i = 0; i < pairs.size; ++i) {
                    const auto& pair = pairs[i];
                    if (pair.value->has_rest)
                        return pair.value;
                }
            } else {
                if (prefix.length >= 2) {
                    prefix.length -= 2;
                    goto retry;
                } else if (prefix.length == 1) {
                    --prefix.length;
                    goto retry;
                }
            }
            return nullptr;
        }
    }

    namespace overload {
        u0 free(overload_t* ol) {
            for (auto param : ol->params)
                param::free(param);
            array::free(ol->params);
        }

        b8 remove(str::slice_t symbol) {
            overload_pair_t pair{};
            if (symtab::find(g_ffi_system.overloads, symbol, pair)) {
                auto ol = pair.overload;
                auto key = slice::make(ol->key, ol->key_len);
                symtab::remove(ol->proto->overloads, key);
                overload::free(ol);
                stable_array::erase(g_ffi_system.overload_slab, pair.idx - 1);
                symtab::remove(g_ffi_system.overloads, symbol);
                return true;
            }
            return false;
        }

        overload_t* find(str::slice_t symbol) {
            overload_pair_t pair{};
            return symtab::find(g_ffi_system.overloads, symbol, pair) ? pair.overload : nullptr;
        }

        status_t append(overload_t* overload, param_t* param) {
            array::append(overload->params, param);
            if (overload->key_len > 14)
                return status_t::parameter_overflow;
            if (param->is_rest) {
                overload->has_rest = true;
                overload->key[overload->key_len++] = '_';
            } else {
                overload->key[overload->key_len++] = param->value.type.user != 0 ?
                    param->value.type.user :
                    u8(param->value.type.cls);
                overload->key[overload->key_len++] = u8(param->value.type.size);
                if (!param->has_dft)
                    ++overload->req_count;
                else
                    overload->has_dft = true;
            }
            return status_t::ok;
        }

        overload_t* make(str::slice_t symbol, param_type_t ret_type, u0* func) {
            overload_pair_t pair{};
            if (symtab::find(g_ffi_system.overloads, symbol, pair))
                return pair.overload;
            overload_pair_t* new_pair{};
            if (symtab::emplace(g_ffi_system.overloads, symbol, &new_pair)) {
                auto ol = &stable_array::append(g_ffi_system.overload_slab);
                new_pair->idx      = g_ffi_system.overload_slab.size;
                new_pair->overload = ol;
                ol->func           = func;
                ol->name           = symbol;
                ol->mode           = call_mode_t::system;
                ol->key_len        = {};
                ol->ret_type       = ret_type;
                array::init(ol->params, g_ffi_system.alloc);
                return ol;
            }
            return nullptr;
        }
    }

    namespace system {
        u0 fini() {
            for (auto lib : g_ffi_system.lib_slab)
                lib::unload(lib);
            for (auto param : g_ffi_system.param_slab)
                array::free(param->members);
            for (auto proto : g_ffi_system.proto_slab)
                proto::free(proto);
            for (auto overload : g_ffi_system.overload_slab)
                overload::free(overload);
            symtab::free(g_ffi_system.libs);
            symtab::free(g_ffi_system.protos);
            symtab::free(g_ffi_system.overloads);
            stable_array::free(g_ffi_system.lib_slab);
            stable_array::free(g_ffi_system.proto_slab);
            stable_array::free(g_ffi_system.param_slab);
            stable_array::free(g_ffi_system.overload_slab);
        }

        status_t init(alloc_t* alloc, u8 num_pages) {
            g_ffi_system.alloc = alloc;
            stable_array::init(g_ffi_system.lib_slab, g_ffi_system.alloc, num_pages);
            stable_array::init(g_ffi_system.param_slab, g_ffi_system.alloc, num_pages);
            stable_array::init(g_ffi_system.proto_slab, g_ffi_system.alloc, num_pages);
            stable_array::init(g_ffi_system.overload_slab, g_ffi_system.alloc, num_pages);
            symtab::init(g_ffi_system.libs, g_ffi_system.alloc);
            symtab::init(g_ffi_system.protos, g_ffi_system.alloc);
            symtab::init(g_ffi_system.overloads, g_ffi_system.alloc);
            return status_t::ok;
        }
    }

    u0 free(ffi_t& ffi) {
        WITH_ALLOC(ffi.alloc, if (ffi.vm) dcFree(ffi.vm););
        ffi.vm        = {};
        ffi.heap_size = {};
    }

    u0 reset(ffi_t& ffi) {
        dcReset(ffi.vm);
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
            default:
                break;
        }
        return status_t::ok;
    }

    status_t init(ffi_t& ffi, u32 heap_size, alloc_t* alloc) {
        ffi.alloc     = alloc;
        ffi.heap_size = heap_size;
        WITH_ALLOC(ffi.alloc, ffi.vm = dcNewCallVM(heap_size););
        return status_t::ok;
    }

    status_t call(ffi_t& ffi, const overload_t* ol, param_alias_t& ret) {
        if (!ol) return status_t::prototype_null;
        switch (ol->mode) {
            case call_mode_t::system:           dcMode(ffi.vm, DC_CALL_C_DEFAULT);           break;
            case call_mode_t::variadic:         dcMode(ffi.vm, DC_CALL_C_ELLIPSIS);          break;
        }
        switch (ol->ret_type.cls) {
            case param_cls_t::ptr:              ret.p = dcCallPointer(ffi.vm, ol->func);     break;
            case param_cls_t::int_:
                switch (ol->ret_type.size) {
                    case param_size_t::none:    dcCallVoid(ffi.vm, ol->func); ret.qw = {};   break;
                    case param_size_t::byte:    ret.b = dcCallChar(ffi.vm, ol->func);        break;
                    case param_size_t::word:    ret.w = dcCallShort(ffi.vm, ol->func);       break;
                    case param_size_t::dword:   ret.dw = dcCallInt(ffi.vm, ol->func);        break;
                    case param_size_t::qword:   ret.qw = dcCallLongLong(ffi.vm, ol->func);   break;
                }
                break;
            case param_cls_t::void_:            dcCallVoid(ffi.vm, ol->func); ret.qw = {};   break;
            case param_cls_t::float_:
                switch (ol->ret_type.size) {
                    case param_size_t::dword:   ret.fdw = dcCallFloat(ffi.vm, ol->func);     break;
                    case param_size_t::qword:   ret.fqw = dcCallDouble(ffi.vm, ol->func);    break;
                    default:                    return status_t::invalid_float_size;
                }
                break;
            case param_cls_t::struct_:          return status_t::struct_by_value_not_implemented;
            default:
                break;
        }
        return status_t::ok;
    }
}
