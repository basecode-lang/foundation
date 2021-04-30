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

#include <basecode/core/types.h>

namespace basecode::gfx {
    // ------------------------------------------------------------------------
    //
    //  vectors
    //
    // ------------------------------------------------------------------------
    struct vec2_t final {
        f32                     x;
        f32                     y;

        vec2_t() : x(), y() {
        }

        vec2_t(f32 x, f32 y) : x(x), y(y) {
        }

        explicit vec2_t(f32 v) : x(v), y(v) {
        }

        inline b8 operator<(const vec2_t& rhs) const {
            if (x != rhs.x)
                return x < rhs.x;
            return y < rhs.y;
        }

        inline b8 operator==(const vec2_t& rhs) const {
            return x == rhs.x && y == rhs.y;
        }
    };

    inline vec2_t clamp(const vec2_t& val,
                        const vec2_t& min,
                        const vec2_t& max) {
        return vec2_t(std::min(max.x, std::max(min.x, val.x)),
                      std::min(max.y, std::max(min.y, val.y)));
    }

    inline vec2_t& operator+=(vec2_t& lhs, f32 rhs) {
        lhs.x += rhs;
        lhs.y += rhs;
        return lhs;
    }

    inline vec2_t& operator-=(vec2_t& lhs, f32 rhs) {
        lhs.x -= rhs;
        lhs.y -= rhs;
        return lhs;
    }

    inline vec2_t& operator*=(vec2_t& lhs, f32 rhs) {
        lhs.x *= rhs;
        lhs.y *= rhs;
        return lhs;
    }

    inline vec2_t& operator/=(vec2_t& lhs, f32 rhs) {
        lhs.x /= rhs;
        lhs.y /= rhs;
        return lhs;
    }

    inline vec2_t operator*(const vec2_t& lhs, f32 rhs) {
        return vec2_t(lhs.x * rhs, lhs.y * rhs);
    }

    inline vec2_t operator/(const vec2_t& lhs, f32 rhs) {
        return vec2_t(lhs.x / rhs, lhs.y / rhs);
    }

    inline vec2_t& operator+=(vec2_t& lhs, const vec2_t& rhs) {
        lhs.x += rhs.x;
        lhs.y += rhs.y;
        return lhs;
    }

    inline vec2_t& operator-=(vec2_t& lhs, const vec2_t& rhs) {
        lhs.x -= rhs.x;
        lhs.y -= rhs.y;
        return lhs;
    }

    inline vec2_t& operator*=(vec2_t& lhs, const vec2_t& rhs) {
        lhs.x *= rhs.x;
        lhs.y *= rhs.y;
        return lhs;
    }

    inline vec2_t& operator/=(vec2_t& lhs, const vec2_t& rhs) {
        lhs.x /= rhs.x;
        lhs.y /= rhs.y;
        return lhs;
    }

    inline vec2_t operator+(const vec2_t& lhs, const vec2_t& rhs) {
        return vec2_t(lhs.x + rhs.x, lhs.y + rhs.y);
    }

    inline vec2_t operator-(const vec2_t& lhs, const vec2_t& rhs) {
        return vec2_t(lhs.x - rhs.x, lhs.y - rhs.y);
    }

    inline vec2_t operator/(const vec2_t& lhs, const vec2_t& rhs) {
        return vec2_t(lhs.x / rhs.x, lhs.y / rhs.y);
    }

    inline vec2_t operator*(const vec2_t& lhs, const vec2_t& rhs) {
        return vec2_t(lhs.x * rhs.x, lhs.y * rhs.y);
    }

    struct vec3_t final {
        f32                     x;
        f32                     y;
        f32                     z;
    };

    struct vec4_t final {
        f32                     x;
        f32                     y;
        f32                     z;
        f32                     w;

        vec4_t() : x(), y(), z(), w() {
        }

        vec4_t(f32 a) : x(a), y(a), z(a), w(a) {
        }

        vec4_t(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {
        }
    };

    // ------------------------------------------------------------------------
    //
    //  rect
    //
    // ------------------------------------------------------------------------
    struct rect_t final {
        vec2_t                  tl;
        vec2_t                  br;

        inline f32 top() const {
            return top_right().y;
        }

        inline b8 empty() const {
            return (height() == 0.0f || width() == 0.0f);
        }

        inline f32 left() const {
            return tl.x;
        }

        inline f32 width() const {
            return br.x - tl.x;
        }

        inline f32 right() const {
            return top_right().x;
        }

        inline f32 height() const {
            return br.y - tl.y;
        }

        inline f32 bottom() const {
            return br.y;
        }

        inline vec2_t size() const {
            return br - tl;
        }

        inline vec2_t center() const {
            return (br + tl) * .5f;
        }

        inline u0 adjust(f32 x, f32 y) {
            tl.x += x;
            tl.y += y;
            br.x += x;
            br.y += y;
        }

        u0 set_size(const vec2_t& size) {
            br = tl + size;
        }

        inline vec2_t top_right() const {
            return vec2_t(br.x, tl.y);
        }

        inline vec2_t bottom_left() const {
            return vec2_t(tl.x, br.y);
        }

        inline b8 contains(const vec2_t& pt) const {
            return tl.x <= pt.x
                   && tl.y <= pt.y
                   && br.x > pt.x
                   && br.y > pt.y;
        }

        inline u0 adjust(f32 x, f32 y, f32 z, f32 w) {
            tl.x += x;
            tl.y += y;
            br.x += z;
            br.y += w;
        }

        inline b8 operator==(const rect_t& rhs) const {
            return tl == rhs.tl && br == rhs.br;
        }
    };
}
