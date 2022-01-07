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

#include <basecode/core/log.h>

namespace basecode {
    enum class spdlog_color_type_t : u8 {
        out,
        err,
    };

    enum class spdlog_type_t : u8 {
        global,
        color,
        basic_file,
        daily_file,
        rotating_file,
    };

    struct spdlog_base_config_t : logger_config_t {
        spdlog_base_config_t(spdlog_type_t type) : pattern(),
                                                   type(type) {}
        str::slice_t            pattern;
        spdlog_type_t           type;
    };

    struct spdlog_color_config_t : spdlog_base_config_t {
        spdlog_color_config_t() : spdlog_base_config_t(spdlog_type_t::color),
                                  color_type() {}
        spdlog_color_type_t     color_type;
    };

    struct spdlog_basic_file_config_t : spdlog_base_config_t {
        spdlog_basic_file_config_t() : spdlog_base_config_t(spdlog_type_t::basic_file),
                                       file_name() {}

        spdlog_basic_file_config_t(spdlog_type_t type) : spdlog_base_config_t(type),
                                                         file_name() {}
        str::slice_t            file_name;
    };

    struct spdlog_daily_file_config_t : spdlog_basic_file_config_t {
        spdlog_daily_file_config_t() : spdlog_basic_file_config_t(spdlog_type_t::daily_file),
                                       hour(),
                                       minute() {}
        u32                     hour;
        u32                     minute;
    };

    struct spdlog_rotating_file_config_t : spdlog_basic_file_config_t {
        spdlog_rotating_file_config_t() : spdlog_basic_file_config_t(spdlog_type_t::rotating_file),
                                          max_size(),
                                          max_files() {}
        u32                     max_size;
        u32                     max_files;
    };

    namespace log::spdlog {
        u0 fini();

        logger_system_t* system();
    }
}
