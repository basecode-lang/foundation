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

#include <basecode/core/ffi.h>
#include <basecode/core/symtab.h>
#include <basecode/core/hashtab.h>

namespace basecode::ffi {
    struct lib_pair_t;
    struct proto_pair_t;
    struct overload_pair_t;

    using lib_slab_t            = stable_array_t<lib_t>;
    using lib_table_t           = hashtab_t<str::slice_t, lib_pair_t>;
    using param_slab_t          = stable_array_t<param_t>;
    using proto_slab_t          = stable_array_t<proto_t>;
    using proto_table_t         = hashtab_t<str::slice_t, proto_pair_t>;
    using overload_slab_t       = stable_array_t<overload_t>;
    using overload_table_t      = hashtab_t<str::slice_t, overload_pair_t>;

    struct lib_pair_t final {
        lib_t*                  lib;
        u32                     idx;
    } __attribute__((aligned(16)));

    struct overload_pair_t final {
        overload_t*             overload;
        u32                     idx;
    } __attribute__((aligned(16)));

    struct proto_pair_t final {
        proto_t*                proto;
        u32                     idx;
    } __attribute__((aligned(16)));

    struct signature_pair_t final {
        overload_t*     overload;
        const str_t*    signature;

        inline auto operator<=>(const signature_pair_t& rhs) const {
            return signature->operator<=>(*rhs.signature);
        }
    };

    struct system_t final {
        alloc_t*                alloc;
        lib_table_t             libs;
        proto_table_t           protos;
        overload_table_t        overloads;
        lib_slab_t              lib_slab;
        param_slab_t            param_slab;
        proto_slab_t            proto_slab;
        overload_slab_t         overload_slab;
    } __attribute__((aligned(128)));

    system_t                    g_ffi_system;

    namespace lib {
        status_t unload(lib_t* lib) {
            if (!lib)
                return status_t::ok;
            symtab::free(lib->symbols);
            if (lib->handle)
                dlFreeLibrary(lib->handle);
            const auto path_slice = slice::make(lib->path.str);
            auto pair = hashtab::find(g_ffi_system.libs, path_slice);
            if (pair) {
                stable_array::erase(g_ffi_system.lib_slab,
                                    pair->idx - 1);
                hashtab::remove(g_ffi_system.libs, path_slice);
                path::free(lib->path);
                return status_t::ok;
            }
            return status_t::lib_not_loaded;
        }

        status_t load(const path_t& path, lib_t** lib) {
            const auto path_slice = slice::make(path);
            auto pair = hashtab::find(g_ffi_system.libs, path_slice);
            if (pair) {
                *lib = pair->lib;
                return status_t::ok;
            }
            auto [new_pair, added] = hashtab::emplace2(g_ffi_system.libs,
                                                       path_slice);
            if (added) {
                new_pair->lib = &stable_array::append(g_ffi_system.lib_slab);
                new_pair->idx = g_ffi_system.libs.size;
                path::init(new_pair->lib->path,
                           path_slice,
                           g_ffi_system.alloc);
                new_pair->lib->alloc  = g_ffi_system.alloc;
                new_pair->lib->handle = dlLoadLibrary(path::c_str(path));
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
            WITH_SLICE_AS_CSTR(name,
                               *address = dlFindSymbol(lib->handle, name););
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

        param_t* make(str::slice_t name,
                      param_type_t type,
                      b8 rest,
                      param_alias_t* dft_val) {
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
            for (auto ol : proto->overloads)
                overload::free(ol);
            array::free(proto->overloads);
        }

        b8 remove(str::slice_t symbol) {
            auto pair = hashtab::find(g_ffi_system.protos, symbol);
            if (pair) {
                proto::free(pair->proto);
                stable_array::erase(g_ffi_system.proto_slab,
                                    pair->idx - 1);
                hashtab::remove(g_ffi_system.protos, symbol);
                return true;
            }
            return false;
        }

        proto_t* find(str::slice_t symbol) {
            auto pair = hashtab::find(g_ffi_system.protos, symbol);
            return pair ? pair->proto : nullptr;
        }

        proto_t* make(str::slice_t symbol, lib_t* lib) {
            auto pair = hashtab::find(g_ffi_system.protos, symbol);
            if (pair)
                return pair->proto;
            auto [new_pair, added] = hashtab::emplace2(g_ffi_system.protos,
                                                       symbol);
            if (added) {
                auto proto = &stable_array::append(g_ffi_system.proto_slab);
                new_pair->idx   = g_ffi_system.proto_slab.size;
                new_pair->proto = proto;
                proto->lib      = lib;
                proto->name     = symbol;
                proto->min_req  = {};
                proto->max_req  = {};
                array::init(proto->overloads, g_ffi_system.alloc);
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
            if (ol->signature.length == 0)
                str::append(ol->signature, u8(param_cls_t::void_));
            array::append(proto->overloads, ol);
            if (ol->params.size > proto->min_req)
                proto->min_req = ol->params.size;
            if (ol->params.size > proto->max_req)
                proto->max_req = ol->params.size;
            return status_t::ok;
        }

        overload_t* match_signature(proto_t* proto, const str_t& candidate) {
            if (array::empty(proto->overloads))
                return nullptr;

            if (proto->overloads.size == 1)
                return proto->overloads[0];

            array_t<signature_pair_t> signatures{};
            array::init(signatures, g_ffi_system.alloc);
            defer(array::free(signatures));
            /* add the candidate */ {
                auto& pair = array::append(signatures);
                pair.overload = {};
                pair.signature = &candidate;
            }
            for (auto ol : proto->overloads) {
                auto& pair = array::append(signatures);
                pair.overload  = ol;
                pair.signature = &ol->signature;
            }
            std::sort(signatures.begin(), signatures.end());
            s32 idx = -1;
            for (u32 i = 0; i < signatures.size; ++i) {
                if (!signatures[i].overload) {
                    idx = i;
                    break;
                }
            }
            if (idx == -1)
                return nullptr;
            const auto test = &signatures[idx];
            const signature_pair_t* prev{};
            const signature_pair_t* next{};
            if (idx == signatures.size - 1) {
                prev = &signatures[idx - 1];
            } else if (idx == 0) {
                next = &signatures[idx + 1];
            } else {
                prev = &signatures[idx - 1];
                next = &signatures[idx + 1];
            }
            if (prev
            &&  (*(prev->signature) == *(test->signature)
                ||  str::back(*(prev->signature)) == '_')) {
                return prev->overload;
            }
            if (next
            &&  (*(next->signature) == *(test->signature)
                ||  str::back(*(next->signature)) == '_')) {
                return next->overload;
            }
            return nullptr;
        }
    }

    namespace overload {
        u0 free(overload_t* ol) {
            for (auto param : ol->params)
                param::free(param);
            array::free(ol->params);
            str::free(ol->signature);
        }

        b8 remove(str::slice_t symbol) {
            auto pair = hashtab::find(g_ffi_system.overloads, symbol);
            if (pair) {
                auto ol = pair->overload;
                array::erase(ol->proto->overloads, ol);
                overload::free(ol);
                stable_array::erase(g_ffi_system.overload_slab,
                                    pair->idx - 1);
                hashtab::remove(g_ffi_system.overloads, symbol);
                return true;
            }
            return false;
        }

        overload_t* find(str::slice_t symbol) {
            auto pair = hashtab::find(g_ffi_system.overloads, symbol);
            return pair ? pair->overload : nullptr;
        }

        status_t append(overload_t* overload, param_t* param) {
            array::append(overload->params, param);
            if (overload->has_rest && param->is_rest)
                return status_t::only_one_rest_param_allowed;
            if (param->is_rest) {
                overload->has_rest = true;
                str::append(overload->signature, '_');
            } else {
                str::append(overload->signature,
                            param->value.type.user != 0 ? param->value.type.user :
                            u8(param->value.type.cls));
                str::append(overload->signature, u8(param->value.type.size));
                if (!param->has_dft)
                    ++overload->req_count;
                else
                    overload->has_dft = true;
            }
            return status_t::ok;
        }

        overload_t* make(str::slice_t symbol, param_type_t ret_type, u0* func) {
            auto pair = hashtab::find(g_ffi_system.overloads, symbol);
            if (pair)
                return pair->overload;
            auto [new_pair, added ] = hashtab::emplace2(g_ffi_system.overloads,
                                                        symbol);
            if (added) {
                auto ol = &stable_array::append(g_ffi_system.overload_slab);
                new_pair->idx      = g_ffi_system.overload_slab.size;
                new_pair->overload = ol;
                ol->func           = func;
                ol->name           = symbol;
                ol->mode           = call_mode_t::system;
                ol->ret_type       = ret_type;
                str::init(ol->signature, g_ffi_system.alloc);
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
            hashtab::free(g_ffi_system.libs);
            hashtab::free(g_ffi_system.protos);
            hashtab::free(g_ffi_system.overloads);
            stable_array::free(g_ffi_system.lib_slab);
            stable_array::free(g_ffi_system.proto_slab);
            stable_array::free(g_ffi_system.param_slab);
            stable_array::free(g_ffi_system.overload_slab);
        }

        status_t init(alloc_t* alloc, u8 num_pages) {
            g_ffi_system.alloc = alloc;
            stable_array::init(g_ffi_system.lib_slab,
                               g_ffi_system.alloc,
                               num_pages);
            stable_array::init(g_ffi_system.param_slab,
                               g_ffi_system.alloc,
                               num_pages);
            stable_array::init(g_ffi_system.proto_slab,
                               g_ffi_system.alloc,
                               num_pages);
            stable_array::init(g_ffi_system.overload_slab,
                               g_ffi_system.alloc,
                               num_pages);
            hashtab::init(g_ffi_system.libs, g_ffi_system.alloc);
            hashtab::init(g_ffi_system.protos, g_ffi_system.alloc);
            hashtab::init(g_ffi_system.overloads, g_ffi_system.alloc);
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
