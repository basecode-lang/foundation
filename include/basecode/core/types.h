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

#if !defined(__cpp_lib_bit_cast)
namespace std {
    template <typename To, typename From> requires (sizeof(From) == sizeof(To)
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

struct sqlite3;
struct sqlite3_stmt;

namespace fmt {
    inline namespace v7 {
        struct format_args;
    }
}

namespace spdlog {
    class logger;
}

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
    struct str_t;

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

    namespace str {
        using slice_t = slice_t<u8>;
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
        using Is_Static         [[maybe_unused]] = std::integral_constant<b8, false>;
        using Value_Type        = T;
        using Size_Per_16       [[maybe_unused]] = std::integral_constant<u32, 16 / sizeof(T)>;

        alloc_t*                alloc;
        Value_Type*             data;
        u32                     size;
        u32                     capacity;

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
    static_assert(sizeof(array_t<s32>) <= 24, "array_t<T> is now larger than 24 bytes!");

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

    template <typename T>
    struct stable_array_t;

    // ------------------------------------------------------------------------
    //
    // assoc_array
    //
    // ------------------------------------------------------------------------
    template <typename V>
    struct assoc_pair_t;

    template <typename V>
    struct assoc_array_t;

    struct assoc_key_idx_t;

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

        struct node_id_t;
        struct node_type_t;
        struct num_lit_flags_t;
    }

    // ------------------------------------------------------------------------
    //
    // avl
    //
    // ------------------------------------------------------------------------
    template <typename T>
    struct avl_t;

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

    template <typename T>
    struct bag_t;

    struct bag_buf_size_t;

    // ------------------------------------------------------------------------
    //
    // bass
    // (basecode awesome serialization system)
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
    struct bitset_t;

    // ------------------------------------------------------------------------
    //
    // bst
    //
    // ------------------------------------------------------------------------
    template <typename t>
    struct bst_t;

    // ------------------------------------------------------------------------
    //
    // buf
    //
    // ------------------------------------------------------------------------
    template <typename T>
    concept Buffer_Concept = String_Concept<T> || requires(const T& t) {
        {t.alloc}       -> std::same_as<alloc_t*>;
        {t.data}        -> std::same_as<u8*>;
        {t.length}      -> std::same_as<u32>;
        {t.capacity}    -> std::same_as<u32>;
    };

    struct buf_t;
    struct buf_crsr_t;
    struct buf_line_t;
    struct buf_node_t;
    struct buf_token_t;

    // ------------------------------------------------------------------------
    //
    // buf_pool
    //
    // ------------------------------------------------------------------------
    struct lease_t;

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
    // context
    //
    // ------------------------------------------------------------------------
    struct context_t {
        struct {
            alloc_t*        main;
            alloc_t*        temp;
            alloc_t*        scratch;
        }                   alloc;
        logger_t*           logger;
        u0*                 user;
        const s8**          argv;
        s32                 argc;
    };

    // ------------------------------------------------------------------------
    //
    // decimal
    //
    // ------------------------------------------------------------------------
    struct decimal_t;

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
    // eav
    //
    // ------------------------------------------------------------------------
    constexpr s32 app_id        = 0x000dead1;

    struct db_t;
    struct txn_t;
    struct tuple_t;
    struct value_t;
    struct entity_t;
    union value_data_t;
    struct tuple_stmt_cache_t;
    struct simple_stmt_cache_t;
    struct entity_stmt_cache_t;

    enum class value_type_t : u8 {
        nil,
        list,
        entity,
        string,
        boolean,
        integer,
        floating_point,
    };

    using txn_list_t            = stable_array_t<txn_t>;
    using tuple_list_t          = array_t<tuple_t>;
    using entity_list_t         = array_t<entity_t>;

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
    // filesys
    //
    // ------------------------------------------------------------------------
    struct glob_result_t;

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
        };
    }

    // ------------------------------------------------------------------------
    //
    // gap_buf
    //
    // ------------------------------------------------------------------------
    struct gap_t;
    struct gap_buf_t;

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
    enum class arg_type_t : u8 {
        none,
        flag,
        file,
        string,
        integer,
        decimal,
    };

    struct arg_t;
    struct getopt_t;
    struct option_t;
    union arg_subclass_t;
    using arg_array_t           = array_t<arg_t>;
    using option_array_t        = array_t<option_t>;

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
    struct hashtab_t;

    // ------------------------------------------------------------------------
    //
    // integer
    //
    // ------------------------------------------------------------------------
    struct integer_t;

    // ------------------------------------------------------------------------
    //
    // intern
    //
    // ------------------------------------------------------------------------
    struct intern_t;
    struct interned_str_t;

    using intern_id             = u32;
    using interned_str_list_t   = array_t<interned_str_t>;

    namespace intern {
        enum class status_t : u8 {
            ok                  = 0,
            no_bucket           = 135,
            not_found           = 136,
        };

        struct result_t;
    }

    using interned_array_t      = array_t<intern::result_t>;

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
    struct job_task_base;

    template <typename Proc, typename... Args>
    struct job_task_t;

    using job_id                = u32;
    using job_list_t            = stable_array_t<job_t>;
    using job_id_list_t         = array_t<job_id>;

    enum class job_state_t : u8 {
        queued                  = 250,
        created,
        running,
        finished,
    };

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
    struct leb128_t;

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

    struct list_node_t;

    template <typename T>
    struct list_t;

    // ------------------------------------------------------------------------
    //
    // locale
    //
    // ------------------------------------------------------------------------
    struct locale_key_t final {
        u32                     id;
        s8                      locale[8];

        b8 operator==(const locale_key_t& other) const {
            return id == other.id
                   && strncmp(locale, other.locale, sizeof(locale)) == 0;
        }
    };

    using localized_strtab_t    = hashtab_t<locale_key_t, str::slice_t>;

    namespace locale{
        enum class status_t : u8 {
            ok,
        };
    }

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
    union alloc_subclass_t;

    using mspace                = u0*;

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

    using mem_fini_callback_t       = u32 (*)(alloc_t*);
    using mem_size_callback_t       = u32 (*)(alloc_t*, u0* mem);
    using mem_free_callback_t       = u32 (*)(alloc_t*, u0* mem);
    using mem_init_callback_t       = u0  (*)(alloc_t*, alloc_config_t*);
    using mem_alloc_callback_t      = mem_result_t (*)(alloc_t*,
                                                       u32 size,
                                                       u32 align);
    using mem_realloc_callback_t    = mem_result_t (*)(alloc_t*,
                                                       u0* mem,
                                                       u32 size,
                                                       u32 align);

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
    struct socket_t;
    struct ip_address_t;

    using socket_read_callback_t    = b8 (*)(socket_t&, u0*);
    using socket_close_callback_t   = u0 (*)(socket_t&);

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
    // plot
    //
    // ------------------------------------------------------------------------
    struct data_point_t;
    struct rolled_view_t;
    struct scrolled_view_t;

    using data_point_array_t    = array_t<data_point_t>;

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

    // ------------------------------------------------------------------------
    //
    // pratt
    //
    // ------------------------------------------------------------------------
    struct rule_t;
    struct grammar_t;
    struct pratt_ctx_t;

    using rule_map_t            = hashtab_t<token_type_t, rule_t>;
    using std_t                 = ast::node_id_t (*)(pratt_ctx_t*);
    using nud_t                 = ast::node_id_t (*)(pratt_ctx_t*);
    using led_t                 = ast::node_id_t (*)(pratt_ctx_t*,
                                                     ast::node_id_t);

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
    template <typename T, typename Backing_Type = T>
    struct proxy_array_t;

    // ------------------------------------------------------------------------
    //
    // queue
    //
    // ------------------------------------------------------------------------
    template <typename T>
    struct queue_t;

    // ------------------------------------------------------------------------
    //
    // rbt
    //
    // ------------------------------------------------------------------------
    template <typename T>
    struct rbt_t;

    enum class rbt_color_t : u8 {
        none  = bintree::color::none,
        red   = bintree::color::red,
        black = bintree::color::black
    };

    // ------------------------------------------------------------------------
    //
    // rpn
    //
    // ------------------------------------------------------------------------
    struct postfix_t;
    struct postfix_expr_t;

    using postfix_expr_list_t   = array_t<postfix_expr_t>;

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

    struct set_buf_size_t;

    template <typename t>
    struct set_t;

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

    template <typename T>
    struct stack_t;

    // ------------------------------------------------------------------------
    //
    // stopwatch
    //
    // ------------------------------------------------------------------------
    struct stopwatch_t;

    using timed_block_callable_t    = std::function<s32()>;

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
            localized_duplicate_key
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
    struct symtab_t;

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
    struct proc_base_t;

    template <typename Proc, typename... Args>
    struct thread_proc_t;

    enum class thread_state_t : u8 {
        exited,
        created,
        running,
        canceled,
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
    struct uuid_t;

    namespace uuid {
        enum class status_t : u32 {
            ok                  = 0,
            parse_error
        };
    }

    // ------------------------------------------------------------------------
    //
    // env
    //
    // ------------------------------------------------------------------------
    enum class env_value_type_t : u8 {
        string,
        array,
    };

    struct env_t;
    struct env_value_t;

    using envvar_table_t        = hashtab_t<str::slice_t, env_value_t>;

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
        u32                     args_size;
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
        inline u64 hash64(const T& key);

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

        inline u64 hash64(option_t* const& key);

        inline u64 hash64(const integer_t& key);

        inline u64 hash64(const decimal_t& key);

        template <typename T>
        inline u64 hash64(const slice_t<T>& key);

        inline u64 hash64(const locale_key_t& key);

        inline u64 hash64(const token_type_t& key);

        inline u64 hash64(const locale_key_t& key) {
            return murmur::hash64(&key, sizeof(locale_key_t));
        }
    }
}
