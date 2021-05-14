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
    constexpr s32 opt_pid         = 0x01;
    constexpr s32 opt_cons        = 0x02;
    constexpr s32 opt_odelay      = 0x04;
    constexpr s32 opt_ndelay      = 0x08;
    constexpr s32 opt_nowait      = 0x10;
    constexpr s32 opt_perror      = 0x20;
    constexpr s32 facility_daemon = (3 << 3);
    constexpr s32 facility_local0 = (16 << 3);
    constexpr s32 facility_local1 = (17 << 3);
    constexpr s32 facility_local2 = (18 << 3);
    constexpr s32 facility_local3 = (19 << 3);
    constexpr s32 facility_local4 = (20 << 3);
    constexpr s32 facility_local5 = (21 << 3);
    constexpr s32 facility_local6 = (22 << 3);
    constexpr s32 facility_local7 = (23 << 3);

    struct syslog_config_t : logger_config_t {
        const s8*               ident;
        s32                     opts;
        s32                     facility;
    };

    namespace log::syslog {
        logger_system_t* system();
    }
}
