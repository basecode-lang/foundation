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

#include <basecode/core/string.h>
#include <basecode/core/filesys.h>
#include <basecode/core/obj_pool.h>
#include <basecode/core/scm/types.h>
#include <basecode/core/scm/system.h>
#include <basecode/core/scm/modules/log.h>
#include <basecode/core/log/system/spdlog.h>
#include <basecode/core/log/system/syslog.h>

namespace basecode::scm::module::log {
    struct named_flag_t final {
        const s8*               name;
        s32                     value;
    };

    struct system_t final {
        scm::ctx_t*             ctx;
        alloc_t*                alloc;
        str_t                   buf;
        obj_pool_t              storage;
    };

    system_t                    g_log_mod;

    static named_flag_t s_log_level_map[] = {
        {"emergency",    s32(log_level_t::emergency)},
        {"alert",        s32(log_level_t::alert)},
        {"critical",     s32(log_level_t::critical)},
        {"error",        s32(log_level_t::error)},
        {"warn",         s32(log_level_t::warn)},
        {"notice",       s32(log_level_t::notice)},
        {"info",         s32(log_level_t::info)},
        {"debug",        s32(log_level_t::debug)},
        {nullptr,        0},
    };

    static named_flag_t s_syslog_opts[] = {
        {"opt-pid",      opt_pid},
        {"opt-cons",     opt_cons},
        {"opt-odelay",   opt_odelay},
        {"opt-ndelay",   opt_ndelay},
        {"opt-nowait",   opt_nowait},
        {"opt-perror",   opt_perror},
        {nullptr,        0},
    };

    static named_flag_t s_syslog_facilities[] = {
        {"daemon",       facility_daemon},
        {"local0",       facility_local0},
        {"local1",       facility_local1},
        {"local2",       facility_local2},
        {"local3",       facility_local3},
        {"local4",       facility_local4},
        {"local5",       facility_local5},
        {"local6",       facility_local6},
        {"local7",       facility_local7},
        {nullptr,        0},
    };

    static s32 find_log_level(str::slice_t name);

    static s32 find_syslog_opt_value(str::slice_t name);

    static s32 find_syslog_facility_value(str::slice_t name);

    static const s8* vlog(str::slice_t fmt_str, scm::rest_array_t& arg);

    static u0 adjust_log_path(path_t& log_path, str::slice_t file_name);

    static s32 find_log_level(str::slice_t name) {
        for (u32 i = 0; s_log_level_map[i].name != nullptr; ++i) {
            const auto cmp = strncmp((const s8*) name.data,
                                     s_log_level_map[i].name,
                                     name.length);
            if (cmp == 0)
                return s_log_level_map[i].value;
        }
        return -1;
    }


    static s32 find_syslog_opt_value(str::slice_t name) {
        for (u32 i = 0; s_syslog_opts[i].name != nullptr; ++i) {
            const auto cmp = strncmp((const s8*) name.data,
                                     s_syslog_opts[i].name,
                                     name.length);
            if (cmp == 0)
                return s_syslog_opts[i].value;
        }
        return -1;
    }

    static s32 find_syslog_facility_value(str::slice_t name) {
        for (u32 i = 0; s_syslog_facilities[i].name != nullptr; ++i) {
            const auto cmp = strncmp((const s8*) name.data,
                                     s_syslog_facilities[i].name,
                                     name.length);
            if (cmp == 0)
                return s_syslog_facilities[i].value;
        }
        return -1;
    }

    static scm::obj_t* log_create_color(str::slice_t* color_type,
                                        str::slice_t* name = {},
                                        str::slice_t* pattern = {},
                                        str::slice_t* maskv = {}) {
        log_level_t mask = log_level_t::debug;
        spdlog_color_config_t config{};
        config.name = name ? *name :
                      string::interned::fold("console"_ss);
        config.pattern = pattern ? *pattern : str::slice_t{};
        if (maskv) {
            auto ll = find_log_level(*maskv);
            if (ll == -1) {
                return scm::error(g_log_mod.ctx,
                                  "#:mask invalid log level symbol");
            }
            mask = log_level_t(ll);
        }

        if (*color_type == "out"_ss) {
            config.color_type = spdlog_color_type_t::out;
        } else if (*color_type == "err"_ss) {
            config.color_type = spdlog_color_type_t::err;
        } else {
            return scm::error(
                g_log_mod.ctx,
                "invalid color-type value; expected: 'out or 'err");
        }

        logger_t* logger{};
        auto status = basecode::log::system::make(&logger,
                                                  logger_type_t::spdlog,
                                                  &config,
                                                  mask);
        if (!OK(status)) {
            return scm::error(g_log_mod.ctx,
                              "failed to create color logger");
        }

        return scm::make_user_ptr(g_log_mod.ctx, logger);
    }

    static scm::obj_t* syslog_create(str::slice_t* ident,
                                     scm::obj_t* opts,
                                     str::slice_t* facility_name,
                                     str::slice_t* name = {},
                                     str::slice_t* maskv = {}) {
        auto ctx = g_log_mod.ctx;

        log_level_t mask = log_level_t::debug;
        syslog_config_t config{};
        config.opts = {};
        config.name = name ? *name :
                      string::interned::fold("syslog"_ss);
        config.ident = (const s8*) ident->data;
        if (maskv) {
            auto ll = find_log_level(*maskv);
            if (ll == -1) {
                return scm::error(ctx,
                                  "#:mask invalid log level symbol");
            }
            mask = log_level_t(ll);
        }

        while (!scm::is_nil(ctx, opts)) {
            auto obj = scm::car(ctx, opts);
            auto type = scm::type(obj);
            if (type == scm::obj_type_t::nil)
                break;
            else if (type != scm::obj_type_t::symbol) {
                return scm::error(ctx,
                                  "syslog opts list may only contain symbols");
            }
            const auto opt_name = scm::to_string(ctx, obj);
            auto opt_value = find_syslog_opt_value(opt_name);
            if (opt_value == -1) {
                return scm::error(ctx,
                                  "invalid syslog opt flag");
            }
            config.opts |= opt_value;
            opts = scm::cdr(ctx, opts);
        }

        auto facility = find_syslog_facility_value(*facility_name);
        if (facility == -1) {
            return scm::error(ctx,
                              "invalid syslog facility value");
        }
        config.facility = facility;

        logger_t* logger{};
        auto status = basecode::log::system::make(&logger,
                                                  logger_type_t::syslog,
                                                  &config,
                                                  mask);
        if (!OK(status)) {
            return scm::error(ctx,
                              "failed to create syslog logger");
        }

        return scm::make_user_ptr(ctx, logger);
    }

    static u0 log_warn(str::slice_t* fmt_msg, scm::rest_array_t* rest) {
        basecode::log::warn(vlog(*fmt_msg, *rest));
    }

    static u0 log_info(str::slice_t* fmt_msg, scm::rest_array_t* rest) {
        basecode::log::info(vlog(*fmt_msg, *rest));
    }

    static scm::obj_t* log_create_daily_file(str::slice_t* file_name,
                                             u32 hour,
                                             u32 minute,
                                             str::slice_t* name = {},
                                             str::slice_t* pattern = {},
                                             str::slice_t* maskv = {}) {
        log_level_t mask = log_level_t::debug;
        spdlog_daily_file_config_t config{};
        config.hour    = hour;
        config.name    = name ? *name : string::interned::fold("daily-file"_ss);
        config.minute  = minute;
        config.pattern = pattern ? *pattern : str::slice_t{};
        if (maskv) {
            auto ll = find_log_level(*maskv);
            if (ll == -1) {
                return scm::error(g_log_mod.ctx,
                                  "#:mask invalid log level symbol");
            }
            mask = log_level_t(ll);
        }

        path_t log_path{};
        adjust_log_path(log_path, *file_name);
        defer(path::free(log_path));
        config.file_name = string::interned::fold(log_path.str);

        logger_t* logger{};
        auto status = basecode::log::system::make(&logger,
                                                  logger_type_t::spdlog,
                                                  &config,
                                                  mask);
        if (!OK(status)) {
            return scm::error(g_log_mod.ctx,
                              "failed to create daily file logger");
        }

        return scm::make_user_ptr(g_log_mod.ctx, logger);
    }

    static scm::obj_t* log_create_basic_file(str::slice_t* file_name,
                                             str::slice_t* name = {},
                                             str::slice_t* pattern = {},
                                             str::slice_t* maskv = {}) {
        log_level_t mask = log_level_t::debug;
        spdlog_basic_file_config_t config{};
        config.name = name ? *name : string::interned::fold("basic-file"_ss);
        config.pattern = pattern ? *pattern : str::slice_t{};
        if (maskv) {
            auto ll = find_log_level(*maskv);
            if (ll == -1) {
                return scm::error(g_log_mod.ctx,
                                  "#:mask invalid log level symbol");
            }
            mask = log_level_t(ll);
        }

        path_t log_path{};
        adjust_log_path(log_path, *file_name);
        defer(path::free(log_path));
        config.file_name = string::interned::fold(log_path.str);

        logger_t* logger{};
        auto status = basecode::log::system::make(&logger,
                                                  logger_type_t::spdlog,
                                                  &config,
                                                  mask);
        if (!OK(status)) {
            return scm::error(g_log_mod.ctx,
                              "failed to create basic file logger");
        }

        return scm::make_user_ptr(g_log_mod.ctx, logger);
    }

    static const s8* vlog(str::slice_t fmt_str, scm::rest_array_t& arg) {
        auto& buf = g_log_mod.buf;
        str::reset(buf); {
            str_buf_t str_buf{&buf};
            fmt::dynamic_format_arg_store<fmt::format_context> fmt_args{};
            for (u32 i = 0; i < arg.size; ++i)
                fmt_args.push_back(scm::printable_t{g_log_mod.ctx, arg[i]});
            fmt::vformat_to(str_buf, (std::string_view) fmt_str, fmt_args);
        }
        return str::c_str(g_log_mod.buf);
    }

    static u0 adjust_log_path(path_t& log_path, str::slice_t file_name) {
        path_t tmp_path{};
        path::init(tmp_path, file_name, g_log_mod.alloc);
        defer(path::free(tmp_path));
        if (!path::absolute(tmp_path)) {
            path::set(tmp_path, "../logs"_ss);
            filesys::bin_rel_path(log_path, tmp_path);
            auto status = filesys::exists(log_path);
            if (!OK(status))
                filesys::mkdir(log_path);
            path::append(log_path, file_name);
        } else {
            path::init(log_path, file_name, g_log_mod.alloc);
        }
    }

    static u0 logger_append_child(scm::obj_t* parent, scm::obj_t* child) {
        if (scm::type(parent) != scm::obj_type_t::ptr) {
            scm::error(g_log_mod.ctx,
                       "parent: expected pointer argument");
            return;
        }
        if (scm::type(child) != scm::obj_type_t::ptr) {
            scm::error(g_log_mod.ctx,
                       "child: expected pointer argument");
            return;
        }
        basecode::log::append_child(
            (logger_t*) scm::to_user_ptr(g_log_mod.ctx, parent),
            (logger_t*) scm::to_user_ptr(g_log_mod.ctx, child));
    }

    static scm::obj_t* log_create_rotating_file(str::slice_t* file_name,
                                                u32 max_size,
                                                u32 max_files,
                                                str::slice_t* name = {},
                                                str::slice_t* pattern = {},
                                                str::slice_t* maskv = {}) {
        log_level_t mask = log_level_t::debug;
        spdlog_rotating_file_config_t config{};
        config.name = name ? *name :
                      string::interned::fold("rotating-file"_ss);
        config.pattern   = pattern ? *pattern : str::slice_t{};
        config.max_size  = max_size;
        config.max_files = max_files;
        if (maskv) {
            auto ll = find_log_level(*maskv);
            if (ll == -1) {
                return scm::error(g_log_mod.ctx,
                                  "#:mask invalid log level symbol");
            }
            mask = log_level_t(ll);
        }

        path_t log_path{};
        adjust_log_path(log_path, *file_name);
        defer(path::free(log_path));
        config.file_name = string::interned::fold(log_path.str);

        logger_t* logger{};
        auto status = basecode::log::system::make(&logger,
                                                  logger_type_t::spdlog,
                                                  &config,
                                                  mask);
        if (!OK(status)) {
            return scm::error(g_log_mod.ctx,
                              "failed to create rotating file logger");
        }

        return scm::make_user_ptr(g_log_mod.ctx, logger);
    }

    static u0 log_alert(str::slice_t* fmt_msg, scm::rest_array_t* rest) {
        basecode::log::alert(vlog(*fmt_msg, *rest));
    }

    static u0 log_debug(str::slice_t* fmt_msg, scm::rest_array_t* rest) {
        basecode::log::debug(vlog(*fmt_msg, *rest));
    }

    static u0 log_error(str::slice_t* fmt_msg, scm::rest_array_t* rest) {
        basecode::log::error(vlog(*fmt_msg, *rest));
    }

    static u0 log_notice(str::slice_t* fmt_msg, scm::rest_array_t* rest) {
        basecode::log::notice(vlog(*fmt_msg, *rest));
    }

    static u0 log_critical(str::slice_t* fmt_msg, scm::rest_array_t* rest) {
        basecode::log::critical(vlog(*fmt_msg, *rest));
    }

    static u0 log_emergency(str::slice_t* fmt_msg, scm::rest_array_t* rest) {
        basecode::log::emergency(vlog(*fmt_msg, *rest));
    }

#define EXPORTS EXPORT_PROC("log-info",                                         \
                    OVERLOAD(log_info, u0,                                      \
                        REQ("fmt_msg", slice_ptr),                              \
                        REST("rest", list_ptr)))                                \
                EXPORT_PROC("log-warn",                                         \
                    OVERLOAD(log_warn, u0,                                      \
                        REQ("fmt_msg", slice_ptr),                              \
                        REST("rest", list_ptr)))                                \
                EXPORT_PROC("log-error",                                        \
                    OVERLOAD(log_error, u0,                                     \
                        REQ("fmt_msg", slice_ptr),                              \
                        REST("rest", list_ptr)))                                \
                EXPORT_PROC("log-alert",                                        \
                    OVERLOAD(log_alert, u0,                                     \
                        REQ("fmt_msg", slice_ptr),                              \
                        REST("rest", list_ptr)))                                \
                EXPORT_PROC("log-debug",                                        \
                    OVERLOAD(log_debug, u0,                                     \
                        REQ("fmt_msg", slice_ptr),                              \
                        REST("rest", list_ptr)))                                \
                EXPORT_PROC("log-notice",                                       \
                    OVERLOAD(log_notice, u0,                                    \
                        REQ("fmt_msg", slice_ptr),                              \
                        REST("rest", list_ptr)))                                \
                EXPORT_PROC("log-critical",                                     \
                    OVERLOAD(log_critical, u0,                                  \
                        REQ("fmt_msg", slice_ptr),                              \
                        REST("rest", list_ptr)))                                \
                EXPORT_PROC("log-emergency",                                    \
                    OVERLOAD(log_emergency, u0,                                 \
                        REQ("fmt_msg", slice_ptr),                              \
                        REST("rest", list_ptr)))                                \
                EXPORT_PROC("log-create-basic-file",                            \
                    OVERLOAD(log_create_basic_file, obj_ptr,                    \
                        REQ("file_name", slice_ptr),                            \
                        OPT("name", slice_ptr, nullptr),                        \
                        OPT("pattern", slice_ptr, nullptr),                     \
                        OPT("mask", slice_ptr, nullptr)))                       \
                EXPORT_PROC("log-create-daily-file",                            \
                    OVERLOAD(log_create_daily_file, obj_ptr,                    \
                        REQ("file_name", slice_ptr),                            \
                        REQ("hour", u32),                                       \
                        REQ("minute", u32),                                     \
                        OPT("name", slice_ptr, nullptr),                        \
                        OPT("pattern", slice_ptr, nullptr),                     \
                        OPT("mask", slice_ptr, nullptr)))                       \
                EXPORT_PROC("log-create-color",                                 \
                    OVERLOAD(log_create_color, obj_ptr,                         \
                        REQ("color_type", slice_ptr),                           \
                        OPT("name", slice_ptr, nullptr),                        \
                        OPT("pattern", slice_ptr, nullptr),                     \
                        OPT("mask", slice_ptr, nullptr)))                       \
                EXPORT_PROC("log-create-rotating-file",                         \
                    OVERLOAD(log_create_rotating_file, obj_ptr,                 \
                        REQ("file_name", slice_ptr),                            \
                        REQ("max_size", u32),                                   \
                        REQ("max_files", u32),                                  \
                        OPT("name", slice_ptr, nullptr),                        \
                        OPT("pattern", slice_ptr, nullptr),                     \
                        OPT("mask", slice_ptr, nullptr)))                       \
                EXPORT_PROC("syslog-create",                                    \
                    OVERLOAD(syslog_create, obj_ptr,                            \
                        REQ("ident", slice_ptr),                                \
                        REQ("opts", list_ptr),                                  \
                        REQ("facility", slice_ptr),                             \
                        OPT("name", slice_ptr, nullptr),                        \
                        OPT("mask", slice_ptr, nullptr)))                       \
                EXPORT_PROC("logger-append-child",                              \
                    OVERLOAD(logger_append_child, u0,                           \
                        REQ("parent", obj_ptr),                                 \
                        REQ("child", obj_ptr)))

namespace system {
    namespace exports {
        using namespace scm::kernel;
        static proc_export_t s_exports[] = {
#define EXPORT_PROC(n, ...)    basecode::scm::kernel::proc_export_t{    \
    n##_ss, \
    u32(VA_COUNT(__VA_ARGS__)), \
    __VA_ARGS__},
            EXPORTS
#undef EXPORT_PROC
                {str::slice_t{}},
            };
        }

        u0 fini() {
            obj_pool::free(g_log_mod.storage);
            str::free(g_log_mod.buf);
        }

        status_t init(scm::ctx_t* ctx, alloc_t* alloc) {
            g_log_mod.ctx   = ctx;
            g_log_mod.alloc = alloc;

            str::init(g_log_mod.buf, g_log_mod.alloc);
            str::reserve(g_log_mod.buf, 8192);

            obj_pool::init(g_log_mod.storage, g_log_mod.alloc);
            kernel::create_exports(g_log_mod.ctx, exports::s_exports);

            return status_t::ok;
        }
    }
}
