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

#include <basecode/core/str.h>
#include <basecode/core/format_types.h>

namespace basecode {
    struct uuid_t final {
        u32                     data1;
        u16                     data2;
        u16                     data3;
        u8                      data4[8];

        b8 operator==(const uuid_t& other) const {
            return data1 == other.data1
                && data2 == other.data2
                && data3 == other.data3
                && std::memcmp(data4, other.data4, 8) == 0;
        }
    };

    namespace uuid {
        enum class status_t : u32 {
            ok                  = 0,
            parse_error
        };

        uuid_t make();

        status_t parse(const s8* str, uuid_t* u);

        status_t parse(const String_Concept auto& str, uuid_t* u) {
            return parse((const s8*) str.data, u);
        }
    }
}

FORMAT_TYPE(basecode::uuid_t,
    format_to(ctx.out(),
              "{{{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}}}",
              data.data1,
              data.data2,
              data.data3,
              data.data4[0], data.data4[1], data.data4[2], data.data4[3],
              data.data4[4], data.data4[5], data.data4[6], data.data4[7]));
