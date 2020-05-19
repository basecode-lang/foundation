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

#include <basecode/core/log.h>
#include <basecode/core/log/system/syslog.h>
#include <basecode/core/log/system/spdlog.h>
#include <basecode/core/log/system/default.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode::log {
    static str::slice_t         s_type_names[] = {
        "default"_ss,
        "spdlog"_ss,
        "syslog"_ss,
    };

    static str::slice_t         s_status_names[] = {
        "ok"_ss,
        "invalid logger"_ss,
        "invalid logger system"_ss,
        "invalid default logger"_ss,
    };

    static str::slice_t         s_level_names[] = {
        "EMERGENCY"_ss,
        "ALERT"_ss,
        "CRITICAL"_ss,
        "ERROR"_ss,
        "WARN"_ss,
        "NOTICE"_ss,
        "INFO"_ss,
        "DEBUG"_ss,
    };

    struct system_t final {
        alloc_t*                alloc;
        alloc_t                 slab_alloc;
        fmt_buf_t*              buf;
        logger_array_t          loggers;
        logger_t                default_logger;
    };

    system_t                    g_log_system{};

    namespace system {
        status_t fini() {
            log::fini(&g_log_system.default_logger);
            g_log_system.buf->~fmt_buf_t();
            memory::free(g_log_system.alloc, g_log_system.buf);
            for (auto logger : g_log_system.loggers)
                log::fini(logger);
            array::free(g_log_system.loggers);
            memory::fini(&g_log_system.slab_alloc);
            return log::status_t::ok;
        }

        fmt_buf_t& buf() {
            return *g_log_system.buf;
        }

        u0 free(logger_t* logger) {
            if (!array::erase(g_log_system.loggers, logger))
                return;
            log::fini(logger);
            memory::free(&g_log_system.slab_alloc, logger);
        }

        logger_t* default_logger() {
            return &g_log_system.default_logger;
        }

        const logger_array_t& loggers() {
            return g_log_system.loggers;
        }

        logger_t* make(logger_type_t type, logger_config_t* config) {
            auto logger = (logger_t*) memory::alloc(&g_log_system.slab_alloc);
            log::init(logger, type, config);
            array::append(g_log_system.loggers, logger);
            return logger;
        }

        status_t init(logger_type_t type, logger_config_t* config, log_level_t mask, alloc_t* alloc) {
            g_log_system.alloc = alloc;

            g_log_system.buf = (fmt_buf_t*) memory::alloc(g_log_system.alloc, sizeof(fmt_buf_t), alignof(fmt_buf_t));
            new(g_log_system.buf) fmt_buf_t(fmt_alloc_t(alloc));

            array::init(g_log_system.loggers, g_log_system.alloc);

            slab_config_t slab_config{};
            slab_config.backing = g_log_system.alloc;
            slab_config.buf_size = sizeof(logger_t);
            slab_config.buf_align = alignof(logger_t);
            memory::init(&g_log_system.slab_alloc, alloc_type_t::slab, &slab_config);

            return log::init(&g_log_system.default_logger, type, config, mask, alloc);
        }
    }

    u0 fini(logger_t* logger) {
        for (auto child_logger : logger->children)
            fini(child_logger);
        if (logger->system && logger->system->fini)
            logger->system->fini(logger);
        array::free(logger->children);
    }

    str::slice_t status_name(status_t status) {
        return s_status_names[(u32) status];
    }

    str::slice_t type_name(logger_type_t type) {
        return s_type_names[(u32) type];
    }

    str::slice_t level_name(log_level_t level) {
        return s_level_names[(u32) level];
    }

    u0 append_child(logger_t* logger, logger_t* child) {
        array::append(logger->children, child);
    }

    b8 remove_child(logger_t* logger, logger_t* child) {
        return array::erase(logger->children, child);
    }

    u0 emit(log_level_t level, str::slice_t msg, logger_t* logger) {
        if (!logger->system || !logger->system->emit) return;
        logger->system->emit(logger, level, msg);
        for (auto child_logger : logger->children)
            emit(level, msg, child_logger);
    }

    status_t init(logger_t* logger, logger_type_t type, logger_config_t* config, log_level_t mask, alloc_t* alloc) {
        if (!logger)
            return status_t::invalid_logger;
        switch (type) {
            case logger_type_t::default_:   logger->system = default_::system();    break;
            case logger_type_t::spdlog:     logger->system = spdlog::system();      break;
            case logger_type_t::syslog:     logger->system = syslog::system();      break;
        }
        logger->mask  = mask;
        logger->alloc = alloc;
        array::init(logger->children, logger->alloc);
        if (logger->system && logger->system->init)
            logger->system->init(logger, config);
        return status_t::ok;
    }
}
