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

#include <utf8proc.h>
#include <basecode/core/str.h>
#include <basecode/core/memory.h>
#include <basecode/core/context.h>

namespace basecode::utf {
    template <Utf_String_Concept T,
              typename Value_Type = typename T::Value_Type>
    u0 reserve(T& str, u32 new_capacity);

    template <Utf_String_Concept T>
    u0 grow(T& str, u32 min_capacity = 8);

    template <Utf_String_Concept T>
    inline u0 append(T& str, utf32_codepoint_t cp);

    template <Utf_String_Concept T>
    inline u0 append(T& str, const utf8_codepoint_t& cp);

    template <Utf_String_Concept T>
    inline u0 append(T& str, const utf16_codepoint_t& cp);

    utf8_codepoint_t            utf32_to_utf8(utf32_codepoint_t cp);
    inline utf16_codepoint_t    utf32_to_utf16(utf32_codepoint_t cp);

    utf16_codepoint_t           utf8_to_utf16(const utf8_codepoint_t& cp);
    utf32_codepoint_t           utf8_to_utf32(const utf8_codepoint_t& cp);

    inline utf8_codepoint_t     utf16_to_utf8(const utf16_codepoint_t& cp);
    inline utf32_codepoint_t    utf16_to_utf32(const utf16_codepoint_t& cp);

    b8 isalnum(s32 cp);

    b8 islower(s32 cp);

    b8 isupper(s32 cp);

    b8 isdigit(s32 cp);

    b8 isalpha(s32 cp);

    b8 isxdigit(s32 cp);

    template <Utf_String_Concept T>
    u0 free(T& str) {
        memory::free(str.alloc, str.data);
        str.data     = {};
        str.size     = {};
        str.length   = {};
        str.capacity = {};
    }

    template <Utf_String_Concept T>
    u0 reset(T& str) {
        str.size = str.length = 0;
    }

    template <Utf_String_Concept T>
    inline b8 empty(T& str) {
        return str.size == 0;
    }

    template <Utf_String_Concept T>
    inline u32 length(T& str) {
        if (str.length)
            return str.length;
        // XXX: this implementation is pretty wonky and isn't correct.
        //      for a few limited cases, this interpretation of "length"
        //      will work but it won't fit general use cases.
        if constexpr (T::Is_Utf32) {
            str.length = str.size / 4;
        } else if constexpr (T::Is_Utf16) {
            str.length = str.size / 2;
        } else if constexpr (T::Is_Utf8) {
            utf8proc_int32_t cp32{};
            auto p = str.data;
            str.length = {};
            s32 bytes_read{};
            for (u32 i = 0; i < str.size;
                 p += bytes_read, i += bytes_read, ++str.length) {
                bytes_read = utf8proc_iterate(p, -1, &cp32);
                if (bytes_read < 0)
                    return 0;
            }
        }
        return str.length;
    }

    template <Utf_String_Concept T>
    inline const s8* c_str(T& str) {
        if (str.length + 1 > str.capacity)
            grow(str);
        str.data[str.length] = '\0';
        return (const s8*) str.data;
    }

    template <Utf_String_Concept T>
    u0 grow(T& str, u32 min_capacity) {
        auto new_capacity = std::max(str.capacity,
                                     std::max<u32>(min_capacity, 8));
        reserve(str, new_capacity * 2 + 8);
    }

    template <Utf_String_Concept T, typename Value_Type>
    u0 reserve(T& str, u32 new_capacity) {
        if (new_capacity == str.capacity)
            return;

        if (new_capacity == 0) {
            memory::free(str.alloc, str.data);
            str.data = {};
            str.size = str.length = str.capacity = {};
            return;
        }

        str.data = (Value_Type*) memory::realloc(str.alloc,
                                                 str.data,
                                                 new_capacity * sizeof(Value_Type));
        str.capacity = new_capacity;
        if (new_capacity < str.size)
            str.size = new_capacity;
    }

    template <Utf_String_Concept U, String_Concept S>
    inline u0 append(U& str, const S& value) {
        if (str.size + value.length > str.capacity)
            grow(str, str.capacity + value.length);
        if constexpr (U::Is_Utf32) {
            auto actual_size = utf8proc_decompose(value.data,
                                                  value.length,
                                                  str.data + str.size,
                                                  value.length,
                                                  0);
            str.size += actual_size;
        } else if constexpr (U::Is_Utf16) {
            for (auto ch : value) {
                utf16_codepoint_t cp;
                cp.high = 0;
                cp.low  = ch;
                append(str, cp);
            }
        } else if constexpr (U::Is_Utf8) {
            std::memcpy(str.data + str.size, value.data, value.length);
            str.size += value.length;
        }
    }

    inline s32 char16_length(const char16_t* str) {
        if (!str)
            return -1;
        const char16_t* s = str;
        for (; *s; ++s);
        return s - str;
    }

    inline s32 char32_length(const char32_t* str) {
        if (!str)
            return -1;
        const char32_t* s = str;
        for (; *s; ++s);
        return s - str;
    }

    template <Utf_String_Concept T>
    inline u0 append(T& str, utf32_codepoint_t cp) {
        if constexpr (T::Is_Utf32) {
            if (str.size + 1 > str.capacity)
                grow(str);
            str.data[str.size++] = cp;
        } else if constexpr (T::Is_Utf16) {
            append(str, utf32_to_utf16(cp));
        } else if constexpr (T::Is_Utf8) {
            append(str, utf32_to_utf8(cp));
        }
    }

    template <Utf_String_Concept T>
    inline u0 append(T& str, const utf8_codepoint_t& cp) {
        if constexpr (T::Is_Utf32) {
            append(str, utf8_to_utf32(cp));
        } else if constexpr (T::Is_Utf16) {
            append(str, utf8_to_utf16(cp));
        } else if constexpr (T::Is_Utf8) {
            if (str.size + cp.len > str.capacity)
                grow(str, str.capacity + cp.len);
            std::memcpy(str.data + str.size, cp.data, cp.len);
            str.size += cp.len;
        }
    }

    template <Utf_String_Concept T>
    inline u0 append(T& str, const utf16_codepoint_t& cp) {
        if constexpr (T::Is_Utf32) {
            append(str, utf16_to_utf32(cp));
        } else if constexpr (T::Is_Utf16) {
            if (str.size + 2 > str.capacity)
                grow(str, str.capacity + 2);
            str.data[str.size++] = cp.low;
            if (cp.high != 0)
                str.data[str.size++] = cp.high;
        } else if constexpr (T::Is_Utf8) {
            append(str, utf16_to_utf8(cp));
        }
    }

    u0 resize(Utf_String_Concept auto& str, u32 new_size) {
        if (new_size > str.capacity)
            grow(str, new_size);
        str.size   = new_size;
        str.length = {};
    }

    inline utf8_codepoint_t utf32_to_utf8(utf32_codepoint_t cp) {
        utf8_codepoint_t cp8{};
        cp8.len = utf8proc_encode_char(cp, cp8.data);
        return cp8;
    }

    template <Utf_String_Concept T>
    u0 init(T& str, alloc_t* alloc = context::top()->alloc.main) {
        str.alloc    = alloc;
        str.data     = {};
        str.size     = {};
        str.length   = {};
        str.capacity = {};
    }

    template <Utf_String_Concept T>
    inline u0 append(T& str, const char16_t* value, s32 len = -1) {
        len = len != -1 ? len : char16_length(value);
        if (str.size + len > str.capacity)
            grow(str, str.capacity + len);
        if constexpr (T::Is_Utf32) {
            const char16_t* p = value;
            for(; *p; ++p) {
                utf16_codepoint_t cp16{};
                cp16.low  = u16(*p);
                cp16.high = cp16.low == 0xffff ? u16(*(++p)) : 0;
                utf::append(str, utf16_to_utf32(cp16));
            }
        } else if constexpr (T::Is_Utf16) {
            std::memcpy(str.data + str.size, value, len);
        } else if constexpr (T::Is_Utf8) {
            const char16_t* p = value;
            for(; *p; ++p) {
                utf16_codepoint_t cp16{};
                cp16.low  = u16(*p);
                cp16.high = cp16.low == 0xffff ? u16(*(++p)) : 0;
                utf::append(str, utf16_to_utf8(cp16));
            }
        }
    }

    template <Utf_String_Concept T>
    inline u0 append(T& str, const char32_t* value, s32 len = -1) {
        len = len != -1 ? len : char32_length(value);
        if (str.size + len > str.capacity)
            grow(str, str.capacity + len);
        if constexpr (T::Is_Utf32) {
            std::memcpy(str.data + str.size, value, len);
        } else if constexpr (T::Is_Utf16) {
            const char32_t* p = value;
            for(; *p; ++p)
                append(str, utf32_to_utf16(*p));
        } else if constexpr (T::Is_Utf8) {
            const char32_t* p = value;
            for(; *p; ++p)
                append(str, utf32_to_utf8(*p));
        }
    }

    inline utf16_codepoint_t utf32_to_utf16(utf32_codepoint_t cp) {
        utf16_codepoint_t cp16;
        if (cp < 0x10000) {
            cp16.high = 0;
            cp16.low  = cp;
        } else {
            s32 t = cp - 0x10000;
            cp16.high = ((t << 12U >> 22U) + 0xd800);
            cp16.low  = ((t << 22U >> 22U) + 0xdc00);
        }
        return cp16;
    }

    inline utf16_codepoint_t utf8_to_utf16(const utf8_codepoint_t& cp) {
        return utf32_to_utf16(utf8_to_utf32(cp));
    }

    inline utf32_codepoint_t utf8_to_utf32(const utf8_codepoint_t& cp) {
        utf32_codepoint_t cp32{};
        utf8proc_iterate(cp.data, cp.len, (utf8proc_int32_t*) &cp32);
        return cp32;
    }

    template <Utf_String_Concept T>
    str_t to_str(T& str, alloc_t* alloc = context::top()->alloc.main) {
        str_t new_str{};
        str::init(new_str, alloc);
        str::reserve(new_str, str.size);
        if constexpr (T::Is_Utf32) {
            for (u32 i = 0; i < str.size; ++i) {
                auto cp8 = utf32_to_utf8(str.data[i]);
                str::append(new_str, cp8.data, cp8.len);
            }
        } else if constexpr (T::Is_Utf16) {
            u16* p = str.data;
            u16* e = p + str.size;
            while (p != e) {
                utf16_codepoint_t cp16{};
                cp16.low  = u16(*p);
                cp16.high = cp16.low == 0xffff ? u16(*(++p)) : 0;
                auto cp8 = utf16_to_utf8(cp16);
                str::append(new_str, cp8.data, cp8.len);
            }
        } else if constexpr (T::Is_Utf8) {
            str::append(new_str, str.data, str.size);
        }
        return new_str;
    }

    inline utf8_codepoint_t utf16_to_utf8(const utf16_codepoint_t& cp) {
        return utf32_to_utf8(utf16_to_utf32(cp));
    }

    utf8_str_t utf32_to_utf8(const wchar_t* pwstr,
                             alloc_t* alloc = context::top()->alloc.main);

    utf8_str_t utf16_to_utf8(const wchar_t* pwstr,
                             alloc_t* alloc = context::top()->alloc.main);

    inline utf32_codepoint_t utf16_to_utf32(const utf16_codepoint_t& cp) {
        if (cp.high == 0)
            return cp.low;
        return cp.high << 16U | (cp.low & 0x0000ffffU);
    }
}
