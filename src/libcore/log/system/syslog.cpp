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

#include <syslog.h>
#include <basecode/core/log/system/syslog.h>

namespace basecode::log::syslog {
    static u0 fini(logger_t* logger) {
        UNUSED(logger);
        closelog();
    }

    static u0 init(logger_t* logger, logger_config_t* config) {
        auto cfg = (syslog_config_t*) config;
        auto sc  = &logger->subclass.syslog;
        sc->opts     = cfg->opts;
        sc->ident    = cfg->ident;
        sc->facility = cfg->facility;
        setlogmask(LOG_UPTO((u32) logger->mask));
        openlog(sc->ident, sc->opts, sc->facility);
    }

    static u0 emit(logger_t* logger, log_level_t level, str::slice_t msg) {
        UNUSED(logger);
        ::syslog((s32) level, "%s", msg.data);
    }

    logger_system_t               g_syslog_system{
        .init                           = init,
        .fini                           = fini,
        .emit                           = emit,
        .type                           = logger_type_t::syslog,
    };

    logger_system_t* system() {
        return &g_syslog_system;
    }
}
