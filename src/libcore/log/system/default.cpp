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

#include <basecode/core/path.h>
#include <basecode/core/defer.h>
#include <basecode/core/log/system/default.h>

namespace basecode::log::default_ {
    static u0 fini(logger_t* logger) {
        UNUSED(logger);
    }

    static u0 init(logger_t* logger, logger_config_t* config) {
        auto cfg = (default_config_t*) config;
        auto sc  = &logger->subclass.default_;
        sc->file = !cfg->file ? stdout : cfg->file;

        str::init(sc->process_name, logger->alloc);
        path_t process_path{};
        path::init(process_path, slice::make(cfg->process_arg), logger->alloc);
        str::append(sc->process_name, path::filename(process_path));
        path::free(process_path);
    }

    static u0 emit(logger_t* logger, log_level_t level, str::slice_t msg) {
        auto sc  = &logger->subclass.default_;
        pthread_mutex_lock(&sc->mutex);
        defer(pthread_mutex_unlock(&sc->mutex));
        auto now = std::time(nullptr);
        format::print(sc->file, "{:%Y-%m-%d %T} {} [{}]: {}\n", *std::localtime(&now), sc->process_name, level_name(level), msg);
    }

    logger_system_t               g_default_system{
        .init                           = init,
        .fini                           = fini,
        .emit                           = emit,
        .type                           = logger_type_t::default_,
    };

    logger_system_t* system() {
        return &g_default_system;
    }
}

