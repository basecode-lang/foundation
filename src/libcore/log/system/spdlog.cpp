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

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <basecode/core/log/system/spdlog.h>

namespace basecode::log::spdlog {
    static s32 s_level_map[] = {
        [(u32) log_level_t::emergency]  = SPDLOG_LEVEL_CRITICAL,
        [(u32) log_level_t::alert]      = SPDLOG_LEVEL_CRITICAL,
        [(u32) log_level_t::critical]   = SPDLOG_LEVEL_CRITICAL,
        [(u32) log_level_t::error]      = SPDLOG_LEVEL_ERROR,
        [(u32) log_level_t::warn]       = SPDLOG_LEVEL_WARN,
        [(u32) log_level_t::notice]     = SPDLOG_LEVEL_INFO,
        [(u32) log_level_t::info]       = SPDLOG_LEVEL_INFO,
        [(u32) log_level_t::debug]      = SPDLOG_LEVEL_DEBUG,
    };

    static u0 fini(logger_t* logger) {
        auto sc  = &logger->subclass.spdlog;
        sc->logger = {};
        ::spdlog::shutdown();
    }

    static u0 init(logger_t* logger, logger_config_t* config) {
        auto sc  = &logger->subclass.spdlog;
        auto cfg = (spdlog_base_config_t*) config;
        sc->type = (u8) cfg->type;
        switch (cfg->type) {
            case spdlog_type_t::global: {
                sc->logger = ::spdlog::default_logger();
                break;
            }
            case spdlog_type_t::color: {
                auto color_cfg = (spdlog_color_config_t*) cfg;
                sc->logger = color_cfg->color_type == spdlog_color_type_t::out ? ::spdlog::stdout_color_mt((std::string) logger->name) : ::spdlog::stderr_color_mt((std::string) logger->name);
                break;
            }
            case spdlog_type_t::basic_file: {
                auto file_cfg = (spdlog_basic_file_config_t*) cfg;
                sc->logger = ::spdlog::basic_logger_mt((std::string) logger->name, (std::string) file_cfg->file_name);
                break;
            }
            case spdlog_type_t::daily_file: {
                auto file_cfg = (spdlog_daily_file_config_t*) cfg;
                sc->logger = ::spdlog::daily_logger_mt((std::string) logger->name, (std::string) file_cfg->file_name, file_cfg->hour, file_cfg->minute);
                break;
            }
            case spdlog_type_t::rotating_file: {
                auto file_cfg = (spdlog_rotating_file_config_t*) cfg;
                sc->logger = ::spdlog::rotating_logger_mt((std::string) logger->name, (std::string) file_cfg->file_name, file_cfg->max_size, file_cfg->max_files);
                break;
            }
        }
        if (!slice::empty(cfg->pattern))
            sc->logger->set_pattern((std::string) cfg->pattern);
        sc->logger->set_level((::spdlog::level::level_enum) s_level_map[(u32) logger->mask]);
    }

    static u0 emit(logger_t* logger, log_level_t level, str::slice_t msg) {
        auto sc  = &logger->subclass.spdlog;
        sc->logger->log((::spdlog::level::level_enum) s_level_map[(u32) level], "{}", msg.data);
    }

    logger_system_t               g_spdlog_system{
        .init                           = init,
        .fini                           = fini,
        .emit                           = emit,
        .type                           = logger_type_t::spdlog,
    };

    logger_system_t* system() {
        return &g_spdlog_system;
    }
}

