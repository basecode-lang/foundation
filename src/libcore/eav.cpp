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

#include <sqlite3.h>
#include <basecode/core/log.h>
#include <basecode/core/eav.h>
#include <basecode/core/defer.h>

namespace basecode {
    [[maybe_unused]] static const s8* s_schema_sql =
        "create table if not exists config("
        "   name text not null primary key, "
        "   value blob null, "
        "   created_at integer(4) not null default (strftime('%s', 'now')), "
        "   updated_at integer(4) null); "
        "create table if not exists symbol("
        "   name text not null primary key, "
        "   entity integer not null, "
        "   created_at integer(4) not null default (strftime('%s', 'now')), "
        "   updated_at integer(4) null); "
        "create table if not exists entity("
        "   id integer primary key, "
        "   type integer not null, "
        "   status integer not null default 1, "
        "   created_at integer(4) not null default (strftime('%s', 'now')), "
        "   updated_at integer(4) null); "
        "create table if not exists tuple("
        "   entity integer not null, "
        "   attr integer not null, "
        "   value blob null, "
        "   status integer not null default 1, "
        "   created_at integer(4) not null default (strftime('%s', 'now')), "
        "   updated_at integer(4) null, "
        "   primary key (entity, attr, value)); "
        "create index if not exists idx_entity_type on entity(type); "
        "create index if not exists idx_entity_status on entity(status); "
        "create index if not exists idx_tuple_status on tuple(status); "
        "create index if not exists idx_symbol_entity on symbol(entity); "
        ;

    [[maybe_unused]] static const s8* s_select_config =
        "select value from config where name = ?;"
        ;

    [[maybe_unused]] static const s8* s_upsert_config =
        "insert into config (name, value) values (?, ?) "
        "on conflict (name) do "
        "update set value = excluded.value, updated_at = strftime('%s', 'now');"
        ;

    [[maybe_unused]] static const s8* s_select_symbol =
        "select entity from symbol where name = ?;"
        ;

    [[maybe_unused]] static const s8* s_upsert_symbol =
        "insert into symbol (name, entity) values (?, ?) "
        "on conflict (name) do "
        "update set entity = excluded.entity, updated_at = strftime('%s', 'now');"
        ;

    [[maybe_unused]] static const s8* s_delete_symbol =
        "delete from symbol where name = ?;"
        ;

    [[maybe_unused]] static const s8* s_select_entity =
        "select type, status from entity where id = ?;"
        ;

    [[maybe_unused]] static const s8* s_insert_entity =
        "insert into entity (type, status) values (?, ?);"
        ;

    [[maybe_unused]] static const s8* s_update_entity =
        "update entity set status = ?, updated_at = strftime('%s', 'now') where id = ?;"
        ;

    [[maybe_unused]] static const s8* s_select_tuple_all =
        "select rowid, attr, value from tuple where entity = ? and status = 1;"
        ;

    [[maybe_unused]] static const s8* s_select_tuple_one =
        "select rowid, value from tuple where entity = ? and attr = ? and status = 1;"
        ;

    [[maybe_unused]] static const s8* s_insert_tuple =
        "insert into tuple (entity, attr, value) values (?, ?, ?);"
        ;

    [[maybe_unused]] static const s8* s_update_tuple_one =
        "update tuple set status = ?, updated_at = strftime('%s', 'now') where rowid = ?;"
        ;

    [[maybe_unused]] static const s8* s_update_tuple_all =
        "update tuple set status = ?, updated_at = strftime('%s', 'now') where entity = ?;"
        ;

    namespace eav {
        namespace txn {
            status_t begin(db_t& db, txn_t& txn) {
                auto is_first = stable_array::empty(db.txns);
                txn = stable_array::append(db.txns);
                txn.db    = &db;
                txn.id    = db.txns.size;
                txn.label = intern_label(db, db.txns.size);
                if (is_first)
                    SQL_BEGIN(db.handle);
                SQL_SAVEPOINT(db.handle, txn.label);
                return status_t::ok;
            }

            status_t commit(db_t& db, txn_t& txn) {
                SQL_RELEASE(db.handle, txn.label);
                if (txn.id == 1)
                    SQL_COMMIT(db.handle);
                for (u32 i = txn.id - 1; i < db.txns.size; ++i)
                    str::free(db.txns[i].label);
                stable_array::truncate(db.txns, txn.id - 1);
                return status_t::ok;
            }

            status_t rollback(db_t& db, txn_t& txn) {
                if (txn.id == 1) {
                    SQL_ROLLBACK(db.handle);
                } else {
                    SQL_ROLLBACK_TO(db.handle, txn.label);
                }
                for (u32 i = txn.id - 1; i < db.txns.size; ++i)
                    str::free(db.txns[i].label);
                stable_array::truncate(db.txns, txn.id - 1);
                return status_t::ok;
            }
        }

        namespace value {
            value_t nil() {
                return value_t(value_type_t::nil);
            }

            value_t b8_(b8 value) {
                return value_t(value);
            }

            u0 free(value_t& value) {
                switch (value.type) {
                    case value_type_t::list:
                        array::free(value.data.list);
                        break;
                    case value_type_t::string:
                        str::free(value.data.string);
                        break;
                    default: break;
                }
            }

            value_t f64_(f64 value) {
                return value_t(value);
            }

            value_t u64_(u64 value) {
                return value_t(value);
            }

            value_t list(alloc_t* alloc) {
                value_t value(value_type_t::list);
                array::init(value.data.list, alloc);
                return value;
            }

            value_t entity(entity_t value) {
                return value_t(value);
            }

            value_t string(alloc_t* alloc) {
                value_t value(value_type_t::string);
                str::init(value.data.string, alloc);
                return value;
            }

            u0 string(value_t& value, const s8* data, u32 len, alloc_t* alloc) {
                str::init(value.data.string, alloc);
                str::resize(value.data.string, len);
                std::memcpy(value.data.string.data, data, len);
            }

            u0 list(value_t& value, const entity_t* data, u32 len, alloc_t* alloc) {
                array::init(value.data.list, alloc);
                array::resize(value.data.list, len);
                std::memcpy(value.data.list.data, data, sizeof(entity_t) * len);
            }
        }

        namespace tuple {
            u0 free(tuple_t& tuple) {
                value::free(tuple.value);
            }

            tuple_t nil(entity_t attr) {
                return tuple_t{.attr = attr, .value = value::nil()};
            }

            tuple_t boolean(entity_t attr, b8 value) {
                return tuple_t{.attr = attr, .value = value::b8_(value)};
            }

            tuple_t number(entity_t attr, s32 value) {
                return tuple_t{.attr = attr, .value = value::u64_(value)};
            }

            tuple_t number(entity_t attr, u64 value) {
                return tuple_t{.attr = attr, .value = value::u64_(value)};
            }

            tuple_t number(entity_t attr, f64 value) {
                return tuple_t{.attr = attr, .value = value::f64_(value)};
            }

            status_t remove(txn_t& txn, entity_t id) {
                if (id.empty()) return status_t::invalid_entity;
                status_t status{};
                auto stmt = txn.db->tuple.update_all;
                sqlite3_bind_int(stmt, 1, entity::status::dead);
                sqlite3_bind_int64(stmt, 2, id);
                auto rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE)
                    status = status_t::sql_error;
                sqlite3_clear_bindings(stmt);
                sqlite3_reset(stmt);
                return status;
            }

            tuple_t entity(entity_t attr, entity_t value) {
                return tuple_t{.attr = attr, .value = value::entity(value)};
            }

            status_t remove(txn_t& txn, const tuple_t& tuple) {
                if (tuple.rowid <= 0)
                    return status_t::invalid_rowid;
                status_t status{};
                auto stmt = txn.db->tuple.update_one;
                sqlite3_bind_int(stmt, 1, entity::status::dead);
                sqlite3_bind_int64(stmt, 2, tuple.rowid);
                auto rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE)
                    status = status_t::sql_error;
                sqlite3_clear_bindings(stmt);
                sqlite3_reset(stmt);
                return status;
            }

            status_t set(txn_t& txn, entity_t id, tuple_t& tuple) {
                if (id.empty()) return status_t::invalid_entity;
                status_t status{};
                auto stmt = txn.db->tuple.insert;
                sqlite3_bind_int64(stmt, 1, id);
                sqlite3_bind_int64(stmt, 2, tuple.attr);
                auto& buf = txn.db->buf;
                auto buf_size = 1 + sizeof(u64);
                switch (value_type_t(tuple.value.type)) {
                    case value_type_t::list: {
                        auto& list = tuple.value.data.list;
                        u64 size = list.size;
                        buf_size += size * sizeof(entity_t);
                        str::resize(buf, buf_size);
                        *buf.data = u8(tuple.value.type);
                        std::memcpy(buf.data + 1, &size, sizeof(u64));
                        std::memcpy(buf.data + 1 + sizeof(u64), list.data, size * sizeof(entity_t));
                        break;
                    }
                    case value_type_t::string: {
                        auto& string = tuple.value.data.string;
                        u64 size = string.length;
                        buf_size += size;
                        str::resize(buf, buf_size);
                        *buf.data = u8(tuple.value.type);
                        std::memcpy(buf.data + 1, &size, sizeof(u64));
                        std::memcpy(buf.data + 1 + sizeof(u64), string.data, size);
                        break;
                    }
                    case value_type_t::entity: {
                        str::resize(buf, buf_size);
                        *buf.data = u8(tuple.value.type);
                        std::memcpy(buf.data + 1, &tuple.value.data.entity, sizeof(entity_t));
                        break;
                    }
                    default: {
                        str::resize(buf, buf_size);
                        *buf.data = u8(tuple.value.type);
                        std::memcpy(buf.data + 1, &tuple.value.data.number.u, sizeof(u64));
                        break;
                    }
                }
                sqlite3_bind_blob(stmt, 3, buf.data, buf_size, nullptr);
                auto rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE) {
                    status = status_t::sql_error;
                } else {
                    tuple.rowid = sqlite3_last_insert_rowid(txn.db->handle);
                }
                sqlite3_clear_bindings(stmt);
                sqlite3_reset(stmt);
                return status;
            }

            status_t get(txn_t& txn, entity_t id, tuple_list_t& tuples) {
                if (id.empty()) return status_t::invalid_entity;
                status_t status{};
                auto stmt = txn.db->tuple.select_all;
                sqlite3_bind_int64(stmt, 1, id);
                while (true) {
                    auto rc = sqlite3_step(stmt);
                    if (rc == SQLITE_DONE) {
                        if (array::empty(tuples))
                            status = status_t::not_found;
                        break;
                    } else if (rc == SQLITE_ROW) {
                        auto& tuple = array::append(tuples);
                        tuple.rowid = sqlite3_column_int64(stmt, 0);
                        tuple.attr  = sqlite3_column_int64(stmt, 1);
                        auto blob   = (const u8*) sqlite3_column_blob(stmt, 2);
                        if (blob) {
                            tuple.value.type = value_type_t(*blob);
                            switch (value_type_t(tuple.value.type)) {
                                case value_type_t::list: {
                                    u64 size{};
                                    std::memcpy(&size, blob + 1, sizeof(u64));
                                    value::list(tuple.value, (const entity_t*) blob + 1 + sizeof(u64), size, txn.db->alloc);
                                    break;
                                }
                                case value_type_t::string: {
                                    u64 size{};
                                    std::memcpy(&size, blob + 1, sizeof(u64));
                                    value::string(tuple.value, (const s8*) blob + 1 + sizeof(u64), size, txn.db->alloc);
                                    break;
                                }
                                case value_type_t::entity: {
                                    std::memcpy(&tuple.value.data.entity, blob + 1, sizeof(u64));
                                    break;
                                }
                                default: {
                                    std::memcpy(&tuple.value.data.number.u, blob + 1, sizeof(u64));
                                    break;
                                }
                            }
                        } else {
                            tuple.value.type = value_type_t::nil;
                        }
                        break;
                    } else {
                        status = status_t::sql_error;
                        break;
                    }
                }
                sqlite3_clear_bindings(stmt);
                sqlite3_reset(stmt);
                return status;
            }

            status_t get(txn_t& txn, entity_t id, entity_t attr, tuple_t& tuple) {
                if (id.empty() || attr.empty()) return status_t::invalid_entity;
                status_t status{};
                auto stmt = txn.db->tuple.select_one;
                sqlite3_bind_int64(stmt, 1, id);
                sqlite3_bind_int64(stmt, 2, attr);
                while (true) {
                    auto rc = sqlite3_step(stmt);
                    if (rc == SQLITE_DONE) {
                        status = status_t::not_found;
                        break;
                    } else if (rc == SQLITE_ROW) {
                        tuple.rowid = sqlite3_column_int64(stmt, 0);
                        tuple.attr  = sqlite3_column_int64(stmt, 1);
                        auto blob   = (const u8*) sqlite3_column_blob(stmt, 2);
                        if (blob) {
                            tuple.value.type = value_type_t(*blob);
                            switch (value_type_t(tuple.value.type)) {
                                case value_type_t::list: {
                                    u64 size{};
                                    std::memcpy(&size, blob + 1, sizeof(u64));
                                    value::list(tuple.value, (const entity_t*) blob + 1 + sizeof(u64), size, txn.db->alloc);
                                    break;
                                }
                                case value_type_t::string: {
                                    u64 size{};
                                    std::memcpy(&size, blob + 1, sizeof(u64));
                                    value::string(tuple.value, (const s8*) blob + 1 + sizeof(u64), size, txn.db->alloc);
                                    break;
                                }
                                case value_type_t::entity: {
                                    std::memcpy(&tuple.value.data.entity, blob + 1, sizeof(u64));
                                    break;
                                }
                                default: {
                                    std::memcpy(&tuple.value.data.number.u, blob + 1, sizeof(u64));
                                    break;
                                }
                            }
                        } else {
                            tuple.value.type = value_type_t::nil;
                        }
                        break;
                    } else {
                        status = status_t::sql_error;
                        break;
                    }
                }
                sqlite3_clear_bindings(stmt);
                sqlite3_reset(stmt);
                return status;
            }
        }

        namespace symbol {
            status_t unbind(txn_t& txn, const s8* name, s32 len) {
                status_t status{};
                auto stmt = txn.db->symbol.delete_;
                sqlite3_bind_text(stmt, 1, name, len, nullptr);
                auto rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE)
                    status = status_t::sql_error;
                sqlite3_clear_bindings(stmt);
                sqlite3_reset(stmt);
                return status;
            }

            status_t bind(txn_t& txn, const s8* name, s32 len, entity_t id) {
                status_t status{};
                auto stmt = txn.db->symbol.upsert;
                sqlite3_bind_text(stmt, 1, name, len, nullptr);
                sqlite3_bind_int64(stmt, 2, id);
                auto rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE)
                    status = status_t::sql_error;
                sqlite3_clear_bindings(stmt);
                sqlite3_reset(stmt);
                return status;
            }

            status_t find(txn_t& txn, const s8* name, s32 len, entity_t& id) {
                status_t status{};
                auto stmt = txn.db->symbol.select;
                sqlite3_bind_text(stmt, 1, name, len, nullptr);
                while (true) {
                    auto rc = sqlite3_step(stmt);
                    if (rc == SQLITE_DONE) {
                        status = status_t::not_found;
                        break;
                    } else if (rc == SQLITE_ROW) {
                        id = sqlite3_column_int64(stmt, 0);
                        break;
                    } else {
                        status = status_t::sql_error;
                    }
                }
                sqlite3_clear_bindings(stmt);
                sqlite3_reset(stmt);
                return status;
            }
        }

        namespace entity {
            status_t remove(txn_t& txn, entity_t id) {
                status_t status{};
                auto stmt = txn.db->entity.update;
                sqlite3_bind_int(stmt, 1, status::dead);
                sqlite3_bind_int64(stmt, 2, id);
                auto rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE)
                    status = status_t::sql_error;
                sqlite3_clear_bindings(stmt);
                sqlite3_reset(stmt);
                return status;
            }

            status_t make(txn_t& txn, entity_t type_id, entity_t& id) {
                status_t status{};
                auto stmt = txn.db->entity.insert;
                sqlite3_bind_int64(stmt, 1, type_id);
                sqlite3_bind_int(stmt, 2, status::live);
                auto rc = sqlite3_step(stmt);
                if (rc != SQLITE_DONE)
                    status = status_t::sql_error;
                id = sqlite3_last_insert_rowid(txn.db->handle);
                sqlite3_clear_bindings(stmt);
                sqlite3_reset(stmt);
                return status;
            }

            status_t find(txn_t& txn, entity_t id, entity_t& type_id, u8& status) {
                if (id.empty()) return status_t::invalid_entity;
                status_t result{};
                auto     stmt = txn.db->entity.select;
                sqlite3_bind_int64(stmt, 1, id);
                while (true) {
                    auto rc = sqlite3_step(stmt);
                    if (rc == SQLITE_DONE) {
                        result = status_t::not_found;
                        break;
                    } else if (rc == SQLITE_ROW) {
                        type_id = sqlite3_column_int64(stmt, 0);
                        status  = sqlite3_column_int(stmt, 1);
                        break;
                    } else {
                        result = status_t::sql_error;
                        break;
                    }
                }
                sqlite3_clear_bindings(stmt);
                sqlite3_reset(stmt);
                return result;
            }
        }

        static status_t create_schema(db_t& db) {
            auto rc = sqlite3_exec(db.handle,
                                   s_schema_sql,
                                   nullptr, nullptr,
                                   nullptr);
            return rc == SQLITE_OK ? status_t::ok : status_t::sql_error;
        }

        static status_t prepare_statements(db_t& db) {
            SQL_PREPARE_STMT(db.handle, s_upsert_config,    db.config.upsert);
            SQL_PREPARE_STMT(db.handle, s_select_symbol,    db.symbol.select);
            SQL_PREPARE_STMT(db.handle, s_upsert_symbol,    db.symbol.upsert);
            SQL_PREPARE_STMT(db.handle, s_delete_symbol,    db.symbol.delete_);
            SQL_PREPARE_STMT(db.handle, s_select_entity,    db.entity.select);
            SQL_PREPARE_STMT(db.handle, s_insert_entity,    db.entity.insert);
            SQL_PREPARE_STMT(db.handle, s_update_entity,    db.entity.update);
            SQL_PREPARE_STMT(db.handle, s_select_tuple_one, db.tuple.select_one);
            SQL_PREPARE_STMT(db.handle, s_select_tuple_all, db.tuple.select_all);
            SQL_PREPARE_STMT(db.handle, s_insert_tuple,     db.tuple.insert);
            SQL_PREPARE_STMT(db.handle, s_update_tuple_one, db.tuple.update_one);
            SQL_PREPARE_STMT(db.handle, s_update_tuple_all, db.tuple.update_all);
            return status_t::ok;
        }

        u0 free(db_t& db) {
            str::free(db.buf);
            path::free(db.path);
            stable_array::free(db.txns);
            if (db.handle) {
                SQL_FINALIZE_STMT(db.tuple.update_all);
                SQL_FINALIZE_STMT(db.tuple.update_one);
                SQL_FINALIZE_STMT(db.tuple.insert);
                SQL_FINALIZE_STMT(db.tuple.select_all);
                SQL_FINALIZE_STMT(db.tuple.select_one);
                SQL_FINALIZE_STMT(db.entity.update);
                SQL_FINALIZE_STMT(db.entity.insert);
                SQL_FINALIZE_STMT(db.entity.select);
                SQL_FINALIZE_STMT(db.symbol.delete_);
                SQL_FINALIZE_STMT(db.symbol.upsert);
                SQL_FINALIZE_STMT(db.symbol.select);
                SQL_FINALIZE_STMT(db.config.upsert);
                SQL_FINALIZE_STMT(db.config.select);
                sqlite3_close(db.handle);
                db.handle = {};
            }
        }

        str::slice_t intern_label(db_t& db, u32 id) {
            str::reset(db.buf);
            {
                str_buf_t fmt_buf(&db.buf);
                format::format_to(fmt_buf, "SP_{}", id);
            }
            return string::interned::fold(db.buf);
        }

        status_t init(db_t& db, const path_t& path, alloc_t* alloc) {
            db.alloc = alloc;
            path::init(db.path, db.alloc);
            path::set(db.path, path.str);
            str::init(db.buf, db.alloc);
            str::reserve(db.buf, 64);
            stable_array::init(db.txns, db.alloc);

            status_t status{};
            defer({
                if (!OK(status))
                    log::error("eav::init failed with sql error: {}", sqlite3_errmsg(db.handle));
            });

            auto rc = sqlite3_open_v2(path::c_str(db.path),
                                      &db.handle,
                                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                      nullptr);
            if (rc != SQLITE_OK)
                return status_t::sql_error;

            SQL_BEGIN(db.handle);
            SQL_PRAGMA(db.handle, "application_id", app_id);
            SQL_PRAGMA(db.handle, "journal_mode", "truncate");
            status = create_schema(db);
            if (!OK(status)) return status;
            status = prepare_statements(db);
            if (!OK(status)) return status;
            SQL_COMMIT(db.handle);
            return status_t::ok;
        }
    }
}
