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

        b8 next_record(cursor_t& cursor) {
            const auto page_size = memory::bump::end_offset(cursor.storage->bump_alloc);
            if (cursor.offset + sizeof(field_t) >= page_size) {
                u8* next_page = (u8*) ((page_header_t*) cursor.page)->next;
                if (!next_page)
                    return false;
                cursor.offset = {};
                cursor.page = next_page;
            } else {
                cursor.offset += cursor.end_offset - cursor.offset;
            }
            cursor.field = {};
            cursor.header = (field_t*) cursor.page + cursor.offset;
            cursor.end_offset = cursor.offset + cursor.header->value;
            return cursor.header && cursor.header->kind == kind::header;
        }

        u0 reset_cursor(cursor_t& cursor) {
            cursor.offset = cursor.start_offset;
        }

        b8 seek_first(bass_t& storage, cursor_t& cursor) {
            cursor.id = {};
            cursor.field = {};
            cursor.storage = &storage;
            cursor.page = (u8*) memory::bump::buf(storage.bump_alloc);
            cursor.header = (field_t*) cursor.page;
            cursor.end_offset = cursor.header->value;
            cursor.offset = cursor.start_offset = {};
            return cursor.header && cursor.header->kind == kind::header;
        }

        b8 seek_current(bass_t& storage, cursor_t& cursor) {
            cursor.id = {};
            cursor.field = {};
            cursor.header = {};
            cursor.storage = &storage;
            cursor.page = (u8*) memory::bump::buf(storage.bump_alloc);
            cursor.offset = cursor.start_offset = memory::bump::offset(storage.bump_alloc);
            return true;
        }

        b8 next_field(cursor_t& cursor, u32& value, u8 type) {
            value = 0;
            if (!move_next(cursor, type))
                return false;
            value = cursor.field->value;
            return true;
        }

        b8 write_field(cursor_t& cursor, u8 type, u32 value) {
            cursor.field = (field_t*) cursor.page + cursor.offset;
            cursor.field->type = type;
            cursor.field->value = value;
            cursor.field->kind = kind::field;
            return move_next(cursor);
        }

        u0 init(bass_t& storage, alloc_t* alloc, u32 num_pages) {
            storage.id = 1;
            storage.alloc = alloc;

            hashtable::init(storage.index, storage.alloc, .98f);

            page_config_t page_config{};
            page_config.backing = storage.alloc;
            page_config.page_size = memory::system::os_page_size() * num_pages;
            storage.page_alloc = memory::system::make(alloc_type_t::page, &page_config);

            bump_config_t bump_config{};
            bump_config.type = bump_type_t::allocator;
            bump_config.backing.alloc = storage.page_alloc;
            storage.bump_alloc = memory::system::make(alloc_type_t::bump, &bump_config);
        }

        b8 new_record(cursor_t& cursor, u8 type, u32 num_fields) {
            const u32 record_size = RECORD_BYTE_SIZE(num_fields + 2);
            memory::alloc(cursor.storage->bump_alloc, record_size, alignof(field_t));
            cursor.page = (u8*) memory::bump::buf(cursor.storage->bump_alloc);

            cursor.field = {};
            cursor.id = cursor.storage->id++;
            cursor.end_offset = cursor.offset + record_size;
            cursor.header = (field_t*) cursor.page + cursor.offset;
            cursor.header->type = type;
            cursor.header->value = record_size;
            cursor.header->kind = kind::header;

            auto index = hashtable::emplace(cursor.storage->index, cursor.id);
            index->page = cursor.page;
            index->offset = cursor.offset;

            u32 value{};
            if (!next_field(cursor, value))
                return false;
            return write_field(cursor, field::id, cursor.id);
        }

        b8 seek_record(bass_t& storage, u32 id, cursor_t& cursor) {
            auto index = hashtable::find(storage.index, id);
            if (index) {
                cursor.page = index->page;
                cursor.offset = index->offset;
                cursor.start_offset = index->offset;
                cursor.header = (field_t*) cursor.page + cursor.offset;
                cursor.end_offset = cursor.offset + cursor.header->value;
            }
            return cursor.header && cursor.header->kind == kind::header;
        }
    }
}
