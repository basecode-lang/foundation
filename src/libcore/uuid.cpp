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

#include <cassert>
#include <basecode/core/uuid.h>
#include <basecode/core/bits.h>
#include <basecode/core/numbers.h>

namespace basecode::uuid {
    uuid_t make() {
        uuid_t u{};
#ifdef _MSC_VER
        auto hr = CoCreateGuid((GUID*) &u);
        assert(!FAILED(hr));
#else
        // XXX: *nix approach?
        //      find way to do this without libuuid
#endif
        return u;
    }

    status_t parse(const s8* str, uuid_t* u) {
        u32 temp{};
        auto p = str;
        if (*p == '{') ++p;
        auto rc = numbers::integer::parse(slice::make(p, 8),
                                          16,
                                          u->data1);
        p += 8;
        if (!OK(rc) && *p != '-')
            return status_t::parse_error;
        ++p;
        rc = numbers::integer::parse(slice::make(p, 4),
                                     16,
                                     temp);
        p += 4;
        if (!OK(rc) && *p != '-')
            return status_t::parse_error;
        ++p;
        u->data2 = temp;
        rc = numbers::integer::parse(slice::make(p, 4),
                                     16,
                                     temp);
        p += 4;
        if (!OK(rc) && *p != '-')
            return status_t::parse_error;
        ++p;
        u->data3 = temp;
        u64_bytes_t thunk{};
        u32 byte_idx{};
        for (u32 i = 0; i < 16; ++i) {
            if (*p == '-')
                ++p;
            if (!isxdigit(*p))
                return status_t::parse_error;
            auto nybble = from_hex(*p);
            if (i == 0 || (i % 2) == 0)
                thunk.bytes[byte_idx] = nybble << u32(4);
            else {
                thunk.bytes[byte_idx] |= nybble;
                ++byte_idx;
            }
            ++p;
        }
        std::memcpy(u->data4, thunk.bytes, 8);
        return status_t::ok;
    }
}
