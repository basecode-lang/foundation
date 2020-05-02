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

#include "bass.h"
#include "memory/bump_system.h"
#include "memory/page_system.h"
#include "memory/proxy_system.h"

namespace basecode {
    static string::slice_t s_kinds[] = {
        "none"_ss,
        "blob"_ss,
        "field"_ss,
        "header"_ss,
    };

    string::slice_t kind::name(u8 value) {
        return s_kinds[value];
    }

    namespace bass {
        namespace dict {
            field_dict_t make(cursor_t& cursor) {
                field_dict_t dict{};
                u32 i{};
                u32 value{};
                while (next_field(cursor, value) && i < 32) {
                    dict.values[cursor.field->type] = value;
                    ++i;
                }
                reset_cursor(cursor);
                return dict;
            }

            u32 get(const field_dict_t& dict, u8 key) {
                return dict.values[key];
            }

            u0 set(field_dict_t& dict, u8 key, u32 value) {
                dict.values[key] = value;
            }
        }

        static b8 move_next(cursor_t& cursor, u8 type = 0) {
            if (!cursor.header)
                return false;
            cursor.offset += sizeof(field_t);
            while (cursor.offset < cursor.end_offset) {
                cursor.field = (field_t*) cursor.page + cursor.offset;
                if (cursor.field->kind != kind::header
                && (!type || cursor.field->type == type)) {
                    return true;
                }
                cursor.offset += sizeof(field_t);
            }
            cursor.field = {};
            return false;
        }

        u0 free(bass_t& storage) {
            hashtable::free(storage.index);
            memory::system::free(memory::unwrap(storage.bump_alloc));
            memory::system::free(memory::unwrap(storage.page_alloc));
        }

        b8 next_header(cursor_t& cursor) {
            const auto page_size = memory::page::size(cursor.storage->page_alloc);
            if (cursor.offset + sizeof(field_t) >= page_size) {
                u8* next_page = (u8*) ((page_header_t*) cursor.page)->next;
                if (!next_page) {
                    cursor.ok = false;
                    return false;
                }
                cursor.offset = {};
                cursor.page = next_page;
            } else {
                cursor.offset += cursor.end_offset - cursor.offset;
            }
            cursor.field = {};
            cursor.header = (field_t*) cursor.page + cursor.offset;
            cursor.end_offset = cursor.offset + cursor.header->value;
            cursor.ok = cursor.header && cursor.header->kind == kind::header;
            return cursor.ok;
        }

        u0 reset_cursor(cursor_t& cursor) {
            cursor.offset = cursor.start_offset;
        }

        cursor_t make_cursor(bass_t& storage) {
            cursor_t cursor{};
            cursor.id = {};
            cursor.ok = true;
            cursor.field = {};
            cursor.header = {};
            cursor.storage = &storage;
            cursor.page = (u8*) memory::bump::buf(storage.bump_alloc);
            cursor.offset = cursor.start_offset = memory::bump::offset(storage.bump_alloc);
            return cursor;
        }

        cursor_t first_header(bass_t& storage) {
            cursor_t cursor{};
            cursor.id = {};
            cursor.ok = true;
            cursor.field = {};
            cursor.storage = &storage;
            cursor.page = (u8*) memory::page::tail(storage.page_alloc);
            cursor.header = (field_t*) cursor.page;
            cursor.end_offset = cursor.header->value;
            cursor.offset = cursor.start_offset = {};
            return cursor;
        }

        cursor_t get_header(bass_t& storage, u32 id) {
            auto result = make_cursor(storage);
            auto index = hashtable::find(storage.index, id);
            if (index) {
                result.page = index->page;
                result.offset = index->offset;
                result.start_offset = index->offset;
                result.header = (field_t*) result.page + result.offset;
                result.end_offset = result.offset + result.header->value;
            }
            result.ok = result.header && result.header->kind == kind::header;
            return result;
        }

        b8 next_field(cursor_t& cursor, u32& value, u8 type) {
            value = 0;
            cursor.ok = false;
            if (!move_next(cursor, type))
                return false;
            cursor.ok = true;
            value = cursor.field->value;
            return cursor.ok;
        }

        b8 write_field(cursor_t& cursor, u8 type, u32 value) {
            cursor.field = (field_t*) cursor.page + cursor.offset;
            cursor.field->type = type;
            cursor.field->value = value;
            cursor.field->kind = kind::field;
            cursor.ok = move_next(cursor);
            return cursor.ok;
        }

        u0 init(bass_t& storage, alloc_t* alloc, u32 num_pages) {
            storage.id = 1;
            storage.alloc = alloc;
            
            auto index_alloc = memory::proxy::make(storage.alloc, "bass::index"_ss);
            hashtable::init(storage.index, index_alloc, .98f);

            page_config_t page_config{};
            page_config.backing = storage.alloc;
            page_config.page_size = memory::system::os_page_size() * num_pages;
            auto page_alloc = memory::system::make(alloc_type_t::page, &page_config);
            auto page_alloc_proxy = memory::proxy::make(page_alloc, "bass::page"_ss );
            storage.page_alloc = page_alloc_proxy;

            bump_config_t bump_config{};
            bump_config.type = bump_type_t::allocator;
            bump_config.backing.alloc = storage.page_alloc;
            auto bump_alloc = memory::system::make(alloc_type_t::bump, &bump_config);
            storage.bump_alloc = bump_alloc;
        }

        cursor_t write_header(bass_t& storage, u8 type, u32 size) {
            const auto node_size = (size + 3) * sizeof(field_t);

            u32 alloc_size{};
            memory::alloc(storage.bump_alloc, node_size, alignof(field_t), &alloc_size);

            auto result = make_cursor(storage);
            result.offset -= alloc_size;
            result.id = result.storage->id++;
            result.field = {};
            result.header = (field_t*) result.page + result.offset;
            result.header->type = type;
            result.header->value = node_size;
            result.header->kind = kind::header;

            auto index = hashtable::emplace(storage.index, result.id);
            index->page = result.page;
            index->offset = result.offset;

            result.start_offset = result.offset;
            result.end_offset = result.offset + node_size;

            u32 value{};
            result.ok = next_field(result, value);
            if (result.ok)
                result.ok = write_field(result, field::id, result.id);
            return result;
        }
    }
}
