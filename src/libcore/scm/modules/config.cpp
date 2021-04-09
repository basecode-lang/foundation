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
#include <basecode/core/error.h>
#include <basecode/core/string.h>
#include <basecode/core/filesys.h>
#include <basecode/core/log/system/spdlog.h>
#include <basecode/core/log/system/syslog.h>
#include <basecode/core/scm/modules/kernel.h>
#include <basecode/core/scm/modules/config.h>

namespace basecode::config {
    constexpr u32 max_cvar_size = 256;

    struct system_t final {
        alloc_t*                alloc;
        scm::ctx_t*             ctx;
        scm::obj_t*             current_user;
        scm::obj_t*             current_alloc;
        scm::obj_t*             current_logger;
        scm::obj_t*             current_command_line;
        cvar_t                  vars[max_cvar_size];
        str_t                   buf;
        u32                     heap_size;
    };

    struct named_flag_t final {
        const s8*               name;
        s32                     value;
    };

    system_t                    g_cfg_sys;

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

    static s32 find_syslog_facility_value(str::slice_t name);

    static u0 adjust_log_path(path_t& log_path, str::slice_t file_name);

    static const s8* vlog(str::slice_t fmt_str, scm::rest_array_t& arg);

    static scm::obj_t* current_user() {
        if (scm::is_nil(g_cfg_sys.ctx, g_cfg_sys.current_user))
            g_cfg_sys.current_user = scm::make_user_ptr(g_cfg_sys.ctx, context::top()->user);
        return g_cfg_sys.current_user;
    }

    static scm::obj_t* current_alloc() {
        if (scm::is_nil(g_cfg_sys.ctx, g_cfg_sys.current_alloc))
            g_cfg_sys.current_alloc = scm::make_user_ptr(g_cfg_sys.ctx, context::top()->alloc);
        return g_cfg_sys.current_alloc;
    }

    static scm::obj_t* current_logger() {
        if (scm::is_nil(g_cfg_sys.ctx, g_cfg_sys.current_logger))
            g_cfg_sys.current_logger = scm::make_user_ptr(g_cfg_sys.ctx, context::top()->logger);
        return g_cfg_sys.current_logger;
    }

    static scm::obj_t* cvar_ref(u32 id) {
        cvar_t* cvar{};
        if (!OK(cvar::get(id, &cvar)))
            scm::error(g_cfg_sys.ctx, "XXX: unable to find cvar");

        switch (cvar->type) {
            case cvar_type_t::flag:
                return scm::make_bool(g_cfg_sys.ctx, cvar->value.flag);
            case cvar_type_t::real:
                return scm::make_flonum(g_cfg_sys.ctx, cvar->value.real);
            case cvar_type_t::integer:
                return scm::make_fixnum(g_cfg_sys.ctx, cvar->value.integer);
            case cvar_type_t::string:
                return scm::make_string(g_cfg_sys.ctx, (const s8*) cvar->value.ptr);
            case cvar_type_t::pointer:
                return scm::make_user_ptr(g_cfg_sys.ctx, (u0*) cvar->value.ptr);
            default:
                scm::error(g_cfg_sys.ctx, "invalid cvar type");
        }

        return scm::nil(g_cfg_sys.ctx);
    }

    static b8 cvar_set_flag(u32 id, b8 value) {
        cvar_t* cvar{};
        if (!OK(cvar::get(id, &cvar)))
            scm::error(g_cfg_sys.ctx, "XXX: unable to find cvar");
        cvar->value.flag = value;
        return true;
    }

    static scm::obj_t* current_command_line() {
        if (scm::is_nil(g_cfg_sys.ctx, g_cfg_sys.current_command_line)) {
            const auto argc = context::top()->argc;
            const auto argv = context::top()->argv;
            scm::obj_t* objs[argc];
            for (u32 i = 0; i < argc; ++i)
                objs[i] = scm::make_string(g_cfg_sys.ctx, argv[i]);
            g_cfg_sys.current_command_line = scm::make_list(g_cfg_sys.ctx, &objs[0], argc);
        }
        return g_cfg_sys.current_command_line;
    }

    static s32 find_log_level(str::slice_t name) {
        for (u32 i = 0; s_log_level_map[i].name != nullptr; ++i) {
            if (strncmp((const s8*) name.data, s_log_level_map[i].name, name.length) == 0)
                return s_log_level_map[i].value;
        }
        return -1;
    }

    static b8 cvar_set_number(u32 id, f32 value) {
        cvar_t* cvar{};
        if (!OK(cvar::get(id, &cvar)))
            scm::error(g_cfg_sys.ctx, "XXX: unable to find cvar");
        cvar->value.real = value;
        return true;
    }

    static b8 cvar_set_integer(u32 id, u32 value) {
        cvar_t* cvar{};
        if (!OK(cvar::get(id, &cvar)))
            scm::error(g_cfg_sys.ctx, "XXX: unable to find cvar");
        cvar->value.integer = value;
        return true;
    }

    static s32 find_syslog_opt_value(str::slice_t name) {
        for (u32 i = 0; s_syslog_opts[i].name != nullptr; ++i) {
            if (strncmp((const s8*) name.data, s_syslog_opts[i].name, name.length) == 0)
                return s_syslog_opts[i].value;
        }
        return -1;
    }

    static b8 cvar_set_string(u32 id, str::slice_t* value) {
        cvar_t* cvar{};
        if (!OK(cvar::get(id, &cvar)))
            scm::error(g_cfg_sys.ctx, "XXX: unable to find cvar");
        cvar->value.ptr = value->data;
        return true;
    }

    static s32 find_syslog_facility_value(str::slice_t name) {
        for (u32 i = 0; s_syslog_facilities[i].name != nullptr; ++i) {
            if (strncmp((const s8*) name.data, s_syslog_facilities[i].name, name.length) == 0) {
                return s_syslog_facilities[i].value;
            }
        }
        return -1;
    }

    static scm::obj_t* log_create_color(str::slice_t* color_type,
                                        str::slice_t* name = {},
                                        str::slice_t* pattern = {},
                                        str::slice_t* maskv = {}) {
        log_level_t mask = log_level_t::debug;
        spdlog_color_config_t config{};
        config.name = name ? *name : string::interned::fold("console"_ss);
        config.pattern = pattern ? *pattern : str::slice_t{};
        if (maskv) {
            auto ll = find_log_level(*maskv);
            if (ll == -1)
                scm::error(g_cfg_sys.ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        if (*color_type == "out"_ss) {
            config.color_type = spdlog_color_type_t::out;
        } else if (*color_type == "err"_ss) {
            config.color_type = spdlog_color_type_t::err;
        } else {
            scm::error(g_cfg_sys.ctx,
                       "invalid color-type value; expected: 'out or 'err");
        }

        logger_t* logger{};
        auto status = log::system::make(&logger,
                                        logger_type_t::spdlog,
                                        &config,
                                        mask);
        if (!OK(status))
            scm::error(g_cfg_sys.ctx, "failed to create color logger");

        return scm::make_user_ptr(g_cfg_sys.ctx, logger);
    }

    static scm::obj_t* syslog_create(str::slice_t* ident,
                                     scm::obj_t* opts,
                                     str::slice_t* facility_name,
                                     str::slice_t* name = {},
                                     str::slice_t* maskv = {}) {
        auto ctx = g_cfg_sys.ctx;

        log_level_t mask = log_level_t::debug;
        syslog_config_t config{};
        config.opts  = {};
        config.name  = name ? *name : string::interned::fold("syslog"_ss);
        config.ident = (const s8*) ident->data;
        if (maskv) {
            auto ll = find_log_level(*maskv);
            if (ll == -1)
                scm::error(ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        while (!scm::is_nil(ctx, opts)) {
            auto obj = scm::car(ctx, opts);
            auto type = scm::type(obj);
            if (type == scm::obj_type_t::nil)
                break;
            else if (type != scm::obj_type_t::symbol) {
                scm::error(ctx,
                           "syslog opts list may only contain symbols");
            }
            const auto opt_name = scm::to_string(ctx, obj);
            auto opt_value = find_syslog_opt_value(opt_name);
            if (opt_value == -1)
                scm::error(ctx, "invalid syslog opt flag");
            config.opts |= opt_value;
            opts = scm::cdr(ctx, opts);
        }

        auto facility = find_syslog_facility_value(*facility_name);
        if (facility == -1)
            scm::error(ctx, "invalid syslog facility value");
        config.facility = facility;

        logger_t* logger{};
        auto status = log::system::make(&logger,
                                        logger_type_t::syslog,
                                        &config,
                                        mask);
        if (!OK(status))
            scm::error(ctx, "failed to create syslog logger");

        return scm::make_user_ptr(ctx, logger);
    }

    static u0 log_warn(str::slice_t* fmt_msg, scm::rest_array_t* rest) {
        log::warn(vlog(*fmt_msg, *rest));
    }

    static u0 log_info(str::slice_t* fmt_msg, scm::rest_array_t* rest) {
        log::info(vlog(*fmt_msg, *rest));
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
            if (ll == -1)
                scm::error(g_cfg_sys.ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        path_t log_path{};
        adjust_log_path(log_path, *file_name);
        defer(path::free(log_path));
        config.file_name = string::interned::fold(log_path.str);

        logger_t* logger{};
        auto status = log::system::make(&logger,
                                        logger_type_t::spdlog,
                                        &config,
                                        mask);
        if (!OK(status))
            scm::error(g_cfg_sys.ctx, "failed to create daily file logger");

        return scm::make_user_ptr(g_cfg_sys.ctx, logger);
    }

    static u0 log_alert(str::slice_t* fmt_msg, scm::rest_array_t* rest) {
        log::alert(vlog(*fmt_msg, *rest));
    }

    static u0 adjust_log_path(path_t& log_path, str::slice_t file_name) {
        path_t tmp_path{};
        path::init(tmp_path, file_name, g_cfg_sys.alloc);
        defer(path::free(tmp_path));
        if (!path::absolute(tmp_path)) {
            path::set(tmp_path, format::format("../logs"));
            filesys::bin_rel_path(log_path, tmp_path);
            auto status = filesys::exists(log_path);
            if (!OK(status)) {
                filesys::mkdir(log_path);
            }
            path::set(tmp_path, file_name);
            path::append(log_path, tmp_path);
        } else {
            path::init(log_path, file_name, g_cfg_sys.alloc);
        }
    }

    static u0 log_debug(str::slice_t* fmt_msg, scm::rest_array_t* rest) {
        log::debug(vlog(*fmt_msg, *rest));
    }

    static u0 log_error(str::slice_t* fmt_msg, scm::rest_array_t* rest) {
        log::error(vlog(*fmt_msg, *rest));
    }

    static const s8* vlog(str::slice_t fmt_str, scm::rest_array_t& arg) {
        auto& buf = g_cfg_sys.buf;
        str::reset(buf); {
            str_buf_t str_buf{&buf};
            fmt::dynamic_format_arg_store<fmt::format_context> fmt_args{};
            for (u32 i = 0; i < arg.size; ++i)
                fmt_args.push_back(scm::printable_t{g_cfg_sys.ctx, arg[i]});
            fmt::vformat_to(str_buf, (std::string_view) fmt_str, fmt_args);
        }
        return str::c_str(g_cfg_sys.buf);
    }

    static u0 log_notice(str::slice_t* fmt_msg, scm::rest_array_t* rest) {
        log::notice(vlog(*fmt_msg, *rest));
    }

    static u0 log_critical(str::slice_t* fmt_msg, scm::rest_array_t* rest) {
        log::critical(vlog(*fmt_msg, *rest));
    }

    static u0 log_emergency(str::slice_t* fmt_msg, scm::rest_array_t* rest) {
        log::emergency(vlog(*fmt_msg, *rest));
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
            if (ll == -1)
                scm::error(g_cfg_sys.ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        path_t log_path{};
        adjust_log_path(log_path, *file_name);
        defer(path::free(log_path));
        config.file_name = string::interned::fold(log_path.str);

        logger_t* logger{};
        auto status = log::system::make(&logger,
                                        logger_type_t::spdlog,
                                        &config,
                                        mask);
        if (!OK(status))
            scm::error(g_cfg_sys.ctx, "failed to create basic file logger");

        return scm::make_user_ptr(g_cfg_sys.ctx, logger);
    }

    static scm::obj_t* log_create_rotating_file(str::slice_t* file_name,
                                                u32 max_size,
                                                u32 max_files,
                                                str::slice_t* name = {},
                                                str::slice_t* pattern = {},
                                                str::slice_t* maskv = {}) {
        log_level_t mask = log_level_t::debug;
        spdlog_rotating_file_config_t config{};
        config.name      = name ? *name : string::interned::fold("rotating-file"_ss);
        config.pattern   = pattern ? *pattern : str::slice_t{};
        config.max_size  = max_size;
        config.max_files = max_files;
        if (maskv) {
            auto ll = find_log_level(*maskv);
            if (ll == -1)
                scm::error(g_cfg_sys.ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        path_t log_path{};
        adjust_log_path(log_path, *file_name);
        defer(path::free(log_path));
        config.file_name = string::interned::fold(log_path.str);

        logger_t* logger{};
        auto status = log::system::make(&logger,
                                        logger_type_t::spdlog,
                                        &config,
                                        mask);
        if (!OK(status)) {
            scm::error(g_cfg_sys.ctx,
                       "failed to create rotating file logger");
        }

        return scm::make_user_ptr(g_cfg_sys.ctx, logger);
    }

    static u0 logger_append_child(scm::obj_t* parent, scm::obj_t* child) {
        if (scm::type(parent) != scm::obj_type_t::ptr)
            scm::error(g_cfg_sys.ctx, "parent: expected pointer argument");
        if (scm::type(child) != scm::obj_type_t::ptr)
            scm::error(g_cfg_sys.ctx, "child: expected pointer argument");
        log::append_child((logger_t*) scm::to_user_ptr(g_cfg_sys.ctx, parent),
                          (logger_t*) scm::to_user_ptr(g_cfg_sys.ctx, child));
    }

    static u32 localized_string(u32 id, str::slice_t* locale, str::slice_t* value) {
        string::localized::add(id, *locale, *value);
        return id;
    }

    static b8 localized_error(u32 id, str::slice_t* locale, str::slice_t* code, u32 str_id) {
        return OK(error::localized::add(id, str_id, *locale, *code));
    }

    namespace system {
        namespace exports {
            using namespace scm::kernel;

            [[maybe_unused]] static proc_export_t s_exports[] = {
                {"cvar-ref"_ss, 1,
                    {
                        {(u0*) cvar_ref, "cvar_ref"_ss, type_decl::obj_ptr, 1,
                            {
                                {"id"_ss, type_decl::u32_}
                            }
                        }
                    }
                },
                {"cvar-set!"_ss, 4,
                    {
                        {
                         (u0*) cvar_set_flag, "cvar_set_flag"_ss, type_decl::b8_, 2,
                            {
                                {"id"_ss, type_decl::u32_},
                                {"value"_ss, type_decl::b8_},
                            }
                        },
                        {
                            (u0*) cvar_set_number, "cvar_set_number"_ss, type_decl::b8_, 2,
                            {
                                {"id"_ss, type_decl::u32_},
                                {"value"_ss, type_decl::f32_},
                            }
                        },
                        {
                            (u0*) cvar_set_integer, "cvar_set_integer"_ss, type_decl::b8_, 2,
                            {
                                {"id"_ss, type_decl::u32_},
                                {"value"_ss, type_decl::u32_},
                            }
                        },
                        {
                            (u0*) cvar_set_string, "cvar_set_string"_ss, type_decl::b8_, 2,
                            {
                                {"id"_ss, type_decl::u32_},
                                {"value"_ss, type_decl::slice_ptr},
                            }
                        },
                    }
                },
                {"localized-string"_ss, 1,
                    {
                        {(u0*) localized_string, "localized_string"_ss, type_decl::u32_, 3,
                            {
                                {"id"_ss, type_decl::u32_},
                                {"locale"_ss, type_decl::slice_ptr},
                                {"value"_ss, type_decl::slice_ptr},
                            }
                        }
                    }
                },
                {"localized-error"_ss, 1,
                    {
                        {(u0*) localized_error, "localized_error"_ss, type_decl::u32_, 4,
                            {
                                {"id"_ss, type_decl::u32_},
                                {"locale"_ss, type_decl::slice_ptr},
                                {"code"_ss, type_decl::slice_ptr},
                                {"str_id"_ss, type_decl::u32_},
                            }
                        }
                    }
                },
                {"log-info"_ss, 1,
                    {
                        {(u0*) log_info, "log_info"_ss, type_decl::u0_, 2,
                            {
                                {"fmt_msg"_ss, type_decl::slice_ptr},
                                {"rest"_ss, type_decl::list_ptr, .is_rest = true },
                            }
                        }
                    }
                },
                {"log-warn"_ss, 1,
                    {
                        {(u0*) log_warn, "log_warn"_ss, type_decl::u0_, 2,
                            {
                                {"fmt_msg"_ss, type_decl::slice_ptr},
                                {"rest"_ss, type_decl::list_ptr, .is_rest = true },
                            }
                        }
                    }
                },
                {"log-error"_ss, 1,
                    {
                        {(u0*) log_error, "log_error"_ss, type_decl::u0_, 2,
                            {
                                {"fmt_msg"_ss, type_decl::slice_ptr},
                                {"rest"_ss, type_decl::list_ptr, .is_rest = true },
                            }
                        }
                    }
                },
                {"log-alert"_ss, 1,
                    {
                        {(u0*) log_alert, "log_alert"_ss, type_decl::u0_, 2,
                            {
                                {"fmt_msg"_ss, type_decl::slice_ptr},
                                {"rest"_ss, type_decl::list_ptr, .is_rest = true },
                            }
                        }
                    }
                },
                {"log-debug"_ss, 1,
                    {
                        {(u0*) log_debug, "log_debug"_ss, type_decl::u0_, 2,
                            {
                                {"fmt_msg"_ss, type_decl::slice_ptr},
                                {"rest"_ss, type_decl::list_ptr, .is_rest = true },
                            }
                        }
                    }
                },
                {"log-notice"_ss, 1,
                    {
                        {(u0*) log_notice, "log_notice"_ss, type_decl::u0_, 2,
                            {
                                {"fmt_msg"_ss, type_decl::slice_ptr},
                                {"rest"_ss, type_decl::list_ptr, .is_rest = true },
                            }
                        }
                    }
                },
                {"log-critical"_ss, 1,
                    {
                        {(u0*) log_critical, "log_critical"_ss, type_decl::u0_, 2,
                            {
                                {"fmt_msg"_ss, type_decl::slice_ptr},
                                {"rest"_ss, type_decl::list_ptr, .is_rest = true },
                            }
                        }
                    }
                },
                {"log-emergency"_ss, 1,
                    {
                        {(u0*) log_emergency, "log_emergency"_ss, type_decl::u0_, 2,
                            {
                                {"fmt_msg"_ss, type_decl::slice_ptr},
                                {"rest"_ss, type_decl::list_ptr, .is_rest = true },
                            }
                        }
                    }
                },
                {"current-user"_ss, 1,
                    {
                        {(u0*) current_user, "current_user"_ss, type_decl::obj_ptr, 0}
                    }
                },
                {"current-alloc"_ss, 1,
                    {
                        {(u0*) current_alloc, "current_alloc"_ss, type_decl::obj_ptr, 0}
                    }
                },
                {"current-logger"_ss, 1,
                    {
                        {(u0*) current_logger, "current_logger"_ss, type_decl::obj_ptr, 0}
                    }
                },
                {"current-command-line"_ss, 1,
                    {
                        {(u0*) current_command_line, "current_command_line"_ss, type_decl::obj_ptr, 0}
                    }
                },
                {"log-create-basic-file"_ss, 1,
                    {
                        {(u0*) log_create_basic_file, "log_create_basic_file"_ss, type_decl::obj_ptr, 4,
                            {
                                {"file_name"_ss, type_decl::slice_ptr},
                                {"name"_ss, type_decl::slice_ptr, .default_value.p = {}, .has_default = true},
                                {"pattern"_ss, type_decl::slice_ptr, .default_value.p = {}, .has_default = true},
                                {"mask"_ss, type_decl::slice_ptr, .default_value.p = {}, .has_default = true},
                            }
                        }
                    }
                },
                {"log-create-daily-file"_ss, 1,
                    {
                        {(u0*) log_create_daily_file, "log_create_daily_file"_ss, type_decl::obj_ptr, 6,
                            {
                                {"file_name"_ss, type_decl::slice_ptr},
                                {"hour"_ss, type_decl::u32_},
                                {"minute"_ss, type_decl::u32_},
                                {"name"_ss, type_decl::slice_ptr, .default_value.p = {}, .has_default = true},
                                {"pattern"_ss, type_decl::slice_ptr, .default_value.p = {}, .has_default = true},
                                {"mask"_ss, type_decl::slice_ptr, .default_value.p = {}, .has_default = true},
                            }
                        }
                    }
                },
                {"log-create-color"_ss, 1,
                    {
                        {(u0*) log_create_color, "log_create_color"_ss, type_decl::obj_ptr, 4,
                            {
                                {"color_type"_ss, type_decl::slice_ptr},
                                {"name"_ss, type_decl::slice_ptr, .default_value.p = {}, .has_default = true},
                                {"pattern"_ss, type_decl::slice_ptr, .default_value.p = {}, .has_default = true},
                                {"mask"_ss, type_decl::slice_ptr, .default_value.p = {}, .has_default = true},
                            }
                        }
                    }
                },
                {"log-create-rotating-file"_ss, 1,
                    {
                        {(u0*) log_create_rotating_file, "log_create_rotating_file"_ss, type_decl::obj_ptr, 6,
                            {
                                {"file_name"_ss, type_decl::slice_ptr},
                                {"max_size"_ss, type_decl::u32_},
                                {"max_files"_ss, type_decl::u32_},
                                {"name"_ss, type_decl::slice_ptr, .default_value.p = {}, .has_default = true},
                                {"pattern"_ss, type_decl::slice_ptr, .default_value.p = {}, .has_default = true},
                                {"mask"_ss, type_decl::slice_ptr, .default_value.p = {}, .has_default = true},
                            }
                        }
                    }
                },
                {"syslog-create"_ss, 1,
                    {
                        {(u0*) syslog_create, "syslog_create"_ss, type_decl::obj_ptr, 5,
                            {
                                {"ident"_ss, type_decl::slice_ptr},
                                {"opts"_ss, type_decl::list_ptr},
                                {"facility"_ss, type_decl::slice_ptr},
                                {"name"_ss, type_decl::slice_ptr, .default_value.p = {}, .has_default = true},
                                {"mask"_ss, type_decl::slice_ptr, .default_value.p = {}, .has_default = true},
                            }
                        }
                    }
                },
                {"logger-append-child"_ss, 1,
                    {
                        {(u0*) logger_append_child, "logger_append_child"_ss, type_decl::obj_ptr, 2,
                            {
                                {"parent"_ss, type_decl::obj_ptr},
                                {"child"_ss, type_decl::obj_ptr},
                            }
                        }
                    }
                },
            };
        }

        u0 fini() {
            scm::free(g_cfg_sys.ctx);
            str::free(g_cfg_sys.buf);
            memory::free(g_cfg_sys.alloc, g_cfg_sys.ctx);
        }

        scm::ctx_t* context() {
            return g_cfg_sys.ctx;
        }

        status_t init(const config_settings_t& settings, alloc_t* alloc) {
            g_cfg_sys.alloc          = alloc;
            g_cfg_sys.heap_size      = settings.heap_size;
            g_cfg_sys.ctx            = (scm::ctx_t*) memory::alloc(g_cfg_sys.alloc,
                                                                   g_cfg_sys.heap_size);
            str::init(g_cfg_sys.buf, g_cfg_sys.alloc);
            str::reserve(g_cfg_sys.buf, 8192);

            scm::init(g_cfg_sys.ctx, g_cfg_sys.heap_size, g_cfg_sys.alloc);
            g_cfg_sys.current_user         = scm::nil(g_cfg_sys.ctx);
            g_cfg_sys.current_alloc        = scm::nil(g_cfg_sys.ctx);
            g_cfg_sys.current_logger       = scm::nil(g_cfg_sys.ctx);
            g_cfg_sys.current_command_line = scm::nil(g_cfg_sys.ctx);

            scm::kernel::create_common_types();
            scm::kernel::create_exports(g_cfg_sys.ctx, exports::s_exports, 22);

            std::memset(g_cfg_sys.vars, 0, sizeof(cvar_t) * max_cvar_size);

            cvar_t* cvar{};
            auto status = config::cvar::add(var_t::test_runner,
                                            "test-runner",
                                            cvar_type_t::flag);
            if (!OK(status))
                return status;
            config::cvar::get(var_t::test_runner, &cvar);
            cvar->value.flag = settings.test_runner;

            status = config::cvar::add(var_t::build_type,
                                       "build-type",
                                       cvar_type_t::string);
            if (!OK(status))
                return status;
            config::cvar::get(var_t::build_type, &cvar);
            cvar->value.ptr = (u8*) settings.build_type.data;

            status = config::cvar::add(var_t::platform,
                                       "platform",
                                       cvar_type_t::string);
            if (!OK(status))
                return status;
            config::cvar::get(var_t::platform, &cvar);
            str::slice_t platform;
#ifdef _WIN32
            platform = "Windows"_ss;
#elif __linux__
            platform = "Linux"_ss;
#elif __FreeBSD__
            platform = "FreeBSD"_ss;
#elif __OpenBSD__
            platform = "OpenBSD"_ss;
#elif __NetBSD__
            platform = "NetBSD"_ss;
#elif __APPLE__
            platform = "macOS"_ss;
#else
            platform = "unknown"_ss;
#endif
            cvar->value.ptr = string::interned::fold(platform).data;

            status = config::cvar::add(var_t::product_name,
                                       "product-name",
                                       cvar_type_t::string);
            if (!OK(status))
                return status;
            config::cvar::get(var_t::product_name, &cvar);
            cvar->value.ptr = settings.product_name.data;

            status = config::cvar::add(var_t::version_major,
                                       "version-major",
                                       cvar_type_t::integer);
            if (!OK(status))
                return status;
            config::cvar::get(var_t::version_major, &cvar);
            cvar->value.integer = settings.version.major;

            status = config::cvar::add(var_t::version_minor,
                                       "version-minor",
                                       cvar_type_t::integer);
            if (!OK(status))
                return status;
            config::cvar::get(var_t::version_minor, &cvar);
            cvar->value.integer = settings.version.minor;

            status = config::cvar::add(var_t::version_revision,
                                       "version-revision",
                                       cvar_type_t::integer);
            if (!OK(status))
                return status;
            config::cvar::get(var_t::version_revision, &cvar);
            cvar->value.integer = settings.version.revision;

            return config::status_t::ok;
        }
    }

    namespace cvar {
        static u0 add_binding(cvar_t* cvar) {
            auto sym_name = format::format("*{}*", cvar->name);
            auto symbol = scm::make_symbol(g_cfg_sys.ctx, str::c_str(sym_name));
            scm::set(g_cfg_sys.ctx, symbol, scm::make_fixnum(g_cfg_sys.ctx, cvar->id));
        }

        static u0 remove_binding(cvar_t* cvar) {
            auto sym_name = format::format("*{}*", cvar->name);
            auto symbol = scm::make_symbol(g_cfg_sys.ctx, str::c_str(sym_name));
            scm::set(g_cfg_sys.ctx, symbol, scm::nil(g_cfg_sys.ctx));
        }

        u0 clear() {
            for (auto& cvar : g_cfg_sys.vars) {
                if (cvar.type == cvar_type_t::none)
                    continue;
                remove_binding(&cvar);
                cvar.type = cvar_type_t::none;
            }
            scm::collect_garbage(g_cfg_sys.ctx);
        }

        status_t remove(u32 id) {
            if (id > (max_cvar_size - 1))
                return status_t::cvar_id_out_of_range;
            auto cvar = &g_cfg_sys.vars[id];
            if (cvar->type == cvar_type_t::none)
                return status_t::cvar_not_found;
            remove_binding(cvar);
            cvar->type = cvar_type_t::none;
            scm::collect_garbage(g_cfg_sys.ctx);
            return status_t::ok;
        }

        status_t get(u32 id, cvar_t** var) {
            *var = nullptr;
            if (id > (max_cvar_size - 1))
                return status_t::cvar_id_out_of_range;
            auto cvar = &g_cfg_sys.vars[id];
            if (cvar->type == cvar_type_t::none)
                return status_t::cvar_not_found;
            *var = cvar;
            return status_t::ok;
        }

        status_t add(u32 id, const s8* name, cvar_type_t type, s32 len) {
            if (id > (max_cvar_size - 1))
                return status_t::cvar_id_out_of_range;
            auto cvar = &g_cfg_sys.vars[id];
            if (cvar->type != cvar_type_t::none)
                return status_t::duplicate_cvar;
            cvar->name          = string::interned::fold(name, len);
            cvar->id            = id;
            cvar->type          = type;
            cvar->value.integer = 0;
            add_binding(cvar);
            return status_t::ok;
        }
    }

    status_t eval(const path_t& path, scm::obj_t** obj) {
        buf_t buf{};
        buf::init(buf, g_cfg_sys.alloc);
        auto status = buf::map_existing(buf, path);
        if (!OK(status))
            return status_t::bad_input;
        buf_crsr_t crsr{};
        buf::cursor::init(crsr, buf);
        defer(
            buf::cursor::free(crsr);
            buf::free(buf);
        );
        auto gc = scm::save_gc(g_cfg_sys.ctx);
        while (true) {
            auto expr = scm::read(g_cfg_sys.ctx, crsr);
            if (!expr) break;
            *obj = scm::eval(g_cfg_sys.ctx, expr);
            scm::restore_gc(g_cfg_sys.ctx, gc);
        }
        scm::restore_gc(g_cfg_sys.ctx, gc);
        return status_t::ok;
    }

    status_t eval(const u8* source, u32 len, scm::obj_t** obj) {
        buf_t buf{};
        buf::init(buf, g_cfg_sys.alloc);
        auto status = buf::load(buf, source, len);
        if (!OK(status))
            return status_t::bad_input;
        buf_crsr_t crsr{};
        buf::cursor::init(crsr, buf);
        auto gc = scm::save_gc(g_cfg_sys.ctx);
        defer(
            scm::restore_gc(g_cfg_sys.ctx, gc);
            buf::cursor::free(crsr);
            buf::free(buf));
        auto expr = scm::read(g_cfg_sys.ctx, crsr);
        if (!expr) {
            *obj = scm::nil(g_cfg_sys.ctx);
            return status_t::ok;
        }
        *obj = scm::eval(g_cfg_sys.ctx, expr);
        return status_t::ok;
    }
}

