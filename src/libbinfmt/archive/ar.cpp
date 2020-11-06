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

#include <basecode/core/bits.h>
#include <basecode/binfmt/ar.h>
#include <basecode/core/numbers.h>
#include <basecode/core/stopwatch.h>

namespace basecode::binfmt::ar {
    struct sym_offset_t final {
        str::slice_t            name;
        u32                     offset;
    };

    struct header_t final {
        s8                      name[16];
        s8                      date[12];
        s8                      uid[6];
        s8                      gid[6];
        s8                      mode[8];
        s8                      size[10];
    };

    static status_t parse_headers(ar_t& ar, buf_crsr_t& c) {
        s8          marker[2];
        header_t    hdr{};
        u64_bytes_t thunk_u64{};

        if (!OK(buf::cursor::read_u64(c, thunk_u64.value)))
            return status_t::read_error;

        if (memcmp(thunk_u64.bytes, "!<arch>\n", 8) != 0)
            return status_t::read_error;

        while (CRSR_MORE(c)) {
            const auto hdr_offset = CRSR_POS(c);

            if (!OK(buf::cursor::read_obj(c, &hdr, sizeof(header_t))))
                break;

            hdr.name[15] = 0;
            hdr.date[11] = 0;
            hdr.uid[5]   = 0;
            hdr.gid[5]   = 0;
            hdr.mode[7]  = 0;
            hdr.size[9]  = 0;

            if (!OK(buf::cursor::read_obj(c, marker, 2)))
                break;

            if (marker[0] != 0x60 || marker[1] != 0x0a)
                return status_t::read_error;

            auto& member = array::append(ar.members);
            member.name.data     = c.buf->data + hdr_offset;
            member.name.length   = 16;
            member.offset.header = hdr_offset;

            numbers::status_t parse_rc;

            const auto date_slice = slice::make(hdr.date, 12);
            parse_rc = numbers::integer::parse(date_slice, 10, member.date);
            if (!OK(parse_rc))
                return status_t::read_error;

            const auto uid_slice = slice::make(hdr.uid, 6);
            parse_rc = numbers::integer::parse(uid_slice, 10, member.uid);
            if (!OK(parse_rc))
                return status_t::read_error;

            const auto gid_slice = slice::make(hdr.gid, 6);
            parse_rc = numbers::integer::parse(gid_slice, 10, member.gid);
            if (!OK(parse_rc))
                return status_t::read_error;

            const auto mode_slice = slice::make(hdr.mode, 8);
            parse_rc = numbers::integer::parse(mode_slice, 8, member.mode);
            if (!OK(parse_rc))
                return status_t::read_error;

            const auto size_slice = slice::make(hdr.size, 10);
            parse_rc = numbers::integer::parse(size_slice, 10, member.content.length);
            if (!OK(parse_rc))
                return status_t::read_error;

            member.offset.data  = CRSR_POS(c);
            member.content.data = CRSR_PTR(c);
            buf::cursor::seek_fwd(c, member.content.length);

            while (CRSR_MORE(c) && CRSR_READ(c) == '\n')
                CRSR_NEXT(c);
        }

        return status_t::ok;
    }

    // XXX: this isn't finished but m$ sucks so...there
    static status_t parse_ecoff_symbol_table(ar_t& ar, buf_crsr_t& c) {
        ar.known.ecoff_symbol_table = 2;

        auto& symbol_table = ar.members[ar.known.ecoff_symbol_table - 1];
        buf::cursor::push(c);
        defer(buf::cursor::pop(c));

        buf::cursor::seek(c, symbol_table.offset.data);

        u32 num_members{};
        if (!OK(buf::cursor::read_u32(c, num_members)))
            return status_t::read_error;

        u32 mem_offsets[num_members];
        for (u32 i = 0; i < num_members; ++i) {
            if (!OK(buf::cursor::read_u32(c, mem_offsets[i])))
                return status_t::read_error;
        }
        u32 num_symbols{};

        if (!OK(buf::cursor::read_u32(c, num_symbols)))
            return status_t::read_error;

        u16 sym_indexes[num_symbols];
        for (u32 i = 0; i < num_symbols; ++i) {
            if (!OK(buf::cursor::read_u16(c, sym_indexes[i])))
                return status_t::read_error;
        }
        u8 ch{};
        while (num_symbols) {
            if (!OK(buf::cursor::read_u8(c, ch)))
                return status_t::read_error;
            if (ch == 0)
                --num_symbols;
        }

        return status_t::ok;
    }

    static status_t parse_symbol_table(ar_t& ar, buf_crsr_t& c) {
        if (ar.members.size == 0)
            return status_t::ok;

        auto& symbol_table = ar.members[0];
        if (symbol_table.name[0] != '/' && symbol_table.name[1] != ' ')
            return status_t::ok;

        ar.known.symbol_table = 1;
        if (ar.members.size > 1) {
            const auto& ecoff_table = ar.members[1];
            if (ecoff_table.name[0] == '/' && ecoff_table.name[1] == ' ')
                return parse_ecoff_symbol_table(ar, c);
        }

        hashtab_t<u32, u32> offset_member_idx{};
        hashtab::init(offset_member_idx, ar.alloc);

        array_t<sym_offset_t> sym_offsets{};
        array::init(sym_offsets, ar.alloc);

        buf::cursor::push(c);
        buf::cursor::seek(c, symbol_table.offset.data);

        defer(
            buf::cursor::pop(c);
            array::free(sym_offsets);
            hashtab::free(offset_member_idx));

        u32 num_symbols{};
        u8 ch{};

        if (!OK(buf::cursor::read_u32(c, num_symbols)))
            return status_t::read_error;
        num_symbols = endian_swap_dword(num_symbols);
        array::resize(sym_offsets, num_symbols);

        for (u32 i = 0; i < num_symbols; ++i) {
            auto& sym = sym_offsets[i];
            if (!OK(buf::cursor::read_u32(c, sym.offset)))
                return status_t::read_error;
            sym.offset = endian_swap_dword(sym.offset);
        }

        u32  idx{};
        auto symbol_offset = CRSR_POS(c);
        while (num_symbols) {
            if (!OK(buf::cursor::read_u8(c, ch)))
                return status_t::read_error;
            if (ch == 0) {
                auto& sym = sym_offsets[idx++];
                sym.name = slice::make(c.buf->data + symbol_offset,
                                       CRSR_POS(c) - symbol_offset - 1);
                hashtab::insert(ar.symbol_map, sym.name, 0);
                symbol_offset = CRSR_POS(c);
                --num_symbols;
            }
        }

        while (CRSR_READ(c) == 0)
            CRSR_NEXT(c);

        for (u32 i = 0; i < ar.members.size; ++i)
            hashtab::insert(offset_member_idx, ar.members[i].offset.header, i);

        bitset::resize(ar.symbol_module_bitmap,
                       ar.symbol_map.size * ar.members.size);

        hashtab::for_each_pair(ar.symbol_map,
                               [](const auto idx, const auto& key, auto& value, u0* user) -> u32 {
                                   value = idx * *((u32*) user);
                                   return 0;
                               },
                               &ar.members.size);

        for (u32 i = 0; i < sym_offsets.size; ++i) {
            auto& sym = sym_offsets[i];
            const auto bm_offset = *hashtab::find(ar.symbol_map, sym.name);
            auto member_idx = *hashtab::find(offset_member_idx, sym.offset);
            bitset::write(ar.symbol_module_bitmap, bm_offset + member_idx, true);
        }

        return status_t::ok;
    }

    static status_t parse_long_name_ref(ar_t& ar, member_t& member) {
        if (member.name[0] != '/' || !isdigit(member.name[1]))
            return status_t::ok;

        auto& long_names = ar.members[ar.known.long_names - 1];

        u32 offset{};
        const auto name_slice = slice::make(member.name.data + 1, 15);
        auto status = numbers::integer::parse(name_slice, 10, offset);
        if (!OK(status))
            return status_t::read_error;

        member.name.data    = long_names.content.data + offset;
        auto p = member.name.data;
        while (*p != '\n') p++;
        member.name.length  = p - member.name.data;

        return status_t::ok;
    }

    u0 free(ar_t& ar) {
        buf::free(ar.buf);
        array::free(ar.members);
        hashtab::free(ar.symbol_map);
        bitset::free(ar.symbol_module_bitmap);
    }

    u0 reset(ar_t& ar) {
        buf::reset(ar.buf);
        array::reset(ar.members);
        hashtab::reset(ar.symbol_map);
        bitset::reset(ar.symbol_module_bitmap);
    }

    status_t init(ar_t& ar, alloc_t* alloc) {
        ar.alloc = alloc;
        buf::init(ar.buf, ar.alloc);
        array::init(ar.members, ar.alloc);
        hashtab::init(ar.symbol_map, ar.alloc);
        bitset::init(ar.symbol_module_bitmap, ar.alloc);
        return status_t::ok;
    }

    status_t read(ar_t& ar, const path_t& path) {
        {
            stopwatch_t timer{};
            stopwatch::start(timer);

            auto status = buf::map(ar.buf, path);
            if (!OK(status))
                return status_t::read_error;

            stopwatch::stop(timer);
            stopwatch::print_elapsed("ar buf::map time"_ss, 40, timer);
        }

        buf_crsr_t c{};
        buf::cursor::init(c, ar.buf);
        defer(buf::cursor::free(c));

        auto status = parse_headers(ar, c);
        if (!OK(status))
            return status;

        for (u32 i = 0; i < ar.members.size; ++i) {
            const auto& m = ar.members[i];
            if (m.name[0] == '/' && m.name[1] == '/') {
                ar.known.long_names = i + 1;
                for (auto& member : ar.members) {
                    status = parse_long_name_ref(ar, member);
                    if (!OK(status))
                        return status;
                }
                break;
            }
        }

        status = parse_symbol_table(ar, c);
        if (!OK(status))
            return status;

        return status_t::ok;
    }

    status_t write(ar_t& ar, const path_t& path) {
        return status_t::ok;
    }

    u0 add_member(ar_t& ar, const member_t& member) {
        array::append(ar.members, member);
    }

    u0 find_member(ar_t& ar, str::slice_t name, member_ptr_list_t& list) {
        for (auto& member : ar.members) {
            if (member.name == name)
                array::append(list, &member);
        }
    }
}
