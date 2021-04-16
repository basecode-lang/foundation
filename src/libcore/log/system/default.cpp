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
#include <basecode/core/log/system/default.h>

namespace basecode::log::default_ {
    static u0 fini(logger_t* logger) {
        auto sc  = &logger->subclass.default_;
        str::free(sc->process_name);
        str::free(sc->buf);
    }

    static u0 init(logger_t* logger, logger_config_t* config) {
        auto cfg = (default_config_t*) config;
        auto sc  = &logger->subclass.default_;
        sc->file = !cfg->file ? stdout : cfg->file;
#ifdef _WIN32
        sc->is_redirected = isatty(_fileno(sc->file));
#else
        sc->is_redirected = isatty(fileno(sc->file));
#endif

        str::init(sc->buf, logger->alloc);
        str::reserve(sc->buf, 4096);

        str::init(sc->process_name, logger->alloc);
        path_t process_path{};
        path::init(process_path, slice::make(cfg->process_arg), logger->alloc);
        str::append(sc->process_name, path::filename(process_path));
        path::free(process_path);
    }

    static u0 emit(logger_t* logger, log_level_t level, fmt_str_t format_str, const fmt_args_t& args) {
        if (s32(level) > s32(logger->mask))
            return;
        auto sc  = &logger->subclass.default_;
        scoped_lock_t lock(&logger->lock);
        {
            auto now = std::time(nullptr);
            str::reset(sc->buf);
            str_buf_t fmt_buf(&sc->buf);
            format::format_to(fmt_buf,
                              "[{:%Y-%m-%d %H:%M:%S}] [{}] [{:<9}] ",
                              fmt::localtime(now),
                              sc->process_name,
                              level_name(level));
            fmt::vformat_to(fmt_buf, format_str, args);
            format::format_to(fmt_buf, "\n");
        }
        if (sc->is_redirected)
            format::print("{}", sc->buf);
        else
            format::print(sc->file, "{}", sc->buf);
    }

    logger_system_t             g_default_system{
        .init   = init,
        .fini   = fini,
        .emit   = emit,
        .type   = logger_type_t::default_,
    };

    logger_system_t* system() {
        return &g_default_system;
    }
}

