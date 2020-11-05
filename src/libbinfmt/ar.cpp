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
            auto status = buf::load(ar.buf, path);
            if (!OK(status))
                return status_t::read_error;
        }

        array_t<sym_offset_t> sym_offsets{};
        array::init(sym_offsets, ar.alloc);
        defer(array::free(sym_offsets));

        buf_crsr_t c{};
        buf::cursor::init(c, ar.buf);

        u32 name_base_off{};
        u32 linker_mem_no{};

        u64_bytes_t t{};
        t.value = buf::cursor::read_u64(c);
        if (memcmp(t.bytes, "!<arch>\n", 8) != 0)
            return status_t::read_error;

        while (CRSR_MORE(c)) {
            const auto hdr_offset = CRSR_POS(c);

            header_t hdr{};
            if (!buf::cursor::read_obj(c, &hdr, sizeof(header_t)))
                break;

            hdr.name[15] = 0;
            hdr.date[11] = 0;
            hdr.uid[5]   = 0;
            hdr.gid[5]   = 0;
            hdr.mode[7]  = 0;
            hdr.size[9]  = 0;

            s8 marker[2];
            if (!buf::cursor::read_obj(c, marker, 2))
                break;

            if (marker[0] != 0x60 || marker[1] != 0x0a)
                return status_t::read_error;

            member_t member{};
            numbers::integer::parse(slice::make(hdr.date, 12),
                                    10,
                                    member.date);

            numbers::integer::parse(slice::make(hdr.uid, 6),
                                    10,
                                    member.uid);

            numbers::integer::parse(slice::make(hdr.gid, 6),
                                    10,
                                    member.gid);

            numbers::integer::parse(slice::make(hdr.mode, 8),
                                    8,
                                    member.mode);

            numbers::integer::parse(slice::make(hdr.size, 10),
                                    10,
                                    member.content.length);

            if (hdr.name[0] == '/') {
                switch (hdr.name[1]) {
                    case '/': {
                        name_base_off = CRSR_POS(c);
                        auto size = member.content.length;
                        while (size) {
                            u8 ch = buf::cursor::read_u8(c);
                            if (ch == '\n') {
                                ar.buf.data[c.pos - 1] = 0;
                            }
                            --size;
                        }
                        break;
                    }
                    case '0' ... '9': {
                        u32 offset{};
                        numbers::integer::parse(slice::make(hdr.name + 1, 15),
                                                10,
                                                offset);
                        member.name.data    = ar.buf.data + name_base_off + offset;
                        member.name.length  = strlen((const s8*) member.name.data);
                        member.content.data = c.buf->data + c.pos;
                        member.offset       = hdr_offset;
                        add_member(ar, member);
                        buf::cursor::seek(c, c.pos + member.content.length);
                        break;
                    }
                    default: {
                        if (linker_mem_no == 0) {
                            u32 num_symbols = endian_swap_dword(buf::cursor::read_u32(c));
                            array::resize(sym_offsets, num_symbols);
                            for (u32 i = 0; i < num_symbols; ++i) {
                                auto& sym = sym_offsets[i];
                                sym.offset = endian_swap_dword(buf::cursor::read_u32(c));
                            }
                            u32 i{};
                            auto symbol_offset = CRSR_POS(c);
                            while (num_symbols) {
                                u8 ch = buf::cursor::read_u8(c);
                                if (ch == 0) {
                                    auto& sym = sym_offsets[i++];
                                    sym.name = slice::make(c.buf->data + symbol_offset,
                                                           CRSR_POS(c) - symbol_offset - 1);
                                    symbol_offset = CRSR_POS(c);
                                    --num_symbols;
                                }
                            }
                            while (CRSR_READ(c) == 0)
                                CRSR_NEXT(c);
                            linker_mem_no++;
                        } else if (linker_mem_no == 1) {
                            u32 num_members = buf::cursor::read_u32(c);
                            u32 mem_offsets[num_members];
                            for (u32 i = 0; i < num_members; ++i)
                                mem_offsets[i] = buf::cursor::read_u32(c);
                            u32 num_symbols = buf::cursor::read_u32(c);
                            u32 sym_indexes[num_symbols];
                            for (u32 i = 0; i < num_symbols; ++i)
                                sym_indexes[i] = buf::cursor::read_u16(c);
                            while (num_symbols) {
                                u8 ch = buf::cursor::read_u8(c);
                                if (ch == 0)
                                    --num_symbols;
                            }
                            linker_mem_no++;
                        } else {
                            return status_t::read_error;
                        }
                        break;
                    }
                }
            } else {
                auto end_ptr = c.buf->data + hdr_offset;
                while (*end_ptr != '/')
                    ++end_ptr;
                member.name.data    = ar.buf.data + hdr_offset;
                member.name.length  = end_ptr - member.name.data;
                member.content.data = c.buf->data + c.pos;
                member.offset       = hdr_offset;
                add_member(ar, member);
                buf::cursor::seek(c, c.pos + member.content.length);
            }
            while (CRSR_MORE(c) && CRSR_READ(c) == '\n')
                CRSR_NEXT(c);
        }

        hashtab_t<u32, u32> offset_member_idx{};
        hashtab::init(offset_member_idx, ar.alloc);
        defer(hashtab::free(offset_member_idx));

        for (u32 i = 0; i < ar.members.size; ++i)
            hashtab::insert(offset_member_idx, ar.members[i].offset, i);

        for (u32 i = 0; i < sym_offsets.size; ++i)
            hashtab::insert(ar.symbol_map, sym_offsets[i].name, i * ar.members.size);

        bitset::resize(ar.symbol_module_bitmap,
                       ar.symbol_map.size * ar.members.size);

        for (u32 i = 0; i < sym_offsets.size; ++i) {
            auto& sym = sym_offsets[i];
            const auto bm_offset = *hashtab::find(ar.symbol_map, sym.name);
            auto member_idx = *hashtab::find(offset_member_idx, sym.offset);
            bitset::write(ar.symbol_module_bitmap, bm_offset + member_idx, true);
        }

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
