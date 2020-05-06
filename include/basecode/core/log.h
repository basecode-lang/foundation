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

#pragma once

#include <memory>
#include <pthread.h>
#include <basecode/core/str.h>
#include <basecode/core/array.h>
#include <basecode/core/format.h>

namespace spdlog {
    class logger;
}

namespace basecode {
    enum class logger_type_t : u8 {
        default_,
        spdlog,
        syslog,
    };

    struct logger_t;
    struct logger_system_t;
    struct logger_config_t {};

    using logger_array_t        = array_t<logger_t*>;
    using shared_logger_t       = std::shared_ptr<::spdlog::logger>;

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

    union logger_subclass_t {
        struct default_t {
            FILE*                   file;
            str_t                   process_name;
            pthread_mutex_t         mutex = PTHREAD_MUTEX_INITIALIZER;
        }                           default_;
        struct {
            const s8*               ident;
            s32                     opts;
            s32                     facility;
        }                           syslog;
        struct {
            u8                      type;
            shared_logger_t         logger;
        }                           spdlog;
        ~logger_subclass_t()        {}
    };

    struct logger_t final {
        alloc_t*                    alloc;
        logger_system_t*            system;
        logger_array_t              children;
        str::slice_t                name;
        logger_subclass_t           subclass;
        log_level_t                 mask;
    };

    using logger_init_callback_t    = u0 (*)(logger_t*, logger_config_t*);
    using logger_fini_callback_t    = u0 (*)(logger_t*);
    using logger_emit_callback_t    = u0 (*)(logger_t*, log_level_t, str::slice_t);

    struct logger_system_t final {
        logger_init_callback_t  init;
        logger_fini_callback_t  fini;
        logger_emit_callback_t  emit;
        logger_type_t           type;
    };

    namespace log {
        enum class status_t : u8 {
            ok,
            invalid_logger,
            invalid_logger_system,
            invalid_default_logger,
        };

        namespace system {
            status_t fini();

            fmt_buf_t& buf();

            u0 free(logger_t* logger);

            logger_t* default_logger();

            const logger_array_t& loggers();

            logger_t* make(logger_type_t type, logger_config_t* config = {});

            status_t init(logger_type_t type, logger_config_t* config = {}, log_level_t mask = log_level_t::debug, alloc_t* alloc = context::top()->alloc);
        }

        u0 fini(logger_t* logger);

        str::slice_t status_name(status_t status);

        str::slice_t level_name(log_level_t level);

        str::slice_t type_name(logger_type_t type);

        u0 append_child(logger_t* logger, logger_t* child);

        b8 remove_child(logger_t* logger, logger_t* child);

        u0 emit(log_level_t level, str::slice_t msg, logger_t* logger = context::top()->logger);

        status_t init(logger_t* logger, logger_type_t type, logger_config_t* config = {}, log_level_t mask = log_level_t::debug, alloc_t* alloc = context::top()->alloc);

        template <typename... Args> u0 info(fmt_str_t format_str, const Args&... args, logger_t* logger = context::top()->logger) {
            auto& buf = system::buf();
            buf.clear();
            fmt::vformat_to(buf, format_str, fmt::make_format_args(args...));
            format::format_to(buf, "{}", '\0');
            emit(log_level_t::info, slice::make(buf.data(), buf.size() - 1), logger);
        }

        template <typename... Args> u0 warn(fmt_str_t format_str, const Args&... args, logger_t* logger = context::top()->logger) {
            auto& buf = system::buf();
            buf.clear();
            fmt::vformat_to(buf, format_str, fmt::make_format_args(args...));
            format::format_to(buf, "{}", '\0');
            emit(log_level_t::warn, slice::make(buf.data(), buf.size() - 1), logger);
        }

        template <typename... Args> u0 debug(fmt_str_t format_str, const Args&... args, logger_t* logger = context::top()->logger) {
            auto& buf = system::buf();
            buf.clear();
            fmt::vformat_to(buf, format_str, fmt::make_format_args(args...));
            format::format_to(buf, "{}", '\0');
            emit(log_level_t::debug, slice::make(buf.data(), buf.size() - 1), logger);
        }

        template <typename... Args> u0 error(fmt_str_t format_str, const Args&... args, logger_t* logger = context::top()->logger) {
            auto& buf = system::buf();
            buf.clear();
            fmt::vformat_to(buf, format_str, fmt::make_format_args(args...));
            format::format_to(buf, "{}", '\0');
            emit(log_level_t::error, slice::make(buf.data(), buf.size() - 1), logger);
        }

        template <typename... Args> u0 notice(fmt_str_t format_str, const Args&... args, logger_t* logger = context::top()->logger) {
            auto& buf = system::buf();
            buf.clear();
            fmt::vformat_to(buf, format_str, fmt::make_format_args(args...));
            format::format_to(buf, "{}", '\0');
            emit(log_level_t::notice, slice::make(buf.data(), buf.size() - 1), logger);
        }

        template <typename... Args> u0 alert(fmt_str_t format_str, const Args&... args, logger_t* logger = context::top()->logger) {
            auto& buf = system::buf();
            buf.clear();
            fmt::vformat_to(buf, format_str, fmt::make_format_args(args...));
            format::format_to(buf, "{}", '\0');
            emit(log_level_t::alert, slice::make(buf.data(), buf.size() - 1), logger);
        }

        template <typename... Args> u0 critical(fmt_str_t format_str, const Args&... args, logger_t* logger = context::top()->logger) {
            auto& buf = system::buf();
            buf.clear();
            fmt::vformat_to(buf, format_str, fmt::make_format_args(args...));
            format::format_to(buf, "{}", '\0');
            emit(log_level_t::critical, slice::make(buf.data(), buf.size() - 1), logger);
        }

        template <typename... Args> u0 emergency(fmt_str_t format_str, const Args&... args, logger_t* logger = context::top()->logger) {
            auto& buf = system::buf();
            buf.clear();
            fmt::vformat_to(buf, format_str, fmt::make_format_args(args...));
            format::format_to(buf, "{}", '\0');
            emit(log_level_t::emergency, slice::make(buf.data(), buf.size() - 1), logger);
        }
    }
}
