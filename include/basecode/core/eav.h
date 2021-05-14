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

#include <basecode/core/string.h>

namespace basecode::eav {
    namespace txn {
        status_t begin(db_t& db, txn_t& txn);

        status_t commit(db_t& db, txn_t& txn);

        status_t rollback(db_t& db, txn_t& txn);
    }

    namespace value {
        value_t nil();

        value_t b8_(b8 value);

        u0 free(value_t& value);

        value_t u64_(u64 value);

        value_t f64_(f64 value);

        value_t entity(entity_t value);

        u0 list(value_t& value,
                const entity_t* data,
                u32 len,
                alloc_t* alloc = context::top()->alloc.main);

        u0 string(value_t& value,
                  const s8* data,
                  u32 len,
                  alloc_t* alloc = context::top()->alloc.main);

        value_t list(alloc_t* alloc = context::top()->alloc.main);

        value_t string(alloc_t* alloc = context::top()->alloc.main);

        template <String_Concept T>
        value_t string(const T& str,
                       alloc_t* alloc = context::top()->alloc.main) {
            value_t value(value_type_t::string);
            string(value, (const s8*) str.data, str.length, alloc);
            return value;
        }
    }

    namespace tuple {
        u0 free(tuple_t& tuple);

        tuple_t nil(entity_t attr);

        status_t get(txn_t& txn,
                     entity_t id,
                     entity_t attr,
                     tuple_t& tuple);

        tuple_t boolean(entity_t attr, b8 value);

        tuple_t number(entity_t attr, s32 value);

        tuple_t number(entity_t attr, u64 value);

        tuple_t number(entity_t attr, f64 value);

        status_t remove(txn_t& txn, entity_t id);

        tuple_t entity(entity_t attr, entity_t value);

        status_t remove(txn_t& txn, const tuple_t& tuple);

        status_t set(txn_t& txn, entity_t id, tuple_t& tuple);

        status_t get(txn_t& txn, entity_t id, tuple_list_t& tuples);

        template <String_Concept T>
        tuple_t string(entity_t attr,
                       const T& str,
                       alloc_t* alloc = context::top()->alloc.main) {
            return tuple_t{.attr = attr,
                           .value = value::string(str, alloc)};
        }
    }

    namespace symbol {
        status_t find(txn_t& txn,
                      const s8* name,
                      s32 len,
                      entity_t& id);

        status_t bind(txn_t& txn,
                      const s8* name,
                      s32 len,
                      entity_t id);

        template <String_Concept T>
        status_t bind(txn_t& txn,
                      const T& name,
                      entity_t id) {
            return bind(txn, (const s8*) name.data, name.length, id);
        }

        template <String_Concept T>
        status_t find(txn_t& txn,
                      const T& name,
                      entity_t& id) {
            return find(txn, (const s8*) name.data, name.length, id);
        }

        template <String_Concept T>
        status_t unbind(txn_t& txn, const T& name) {
            return unbind(txn, (const s8*) name.data, name.length);
        }

        status_t unbind(txn_t& txn, const s8* name, s32 len);
    }

    namespace entity {
        status_t find(txn_t& txn,
                      entity_t id,
                      entity_t& type_id,
                      u8& status);

        status_t remove(txn_t& txn, entity_t id);

        status_t make(txn_t& txn, entity_t type_id, entity_t& id);
    }

    u0 free(db_t& db);

    str::slice_t intern_label(db_t& db, u32 id);

    template <String_Concept T>
    str::slice_t intern_str(db_t& db, const T& value) {
        return string::interned::fold(value);
    }

    status_t init(db_t& db,
                  const path_t& path,
                  alloc_t* alloc = context::top()->alloc.main);
}
