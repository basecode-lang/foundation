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

#include <basecode/core/sql.h>
#include <basecode/core/path.h>
#include <basecode/core/array.h>
#include <basecode/core/string.h>
#include <basecode/core/hashtab.h>
#include <basecode/core/stable_array.h>

namespace basecode {
    [[maybe_unused]] constexpr s32 app_id       = 0x000dead1;

    struct db_t;
    struct txn_t;
    struct tuple_t;
    struct entity_t;

    using txn_list_t            = stable_array_t<txn_t>;
    using tuple_list_t          = array_t<tuple_t>;
    using entity_list_t         = array_t<entity_t>;

    enum class value_type_t : u8 {
        nil,
        list,
        entity,
        string,
        boolean,
        integer,
        floating_point,
    };

    struct entity_t final {
        constexpr entity_t()        : id(0)         {}
        constexpr entity_t(s64 id)  : id(id)        {}
        constexpr operator s64() const              { return id;        }
        [[nodiscard]] constexpr b8 empty() const    { return id == 0;   }
    private:
        s64                     id;
    };

    union value_data_t final {
        entity_t                entity;
        union {
            u64                 u;
            f64                 f;
        }                       number;
        str_t                   string;
        entity_list_t           list;

        ~value_data_t()                                         {}
        value_data_t() : list()                                 {}
        explicit value_data_t(u64 value) : number({.u = value}) {}
        explicit value_data_t(f64 value) : number({.f = value}) {}
        explicit value_data_t(entity_t value) : entity(value)   {}
    };

    struct value_t final {
        value_type_t            type;
        value_data_t            data;

        value_t()          : type(value_type_t::nil), data()                   {}
        value_t(b8 value)  : type(value_type_t::boolean), data(u64(value))     {}
        value_t(s8 value)  : type(value_type_t::integer), data(u64(value))     {}
        value_t(u8 value)  : type(value_type_t::integer), data(u64(value))     {}
        value_t(u64 value) : type(value_type_t::integer), data(value)          {}
        value_t(s16 value) : type(value_type_t::integer), data(u64(value))     {}
        value_t(u16 value) : type(value_type_t::integer), data(u64(value))     {}
        value_t(s32 value) : type(value_type_t::integer), data(u64(value))     {}
        value_t(u32 value) : type(value_type_t::integer), data(u64(value))     {}
        value_t(s64 value) : type(value_type_t::integer), data(u64(value))     {}
        value_t(f64 value) : type(value_type_t::floating_point), data(value)   {}
        value_t(entity_t value) : type(value_type_t::entity), data(value)      {}
        explicit value_t(value_type_t type) : type(type), data()               {}
        value_t(const value_t& other)                                          { operator=(other); }

        constexpr operator u64() const {
            return data.number.u;
        }

        value_t& operator=(const value_t& other) {
            if (this != &other) {
                type = other.type;
                switch (type) {
                    case value_type_t::list: {
                        if (&data.list != &other.data.list) {
                            if (!data.list.alloc)
                                data.list.alloc = other.data.list.alloc;
                            const auto n = other.data.list.size;
                            array::grow(data.list, n);
                            std::memcpy(data.list.data, other.data.list.data, n * sizeof(entity_t));
                            data.list.size = n;
                        }
                        break;
                    }
                    case value_type_t::entity:
                        data.entity = other.data.entity;
                        break;
                    case value_type_t::string:
                        data.string = other.data.string;
                        break;
                    case value_type_t::nil:
                    case value_type_t::boolean:
                    case value_type_t::integer:
                    case value_type_t::floating_point:
                        data.number = other.data.number;
                        break;
                }
            }
            return *this;
        }
    };
    static_assert(sizeof(value_t) <= 32, "value_t is now larger than 32 bytes!");

    struct tuple_t final {
        s64                     rowid;
        entity_t                attr;
        value_t                 value;
    };
    static_assert(sizeof(tuple_t) <= 48, "tuple_t is now larger than 48 bytes!");

    struct simple_stmt_cache_t final {
        sqlite3_stmt*           select      {};
        sqlite3_stmt*           upsert      {};
        sqlite3_stmt*           delete_     {};
    };

    struct entity_stmt_cache_t final {
        sqlite3_stmt*           select      {};
        sqlite3_stmt*           update      {};
        sqlite3_stmt*           insert      {};
    };

    struct tuple_stmt_cache_t final {
        sqlite3_stmt*           insert      {};
        sqlite3_stmt*           select_one  {};
        sqlite3_stmt*           select_all  {};
        sqlite3_stmt*           update_one  {};
        sqlite3_stmt*           update_all  {};
    };

    struct txn_t final {
        db_t*                   db;
        str::slice_t            label;
        u32                     id;
    };

    struct db_t final {
        alloc_t*                alloc       {};
        str_t                   buf;
        path_t                  path;
        sqlite3*                handle      {};
        txn_list_t              txns        {};
        tuple_stmt_cache_t      tuple       {};
        simple_stmt_cache_t     config      {};
        entity_stmt_cache_t     entity      {};
        simple_stmt_cache_t     symbol      {};
    };

    namespace eav {
        enum class status_t : u8 {
            ok,
            error,
            not_found,
            sql_error,
            invalid_rowid,
            invalid_entity,
        };

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

            value_t list(alloc_t* alloc = context::top()->alloc);

            value_t string(alloc_t* alloc = context::top()->alloc);

            u0 string(value_t& value, const s8* data, u32 len, alloc_t* alloc = context::top()->alloc);

            value_t string(const String_Concept auto& str, alloc_t* alloc = context::top()->alloc) {
                value_t value(value_type_t::string);
                string(value, (const s8*) str.data, str.length, alloc);
                return value;
            }

            u0 list(value_t& value, const entity_t* data, u32 len, alloc_t* alloc = context::top()->alloc);
        }

        namespace tuple {
            u0 free(tuple_t& tuple);

            tuple_t nil(entity_t attr);

            tuple_t boolean(entity_t attr, b8 value);

            tuple_t number(entity_t attr, s32 value);

            tuple_t number(entity_t attr, u64 value);

            tuple_t number(entity_t attr, f64 value);

            status_t remove(txn_t& txn, entity_t id);

            tuple_t entity(entity_t attr, entity_t value);

            status_t remove(txn_t& txn, const tuple_t& tuple);

            status_t set(txn_t& txn, entity_t id, tuple_t& tuple);

            status_t get(txn_t& txn, entity_t id, tuple_list_t& tuples);

            status_t get(txn_t& txn, entity_t id, entity_t attr, tuple_t& tuple);

            tuple_t string(entity_t attr, const String_Concept auto& str, alloc_t* alloc = context::top()->alloc) {
                return tuple_t{.attr = attr, .value = value::string(str, alloc)};
            }
        }

        namespace symbol {
            status_t unbind(txn_t& txn, const s8* name, s32 len);

            status_t unbind(txn_t& txn, const String_Concept auto& name) {
                return unbind(txn, (const s8*) name.data, name.length);
            }

            status_t bind(txn_t& txn, const s8* name, s32 len, entity_t id);

            status_t find(txn_t& txn, const s8* name, s32 len, entity_t& id);

            status_t bind(txn_t& txn, const String_Concept auto& name, entity_t id) {
                return bind(txn, (const s8*) name.data, name.length, id);
            }

            status_t find(txn_t& txn, const String_Concept auto& name, entity_t& id) {
                return find(txn, (const s8*) name.data, name.length, id);
            }
        }

        namespace entity {
            namespace status {
                [[maybe_unused]] constexpr u8 dead          = 0;
                [[maybe_unused]] constexpr u8 live          = 1;
            }

            status_t remove(txn_t& txn, entity_t id);

            status_t make(txn_t& txn, entity_t type_id, entity_t& id);

            status_t find(txn_t& txn, entity_t id, entity_t& type_id, u8& status);
        }

        u0 free(db_t& db);

        str::slice_t status_name(status_t status);

        str::slice_t intern_label(db_t& db, u32 id);

        str::slice_t intern_str(db_t& db, const String_Concept auto& value) {
            return string::interned::fold(value);
        }

        status_t init(db_t& db, const path_t& path, alloc_t* alloc = context::top()->alloc);
    }
}
