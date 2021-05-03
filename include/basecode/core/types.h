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

#ifdef __has_include
#   if __has_include(<version>)
#       include <version>
#   endif
#endif

#include <any>
#include <bit>
#include <cmath>
#include <cstdio>
#include <memory>
#include <chrono>
#include <glob.h>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <functional>
#include <sys/types.h>
#include <x86intrin.h>
#include <type_traits>

#if defined(WIN32)
#   define API_EXPORT           __declspec(dllexport)
#else
#   define API_EXPORT
#endif

#if defined(__GNUC__)
#   if defined(__MINGW64__)
#       define DEBUG_TRAP()     __asm__ volatile("int $0x03");
#   else
#       define DEBUG_TRAP()     raise(SIGTRAP)
#   endif
#   define FORCE_INLINE         inline __attribute__((always_inline, unused))
#   define NEVER_INLINE         inline __attribute__((noinline, unused))
#   ifndef LIKELY
#       define LIKELY(x)        __builtin_expect(!!(x), 1)
#   endif
#   ifndef UNLIKELY
#       define UNLIKELY(x)      __builtin_expect(!!(x), 0)
#   endif
#elif defined(_MSC_VER)
#   define DEBUG_TRAP()         __debugbreak()
#   define FORCE_INLINE         __forceinline
#   define NEVER_INLINE         __declspec(noinline)
#   ifndef LIKELY
#       define LIKELY(x)        x
#   endif
#   ifndef UNLIKELY
#       define UNLIKELY(x)      x
#   endif
#else
#   define FORCE_INLINE         inline
#   define NEVER_INLINE
#   ifndef LIKELY
#       define LIKELY(x)        x
#   endif
#   ifndef UNLIKELY
#       define UNLIKELY(x)      x
#   endif
#endif

#ifdef _WIN32
#   define  DEFAULT_NUM_PAGES   1
#else
#   define  DEFAULT_NUM_PAGES   8
#endif

#define ALIGNED16               __attribute__((aligned(16)))
#define UNIQUE_NAME_1(x, y)     x##y
#define UNIQUE_NAME_2(x, y)     UNIQUE_NAME_1(x, y)
#define UNIQUE_NAME_(x)         UNIQUE_NAME_2(x, __COUNTER__)
#define UNUSED(x)               ((void) x)
#define OK(x)                   (0 == (u32) x)
#define SAFE_SCOPE(x)           do { x } while (false)
#define ZERO_MEM(x, s)          std::memset((x), 0, sizeof(s))
#define HAS_ZERO(v)             (((v)-UINT64_C(0x0101010101010101))             \
                                 & ~(v)&UINT64_C(0x8080808080808080))
#define VA_COUNT(...)           basecode::va_count(__VA_ARGS__)
#define KB(x)                   (1024 * (x))
#define MB(x)                   (1024 * KB(x))
#define GB(x)                   (1024 * MB(x))
#define MS_TO_NS(x)             (1000000 * (x))

#define FORMAT_TYPE(Type, Code)                                                 \
    template <>                                                                 \
    struct fmt::formatter<Type> {                                               \
        template <typename ParseContext> auto parse(ParseContext& ctx) {        \
            return ctx.begin();                                                 \
        }                                                                       \
        template <typename FormatContext>                                       \
        auto format(const Type& data,                                           \
                    FormatContext& ctx) -> decltype(ctx.out()) {                \
            Code;                                                               \
            return ctx.out();                                                   \
        }                                                                       \
    }

#define FORMAT_TYPE_AS(Type, Base)                                              \
    template <typename Char>                                                    \
    struct fmt::formatter<Type, Char> : fmt::formatter<Base, Char> {            \
        template <typename FormatContext>                                       \
        auto format(Type const& val,                                            \
                    FormatContext& ctx) -> decltype(ctx.out()) {                \
            return fmt::formatter<Base, Char>::format(val, ctx);                \
        }                                                                       \
    }

#define WITH_SLICE_AS_CSTR(Slice, Code) \
        [&](str::slice_t s) -> void {                                           \
            s8 Slice[s.length + 1];                                             \
            std::memcpy(Slice, s.data, s.length);                               \
            Slice[s.length] = '\0';                                             \
            Code;                                                               \
        }(Slice)

#if !defined(__cpp_lib_bit_cast)
namespace std {
    template <typename To, typename From>
    requires (sizeof(From) == sizeof(To)
              && std::is_trivially_constructible_v<From>
              && std::is_trivially_constructible_v<To>)
    constexpr To bit_cast(const From& src) noexcept {
        return __builtin_bit_cast(To, src);
    }
}
#endif

#if !defined(__cpp_concepts)
namespace std {
    template <typename From, typename To>
    concept convertible_to = std::is_convertible_v<From, To>
                             && requires(std::add_rvalue_reference_t<From> (&f)()) {
        static_cast<To>(f());
    };

    template <typename T, typename U>
    concept same_helper = std::is_same_v<T, U>;

    template <typename T, typename U>
    concept same_as = same_helper<T, U> && same_helper<U, T>;
}
#else
#   include <concepts>
#endif

// ------------------------------------------------------------------------
//
// iterator macros
//
// ------------------------------------------------------------------------
#define DECL_ITERS(Container, Type, State)                                  \
    DECL_MUT_ITER(Container, Type, State)                                   \
    DECL_CONST_ITER(Container, Type, State)

#define DECL_MUT_ITER(Container, Type, State)                               \
    using iterator = basecode::iterator<Container, Type, State>;            \
    iterator begin() { return iterator::begin(this); }                      \
    iterator end()   { return iterator::end(this);   }

#define DECL_CONST_ITER(Container, Type, State)                                         \
    using const_iterator = basecode::const_iterator<Container, const Type, State>;      \
    const_iterator begin() const { return const_iterator::begin(this); }                \
    const_iterator end()   const { return const_iterator::end(this);   }

#define DECL_RITERS(Container, Type, State)                                             \
    struct State##_reversed : public State {                                            \
        inline u0 next (const Container* ref) { State::prev(ref); }                     \
        inline u0 prev (const Container* ref) { State::next(ref); }                     \
        inline u0 begin(const Container* ref) { State::end(ref); State::prev(ref);}     \
        inline u0 end  (const Container* ref) { State::begin(ref); State::prev(ref);}   \
    };                                                                                  \
    DECL_MUT_RITER(Container, Type, State)                                              \
    DECL_CONST_RITER(Container, Type, State)

#define DECL_MUT_RITER(Container, Type, State)                                          \
    using reverse_iterator = basecode::iterator<Container, Type, State##_reversed >;    \
    reverse_iterator rbegin() { return reverse_iterator::begin(this); }                 \
    reverse_iterator rend()   { return reverse_iterator::end(this); }

#define DECL_CONST_RITER(Container, Type, State)                                        \
    using const_reverse_iterator = basecode::const_iterator<Container, Type, State##_reversed >;\
    const_reverse_iterator rbegin() const {                                             \
        return const_reverse_iterator::begin(this);                                     \
    }                                                                                   \
    const_reverse_iterator rend() const {                                               \
        return const_reverse_iterator::end(this);                                       \
    }

#define DECL_STL_TYPES(Type)                    \
    using difference_type   = std::ptrdiff_t;   \
    using size_type         = size_t;           \
    using value_type        = Type;             \
    using pointer           = Type*;            \
    using const_pointer     = const Type*;      \
    using reference         = Type&;            \
    using const_reference   = const Type&

// ------------------------------------------------------------------------
//
// global forward decls
//
// ------------------------------------------------------------------------
struct sqlite3;
struct sqlite3_stmt;

using pthread_t = void*;

namespace fmt {
    inline namespace v7 {
        struct format_args;
    }
}

namespace spdlog {
    class logger;
}

typedef struct DCCallVM_    DCCallVM;
typedef struct DLLib_       DLLib;

// ------------------------------------------------------------------------
//
// basecode core type system
//
// ------------------------------------------------------------------------
namespace basecode {
    using u0                    = void;
    using u8                    = std::uint8_t;
    using u16                   = std::uint16_t;
    using u32                   = std::uint32_t;
    using u64                   = std::uint64_t;
    using s8                    = char;
    using s16                   = std::int16_t;
    using s32                   = std::int32_t;
    using s64                   = std::int64_t;
    using b8                    = bool;
    using f32                   = float;
    using f64                   = double;
    using s128                  = __int128_t;
    using u128                  = __uint128_t;
    using usize                 = std::size_t;
    using ssize                 = ssize_t;

    union numeric_alias_t final {
        u8                      b;
        s8                      sb;
        u16                     w;
        s16                     sw;
        u32                     dw;
        s32                     sdw;
        u64                     qw;
        s64                     sqw;
        f32                     fdw;
        f64                     qdw;
    };

    union u16_bytes_t final {
        u16                     value;
        u8                      bytes[2];
    };

    union u32_bytes_t final {
        u32                     value;
        u8                      bytes[4];
    };

    union u64_bytes_t final {
        u64                     value;
        u8                      bytes[8];
    };

#ifdef __SSE4_2__
    union m128i_bytes_t final {
        __m128i                 value;
        u8                      bytes[16];
    };
#endif

#ifdef __AVX2__
    union m256i_bytes_t final {
        __m256i                 value;
        u8                      bytes[32];
    };
#endif

    template <typename T>
    concept Integer_Concept = std::is_integral_v<T>;

    template <typename T>
    concept Radix_Concept = Integer_Concept<T> && requires(T radix) {
        radix == 2 || radix == 8 || radix == 10 || radix == 16;
    };

    template <typename T>
    concept Status_Concept = std::is_enum_v<T> || requires(const T& t) {
        typename T::ok;
    };

    template <typename... Args>
    constexpr usize va_count(Args&&...) { return sizeof...(Args); }

    constexpr u32 operator "" _kb(u64 value) {
        return value * 1024;
    }

    constexpr u32 operator "" _mb(u64 value) {
        return 1024 * (value * 1024);
    }

    constexpr u32 operator "" _gb(u64 value) {
        return 1204 * (1024 * (value * 1024));
    }

    // ------------------------------------------------------------------------
    //
    // iterators
    //
    // ------------------------------------------------------------------------
    template <class Container, typename Type, class State>
    struct const_iterator;

    template <class Container, typename Type, class State>
    struct iterator {
        friend struct basecode::const_iterator<Container, Type, State>;

        Container*              ref;
        State                   state;

        static iterator end(Container* ref) {
            iterator it(ref);
            it.end();
            return it;
        }
        static iterator begin(Container* ref) {
            iterator it(ref);
            it.begin();
            return it;
        }

        iterator() {}

        u0 end()                     { state.end(ref);        }
        u0 next()                    { state.next(ref);       }
        u0 prev()                    { state.prev(ref);       }
        u0 begin()                   { state.begin(ref);      }
        Type get()                   { return state.get(ref); }
        b8 cmp(const State& s) const { return state.cmp(s);   }

        Type operator*()             { return get();          }
        iterator& operator++()       { next(); return *this;  }
        iterator& operator--()       { prev(); return *this;  }
        iterator operator--(int)     { iterator temp(*this); prev(); return temp;    }
        iterator operator++(int)     { iterator temp(*this); next(); return temp;    }

        b8 operator!=(const iterator& other) const {
            return ref != other.ref || cmp(other.state);
        }

        b8 operator==(const iterator& other) const {
            return !operator!=(other);
        }

        b8 operator!=(const const_iterator<Container, Type, State>& other) const {
            return ref != other.ref || cmd(other.state);
        }

        b8 operator==(const const_iterator<Container, Type, State>& other) const {
            return !operator!=(other);
        }

    protected:
        iterator(Container* ref) : ref(ref) {}
    };

    template <class Container, typename Type, class State>
    struct iterator<Container, Type&, State> {
        friend struct basecode::const_iterator<Container, Type&, State>;

        Container*              ref;
        State                   state;

        static iterator end(Container* ref) {
            iterator it(ref);
            it.end();
            return it;
        }
        static iterator begin(Container* ref) {
            iterator it(ref);
            it.begin();
            return it;
        }

        iterator() {}

        u0 end()                        { state.end(ref);        }
        u0 prev()                       { state.prev(ref);       }
        u0 next()                       { state.next(ref);       }
        u0 begin()                      { state.begin(ref);      }
        Type& get()                     { return state.get(ref); }
        b8 cmp(const State& s) const    { return state.cmp(s);   }


        Type& operator*()               { return get();          }
        Type* operator->()              { return &get();         }
        iterator& operator--()          { prev(); return *this;  }
        iterator& operator++()          { next(); return *this;  }
        iterator operator++(s32)        { iterator temp(*this); next(); return temp;    }
        iterator operator--(s32)        { iterator temp(*this); prev(); return temp;    }

        b8 operator!=(const iterator& other) const {
            return ref != other.ref || cmp(other.state);
        }

        b8 operator==(const iterator& other) const {
            return !operator!=(other);
        }

        b8 operator!=(const const_iterator<Container, Type&, State>& other) const {
            return ref != other.ref || cmd(other.state);
        }

        b8 operator==(const const_iterator<Container, Type&, State>& other) const {
            return !operator!=(other);
        }

    protected:
        iterator(Container* ref) : ref(ref) {}
    };

    template <class Container, typename Type, class State>
    struct const_iterator {
        friend struct basecode::iterator<Container, Type, State>;

        const Container*        ref;
        State                   state;

        static const_iterator end(const Container* ref) {
            const_iterator it(ref);
            it.end();
            return it;
        }
        static const_iterator begin(const Container* ref) {
            const_iterator it(ref);
            it.begin();
            return it;
        }

        const_iterator() {}

        const_iterator(const iterator<Container, Type, State>& other) : ref(other.ref) {
            state = other.state;
        }

        u0 end()                        { state.end(ref);        }
        u0 prev()                       { state.prev(ref);       }
        u0 next()                       { state.next(ref);       }
        u0 begin()                      { state.begin(ref);      }
        const Type get()                { return state.get(ref); }
        const Type operator*()          { return get();          }
        b8 cmp(const State& s) const    { return state.cmp(s);   }
        const_iterator& operator++()    { next(); return *this;  }
        const_iterator& operator--()    { prev(); return *this;  }
        const_iterator operator++(int)  { const_iterator temp(*this); next(); return temp;  }
        const_iterator operator--(int)  { const_iterator temp(*this); prev(); return temp;  }

        b8 operator==(const const_iterator& other) const {
            return !operator!=(other);
        }

        b8 operator!=(const const_iterator& other) const {
            return ref != other.ref || cmp(other.state);
        }

        b8 operator!=(const iterator<Container, Type, State>& other) const {
            return ref != other.ref || cmp(other.state);
        }

        b8 operator==(const iterator<Container, Type, State>& other) const {
            return !operator!=(other);
        }

        const_iterator& operator=(const iterator<Container, Type, State>& other) {
            ref   = other.ref;
            state = other.state;
            return *this;
        }

    protected:
        const_iterator(const Container* ref) : ref(ref) {}
    };

    template <class Container, typename Type, class State>
    struct const_iterator<Container, Type&, State> {
        friend struct basecode::iterator<Container, Type&, State>;

        const Container*        ref;
        State                   state;

        static const_iterator end(const Container* ref) {
            const_iterator it(ref);
            it.end();
            return it;
        }
        static const_iterator begin(const Container* ref) {
            const_iterator it(ref);
            it.begin();
            return it;
        }

        const_iterator() {}

        const_iterator(const iterator<Container, Type&, State>& other) : ref(other.ref) {
            state = other.state;
        }

        u0 end()                        { state.end(ref);        }
        u0 next()                       { state.next(ref);       }
        u0 prev()                       { state.prev(ref);       }
        u0 begin()                      { state.begin(ref);      }
        const Type& operator*()         { return get();          }
        const Type* operator->()        { return &get();         }
        const Type& get()               { return state.get(ref); }
        b8 cmp(const State& s) const    { return state.cmp(s);   }
        const_iterator& operator++()    { next(); return *this;  }
        const_iterator& operator--()    { prev(); return *this;  }
        const_iterator operator++(int)  { const_iterator temp(*this); next(); return temp;  }
        const_iterator operator--(int)  { const_iterator temp(*this); prev(); return temp;  }

        b8 operator!=(const const_iterator& other) const {
            return ref != other.ref || cmp(other.state);
        }

        b8 operator==(const const_iterator& other) const {
            return !operator!=(other);
        }

        b8 operator!=(const iterator<Container, Type&, State>& other) const {
            return ref != other.ref || cmp(other.state);
        }

        b8 operator==(const iterator<Container, Type&, State>& other) const {
            return !operator!=(other);
        }

        const_iterator& operator=(const iterator<Container, Type&, State>& other) {
            ref   = other.ref;
            state = other.state;
            return *this;
        }

    protected:
        const_iterator(const Container* ref) : ref(ref) {}
    };

    // ------------------------------------------------------------------------
    //
    // common hash{table, set, bag} functionality
    //
    // ------------------------------------------------------------------------
    namespace hash_common {
        u32 prime_capacity(u32 idx);

        b8 read_flag(const u64* data, u32 bit);

        u32 range_reduction(u64 hash, u32 size);

        u32 flag_words_for_capacity(u32 capacity);

        u0 write_flag(u64* data, u32 bit, b8 flag);

        u0 print_flags(const u64* flags, u32 size);

        s32 find_nearest_prime_capacity(u32 capacity);

        b8 requires_rehash(u32 size, u32 capacity, f32 load_factor);

        b8 find_free_bucket(const u64* hashes, u32 size, u32& bucket_idx);

        b8 find_free_bucket2(const u64* flags, u32 size, u32& bucket_idx);
    }

    // ------------------------------------------------------------------------
    //
    // stopwatch
    //
    // ------------------------------------------------------------------------
    struct stopwatch_t final {
        u64                     end;
        u64                     start;
    };

    using timed_block_callable_t    = std::function<s32()>;

    // ------------------------------------------------------------------------
    //
    // context
    //
    // ------------------------------------------------------------------------
    struct alloc_t;
    struct logger_t;

    struct context_t {
        struct {
            alloc_t*            main;
            alloc_t*            temp;
            alloc_t*            scratch;
        }                       alloc;
        logger_t*               logger;
        u0*                     user;
        const s8**              argv;
        s32                     argc;
    };

    namespace context {
        inline u0 pop();

        inline context_t* top();

        inline u0 push(context_t* ctx);

        inline context_t make(s32 argc, const s8** argv);
    }

    // ------------------------------------------------------------------------
    //
    // alloc_t, std_alloc_t
    //
    // ------------------------------------------------------------------------
    struct alloc_t;

    template <typename T>
    class std_alloc_t;

    // ------------------------------------------------------------------------
    //
    // format
    //
    // ------------------------------------------------------------------------
    class mem_buf_t;
    class str_buf_t;

    using fmt_str_t             = std::string_view;
    using fmt_args_t            = fmt::format_args;
    using fmt_alloc_t           = std_alloc_t<char>;

    // ------------------------------------------------------------------------
    //
    // hashable
    //
    // ------------------------------------------------------------------------
    namespace hash {
        template <typename K> u32 hash32(const K& value);

        template <typename K> u64 hash64(const K& value);

        template <typename T> concept Hashable = requires(T hashable) {
            { hash::hash32(hashable) } -> std::convertible_to<u32>;
            { hash::hash64(hashable) } -> std::convertible_to<u64>;
        };
    }

    // ------------------------------------------------------------------------
    //
    // str_t, slice_t
    //
    // ------------------------------------------------------------------------
    template <typename T>
    concept Slice_Concept = std::same_as<typename T::Is_Static,
                                         std::true_type>
                            && requires(const T& t) {
        {t.data}        -> std::same_as<const u8*>;
        {t.length}      -> std::same_as<u32>;
    };

    template <typename T>
    concept Static_String_Concept = std::same_as<typename T::Is_Static,
                                                 std::true_type>
                                    && requires(const T& t) {
        {t.data}        -> std::same_as<u8*>;
        {t.length}      -> std::same_as<u32>;
        {t.capacity}    -> std::same_as<u32>;
    };

    template <typename T>
    concept Dynamic_String_Concept  = std::same_as<typename T::Is_Static,
                                                   std::false_type>
                                      && requires(const T& t) {
        {t.alloc}       -> std::same_as<alloc_t*>;
        {t.data}        -> std::same_as<u8*>;
        {t.length}      -> std::same_as<u32>;
        {t.capacity}    -> std::same_as<u32>;
    };

    template <typename T>
    concept String_Concept = Slice_Concept<T>
                             || Static_String_Concept<T>
                             || Dynamic_String_Concept<T>;

    template<typename T>
    struct slice_t final {
        using Is_Static     = std::integral_constant<b8, true>;

        const T*            data;
        u32                 length;

        const T* end() const                 { return data + length; }
        const T* rend() const                { return data;          }
        const T* begin() const               { return data;          }
        const T* rbegin() const              { return data + length; }
        const T& operator[](u32 index) const { return data[index];   }
        operator std::string_view () const   { return std::string_view(
                (const s8*) data,
                length); }
    };
    static_assert(sizeof(slice_t<u8>) <= 16,
                  "slice_t<T> is now larger than 16 bytes!");

    template <typename T>
    inline b8 operator<(const slice_t<T>& lhs, const slice_t<T>& rhs) {
        if (&lhs == &rhs) return false;
        if (lhs.length < rhs.length) return true;
        return std::memcmp(lhs.data, rhs.data, lhs.length) < 0;
    }

    template <typename T>
    inline b8 operator>(const slice_t<T>& lhs, const slice_t<T>& rhs) {
        if (&lhs == &rhs) return false;
        if (lhs.length > rhs.length) return true;
        return std::memcmp(lhs.data, rhs.data, lhs.length) > 0;
    }

    template <typename T>
    inline b8 operator==(const slice_t<T>& lhs, const slice_t<T>& rhs) {
        if (&lhs == &rhs) return true;
        return lhs.length == rhs.length
               && std::memcmp(lhs.data, rhs.data, lhs.length) == 0;
    }

    struct str_t;

    namespace str {
        using slice_t = slice_t<u8>;

        template <String_Concept T,
                  b8 Is_Static = T::Is_Static::value>
        u0 free(T& str);

        template <Dynamic_String_Concept T>
        u0 grow(T& str, u32 min_capacity = 8);

        template <String_Concept D, String_Concept S>
        inline u0 append(D& str, const S& value);

        template <String_Concept T,
                  b8 Is_Static = T::Is_Static::value>
        u0 append(T& str, const u8* value, s32 len = -1);

        template <String_Concept T>
        inline u0 append(T& str, const s8* value, s32 len = -1);

        str_t make(alloc_t* alloc = context::top()->alloc.main);

        template <String_Concept T,
                  b8 Is_Static = T::Is_Static::value>
        u0 init(T& str, alloc_t* alloc = context::top()->alloc.main);

        template <String_Concept T,
                  b8 Is_Static = T::Is_Static::value>
        u0 insert(T& str, u32 pos, const u8* value, s32 len = -1);
    }

    struct str_t final {
        using Is_Static         = std::integral_constant<b8, false>;

        alloc_t*                alloc{};
        u8*                     data{};
        u32                     length{};
        u32                     capacity{};

        str_t() = default;
        ~str_t()                                        { str::free(*this); }
        str_t(str_t&& other) noexcept                   { operator=(other); }
        str_t(const str_t& other) : alloc(other.alloc)  { operator=(other); }

        explicit str_t(const s8* value,
                       alloc_t* alloc = context::top()->alloc.main);

        explicit str_t(str::slice_t value,
                       alloc_t* alloc = context::top()->alloc.main);

        u8* end()                               { return data + length; }
        u8* rend()                              { return data; }
        u8* begin()                             { return data; }
        u8* rbegin()                            { return data + length; }
        [[nodiscard]]
        const u8* end() const                   { return data + length; }
        [[nodiscard]]
        const u8* rend() const                  { return data; }
        [[nodiscard]]
        const u8* begin() const                 { return data; }
        [[nodiscard]]
        const u8* rbegin() const                { return data + length; }
        u8& operator[](u32 index)               { return data[index]; }
        operator str::slice_t() const           { return str::slice_t{data,
                                                                      length};  }
        operator std::string_view () const      { return std::string_view((const s8*) data,
                                                                          length); }
        const u8& operator[](u32 index) const   { return data[index]; }

        str_t& operator=(const str_t& other);

        str_t& operator=(str_t&& other) noexcept;

        str_t& operator=(const str::slice_t& other);

        inline str_t& operator+(const str_t& other);

        inline auto operator==(const char* other) const {
            const auto n = strlen(other);
            return length == n && std::memcmp(data, other, length) == 0;
        }

        inline auto operator==(const str_t& other) const {
            const auto cmp = std::lexicographical_compare_three_way(begin(),
                                                                    end(),
                                                                    other.begin(),
                                                                    other.end());
            return cmp == std::strong_ordering::equal;
        }

        inline auto operator<=>(const str_t& other) const {
            return std::lexicographical_compare_three_way(begin(),
                                                          end(),
                                                          other.begin(),
                                                          other.end());
        }

        inline auto operator==(const slice_t<u8>& other) const {
            return length == other.length
                   && std::memcmp(data, other.data, length) == 0;
        }
    };
    static_assert(sizeof(str_t) <= 24, "str_t is now larger than 24 bytes!");

    namespace slice {
        template<typename T>
        inline b8 empty(const slice_t<T>& slice) {
            return slice.length == 0 || slice.data == nullptr;
        }

        inline str::slice_t make(const str_t& str) {
            return str::slice_t{.data = str.data, .length = str.length};
        }

        inline str::slice_t make(const u8* str, u32 length) {
            return str::slice_t{.data = str, .length = length};
        }

        inline b8 contains(const str::slice_t& slice, s8 ch) {
            for (u32 i = 0; i < slice.length; ++i) {
                if (slice[i] == ch)
                    return true;
            }
            return false;
        }

        inline str::slice_t make(const s8* str, s32 length = -1) {
            return str::slice_t{
                .data = (const u8*) str,
                .length = length == -1 ? u32(strlen(str)) : length
            };
        }
    }

    inline str::slice_t operator "" _ss(const s8* value) {
        return slice::make(value);
    }

    inline str::slice_t operator "" _ss(const s8* value, std::size_t length) {
        return slice::make(value, length);
    }

    using line_callback_t = std::function<b8 (str::slice_t)>;

    // ------------------------------------------------------------------------
    //
    // array
    //
    // ------------------------------------------------------------------------
    template <typename T>
    concept Proxy_Array = requires(const T& t) {
        typename                T::Value_Type;
        typename                T::Size_Per_16;
        typename                T::Backing_Array;

        {t.backing}             -> std::same_as<typename T::Backing_Array>;
        {t.start}               -> std::same_as<u32>;
        {t.size}                -> std::same_as<u32>;
    };

    template <typename T>
    concept Static_Array_Concept = std::same_as<typename T::Is_Static, std::true_type>
                                   && requires(const T& t) {
        typename                T::Is_Static;
        typename                T::Value_Type;

        {t.data}                -> std::same_as<typename T::Value_Type*>;
        {t.size}                -> std::same_as<u32>;
        {t.capacity}            -> std::same_as<u32>;
    };

    template <typename T>
    concept Dynamic_Array_Concept = std::same_as<typename T::Is_Static, std::false_type>
                                    && requires(const T& t) {
         typename                T::Is_Static;
         typename                T::Value_Type;
         typename                T::Size_Per_16;

         {t.alloc}               -> std::same_as<alloc_t*>;
         {t.data}                -> std::same_as<typename T::Value_Type*>;
         {t.size}                -> std::same_as<u32>;
         {t.capacity}            -> std::same_as<u32>;
    };

    template <typename T>
    concept Array_Concept = Static_Array_Concept<T>
                            || Dynamic_Array_Concept<T>
                            || Proxy_Array<T>;

    template <typename T, u32 Capacity>
    struct static_array_t final {
        using Is_Static         [[maybe_unused]] = std::integral_constant<b8, true>;
        using Value_Type        = T;
        using Size_Per_16       [[maybe_unused]] = std::integral_constant<u32, 16 / sizeof(T)>;

        Value_Type              data[Capacity];
        u32                     size     = 0;
        u32                     capacity = Capacity;

        Value_Type& operator[](u32 index)               { return data[index];       }
        const Value_Type& operator[](u32 index) const   { return data[index];       }

        T* end()                                        { return data + size;       }
        T* rend()                                       { return data - 1;          }
        T* begin()                                      { return data;              }
        T* rbegin()                                     { return data + size - 1;   }
        [[nodiscard]] const T* end() const              { return data + size;       }
        [[nodiscard]] const T* rend() const             { return data - 1;          }
        [[nodiscard]] const T* begin() const            { return data;              }
        [[nodiscard]] const T* rbegin() const           { return data + size - 1;   }
    };

    template<typename T>
    struct array_t final {
        using Is_Static         = std::integral_constant<b8, false>;
        using Value_Type        = T;
        using Size_Per_16       = std::integral_constant<u32, 16 / sizeof(T)>;

        alloc_t*                alloc;
        Value_Type*             data;
        u32                     size;
        u32                     capacity;

        Value_Type& operator[](u32 index)               { return data[index]; }
        const Value_Type& operator[](u32 index) const   { return data[index]; }

        T* end()                              { return data + size;     }
        T* rend()                             { return data - 1;        }
        T* begin()                            { return data;            }
        T* rbegin()                           { return data + size - 1; }
        [[nodiscard]] const T* end() const    { return data + size;     }
        [[nodiscard]] const T* rend() const   { return data - 1;        }
        [[nodiscard]] const T* begin() const  { return data;            }
        [[nodiscard]] const T* rbegin() const { return data + size - 1; }
    };
    static_assert(sizeof(array_t<s32>) <= 24,
                  "array_t<T> is now larger than 24 bytes!");

    using slice_array_t         = array_t<str::slice_t>;

    namespace slice {
        template <typename T>
        inline slice_t<T> make(const Array_Concept auto& array,
                               u32 start,
                               u32 end) {
            return slice_t{.data = array.data + start, .length = end - start};
        }

        template <typename T>
        inline slice_t<T> make(const Array_Concept auto& array) {
            return slice_t{.data = array.data, .length = array.size};
        }
    }

    // ------------------------------------------------------------------------
    //
    // stable_array
    //
    // ------------------------------------------------------------------------
    template <typename T>
    concept Stable_Array = requires(const T& t) {
        typename                T::Value_Type;

        {t.alloc}               -> std::same_as<alloc_t*>;
        {t.slab}                -> std::same_as<alloc_t*>;
        {t.data}                -> std::same_as<typename T::Value_Type**>;
        {t.size}                -> std::same_as<u32>;
        {t.capacity}            -> std::same_as<u32>;
    };

    template<typename T>
    struct stable_array_t final {
        using Value_Type        = T;
        using Is_Static         = std::integral_constant<b8, false>;
        using Size_Per_16       = std::integral_constant<u32,
                                                         16 / sizeof(Value_Type)>;

        alloc_t*                alloc;
        alloc_t*                slab;
        Value_Type**            data;
        u32                     size;
        u32                     capacity;

        Value_Type& operator[](u32 index)             { return *data[index]; }
        const Value_Type& operator[](u32 index) const { return *data[index]; }

        struct iterator_state_t {
            u32                 pos;
            inline u0 end(const stable_array_t* ref)       { pos = ref->size;       }
            inline u0 next(const stable_array_t* ref)      { UNUSED(ref); ++pos;    }
            inline u0 begin(const stable_array_t* ref)     { UNUSED(ref); pos = 0;  }
            inline Value_Type* get(stable_array_t* ref)    { return ref->data[pos]; }
            inline b8 cmp(const iterator_state_t& s) const { return pos != s.pos;   }
            inline const Value_Type* get(const stable_array_t* ref) {
                return ref->data[pos];
            }
        };
        DECL_ITERS(stable_array_t, Value_Type*, iterator_state_t);
    };

    // ------------------------------------------------------------------------
    //
    // assoc_array
    //
    // ------------------------------------------------------------------------
    struct assoc_key_idx_t final {
        u64                         status: 2;
        u64                         offset: 31;
        u64                         length: 31;
    };

    template <typename V>
    struct assoc_pair_t final {
        using Value_Type            = std::remove_pointer_t<V>;

        str::slice_t                key;
        Value_Type*                 value;
    };

    template <typename V>
    struct assoc_array_t final {
        using Value_Type            = V;

        alloc_t*                    alloc;
        Value_Type*                 values;
        assoc_key_idx_t*            index;
        struct {
            u8*                     data;
            u32                     size;
            u32                     capacity;
        }                           keys;
        u32                         size;
        u32                         capacity;

        assoc_pair_t<Value_Type> operator[](u32 i) const {
            const auto& idx = index[i];
            if constexpr (std::is_pointer_v<Value_Type>) {
                return assoc_pair_t<V>{
                    slice::make(keys.data + idx.offset, idx.length),
                    values[i]
                };
            } else {
                return assoc_pair_t<V>{
                    slice::make(keys.data + idx.offset, idx.length),
                    &values[i]
                };
            }
        }
    };
    static_assert(sizeof(assoc_array_t<s32>) <= 48,
                  "assoc_array_t is now greater than 48 bytes!");

    // ------------------------------------------------------------------------
    //
    // ast
    //
    // ------------------------------------------------------------------------
    namespace ast {
        namespace node {
            namespace field {
                static constexpr u8 none        = 0b00000;
                static constexpr u8 id          = 0b00001;
                static constexpr u8 lhs         = 0b00010;
                static constexpr u8 rhs         = 0b00011;
                static constexpr u8 expr        = 0b00100;
                static constexpr u8 flags       = 0b00101;
                static constexpr u8 token       = 0b00110;
                static constexpr u8 radix       = 0b00111;
                static constexpr u8 child       = 0b01000;
                static constexpr u8 parent      = 0b01001;
                static constexpr u8 custom      = parent + 1;
            }

            namespace header {
                static constexpr u8 none        = 0b00000;
                static constexpr u8 ident       = 0b00001;
                static constexpr u8 unary       = 0b00010;
                static constexpr u8 binary      = 0b00011;
                static constexpr u8 comment     = 0b00100;
                static constexpr u8 str_lit     = 0b00101;
                static constexpr u8 num_lit     = 0b00110;
                static constexpr u8 custom      = num_lit + 1;
            }
        }

        struct node_id_t final {
            constexpr node_id_t()       : id(0)      {}
            constexpr node_id_t(u32 id) : id(id)     {}
            constexpr operator u32() const           { return id;      }
            [[nodiscard]] constexpr b8 empty() const { return id == 0; }
            static constexpr node_id_t null()        { return 0;       }
        private:
            u32                     id;
        };

        struct node_type_t final {
            constexpr node_type_t()         : type(0)    {}
            constexpr node_type_t(u8 type)  : type(type) {}
            constexpr operator u8() const                { return type;      }
            [[nodiscard]] constexpr b8 empty() const     { return type == 0; }
            static constexpr node_type_t none()          { return 0;         }
        private:
            u8                      type:   5;
            [[maybe_unused]] u8     pad:    3{};
        };

        struct num_lit_flags_t final {
            u8                      sign:       1;
            u8                      integer:    1;
            u8                      exponent:   1;
            [[maybe_unused]] u8     pad:        5;
        };
    }

    // ------------------------------------------------------------------------
    //
    // avl
    //
    // ------------------------------------------------------------------------
    template <typename T>
    struct avl_t final {
        struct node_t;

        using Has_Color         = std::integral_constant<b8, false>;
        using Node_Type         = node_t*;
        using Value_Type        = T;
        static constexpr u32    Value_Type_Size    = sizeof(T);
        static constexpr u32    Value_Type_Align   = alignof(T);

        struct node_t final {
            node_t*             lhs;
            node_t*             rhs;
            node_t*             parent;
            Value_Type*         value;
            s8                  balance;
        };
        static_assert(sizeof(node_t) <= 40,
                      "avl<T>::node_t is now larger than 40 bytes!");

        static constexpr u32    Node_Type_Size     = sizeof(node_t);
        static constexpr u32    Node_Type_Align    = alignof(node_t);

        alloc_t*                alloc;
        alloc_t*                node_slab;
        alloc_t*                value_slab;
        Node_Type               root;
        u32                     size;
    };
    static_assert(sizeof(avl_t<u32>) <= 40,
                  "avl_t<u32> is now larger than 40 bytes!");

    // ------------------------------------------------------------------------
    //
    // bag
    //
    // ------------------------------------------------------------------------
    template <typename T>
    concept Hash_Bag = hash::Hashable<T> && requires(const T& t) {
        typename                T::Item_Type;
        typename                T::Value_Type;
        typename                T::Is_Pointer;
        typename                T::Value_Type_Base;

        {t.alloc}               -> std::same_as<alloc_t*>;
        {t.flags}               -> std::same_as<u64*>;
        {t.hashes}              -> std::same_as<u64*>;
        {t.counts}              -> std::same_as<u32*>;
        {t.values}              -> std::same_as<typename T::Value_Type*>;
        {t.size}                -> std::same_as<u32>;
        {t.capacity}            -> std::same_as<u32>;
        {t.load_factor}         -> std::same_as<f32>;
    };

    struct bag_buf_size_t final {
        u32                     total_size;
        u32                     size_of_flags;
        u32                     num_flag_words;
        u32                     size_of_hashes;
        u32                     size_of_counts;
        u32                     size_of_values;
    };

    template <typename T>
    struct bag_t final {
        using Value_Type        = T;
        using Is_Pointer        = std::integral_constant<b8, std::is_pointer_v<T>>;
        using Value_Type_Base   = std::remove_pointer_t<T>;

        struct item_t final {
            Value_Type_Base*    value;
            u32                 count;
        };
        using Item_Type         = item_t;

        alloc_t*                alloc;
        u64*                    flags;
        u64*                    hashes;
        u32*                    counts;
        Value_Type*             values;
        u32                     size;
        u32                     cap_idx;
        u32                     capacity;
        f32                     load_factor;

        struct iterator_state_t {
            u32                 pos;

            inline u0 end(const bag_t* ref) {
                pos = ref->capacity;
            }

            inline u0 next(const bag_t* ref) {
                while (pos < ref->capacity) {
                    if (ref->hashes[++pos])
                        break;
                }
            }

            inline Item_Type get(bag_t* ref) {
                return {&ref->values[pos], ref->counts[pos]};
            }

            inline u0 begin(const bag_t* ref) {
                pos = 0;
                while (pos < ref->capacity) {
                    if (ref->hashes[pos])
                        break;
                    ++pos;
                }
            }

            inline const Item_Type get(const bag_t* ref) {
                return {&ref->values[pos], ref->counts[pos]};
            }

            inline b8 cmp(const iterator_state_t& s) const {
                return pos != s.pos;
            }
        };
        DECL_ITERS(bag_t, Item_Type, iterator_state_t);
    };
    static_assert(sizeof(bag_t<s32>) <= 56,
                  "bag_t<T> is now larger than 56 bytes!");

    // ------------------------------------------------------------------------
    //
    // bass
    // (basecode awesome storage system)
    //
    // ------------------------------------------------------------------------
    struct bass_t;
    struct field_t;
    struct cursor_t;
    struct field_dict_t;
    struct field_index_t;

    enum class format_type_t : u8 {
        header,
        field
    };

    using record_index_t = array_t<field_index_t>;

    namespace kind {
        constexpr u8 none       = 0b000;
        constexpr u8 blob       = 0b001;
        constexpr u8 field      = 0b010;
        constexpr u8 header     = 0b100;

        str::slice_t name(u8);
    }

    namespace field {
        constexpr u8 none       = 0b00000;
        constexpr u8 id         = 0b00001;
    }

    template <typename Buffer>
    using format_record_callback_t = b8 (*)(format_type_t,
                                            cursor_t&,
                                            Buffer&,
                                            u0*);

    // ------------------------------------------------------------------------
    //
    // bintree
    //
    // ------------------------------------------------------------------------
    template <typename T>
    concept Binary_Tree = requires(const T& t) {
        typename                T::Node_Type;
        typename                T::Value_Type;

        {t.alloc}               -> std::same_as<alloc_t*>;
        {t.node_slab}           -> std::same_as<alloc_t*>;
        {t.value_slab}          -> std::same_as<alloc_t*>;
        {t.root}                -> std::same_as<typename T::Node_Type>;
        {t.size}                -> std::same_as<u32>;
    };

    template <Binary_Tree Tree_Type,
              typename Node_Type = typename Tree_Type::Node_Type>
    struct bin_tree_cursor_t;

    namespace bintree {
        namespace color {
            constexpr u8 none   = 0;
            constexpr u8 red    = 1;
            constexpr u8 black  = 2;
        }
    }

    // ------------------------------------------------------------------------
    //
    // bitset
    //
    // ------------------------------------------------------------------------
    struct bitset_t final {
        alloc_t*                alloc;
        u64*                    data;
        u32                     capacity;

        bitset_t& operator<<(u32 bits);

        bitset_t& operator>>(u32 bits);
    };

    // ------------------------------------------------------------------------
    //
    // bst
    //
    // ------------------------------------------------------------------------
    template <typename T>
    struct bst_t final {
        struct node_t;

        using Has_Color         = std::integral_constant<b8, false>;
        using Node_Type         = node_t*;
        using Value_Type        = T;
        static constexpr u32    Value_Type_Size    = sizeof(T);
        static constexpr u32    Value_Type_Align   = alignof(T);

        struct node_t final {
            node_t*             lhs;
            node_t*             rhs;
            node_t*             parent;
            Value_Type*         value;
        };
        static_assert(sizeof(node_t) <= 32,
                      "bst<T>::node_t is now larger than 32 bytes!");

        static constexpr u32    Node_Type_Size     = sizeof(node_t);
        static constexpr u32    Node_Type_Align    = alignof(node_t);

        alloc_t*                alloc;
        alloc_t*                node_slab;
        alloc_t*                value_slab;
        Node_Type               root;
        u32                     size;
    };
    static_assert(sizeof(bst_t<u32>) <= 40,
                  "bst_t<u32> is now larger than 40 bytes!");

    // ------------------------------------------------------------------------
    //
    // buf_pool
    //
    // ------------------------------------------------------------------------
    struct lease_t final {
        alloc_t*                alloc;
        u8*                     buf;
        u32                     size;
    };

    namespace buf_pool {
        enum class status_t : u8 {
            ok                  = 0,
        };
    }

    // ------------------------------------------------------------------------
    //
    // log
    //
    // ------------------------------------------------------------------------
    struct logger_t;
    struct logger_config_t;
    struct logger_system_t;
    union logger_subclass_t;

    enum class logger_type_t : u8 {
        default_                = 240,
        spdlog,
        syslog,
    };

    enum class log_level_t : u8 {
        emergency,
        alert,
        critical,
        error,
        warn,
        notice,
        info,
        debug,
    };

    using logger_array_t        = array_t<logger_t*>;
    using shared_logger_t       = std::shared_ptr<::spdlog::logger>;

    using logger_fini_callback_t = u0 (*)(logger_t*);
    using logger_init_callback_t = u0 (*)(logger_t*, logger_config_t*);
    using logger_emit_callback_t = u0 (*)(logger_t*, log_level_t,
                                          fmt_str_t, const fmt_args_t&);

    namespace log {
        enum class status_t : u8 {
            ok,
            invalid_logger,
            invalid_logger_system,
            invalid_default_logger,
        };
    }

    // ------------------------------------------------------------------------
    //
    // decimal
    //
    // ------------------------------------------------------------------------
    struct decimal_t final {
        alloc_t*                alloc;
        u64*                    data;
        u32                     size;
        u32                     capacity;
    };

    // ------------------------------------------------------------------------
    //
    // digraph
    //
    // ------------------------------------------------------------------------
    template <typename T>
    concept Directed_Graph = requires(const T& t) {
        typename                T::Node;
        typename                T::Edge;
        typename                T::Edge_Set;
        typename                T::Edge_Pair;
        typename                T::Value_Type;
        typename                T::Node_Array;
        typename                T::Edge_Array;
        typename                T::Node_Stack;
        typename                T::Edge_Pair_Array;
        typename                T::Component_Array;

        {t.alloc}               -> std::same_as<alloc_t*>;
        {t.node_slab}           -> std::same_as<alloc_t*>;
        {t.edge_slab}           -> std::same_as<alloc_t*>;
        {t.nodes}               -> std::same_as<typename T::Node_Array>;
        {t.edges}               -> std::same_as<typename T::Edge_Array>;
        {t.outgoing}            -> std::same_as<typename T::Edge_Set>;
        {t.incoming}            -> std::same_as<typename T::Edge_Set>;
        {t.size}                -> std::same_as<u32>;
        {t.id}                  -> std::same_as<u32>;
    };

    template <typename V>
    struct digraph_t;

    // ------------------------------------------------------------------------
    //
    // event
    //
    // ------------------------------------------------------------------------
    struct event_t_;

    using event_t               = event_t_*;
    using event_array_t         = array_t<event_t>;

    namespace event {
        enum class status_t : u8 {
            ok                  = 0,
            error               = 106,
            timeout             = 107,
        };
    }

    // ------------------------------------------------------------------------
    //
    // family
    //
    // ------------------------------------------------------------------------
    template <typename...>
    struct family_t final {
        inline static u32       id  {};

    public:
        using Family_Type       = u32;

        template <typename... Type>
        inline static const Family_Type type = id++;
    };

    // ------------------------------------------------------------------------
    //
    // filesys
    //
    // ------------------------------------------------------------------------
    struct glob_result_t final {
        glob_t                  buf;
        array_t<str_t>          paths;
    };
    static_assert(sizeof(glob_result_t) <= 112,
                  "glob_result_t is now bigger than 112 bytes!");

    namespace filesys {
        enum class status_t : u8 {
            ok                              = 0,
            not_dir                         = 116,
            not_file                        = 117,
            not_exists                      = 118,
            invalid_dir                     = 119,
            chdir_failure                   = 120,
            file_writable                   = 121,
            mkdir_failure                   = 122,
            getcwd_failure                  = 123,
            rename_failure                  = 124,
            remove_failure                  = 125,
            not_equivalent                  = 126,
            mkdtemp_failure                 = 127,
            not_implemented                 = 128,
            unexpected_path                 = 129,
            realpath_failure                = 130,
            cannot_modify_root              = 131,
            unexpected_empty_path           = 132,
            cannot_rename_to_existing_file  = 133,
            xdg_user_dirs_invalid           = 134,
            no_home_path                    = 135,  // FIXME
            invalid_user_place              = 136,  // FIXME
            glob_no_match                   = 137,  // FIXME
        };
    }

    // ------------------------------------------------------------------------
    //
    // gap_buf
    //
    // ------------------------------------------------------------------------
    struct gap_t final {
        u32                     start;
        u32                     end;
    };

    struct gap_buf_t final {
        alloc_t*                alloc;
        u8*                     data;
        gap_t                   gap;
        u32                     size;
        u32                     caret;
        u32                     capacity;
        u32                     gap_size;
    };

    namespace gap_buf {
        enum class status_t : u8 {
            ok                  = 0,
        };
    }

    // ------------------------------------------------------------------------
    //
    // getopt
    //
    // ------------------------------------------------------------------------
    struct arg_t;
    struct option_t;

    using arg_array_t           = array_t<arg_t>;
    using option_array_t        = array_t<option_t>;

    enum class arg_type_t : u8 {
        none,
        flag,
        file,
        string,
        integer,
        decimal,
    };

    struct option_t final {
        str::slice_t            long_name;
        str::slice_t            value_name;
        str::slice_t            description;
        s32                     max_allowed;
        s32                     min_required;
        u8                      radix;
        s8                      short_name;
        arg_type_t              type;
    };

    union arg_subclass_t final {
        b8                      flag;
        str::slice_t            string;
        s64                     integer;
        f64                     decimal;
    };

    struct arg_t final {
        option_t*               option;
        arg_subclass_t          subclass;
        u32                     pos;
        arg_type_t              type;
        b8                      extra;
    };

    struct getopt_t final {
        alloc_t*                alloc;
        const s8**              argv;
        option_t*               file_option;
        str::slice_t            description;
        arg_array_t             args;
        option_array_t          opts;
        arg_array_t             extras;
        s32                     argc;
    };

    namespace getopt {
        enum class status_t : u32 {
            ok                              = 0,
            unconfigured,
            invalid_option,
            expected_value,
            duplicate_option,
            invalid_argument,
            count_exceeds_allowed,
            missing_required_option,
            decimal_conversion_error,
            integer_conversion_error,
        };

        class option_builder_t;
    }

    // ------------------------------------------------------------------------
    //
    // hashtab
    //
    // ------------------------------------------------------------------------
    template <typename T>
    concept Hash_Table = hash::Hashable<typename T::Key_Type>
                         && requires(const T& t) {
        typename                T::Key_Type;
        typename                T::Pair_Type;
        typename                T::Value_Type;

        {t.flags}               -> std::same_as<u64*>;
        {t.hashes}              -> std::same_as<u64*>;
        {t.keys}                -> std::same_as<typename T::Key_Type*>;
        {t.values}              -> std::same_as<typename T::Value_Type*>;
        {t.alloc}               -> std::same_as<alloc_t*>;
        {t.size}                -> std::same_as<u32>;
        {t.capacity}            -> std::same_as<u32>;
        {t.load_factor}         -> std::same_as<f32>;
    };

    template <typename K, typename V>
    struct hashtab_t final {
        struct pair_t;

        using Key_Type          = K;
        using Pair_Type         = pair_t;
        using Value_Type        = V;

        u64*                    flags;
        u64*                    hashes;
        Key_Type*               keys;
        Value_Type*             values;
        alloc_t*                alloc;
        u32                     size;
        u32                     cap_idx;
        u32                     capacity;
        f32                     load_factor;

        struct pair_t final {
            Key_Type            key;
            Value_Type          value;
            u32                 bucket_idx;

            auto operator<(const pair_t& rhs) const {
                return key < rhs.key;
            }
        };

        Value_Type& operator[](u32 bucket_idx) {
            return values[bucket_idx];
        }

        const Pair_Type operator[](u32 idx)  const {
            b8 ok{};
            while (idx < capacity) {
                if (hash_common::read_flag(flags, idx)) {
                    ok = true;
                    break;
                }
                ++idx;
            }
            if (ok) {
                pair_t p;
                p.key        = keys[idx];
                p.value      = values[idx];
                p.bucket_idx = idx;
                return p;
            }
            return {};
        }

        struct iterator_state_t {
            u32                 pos;

            inline u0 end(const hashtab_t* ref) {
                pos = ref->capacity;
            }

            inline u0 next(const hashtab_t* ref) {
                while (pos < ref->capacity) {
                    if (hash_common::read_flag(ref->flags, ++pos))
                        break;
                }
            }

            inline Pair_Type get(hashtab_t* ref) {
                pair_t p{};
                p.key        = ref->keys[pos];
                p.value      = ref->values[pos];
                p.bucket_idx = pos;
                return p;
            }

            inline u0 begin(const hashtab_t* ref) {
                pos = 0;
                while (pos < ref->capacity) {
                    if (hash_common::read_flag(ref->flags, pos))
                        break;
                    ++pos;
                }
            }

            inline const Pair_Type get(const hashtab_t* ref) {
                pair_t p{};
                p.key        = ref->keys[pos];
                p.value      = ref->values[pos];
                p.bucket_idx = pos;
                return p;
            }

            inline b8 cmp(const iterator_state_t& s) const {
                return pos != s.pos;
            }
        };
        DECL_ITERS(hashtab_t, Pair_Type, iterator_state_t);
    };
    static_assert(sizeof(hashtab_t<s32, s32>) <= 56,
                  "hashtab_t<K, V> is now larger than 56 bytes!");

    // ------------------------------------------------------------------------
    //
    // integer
    //
    // ------------------------------------------------------------------------
    struct integer_t final {
        alloc_t*                alloc;
        u64*                    data;
        u32                     size;
        u32                     offset;
        u32                     capacity;
    };
    static_assert(sizeof(integer_t) <= 32,
                  "integer_t is now larger than 32 bytes!");

    namespace integer {
        enum class status_t : u32 {
            ok
        };
    }

    // ------------------------------------------------------------------------
    //
    // intern
    //
    // ------------------------------------------------------------------------
    struct intern_t;
    struct interned_str_t;

    using intern_id             = u32;
    using interned_str_list_t   = array_t<interned_str_t>;

    struct interned_str_t final {
        str::slice_t            value;
        u32                     bucket_index;
    };

    struct intern_t final {
        alloc_t*                alloc;
        intern_id*              ids;
        u64*                    hashes;
        interned_str_list_t     strings;
        u32                     size;
        u32                     cap_idx;
        u32                     capacity;
        f32                     load_factor;
    };
    static_assert(sizeof(intern_t) <= 88,
                  "intern_t is now larger than 88 bytes!");

    namespace intern {
        enum class status_t : u8 {
            ok                  = 0,
            no_bucket           = 135,
            not_found           = 136,
        };

        struct result_t final {
            u64                 hash        {};
            str::slice_t        slice       {};
            intern_id           id          {};
            status_t            status      {};
            b8                  new_value   {};

            b8 operator==(const result_t& other) const {
                return id == other.id;
            }
        };
    }

    using interned_array_t      = array_t<intern::result_t>;

    // ------------------------------------------------------------------------
    //
    // locale
    //
    // ------------------------------------------------------------------------
    struct locale_key_t final {
        u32                     id;
        s8                      locale[8];

        auto operator<(const locale_key_t& other) const {
            return id < other.id;
        }

        auto operator==(const locale_key_t& other) const {
            return id == other.id
                   && strncmp(locale, other.locale, sizeof(locale)) == 0;
        }
    };

    using localized_strtab_t    = hashtab_t<locale_key_t, intern::result_t>;

    namespace locale {
        enum class status_t : u8 {
            ok,
        };
    }

    // ------------------------------------------------------------------------
    //
    // ipc
    //
    // ------------------------------------------------------------------------
    namespace ipc {
        enum class status_t : u8 {
            ok                  = 0,
            error,
        };
    }

    // ------------------------------------------------------------------------
    //
    // job
    //
    // ------------------------------------------------------------------------
    struct job_t;

    using job_id                = u32;
    using job_list_t            = stable_array_t<job_t>;
    using job_id_list_t         = array_t<job_id>;

    struct job_task_base_t {
        virtual ~job_task_base_t() = default;

        virtual u0 run() = 0;

        virtual u0* get_ret_val() = 0;

        virtual u0 set_job(job_t* job) = 0;
    };

    template <typename Proc, typename... Args>
    struct job_task_t : job_task_base_t {
        using Result_Type       = std::invoke_result_t<Proc, Args...>;
        static
        constexpr b8 Is_Void    = std::is_void_v<Result_Type>;

        using Proc_Type         = Proc;
        using Args_Type         = std::tuple<Args...>;
        using Return_Type       = typename std::conditional<Is_Void,
            s32,
            Result_Type>::type;

        job_t*                  job;
        Proc_Type               proc;
        Args_Type               args;
        Return_Type             ret_val;

        job_task_t(Proc_Type proc, Args_Type args) : job(),
                                                     proc(proc),
                                                     args(std::move(args)),
                                                     ret_val() {
        }

        u0 run() override {
            if constexpr (Is_Void) {
                std::apply(proc, args);
                ret_val = {};
            } else {
                ret_val = std::apply(proc, args);
            }
        }

        u0* get_ret_val() override          { return &ret_val;   }
        u0 set_job(job_t* value) override   { job = value;       }
    };

    enum class job_state_t : u8 {
        queued                  = 250,
        created,
        running,
        finished,
    };

    // N.B. job subsystem uses a slab allocator to store instances
    //      of job_task_t.  because job_task_t's size varies based on
    //      the types passed at compile time, we don't know the exact
    //      memory footprint.
    //
    //      assume there's 32 bytes of free memory per job_task_t to store
    //      the parameters passed.
    constexpr u32 job_task_buffer_size = 64;

    struct job_t final {
        job_task_base_t*        task;
        event_t                 finished;
        stopwatch_t             time;
        job_id_list_t           children;
        job_id                  parent;
        job_id                  id;
        u32                     label_id;
        job_state_t             state;

        b8 operator==(const job_t& other) const { return id == other.id; };
    };
    static_assert(sizeof(job_t) <= 72,
                  "sizeof(job_t) is now greater than 72 bytes!");

    namespace job {
        enum class status_t : u8 {
            ok                      = 0,
            busy                    = 138,
            error                   = 139,
            invalid_job_id          = 140,
            invalid_job_state       = 141,
            label_intern_failure    = 142,
        };
    }

    // ------------------------------------------------------------------------
    //
    // leb128
    //
    // ------------------------------------------------------------------------
    struct leb128_t final {
        u8                      data[16];
        u32                     size;
        b8                      is_signed;
    };

    namespace leb {
        constexpr u8 sign_bit         = 0x40;
        constexpr u8 continuation_bit = 0x80;
        constexpr u8 lower_seven_bits = 0x7f;

        enum class status_t {
            ok,
            decode_error
        };
    }

    // ------------------------------------------------------------------------
    //
    // list
    //
    // ------------------------------------------------------------------------
    template <typename T>
    concept Linked_List = requires(const T& t) {
        typename                T::Value_Type;
        typename                T::Node_Array;
        typename                T::Value_Array;

        {t.alloc}               -> std::same_as<alloc_t*>;
        {t.nodes}               -> std::same_as<typename T::Node_Array>;
        {t.values}              -> std::same_as<typename T::Value_Array>;
        {t.head}                -> std::same_as<u32>;
        {t.tail}                -> std::same_as<u32>;
        {t.free}                -> std::same_as<u32>;
        {t.size}                -> std::same_as<u32>;
    };

    struct list_node_t final {
        u64                     free    : 1;
        u64                     prev    : 21;
        u64                     next    : 21;
        u64                     value   : 21;
    };

    template <typename T>
    struct list_t final {
        using Value_Type        = T;
        using Node_Array        = array_t<list_node_t>;
        using Value_Array       = array_t<Value_Type>;

        alloc_t*                alloc;
        Node_Array              nodes;
        Value_Array             values;
        u32                     head;
        u32                     tail;
        u32                     free;
        u32                     size;

        struct iterator_state_t {
            u32                 pos;

            inline u0 end(const list_t* ref) {
                pos = ref->nodes.size + 1;
            }
            inline u0 next(const list_t* ref) {
                pos = (&ref->nodes[pos - 1])->next;
                pos = pos == 0 ? ref->nodes.size + 1 : pos;
            }
            inline u0 begin(const list_t* ref) {
                pos = ref->head;
            }
            inline list_node_t* get(list_t* ref) {
                return &ref->nodes[pos - 1];
            }
            inline b8 cmp(const iterator_state_t& s) const {
                return pos != s.pos;
            }
            inline const list_node_t* get(const list_t* ref) {
                return &ref->nodes[pos - 1];
            }
        };
        DECL_ITERS(list_t, list_node_t*, iterator_state_t);
    };

    // ------------------------------------------------------------------------
    //
    // macro
    //
    // ------------------------------------------------------------------------
    struct macro_t;

    enum class macro_op_t : u8 {
        nop,
        get,
        set,
        cmp,
        je,
        jne,
        jl,
        jle,
        jg,
        jge,
        copy,
    };

    enum class macro_directive_t : u8 {
        none,
        if_,
        odd,
        each,
        loop,
        last,
        even,
        first,
        local,
        after,
        macro,
        upper,
        lower,
        label,
        global,
        substr,
        escape,
        break_,
        binary,
        before,
        between,
        nothing,
        include,
        for_each,
        continue_,
        after_all,
        before_all,
    };

    using macro_array_t         = array_t<macro_t*>;

    namespace macro {
        enum class status_t : u32 {
            ok,
            error
        };
    }

    // ------------------------------------------------------------------------
    //
    // memory
    //
    // ------------------------------------------------------------------------
    struct slab_t;
    struct alloc_t;
    struct mem_result_t;
    struct buddy_block_t;
    struct page_header_t;
    struct alloc_system_t;
    struct alloc_config_t;
    struct system_config_t;
    union  alloc_subclass_t;

    using mspace                = u0*;
    using mem_fini_callback_t   = u32 (*)(alloc_t*);
    using mem_size_callback_t   = u32 (*)(alloc_t*, u0* mem);
    using mem_free_callback_t   = u32 (*)(alloc_t*, u0* mem);
    using mem_init_callback_t   = u0  (*)(alloc_t*, alloc_config_t*);
    using mem_alloc_callback_t  = mem_result_t (*)(alloc_t*, u32 size, u32 align);
    using mem_realloc_callback_t= mem_result_t (*)(alloc_t*, u0* mem, u32 size, u32 align);

    enum class alloc_type_t : u8 {
        none,
        base,
        bump,
        page,
        slab,
        temp,
        proxy,
        trace,
        stack,
        buddy,
        scratch,
        dlmalloc,
    };

    namespace memory {
        enum class status_t : u8 {
            ok,
            invalid_allocator,
            invalid_default_allocator,
            invalid_allocation_system,
        };
    }

    // ------------------------------------------------------------------------
    //
    // mutex
    //
    // ------------------------------------------------------------------------
    struct mutex_t;
    struct scoped_lock_t;

    namespace mutex {
        enum class status_t : u8 {
            ok                          = 0,
            busy                        = 143,
            error                       = 144,
            invalid_mutex               = 145,
            out_of_memory               = 146,
            create_mutex_failure        = 147,
            insufficient_privilege      = 148,
            thread_already_owns_lock    = 149,
        };
    }

    // ------------------------------------------------------------------------
    //
    // network
    //
    // ------------------------------------------------------------------------
#ifdef _WIN32
#   define SOCKET                   unsigned __int64
#else
#   define INVALID_SOCKET           (-1)
#   define SOCKET                   s32
#endif
#ifndef MSG_NOSIGNAL
#   define MSG_NOSIGNAL             0
#endif
    struct socket_t;

    using socket_read_callback_t    = b8 (*)(socket_t&, u0*);
    using socket_close_callback_t   = u0 (*)(socket_t&);

    struct socket_t final {
        alloc_t*                alloc;
        u8*                     buf;
        u8*                     buf_cur;
        u0*                     user;
        socket_close_callback_t close_cb;
        SOCKET                  socket;
        u32                     buf_free;
        u32                     buf_size;
    };

    struct ip_address_t final {
        u32                     number;
        s8                      text[17];
    };

    namespace network {
        enum class status_t : u8 {
            ok                              = 0,
            bind_failure                    = 150,
            listen_failure                  = 151,
            connect_failure                 = 152,
            socket_dgram_error              = 153,
            socket_already_open             = 154,
            winsock_init_failure            = 155,
            socket_already_closed           = 156,
            invalid_address_and_port        = 157,
            socket_option_broadcast_error   = 158,
        };
    }

    // ------------------------------------------------------------------------
    //
    // numbers
    //
    // ------------------------------------------------------------------------
    namespace numbers {
        enum class status_t : u8 {
            ok,
            overflow,
            underflow,
            not_convertible
        };
    }

    // ------------------------------------------------------------------------
    //
    // obj_pool
    //
    // ------------------------------------------------------------------------
    struct obj_pool_t;
    struct obj_type_t;

    using obj_array_t           = array_t<u0*>;
    using slab_table_t          = hashtab_t<u32, obj_type_t>;
    using destroy_callback_t    = u0 (*)(const u0*);

    struct obj_type_t final {
        alloc_t*                alloc;
        destroy_callback_t      destroyer;
        u32                     type_id;
        const s8*               type_name;
        obj_array_t             objects;
    };

    struct obj_pool_t final {
        alloc_t*                alloc;
        slab_table_t            slabs;
        u32                     size;
    };
    static_assert(sizeof(obj_pool_t) <= 72,
                  "obj_pool_t is now larger than 72 bytes!");

    namespace obj_pool {
        enum class status_t : u32 {
            ok                  = 0,
        };
    }

    // ------------------------------------------------------------------------
    //
    // path
    //
    // ------------------------------------------------------------------------
    struct path_t;
    struct path_mark_t;

    using path_array_t          = array_t<path_t>;
    using path_mark_list_t      = array_t<path_mark_t>;

#ifdef _WIN32
    constexpr s8 path_sep       = '\\';
#else
    constexpr s8 path_sep       = '/';
#endif

    struct path_mark_t final {
        u16                     type:       4;
        u16                     value:      12;
    };

    struct path_t final {
        str_t                   str;
        path_mark_list_t        marks;

        b8 operator==(const path_t& other) const {
            return str == other.str;
        }
    };
    static_assert(sizeof(path_t) <= 48,
                  "path_t is now larger than 48 bytes!");

    namespace path {
        namespace marks {
            [[maybe_unused]] constexpr u16 none         = 0;
#ifdef _WIN32
            [[maybe_unused]] constexpr u16 drive_name   = 1;
#endif
            [[maybe_unused]] constexpr u16 root_part    = 2;
            [[maybe_unused]] constexpr u16 path_part    = 3;
            [[maybe_unused]] constexpr u16 extension    = 4;
            [[maybe_unused]] constexpr u16 parent_dir   = 5;
            [[maybe_unused]] constexpr u16 current_dir  = 6;
        }

        enum class status_t : u8 {
            ok                          = 0,
            path_too_long               = 159,
            no_parent_path              = 160,
            unexpected_empty_path       = 161,
            expected_relative_path      = 162,
            unexpected_empty_extension  = 163,
        };
    }

    // ------------------------------------------------------------------------
    //
    // eav
    //
    // ------------------------------------------------------------------------
    constexpr s32 app_id        = 0x000dead1;

    struct db_t;
    struct tuple_t;
    struct entity_t;

    using tuple_list_t          = array_t<tuple_t>;
    using entity_list_t         = array_t<entity_t>;

    enum class value_type_t : u8 {
        nil,
        list,
        entity,
        string,
        boolean,
        integer,
        floating_point,
    };

    struct entity_t final {
        constexpr entity_t()        : id(0)         {}
        constexpr entity_t(s64 id)  : id(id)        {}
        constexpr operator s64() const              { return id;        }
        [[nodiscard]] constexpr b8 empty() const    { return id == 0;   }
    private:
        s64                     id;
    };

    union value_data_t final {
        entity_t                entity;
        union {
            u64                 u;
            f64                 f;
        }                       number;
        str_t                   string;
        entity_list_t           list;

        ~value_data_t()                                         {}
        value_data_t() : list()                                 {}
        explicit value_data_t(u64 value) : number({.u = value}) {}
        explicit value_data_t(f64 value) : number({.f = value}) {}
        explicit value_data_t(entity_t value) : entity(value)   {}
    };

    struct value_t final {
        value_type_t            type;
        value_data_t            data;

        value_t()          : type(value_type_t::nil), data()                 {}
        value_t(b8 value)  : type(value_type_t::boolean), data(u64(value))   {}
        value_t(s8 value)  : type(value_type_t::integer), data(u64(value))   {}
        value_t(u8 value)  : type(value_type_t::integer), data(u64(value))   {}
        value_t(u64 value) : type(value_type_t::integer), data(value)        {}
        value_t(s16 value) : type(value_type_t::integer), data(u64(value))   {}
        value_t(u16 value) : type(value_type_t::integer), data(u64(value))   {}
        value_t(s32 value) : type(value_type_t::integer), data(u64(value))   {}
        value_t(u32 value) : type(value_type_t::integer), data(u64(value))   {}
        value_t(s64 value) : type(value_type_t::integer), data(u64(value))   {}
        value_t(f64 value) : type(value_type_t::floating_point), data(value) {}
        value_t(entity_t value) : type(value_type_t::entity), data(value)    {}
        explicit value_t(value_type_t type) : type(type), data()             {}
        value_t(const value_t& other) { operator=(other); }

        constexpr operator u64() const {
            return data.number.u;
        }

        value_t& operator=(const value_t& other);
    };
    static_assert(sizeof(value_t) <= 32,
                  "value_t is now larger than 32 bytes!");

    struct tuple_t final {
        s64                     rowid;
        entity_t                attr;
        value_t                 value;
    };
    static_assert(sizeof(tuple_t) <= 48,
                  "tuple_t is now larger than 48 bytes!");

    struct simple_stmt_cache_t final {
        sqlite3_stmt*           select      {};
        sqlite3_stmt*           upsert      {};
        sqlite3_stmt*           delete_     {};
    };

    struct entity_stmt_cache_t final {
        sqlite3_stmt*           select      {};
        sqlite3_stmt*           update      {};
        sqlite3_stmt*           insert      {};
    };

    struct tuple_stmt_cache_t final {
        sqlite3_stmt*           insert      {};
        sqlite3_stmt*           select_one  {};
        sqlite3_stmt*           select_all  {};
        sqlite3_stmt*           update_one  {};
        sqlite3_stmt*           update_all  {};
    };

    struct txn_t final {
        db_t*                   db;
        str::slice_t            label;
        u32                     id;
    };

    using txn_list_t            = stable_array_t<txn_t>;

    struct db_t final {
        alloc_t*                alloc       {};
        str_t                   buf;
        path_t                  path;
        sqlite3*                handle      {};
        txn_list_t              txns        {};
        tuple_stmt_cache_t      tuple       {};
        simple_stmt_cache_t     config      {};
        entity_stmt_cache_t     entity      {};
        simple_stmt_cache_t     symbol      {};
    };

    namespace eav {
        enum class status_t : u8 {
            ok                  = 0,
            error               = 101,
            not_found           = 102,
            sql_error           = 103,
            invalid_rowid       = 104,
            invalid_entity      = 105,
        };

        namespace entity {
            namespace status {
                constexpr u8 dead  = 0;
                constexpr u8 live  = 1;
            }
        }
    }

    // ------------------------------------------------------------------------
    //
    // plot
    //
    // ------------------------------------------------------------------------
    struct data_point_t;
    struct rolled_view_t;
    struct scrolled_view_t;

    using data_point_array_t    = array_t<data_point_t>;

    struct data_point_t final {
        f32                     x, y;
    };

    struct rolled_view_t final {
        data_point_array_t      values;
        f32                     span;
        f32                     time;
        f32                     min_y;
        f32                     max_y;
    };

    struct scrolled_view_t final {
        data_point_array_t      values;
        f32                     time;
        f32                     min_y;
        f32                     max_y;
        s32                     offset;
        s32                     max_size;
    };

    // ------------------------------------------------------------------------
    //
    // memory/meta
    //
    // ------------------------------------------------------------------------
    struct alloc_info_t;

    using alloc_info_array_t    = array_t<alloc_info_t*>;

    enum class plot_mode_t : u8 {
        none,
        rolled,
        scrolled,
    };

    struct alloc_info_t final {
        alloc_t*                alloc;
        alloc_info_t*           parent;
        alloc_t*                tracked;
        alloc_info_array_t      children;
        union {
            rolled_view_t       rolled;
            scrolled_view_t     scrolled;
        }                       plot;
        plot_mode_t             mode;
    };

    // ------------------------------------------------------------------------
    //
    // src_loc
    //
    // ------------------------------------------------------------------------
    struct source_loc_t final {
        u32                     line    {};
        u32                     column  {};
    };

    struct source_pos_t final {
        u32                     end     {};
        u32                     start   {};
    };

    struct source_info_t final {
        source_pos_t            pos;
        source_loc_t            start;
        source_loc_t            end;
    };

    // ------------------------------------------------------------------------
    //
    // token
    //
    // ------------------------------------------------------------------------
    struct token_t;
    struct token_id_t;
    struct token_type_t;
    struct token_cache_t;
    struct prefix_rule_t;
    struct operator_precedent_t;

    using token_list_t          = array_t<token_t>;
    using token_id_list_t       = array_t<token_id_t>;

#define TOKEN_CLS(t)            (u32((t)) >> 16U & 0xffffU)
#define TOKEN_TYPE(cls, type)   (u32((cls)) << 16U | u32((type)))

    struct token_id_t final {
        constexpr token_id_t()       : id(0)        {}
        constexpr token_id_t(u32 id) : id(id)       {}
        constexpr operator u32() const              { return id;        }
        [[nodiscard]] constexpr b8 empty() const    { return id == 0;   }
        static constexpr token_id_t null()          { return 0;         }
    private:
        u32                     id;
    };

    struct token_type_t final {
        constexpr token_type_t()         : cls(0), type(0) {
        }
        constexpr token_type_t(u32 type) : cls(type >> 16U & 0xffffU),
                                           type(type & 0xffffU) {
        }
        constexpr operator u32() const           { return TOKEN_TYPE(cls, type); }
        [[nodiscard]] constexpr b8 empty() const { return type == 0 && cls == 0; }
        static constexpr token_type_t none()     { return 0;                     }
    private:
        u32                     cls:    16;
        u32                     type:   16;
    };

    struct token_t final {
        token_id_t              id;
        source_info_t           loc;
        token_type_t            type;
    };

    struct token_cache_t final {
        alloc_t*                alloc;
        token_list_t            tokens;
        u32                     idx;
    };

    struct prefix_rule_t final {
        token_type_t            plus;
        token_type_t            minus;
    };

    struct operator_precedence_t final {
        u8                      left;
        u8                      right;
    };

    // ------------------------------------------------------------------------
    //
    // pratt
    //
    // ------------------------------------------------------------------------
    struct rule_t;
    struct pratt_ctx_t;

    using rule_map_t            = hashtab_t<token_type_t, rule_t>;
    using std_t                 = ast::node_id_t (*)(pratt_ctx_t*);
    using nud_t                 = ast::node_id_t (*)(pratt_ctx_t*);
    using led_t                 = ast::node_id_t (*)(pratt_ctx_t*, ast::node_id_t);

    struct rule_t final {
        std_t                   std;
        nud_t                   nud;
        led_t                   led;
        u32                     lbp;
        u8                      postfix:        1;
        u8                      suppress_led:   1;
        u8                      associativity:  2;
        u8                      pad:            3;
    };

    struct grammar_t final {
        alloc_t*                alloc;
        rule_map_t              rules;
        u32*                    operator_types;
        u32                     default_rbp;
    };

    struct pratt_ctx_t final {
        bass_t*                 ast;
        rule_t*                 rule;
        const token_t*          token;
        grammar_t*              grammar;
        token_cache_t*          token_cache;
    };

    namespace pratt {
        namespace associativity {
            constexpr u8 none   = 0b0000;
            constexpr u8 left   = 0b0001;
            constexpr u8 right  = 0b0010;
        }
    }

    // ------------------------------------------------------------------------
    //
    // profiler
    //
    // ------------------------------------------------------------------------
    namespace profiler {
        enum class status_t : u8 {
            ok                              = 0,
            no_cpu_rtdscp_support           = 164,
            no_cpu_invariant_tsc_support    = 165
        };
    }

    // ------------------------------------------------------------------------
    //
    // proxy_array
    //
    // ------------------------------------------------------------------------
    template<typename T, typename Backing_Type = T>
    struct proxy_array_t final {
        using Is_Static         = std::integral_constant<b8, true>;
        using Value_Type        = T;
        using Size_Per_16       = std::integral_constant<u32, 16 / sizeof(T)>;
        using Backing_Array     = const array_t<Backing_Type>*;

        Backing_Array           backing;
        u32                     start;
        u32                     size;

        const Value_Type& operator[](u32 index) const {
            return (const Value_Type&) backing->data[start + index];
        }

        struct iterator_state_t {
            u32                 pos;

            inline u0 end(const proxy_array_t* ref)               { pos = ref->size;      }
            inline u0 next(const proxy_array_t* ref)              { UNUSED(ref); ++pos;   }
            inline Value_Type get(proxy_array_t* ref)             { return (*ref)[pos];   }
            inline u0 begin(const proxy_array_t* ref)             { UNUSED(ref); pos = 0; }
            inline b8 cmp(const iterator_state_t& s) const        { return pos != s.pos;  }
            inline const Value_Type get(const proxy_array_t* ref) { return (*ref)[pos];   }
        };
        DECL_ITERS(proxy_array_t, Value_Type, iterator_state_t);
    };
    static_assert(sizeof(proxy_array_t<s32>) <= 16,
                  "proxy_array_t<T> is now larger than 16 bytes!");

    // ------------------------------------------------------------------------
    //
    // queue
    //
    // ------------------------------------------------------------------------
    template <typename T>
    struct queue_t final {
        using Value_Type        = T;

        array_t<Value_Type>     items;
        u32                     size;
        u32                     offset;

        T& operator[](u32 index) {
            return items[(index + offset) % items.size];
        }
        const T& operator[](u32 index) const {
            return items[(index + offset) % items.size];
        }
    };
    static_assert(sizeof(queue_t<s32>) <= 32,
                  "queue_t<T> is now larger than 32 bytes!");

    // ------------------------------------------------------------------------
    //
    // rbt
    //
    // ------------------------------------------------------------------------
    enum class rbt_color_t : u8 {
        none  = bintree::color::none,
        red   = bintree::color::red,
        black = bintree::color::black
    };

    template <typename T>
    struct rbt_t final {
        struct node_t;

        using Has_Color         = std::integral_constant<b8, true>;
        using Node_Type         = node_t*;
        using Value_Type        = T;
        static constexpr u32    Value_Type_Size    = sizeof(T);
        static constexpr u32    Value_Type_Align   = alignof(T);

        struct node_t final {
            node_t*             lhs;
            node_t*             rhs;
            node_t*             parent;
            Value_Type*         value;
            rbt_color_t         color;
        };
        static_assert(sizeof(node_t) <= 40,
                      "node_t is now larger than 40 bytes!");

        static constexpr u32    Node_Type_Size     = sizeof(node_t);
        static constexpr u32    Node_Type_Align    = alignof(node_t);

        alloc_t*                alloc;
        alloc_t*                node_slab;
        alloc_t*                value_slab;
        Node_Type               root;
        u32                     size;
    };
    static_assert(sizeof(rbt_t<u32>) <= 40,
                  "rbt_t<u32> is now larger than 40 bytes!");

    // ------------------------------------------------------------------------
    //
    // rpn
    //
    // ------------------------------------------------------------------------
    struct postfix_t;
    struct postfix_expr_t;

    using postfix_expr_list_t   = array_t<postfix_expr_t>;

    struct postfix_expr_t final {
        token_id_list_t         tokens;
        u32                     arg_count:  8;
        u32                     stmt_count: 24;
    };

    struct postfix_t final {
        alloc_t*                alloc;
        token_cache_t*          tokens;
        operator_precedence_t*  operator_precedences;
        postfix_expr_list_t     exprs;
    };

    namespace token::cls {
        constexpr u16 none                 = 0;
        constexpr u16 operator_            = 1;
        constexpr u16 param_end            = 2;
        constexpr u16 scope_end            = 3;
        constexpr u16 param_begin          = 4;
        constexpr u16 scope_begin          = 5;
        constexpr u16 call_operator        = 7;
        constexpr u16 call_terminator      = 8;
        constexpr u16 stmt_terminator      = 9;
        constexpr u16 operator_consumer    = 10;

        str::slice_t name(token_type_t type);
    }

    namespace rpn {
        enum class status_t : u8 {
            ok                                  = 0,
            error                               = 200,
            invalid_operator_precedence_array   = 201
        };
    }

    // ------------------------------------------------------------------------
    //
    // set
    //
    // ------------------------------------------------------------------------
    template <typename T>
    concept Hash_Set = hash::Hashable<T> && requires(const T& t) {
        typename                T::Value_Type;
        typename                T::Is_Pointer;
        typename                T::Value_Type_Base;

        {t.alloc}               -> std::same_as<alloc_t*>;
        {t.flags}               -> std::same_as<u64*>;
        {t.hashes}              -> std::same_as<u64*>;
        {t.values}              -> std::same_as<typename T::Value_Type*>;
        {t.size}                -> std::same_as<u32>;
        {t.capacity}            -> std::same_as<u32>;
        {t.load_factor}         -> std::same_as<f32>;
    };

    struct set_buf_size_t final {
        u32                     total_size;
        u32                     size_of_flags;
        u32                     num_flag_words;
        u32                     size_of_hashes;
        u32                     size_of_values;
    };

    template <typename T>
    struct set_t final {
        using Value_Type        = T;
        using Is_Pointer        = std::integral_constant<b8, std::is_pointer_v<T>>;
        using Value_Type_Base   = std::remove_pointer_t<T>;

        alloc_t*                alloc;
        u64*                    flags;
        u64*                    hashes;
        Value_Type*             values;
        u32                     size;
        u32                     cap_idx;
        u32                     capacity;
        f32                     load_factor;

        struct iterator_state_t {
            u32                 pos;

            inline u0 end(const set_t* ref) {
                pos = ref->capacity;
            }

            inline u0 next(const set_t* ref) {
                while (pos < ref->capacity) {
                    if (ref->hashes[++pos])
                        break;
                }
            }

            inline Value_Type get(set_t* ref) {
                return ref->values[pos];
            }

            inline u0 begin(const set_t* ref) {
                pos = 0;
                while (pos < ref->capacity) {
                    if (ref->hashes[pos])
                        break;
                    ++pos;
                }
            }

            inline const Value_Type get(const set_t* ref) {
                return ref->values[pos];
            }

            inline b8 cmp(const iterator_state_t& s) const {
                return pos != s.pos;
            }
        };
        DECL_ITERS(set_t, Value_Type, iterator_state_t);
    };
    static_assert(sizeof(set_t<s32>) <= 56,
                  "set_t<T> is now larger than 56 bytes!");

    // ------------------------------------------------------------------------
    //
    // stack
    //
    // ------------------------------------------------------------------------
    template <typename T>
    concept Fixed_Stack = requires(const T& t) {
        typename                T::Value_Type;
        typename                T::Base_Value_Type;

        {t.data}                -> std::same_as<typename T::Value_Type*>;
        {t.size}                -> std::same_as<u32>;
        {t.capacity}            -> std::same_as<u32>;
    };

    template <typename T>
    concept Dynamic_Stack = requires(const T& t) {
        typename                T::Value_Type;
        typename                T::Base_Value_Type;

        {t.alloc}               -> std::same_as<alloc_t*>;
        {t.data}                -> std::same_as<typename T::Value_Type*>;
        {t.size}                -> std::same_as<u32>;
        {t.capacity}            -> std::same_as<u32>;
    };

    template <typename T>
    concept Stack = Fixed_Stack<T> || Dynamic_Stack<T>;

    template <typename T, u32 Size = 8>
    struct fixed_stack_t final {
        using Value_Type        = T;
        using Base_Value_Type   = std::remove_pointer_t<T>;

        T                       data[Size];
        u32                     size;
        u32                     capacity = Size;

        T& operator[](u32 index)                { return data[index]; }
        const T& operator[](u32 index) const    { return data[index]; }
    };

    template <typename T>
    struct stack_t final {
        using Value_Type        = T;
        using Base_Value_Type   = std::remove_pointer_t<T>;

        alloc_t*                alloc;
        T*                      data;
        u32                     size;
        u32                     capacity;

        T& operator[](u32 index)                { return data[index]; }
        const T& operator[](u32 index) const    { return data[index]; }
    };
    static_assert(sizeof(stack_t<s32>) <= 24,
                  "stack_t<T> is now larger than 24 bytes!");

    // ------------------------------------------------------------------------
    //
    // buf
    //
    // ------------------------------------------------------------------------
    template <typename T>
    concept Buffer_Concept = String_Concept<T> || requires(const T& t) {
        {t.alloc}               -> std::same_as<alloc_t*>;
        {t.data}                -> std::same_as<u8*>;
        {t.length}              -> std::same_as<u32>;
        {t.capacity}            -> std::same_as<u32>;
    };

    struct buf_line_t final {
        u32                     pos;
        u32                     len;
    };

    struct buf_token_t final {
        u32                     pos;
        u32                     len;
    };

    enum class buf_mode_t : u8 {
        none,
        alloc,
        mapped
    };

    struct buf_t final {
        alloc_t*                alloc;
        u8*                     data;
        array_t<buf_line_t>     lines;
        array_t<buf_token_t>    tokens;
        path_t                  path;
        u32                     length;
        u32                     capacity;
        s32                     file;
        buf_mode_t              mode;

        u8& operator[](u32 idx)      { return data[idx]; }
        u8 operator[](u32 idx) const { return data[idx]; }
    };
    static_assert(sizeof(buf_t) <= 128,
                  "buf_t is now larger than 128 bytes!");

    struct buf_crsr_t final {
        buf_t*                  buf;
        stack_t<u32>            stack;
        u32                     pos;
        u32                     line;
        u32                     column;

        u8* operator*()                 { return buf->data + pos; }
        u8& operator[](u32 idx)         { return (*buf)[idx];     }
        u8 operator[](u32 idx) const    { return (*buf)[idx];     }
    };
    static_assert(sizeof(buf_crsr_t) <= 56,
                  "buf_crsr_t is now larger than 56 bytes!");

    namespace buf {
        enum class status_t : u8 {
            ok                              = 0,
            unable_to_open_file             = 100,
            mmap_error,
            munmap_error,
            end_of_buffer,
            cannot_unmap_buf,
            buf_already_mapped,
            unable_to_truncate_file,
            cannot_reset_mapped_buf,
            cannot_save_zero_length_buf,
            cannot_save_over_mapped_path,
        };
    }

    // ------------------------------------------------------------------------
    //
    // str_array
    //
    // ------------------------------------------------------------------------
    struct str_idx_t final {
        u8*                     buf;
        u32                     len;
    };

    struct str_array_t final {
        alloc_t*                alloc;
        alloc_t*                page_alloc;
        alloc_t*                bump_alloc;
        str_idx_t*              index;
        u32                     size;
        u32                     capacity;

        inline str::slice_t operator[](u32 i) const;
    };
    static_assert(sizeof(str_array_t) <= 40,
                  "str_array_t is now greater than 40 bytes!");

    // ------------------------------------------------------------------------
    //
    // string
    //
    // ------------------------------------------------------------------------
    namespace string {
        enum class status_t : u8 {
            ok,
            localized_not_found,
            localized_duplicate_key,
            localized_intern_failure,
        };
    }

    // ------------------------------------------------------------------------
    //
    // symtab
    //
    // ------------------------------------------------------------------------
    template <typename T>
    concept Symbol_Table = requires(const T& t) {
        typename                T::Value_Type;
        typename                T::Value_Array;
        typename                T::Pair_Array;
        typename                T::Node_Type;
        typename                T::Node_Array;

        {t.alloc}               -> std::same_as<alloc_t*>;
        {t.nodes}               -> std::same_as<typename T::Node_Array>;
        {t.values}              -> std::same_as<typename T::Value_Array>;
        {t.size}                -> std::same_as<u32>;
    };

    enum class symtab_node_type_t : u8 {
        empty,
        used,
        leaf,
    };

    template <typename V>
    struct symtab_t final {
        using Value_Type        = V;
        using Value_Array       = stable_array_t<Value_Type>;
        using Pair_Array        = assoc_array_t<std::remove_pointer_t<Value_Type>*>;

        struct symtab_node_t final {
            symtab_node_t*      next;
            symtab_node_t*      child;
            Value_Type*         value;
            u32                 id;
            s8                  sym;
            symtab_node_type_t  type;
        };

        using Node_Type         = symtab_node_t;
        using Node_Array        = stable_array_t<Node_Type>;

        alloc_t*                alloc;
        Node_Array              nodes;
        Value_Array             values;
        u32                     size;
    };
    static_assert(sizeof(symtab_t<s32>) <= 80,
                  "symtab_t<V> is now larger than 80 bytes!");

    // ------------------------------------------------------------------------
    //
    // ffi
    //
    // ------------------------------------------------------------------------
    struct ffi_t;
    struct lib_t;
    struct param_t;
    struct proto_t;
    struct overload_t;
    struct param_type_t;
    struct param_value_t;
    union  param_alias_t;

    using param_array_t         = array_t<param_t*>;
    using symbol_array_t        = assoc_array_t<u0*>;
    using overload_array_t      = array_t<overload_t*>;
    using param_value_array_t   = array_t<param_value_t>;

    enum class call_mode_t : u8 {
        system                  = 1,
        variadic,
    };

    enum class param_cls_t : u8 {
        ptr                     = 'P',
        int_                    = 'I',
        void_                   = 'V',
        float_                  = 'F',
        struct_                 = 'S',
    };

    enum class param_size_t : u8 {
        none                    = '-',
        byte                    = 'b',
        word                    = 'w',
        dword                   = 'd',
        qword                   = 'q',
    };

    struct lib_t final {
        alloc_t*                alloc;
        DLLib*                  handle;
        symtab_t<u0*>           symbols;
        path_t                  path;
    } __attribute__((aligned(128)));

    struct param_type_t final {
        param_cls_t             cls;
        param_size_t            size;
        u8                      user;
    } __attribute__((aligned(4)));

    union param_alias_t final {
        u0*                     p;
        u8                      b;
        u16                     w;
        u32                     dw;
        u64                     qw;
        f32                     fdw;
        f64                     fqw;
    };

    struct param_value_t final {
        param_alias_t           alias;
        param_type_t            type;
    } __attribute__((aligned(16)));

    struct param_t final {
        param_value_t           value;
        param_array_t           members;
        str::slice_t            name;
        u8                      has_dft:    1;
        u8                      is_rest:    1;
        u8                      pad:        6;
    } __attribute__((aligned(64)));

    struct overload_t final {
        proto_t*                proto;
        u0*                     func;
        str_t                   signature;
        str::slice_t            name;
        param_array_t           params;
        param_type_t            ret_type;
        u32                     req_count;
        u8                      has_dft:    1;
        u8                      has_rest:   1;
        u8                      pad:        6;
        call_mode_t             mode;
    } __attribute__((aligned(128)));

    struct proto_t final {
        lib_t*                  lib;
        str::slice_t            name;
        overload_array_t        overloads;
        u32                     min_req;
        u32                     max_req;
    } __attribute__((aligned(128)));

    struct ffi_t final {
        alloc_t*                alloc;
        DCCallVM*               vm;
        u32                     heap_size;
    } __attribute__((aligned(32)));

    namespace ffi {
        enum class status_t : u8 {
            ok                                  = 0,
            address_null                        = 108,
            prototype_null                      = 109,
            lib_not_loaded                      = 110,
            symbol_not_found                    = 111,
            invalid_int_size                    = 112,
            invalid_float_size                  = 113,
            parameter_overflow                  = 117,
            duplicate_overload                  = 116,
            load_library_failure                = 114,
            struct_by_value_not_implemented     = 115,
            only_one_rest_param_allowed         = 118,  // FIXME
        };
    }

    // ------------------------------------------------------------------------
    //
    // term
    //
    // ------------------------------------------------------------------------
    struct rgb_t;
    struct term_t;
    struct term_command_t;
    struct cursor_pos_t;
    struct color_value_t;

    using style_t               = u32;
    using term_command_array_t  = array_t<term_command_t>;

    enum class clear_mode_t : u8 {
        cursor_to_bottom,
        cursor_to_top,
        entire
    };

    enum class color_mode_t : u8 {
        indexed,
        palette,
        true_color
    };

    enum class term_command_type_t : u8 {
        cursor_up                   = 'A',
        cursor_to                   = 'H',
        scroll_up                   = 'S',
        erase_line                  = 'K',
        cursor_down                 = 'B',
        scroll_down                 = 'T',
        cursor_back                 = 'D',
        erase_display               = 'J',
        cursor_forward              = 'C',
        cursor_next_line            = 'E',
        cursor_prev_line            = 'F',
        cursor_horiz_abs            = 'G',
        cursor_horiz_vert_pos       = 'f',
        select_graphic_rendition    = 'm',
    };

    namespace term {
        enum class color_t : u8 {
            black           = 30,
            red,
            green,
            yellow,
            blue,
            magenta,
            cyan,
            white,
            fg_default      = 39,

            bg_black,
            bg_red,
            bg_green,
            bg_yellow,
            bg_blue,
            bg_magenta,
            bg_cyan,
            bg_white,
            bg_default      = 49,

            bright_black    = 90,
            bright_red,
            bright_green,
            bright_yellow,
            bright_blue,
            bright_magenta,
            bright_cyan,
            bright_white
        };

        namespace style {
            constexpr u32 none              = 0b00000000000000000000000000000000;
            constexpr u32 bold              = 0b00000000000000000000000000000001;
            constexpr u32 dim               = 0b00000000000000000000000000000010;
            constexpr u32 italic            = 0b00000000000000000000000000000100;
            constexpr u32 underline         = 0b00000000000000000000000000001000;
            constexpr u32 slow_blink        = 0b00000000000000000000000000010000;
            constexpr u32 fast_blink        = 0b00000000000000000000000000100000;
            constexpr u32 reverse           = 0b00000000000000000000000001000000;
            constexpr u32 hidden            = 0b00000000000000000000000010000000;
            constexpr u32 strike            = 0b00000000000000000000000100000000;
            constexpr u32 double_underline  = 0b00000000000100000000000000000000;
        }

        enum class status_t : u8 {
            ok
        };
    }

    // ------------------------------------------------------------------------
    //
    // test_suite
    //
    // ------------------------------------------------------------------------
    struct test_suite_t;

    using suite_runner_t        = s32 (*)(test_suite_t&);

    // ------------------------------------------------------------------------
    //
    // thread
    //
    // ------------------------------------------------------------------------
    struct thread_t;

    enum class thread_state_t : u8 {
        exited,
        created,
        running,
        canceled,
    };

    struct proc_base_t {
        virtual u0* invoke() = 0;

        virtual thread_t* self() = 0;

        virtual ~proc_base_t() = default;
    };

    template <typename Proc, typename... Args>
    struct thread_proc_t final : public proc_base_t {
        using Result_Type       = std::invoke_result_t<Proc, Args...>;
        static constexpr b8     Is_Void = std::is_void_v<Result_Type>;

        using Proc_Type         = Proc;
        using Args_Type         = std::tuple<Args...>;
        using Return_Type       = typename std::conditional<Is_Void,
            int,
            Result_Type>::type;

        thread_t*               thread;
        Proc_Type               proc;
        Args_Type               args;
        Return_Type             ret_val;

        thread_proc_t(thread_t* thread,
                      Proc_Type proc,
                      Args_Type args) : thread(thread),
                                        proc(proc),
                                        args(std::move(args)) {
        }

        u0* invoke() override {
            if constexpr (Is_Void) {
                std::apply(proc, args);
                ret_val = {};
                return nullptr;
            } else {
                ret_val = std::apply(proc, args);
                return &ret_val;
            }
        }

        thread_t* self() override { return thread; }
    };

    struct thread_t final {
        pthread_t               handle;
        proc_base_t*            proc;
        str::slice_t            name;
        thread_state_t          state;
        u8                      joined:     1;
        u8                      detached:   1;
        u8                      joinable:   1;
        u8                      canceled:   1;
        u8                      cancelable: 1;
        u8                      pad:        3;

        auto operator==(const thread_t& other) const;
    };

    namespace thread {
        enum class status_t : u8 {
            ok                      = 0,
            error                   = 167,
            deadlock                = 168,
            not_joinable            = 169,
            invalid_state           = 170,
            name_too_long           = 171,
            invalid_thread          = 172,
            already_joined          = 173,
            not_cancelable          = 174,
            already_canceled        = 175,
            already_detached        = 176,
            create_thread_failure   = 177,
            insufficient_privilege  = 178,
        };
    }

    // ------------------------------------------------------------------------
    //
    // timer
    //
    // ------------------------------------------------------------------------
    struct timer_t;

    using timer_callback_t      = b8 (*)(timer_t*, u0*);

    struct timer_t final {
        u0*                     user;
        timer_callback_t        callback;
        s64                     expiry;
        s64                     duration;
        b8                      active;
    };
    static_assert(sizeof(timer_t) <= 40,
                  "timer_t is now greater than 40 bytes!");

    namespace timer {
        enum class status_t : u8 {
            ok                  = 0,
            error               = 179
        };
    }

    // ------------------------------------------------------------------------
    //
    // utf
    //
    // ------------------------------------------------------------------------
    template <typename T>
    concept Utf_String_Concept = (std::same_as<typename T::Value_Type, u8>
                                  || std::same_as<typename T::Value_Type, u32>
                                  || std::same_as<typename T::Value_Type, u16>)
                                 && requires(const T& t) {
        typename                T::Value_Type;

        {t.alloc}               -> std::same_as<alloc_t*>;
        {t.data}                -> std::same_as<typename T::Value_Type*>;
        {t.size}                -> std::same_as<u32>;
        {t.length}              -> std::same_as<u32>;
        {t.capacity}            -> std::same_as<u32>;
    };

    using utf32_codepoint_t     = u32;

    struct utf8_codepoint_t final {
        u8                      data[4];
        u8                      len;
    };

    struct utf16_codepoint_t final {
        u16                     low;
        u16                     high;
    };

    template <typename T>
    struct utf_str_t final {
        using Value_Type        = T;
        static constexpr b8 Is_Utf8    = sizeof(Value_Type) == 1;
        static constexpr b8 Is_Utf16   = sizeof(Value_Type) == 2;
        static constexpr b8 Is_Utf32   = sizeof(Value_Type) == 4;

        alloc_t*                alloc;
        T*                      data;
        u32                     size;
        u32                     length;
        u32                     capacity;
        T& operator[](u32 index)                { return data[index]; }
        const T& operator[](u32 index) const    { return data[index]; }
    };

    using utf8_str_t  = utf_str_t<u8>;
    using utf16_str_t = utf_str_t<u16>;
    using utf32_str_t = utf_str_t<u32>;

    // ------------------------------------------------------------------------
    //
    // uuid
    //
    // ------------------------------------------------------------------------
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
    }

    // ------------------------------------------------------------------------
    //
    // variant
    //
    // ------------------------------------------------------------------------
    template <typename T>
    struct variant_t final {
        using Value_Type        = T;

    private:
        Value_Type              _value;

    public:
        variant_t() : _value() {
        }
        variant_t(const T& value) : _value(value) {
        }
        variant_t(const variant_t& other) : _value(other._value) {
        }
        ~variant_t() {
            if constexpr (std::is_destructible_v<Value_Type>) {
                (&_value)->~Value_Type();
            }
        }
        Value_Type& operator*() {
            return _value;
        }
        const Value_Type& operator*() const {
            return _value;
        }
    };

    struct variant_array_t final {
        hashtab_t<u32, alloc_t*>    values;
    };

    // ------------------------------------------------------------------------
    //
    // env
    //
    // ------------------------------------------------------------------------
    enum class env_value_type_t : u8 {
        string,
        array,
    };

    struct env_value_t final {
        union {
            str::slice_t        str;
            slice_array_t       list;
        }                       kind;
        env_value_type_t        type;
    };

    using envvar_table_t        = hashtab_t<str::slice_t, env_value_t>;

    struct env_t final {
        alloc_t*                alloc;
        env_t*                  parent;
        str::slice_t            name;
        envvar_table_t          vartab;
    };

    namespace env {
        enum class status_t : u32 {
            ok,
            error,
            key_not_found,
            load_config_error,
            expected_non_null_pairs
        };
    }

    // ------------------------------------------------------------------------
    //
    // error
    //
    // ------------------------------------------------------------------------
    using error_arg_array_t     = array_t<std::any>;

    template <typename T>
    concept Error_Id = std::is_enum_v<T>
                       || std::same_as<T, u32>
                       || std::same_as<T, s32>;

    enum class error_report_level_t : u8 {
        warning,
        error,
    };

    enum class error_report_type_t : u8 {
        default_,
        source,
    };

    struct error_def_t final {
        str::slice_t            code;
        str::slice_t            locale;
        u32                     id;
        u32                     lc_str_id;
    };

    struct error_report_t final {
        buf_t*                  buf;
        error_arg_array_t       args;
        time_t                  ts;
        source_info_t           src_info;
        u32                     id;
        error_report_type_t     type;
        error_report_level_t    level;
    };

    using error_report_list_t   = array_t<error_report_t>;
    using localized_error_map_t = hashtab_t<locale_key_t, error_def_t*>;

    namespace error {
        enum class status_t : u8 {
            ok                  = 0,
            localized_dup_key,
            localized_not_found,
        };
    }

    // ------------------------------------------------------------------------
    //
    // hashable implementation/prototypes
    //
    // ------------------------------------------------------------------------
    namespace hash {
        namespace murmur {
            u64 hash64(const u0* src, usize len);
        }

        static const u64 s_fixed_random = std::chrono::steady_clock::now()
            .time_since_epoch()
            .count();

        // http://xorshift.di.unimi.it/splitmix64.c
        inline u64 splitmix64(u64 x) {
            x += 0x9e3779b97f4a7c15;
            x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
            x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
            return x ^ (x >> 31);
        }

        template <String_Concept T>
        inline u64 hash64(const T& key) {
            return murmur::hash64(key.data, key.length);
        }

        inline u32 hash32(const u8& key) {
            return hash32((u32) key);
        }

        inline u64 hash64(const u8& key) {
            return hash64((u64) key);
        }

        inline u32 hash32(const s8& key) {
            return hash32((s32) key);
        }

        inline u64 hash64(const s8& key) {
            return hash64((s64) key);
        }

        inline u32 hash32(const u16& key) {
            return hash32((u32) key);
        }

        inline u64 hash64(u0* const& key) {
            static const usize shift = std::log2(1 + sizeof(u0*));
            return splitmix64(s_fixed_random + (usize(key) >> shift));
        }

        inline u64 hash64(const u16& key) {
            return hash64((u64) key);
        }

        inline u32 hash32(const s16& key) {
            return hash32((s32) key);
        }

        inline u64 hash64(const s16& key) {
            return hash64((s64) key);
        }

        inline u32 hash32(const u32& key) {
            return splitmix64(s_fixed_random + key);
        }

        inline u64 hash64(const u32& key) {
            return splitmix64(s_fixed_random + key);
        }

        inline u32 hash32(const s32& key) {
            return splitmix64(s_fixed_random + key);
        }

        inline u64 hash64(const s32& key) {
            return splitmix64(s_fixed_random + key);
        }

        inline u64 hash64(const s64& key) {
            return splitmix64(s_fixed_random + key);
        }

        inline u64 hash64(const u64& key) {
            return splitmix64(s_fixed_random + key);
        }

        inline u64 hash64(const integer_t& key) {
            u64 hash{};
            for (u32 i = 0; i < key.size; ++i)
                hash = 63 * hash + key.data[i];
            return hash;
        }

        inline u64 hash64(const decimal_t& key) {
            u64 hash{};
            for (u32 i = 0; i < key.size; ++i)
                hash = 63 * hash + key.data[i];
            return hash;
        }

        inline u64 hash64(option_t* const& key) {
            return murmur::hash64((const u0*) key, sizeof(u0*));
        }

        template <typename T>
        inline u64 hash64(const slice_t<T>& key) {
            return murmur::hash64(key.data, key.length);
        }

        inline u64 hash64(const token_type_t& key) {
            u32 value = key;
            return murmur::hash64(&value, sizeof(u32));
        }

        inline u64 hash64(const locale_key_t& key) {
            return murmur::hash64(&key, sizeof(locale_key_t));
        }
    }
}
