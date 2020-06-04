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

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <basecode/core/numbers.h>

namespace basecode::numbers {
    namespace fp {
        result_t parse(str::slice_t value, f32& out) {
            const s8* begin = (const s8*) value.data;
            s8* end{};
            errno = 0;
            out = strtof(begin, &end);
            if (errno == ERANGE)
                return result_t::overflow;
            if (*begin == '\0' || *end != '\0')
                return result_t::not_convertible;
            return result_t::ok;
        }

        result_t parse(str::slice_t value, f64& out) {
            const s8* begin = (const s8*) value.data;
            s8* end{};
            errno = 0;
            out = strtod(begin, &end);
            if (errno == ERANGE)
                return result_t::overflow;
            if (*begin == '\0' || *end != '\0')
                return result_t::not_convertible;
            return result_t::ok;
        }
    }

    namespace integer {
        result_t parse(str::slice_t value, u8 radix, s32& out) {
            const s8* begin = (const s8*) value.data;
            s8* end;
            errno = 0;
            out = strtol(begin, &end, radix);
            if ((errno == ERANGE && out == INT_MAX)
                ||   out > UINT_MAX)
                return result_t::overflow;
            if ((errno == ERANGE && out == INT_MIN))
                return result_t::underflow;
            if (*begin == '\0' || *end != '\0')
                return result_t::not_convertible;
            return result_t::ok;
        }

        result_t parse(str::slice_t value, u8 radix, s64& out) {
            const s8* begin = (const s8*) value.data;
            s8* end;
            errno = 0;
            out = strtoll(begin, &end, radix);
            if ((errno == ERANGE && out == LONG_MAX)
                ||   out > UINT_MAX)
                return result_t::overflow;
            if ((errno == ERANGE && out == LONG_MIN))
                return result_t::underflow;
            if (*begin == '\0' || *end != '\0')
                return result_t::not_convertible;
            return result_t::ok;
        }
    }
}
