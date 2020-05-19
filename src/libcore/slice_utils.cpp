// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
// V I R T U A L  M A C H I N E  P R O J E C T
//
// Copyright (C) 2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <basecode/core/slice_utils.h>

namespace basecode::slice {
    b8 to_fields(const str::slice_t& value, array_t<str::slice_t>& fields, s8 sep) {
        u32 curr_pos{}, start_pos{};
        for (auto c : value) {
            if (c == sep) {
                array::append(fields, str::slice_t{.data = value.data + start_pos, .length = curr_pos - start_pos});
                start_pos = curr_pos + 1;
            }
            ++curr_pos;
        }
        if (curr_pos > start_pos) {
            array::append(fields, str::slice_t{.data = value.data + start_pos, .length = curr_pos - start_pos});
        }
        return true;
    }
}

