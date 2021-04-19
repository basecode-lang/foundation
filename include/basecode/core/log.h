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

#include <memory>
#include <basecode/core/str.h>
#include <basecode/core/array.h>
#include <basecode/core/mutex.h>
#include <basecode/core/format.h>

namespace spdlog {
    class logger;
}

namespace basecode {
    enum class logger_type_t : u8 {
        default_                = 240,
        spdlog,
        syslog,
    };

    struct logger_t;
    struct logger_system_t;
    struct logger_config_t {
        str::slice_t            name;
    };

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
        struct {
            FILE*               file;
            str_t               process_name;
            str_t               buf;
            b8                  is_redirected;
        }                       default_;
        struct {
            const s8*           ident;
            s32                 opts;
            s32                 facility;
        }                       syslog;
        struct {
            shared_logger_t     logger;
            str_t               buf;
            u8                  type;
        }                       spdlog  {};
        logger_subclass_t()             {}
        ~logger_subclass_t()            {}
    };

    struct logger_t final {
        alloc_t*                alloc;
        logger_system_t*        system;
        mutex_t                 lock;
        logger_array_t          children;
        str::slice_t            name;
        logger_subclass_t       subclass;
        log_level_t             mask;

        logger_t()                      {}
    };

    using logger_fini_callback_t = u0 (*)(logger_t*);
    using logger_init_callback_t = u0 (*)(logger_t*, logger_config_t*);
    using logger_emit_callback_t = u0 (*)(logger_t*, log_level_t, fmt_str_t, const fmt_args_t&);

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

            u0 free(logger_t* logger);

            logger_t* default_logger();

            const logger_array_t& loggers();

            status_t make(logger_t** logger,
                          logger_type_t type,
                          logger_config_t* config,
                          log_level_t mask = log_level_t::debug);

            status_t init(logger_type_t type,
                          logger_config_t* config = {},
                          log_level_t mask = log_level_t::debug,
                          alloc_t* alloc = context::top()->alloc);
        }

        u0 fini(logger_t* logger);

        u0 emit(log_level_t level,
                fmt_str_t format_str,
                const fmt_args_t& args,
                logger_t* logger = context::top()->logger);

        status_t init(logger_t* logger,
                      logger_type_t type,
                      logger_config_t* config = {},
                      log_level_t mask = log_level_t::debug,
                      alloc_t* alloc = context::top()->alloc);

        str::slice_t status_name(status_t status);

        str::slice_t level_name(log_level_t level);

        str::slice_t type_name(logger_type_t type);

        u0 append_child(logger_t* logger, logger_t* child);

        b8 remove_child(logger_t* logger, logger_t* child);

        template <typename... Args>
        u0 info(logger_t* logger, fmt_str_t format_str, const Args&... args) {
            emit(log_level_t::info, format_str, fmt::make_format_args(args...), logger);
        }

        template <typename... Args>
        u0 info(fmt_str_t format_str, Args&&... args) {
            info(context::top()->logger, format_str, std::forward<Args>(args)...);
        }

        template <typename... Args>
        u0 warn(logger_t* logger, fmt_str_t format_str, const Args&... args) {
            emit(log_level_t::warn, format_str, fmt::make_format_args(args...), logger);
        }

        template <typename... Args>
        u0 warn(fmt_str_t format_str, Args&&... args) {
            warn(context::top()->logger, format_str, std::forward<Args>(args)...);
        }

        template <typename... Args>
        u0 debug(logger_t* logger, fmt_str_t format_str, const Args&... args) {
            emit(log_level_t::debug, format_str, fmt::make_format_args(args...), logger);
        }

        template <typename... Args>
        u0 debug(fmt_str_t format_str, Args&&... args) {
            debug(context::top()->logger, format_str, std::forward<Args>(args)...);
        }

        template <typename... Args>
        u0 error(logger_t* logger, fmt_str_t format_str, const Args&... args) {
            emit(log_level_t::error, format_str, fmt::make_format_args(args...), logger);
        }

        template <typename... Args>
        u0 error(fmt_str_t format_str, Args&&... args) {
            error(context::top()->logger, format_str, std::forward<Args>(args)...);
        }

        template <typename... Args>
        u0 notice(logger_t* logger, fmt_str_t format_str, const Args&... args) {
            emit(log_level_t::notice, format_str, fmt::make_format_args(args...), logger);
        }

        template <typename... Args>
        u0 notice(fmt_str_t format_str, Args&&... args) {
            notice(context::top()->logger, format_str, std::forward<Args>(args)...);
        }

        template <typename... Args>
        u0 alert(logger_t* logger, fmt_str_t format_str, const Args&... args) {
            emit(log_level_t::alert, format_str, fmt::make_format_args(args...), logger);
        }

        template <typename... Args>
        u0 alert(fmt_str_t format_str, Args&&... args) {
            alert(context::top()->logger, format_str, std::forward<Args>(args)...);
        }

        template <typename... Args>
        u0 critical(logger_t* logger, fmt_str_t format_str, const Args&... args) {
            emit(log_level_t::critical, format_str, fmt::make_format_args(args...), logger);
        }

        template <typename... Args>
        u0 critical(fmt_str_t format_str, Args&&... args) {
            critical(context::top()->logger, format_str, std::forward<Args>(args)...);
        }

        template <typename... Args>
        u0 emergency(logger_t* logger, fmt_str_t format_str, const Args&... args) {
            emit(log_level_t::emergency, format_str, fmt::make_format_args(args...), logger);
        }

        template <typename... Args>
        u0 emergency(fmt_str_t format_str, Args&&... args) {
            emergency(context::top()->logger, format_str, std::forward<Args>(args)...);
        }
    }
}
