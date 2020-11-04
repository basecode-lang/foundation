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
    struct ar_member_hdr_t final {
        s8                      name[16];
        s8                      date[12];
        s8                      uid[6];
        s8                      gid[6];
        s8                      mode[8];
        s8                      size[10];
    };

    u0 free(ar_t& ar) {
        buf::free(ar.buf);
        hashtab::free(ar.catalog);
    }

    u0 reset(ar_t& ar) {
        buf::reset(ar.buf);
        hashtab::reset(ar.catalog);
    }

    status_t init(ar_t& ar, alloc_t* alloc) {
        ar.alloc = alloc;
        buf::init(ar.buf, ar.alloc);
        hashtab::init(ar.catalog, ar.alloc);
        return status_t::ok;
    }

    status_t read(ar_t& ar, const path_t& path) {
        str_t tmp{};
        str::init(tmp, ar.alloc);

        auto status = buf::load(ar.buf, path);
        if (!OK(status))
            return status_t::read_error;
        buf_crsr_t c{};
        buf::cursor::init(c, ar.buf);

        u32 linker_mem_no{};

        u64_bytes_t t{};
        t.value = buf::cursor::read_u64(c);
        if (memcmp(t.bytes, "!<arch>\n", 8) != 0)
            return status_t::read_error;

        while (true) {
            ar_member_hdr_t hdr{};
            buf::cursor::read_obj(c, &hdr, sizeof(ar_member_hdr_t));

            s8 marker[2];
            buf::cursor::read_obj(c, marker, 2);

            if (marker[0] != 0x60 || marker[1] != 0x0a)
                return status_t::read_error;

            s32 data_size{};
            auto rc = numbers::integer::parse(slice::make(hdr.size, 10), 10, data_size);
//          if (!OK(rc))
//              return status_t::read_error;

            if (hdr.name[0] == '/') {
                switch (hdr.name[1]) {
                    case '/': {
                        format::print("long names member\n");
                        while (data_size) {
                            u8 ch = buf::cursor::read_u8(c);
                            if (ch == 0) {
                                format::print("\n");
                            } else {
                                format::print("{}", (char) ch);
                            }
                            --data_size;
                        }
                        break;
                    }
                    case '0' ... '9': {
                        format::print("offset of long name member\n");
                        s32 offset{};
                        auto rc = numbers::integer::parse(slice::make(hdr.name + 1, 15), 10, offset);
//                        if (!OK(rc))
//                            return status_t::read_error;
                        format::print("long name offset = {}\n", offset);
                        {
                            str::reset(tmp);
                            str_buf_t s(&tmp);
                            format::hex_dump(s, c.buf->data + c.pos, 1024);
                        }
                        format::print("{}", tmp);
                        buf::cursor::seek(c, c.pos + data_size);
                        break;
                    }
                    default: {
                        if (linker_mem_no == 0) {
                            format::print("ancient, dumb, linker member\n");
                            u32 num_symbols = endian_swap_dword(buf::cursor::read_u32(c));
                            for (u32 i = 0; i < num_symbols; ++i) {
                                u32 offset = endian_swap_dword(buf::cursor::read_u32(c));
                                //format::print("offset = {}\n", offset);
                            }
                            while (num_symbols) {
                                while (true) {
                                    u8 ch = buf::cursor::read_u8(c);
                                    if (ch == 0)
                                        break;
//                                    format::print("{}", (char) ch);
                                }
//                                format::print("\n");
                                --num_symbols;
                            }
                            linker_mem_no++;
                        } else if (linker_mem_no == 1) {
                            format::print("modern, hip, linker member\n");
                            u32 num_members = buf::cursor::read_u32(c);
                            for (u32 i = 0; i < num_members; ++i) {
                                u32 offset = buf::cursor::read_u32(c);
                                //format::print("offset = {}\n", offset);
                            }
                            u32 num_symbols = buf::cursor::read_u32(c);
                            for (u32 i = 0; i < num_symbols; ++i) {
                                u16 index = buf::cursor::read_u16(c);
                                format::print("symbol index = {}\n", index);
                            }
                            while (num_symbols) {
                                while (true) {
                                    u8 ch = buf::cursor::read_u8(c);
                                    if (ch == 0)
                                        break;
//                                    format::print("{}", (char) ch);
                                }
//                                format::print("\n");
                                --num_symbols;
                            }
                        } else {
                            return status_t::read_error;
                        }
                        break;
                    }
                }
            } else {
                format::print("short name");
                {
                    str::reset(tmp);
                    str_buf_t s(&tmp);
                    format::hex_dump(s, c.buf->data + c.pos, 1024);
                }
                format::print("{}", tmp);
                buf::cursor::seek(c, c.pos + data_size);
                u8 mark = buf::cursor::read_u8(c);
                if (mark != '\n')
                    return status_t::read_error;
                break;
            }
        }

        return status_t::ok;
    }

    status_t write(ar_t& ar, const path_t& path) {
        return status_t::ok;
    }
}
