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
#include <basecode/core/config.h>
#include <basecode/core/string.h>
#include <basecode/core/filesys.h>
#include <basecode/core/str_array.h>
#include <basecode/core/log/system/spdlog.h>
#include <basecode/core/log/system/syslog.h>

//auto proto     = ffi::proto::make("cvar_set"_ss);
//auto id_param  = ffi::param::make("id"_ss, u32_type);
//auto ctx_param = ffi::param::make("ctx"_ss, ctx_type);
//
//auto ol_flag   = ffi::overload::make("cvar_set_flag"_ss, b8_type, (u0*) cvar_set_flag);
//ffi::overload::append(ol_flag, ctx_param);
//ffi::overload::append(ol_flag, id_param);
//ffi::overload::append(ol_flag, ffi::param::make("value"_ss, b8_type));
//ffi::proto::append(proto, ol_flag);
//
//auto ol_number = ffi::overload::make("cvar_set_number"_ss, b8_type, (u0*) cvar_set_number);
//ffi::overload::append(ol_number, ctx_param);
//ffi::overload::append(ol_number, id_param);
//ffi::overload::append(ol_number, ffi::param::make("value"_ss, f32_type));
//ffi::proto::append(proto, ol_number);
//
//auto ol_integer = ffi::overload::make("cvar_set_integer"_ss, b8_type, (u0*) cvar_set_integer);
//ffi::overload::append(ol_integer, ctx_param);
//ffi::overload::append(ol_integer, id_param);
//ffi::overload::append(ol_integer, ffi::param::make("value"_ss, u32_type));
//ffi::proto::append(proto, ol_integer);
//
//auto ol_string = ffi::overload::make("cvar_set_string"_ss, b8_type, (u0*) cvar_set_string);
//ffi::overload::append(ol_string, ctx_param);
//ffi::overload::append(ol_string, id_param);
//ffi::overload::append(ol_string, ffi::param::make("value"_ss, slice_type));
//ffi::proto::append(proto, ol_string);
//

namespace basecode::config {
    constexpr u32 max_cvar_size = 256;

    using arg_map_t             = symtab_t<scm::obj_t*>;

    struct system_t final {
        alloc_t*                alloc;
        scm::ctx_t*             ctx;
        cvar_t                  vars[max_cvar_size];
        str_t                   buf;
        u32                     heap_size;
    };

    struct kernel_func_t final {
        const s8*               symbol;
        scm::native_func_t      func;
    };

    struct named_flag_t final {
        const s8*               name;
        s32                     value;
    };

    system_t                    g_cfg_sys;

    static named_flag_t s_log_level_map[] = {
        {"emergency", s32(log_level_t::emergency)},
        {"alert",     s32(log_level_t::alert)},
        {"critical",  s32(log_level_t::critical)},
        {"error",     s32(log_level_t::error)},
        {"warn",      s32(log_level_t::warn)},
        {"notice",    s32(log_level_t::notice)},
        {"info",      s32(log_level_t::info)},
        {"debug",     s32(log_level_t::debug)},
        {nullptr,     0},
    };

    static named_flag_t s_syslog_opts[] = {
        {"opt-pid",     opt_pid},
        {"opt-cons",    opt_cons},
        {"opt-odelay",  opt_odelay},
        {"opt-ndelay",  opt_ndelay},
        {"opt-nowait",  opt_nowait},
        {"opt-perror",  opt_perror},
        {nullptr,       0},
    };

    static named_flag_t s_syslog_facilities[] = {
        {"daemon",      facility_daemon},
        {"local0",      facility_local0},
        {"local1",      facility_local1},
        {"local2",      facility_local2},
        {"local3",      facility_local3},
        {"local4",      facility_local4},
        {"local5",      facility_local5},
        {"local6",      facility_local6},
        {"local7",      facility_local7},
        {nullptr,       0},
    };

    static s32 find_log_level(const str_t& name) {
        for (u32 i = 0; s_log_level_map[i].name != nullptr; ++i) {
            if (name == s_log_level_map[i].name)
                return s_log_level_map[i].value;
        }
        return -1;
    }

    static s32 find_syslog_opt_value(const str_t& name) {
        for (u32 i = 0; s_syslog_opts[i].name != nullptr; ++i) {
            if (name == s_syslog_opts[i].name)
                return s_syslog_opts[i].value;
        }
        return -1;
    }

    static str_t& to_str(scm::ctx_t* ctx, scm::obj_t* obj) {
        str::reset(g_cfg_sys.buf);
        g_cfg_sys.buf.length = scm::to_string(ctx,
                                              obj,
                                              (s8*) g_cfg_sys.buf.data,
                                              g_cfg_sys.buf.capacity);
        return g_cfg_sys.buf;
    }

    static scm::obj_t* get_map_arg(arg_map_t& args, u32 pos) {
        scm::obj_t* arg;
        ++pos;
        if (symtab::find(args, slice::make((const u8*) &pos, sizeof(u32)), arg))
            return arg;
        return nullptr;
    }

    static s32 find_syslog_facility_value(const str_t& name) {
        for (u32 i = 0; s_syslog_facilities[i].name != nullptr; ++i) {
            if (name == s_syslog_facilities[i].name) {
                return s_syslog_facilities[i].value;
            }
        }
        return -1;
    }

    static u32 make_arg_map(scm::ctx_t* ctx, scm::obj_t* arg, arg_map_t& args) {
        u32 pos = 1;
        while (true) {
            auto obj = scm::next_arg_no_chk(ctx, &arg);
            auto type = scm::type(ctx, obj);
            if (type == scm::obj_type_t::nil) {
                break;
            } else if (type == scm::obj_type_t::keyword) {
                auto value_obj = scm::next_arg_no_chk(ctx, &arg);
                type = scm::type(ctx, value_obj);
                if (type == scm::obj_type_t::nil)
                    scm::error(ctx, "keyword parameter requires value");
                else if (type == scm::obj_type_t::keyword)
                    scm::error(ctx, "keyword parameter value cannot be another keyword");
                else
                    symtab::insert(args, to_str(ctx, obj), value_obj);
            } else {
                symtab::insert(args, slice::make((const u8*) &pos, sizeof(u32)), obj);
                ++pos;
            }
        }
        return pos;
    }

    static const s8* vlog(scm::ctx_t* ctx, scm::obj_t* arg) {
        using fmt_ctx = fmt::format_context;
        using fmt_arg = fmt::basic_format_arg<fmt_ctx>;

        auto fmt_str_arg = to_str(ctx, scm::next_arg(ctx, &arg));
        const auto fmt_str = (const s8*) string::interned::fold(fmt_str_arg).data;

        str_array_t strs{};
        str_array::init(strs, g_cfg_sys.alloc);
        defer(str_array::free(strs));

        while (true) {
            auto obj = scm::next_arg_no_chk(ctx, &arg);
            if (scm::type(ctx, obj) == scm::obj_type_t::nil)
                break;
            str_array::append(strs, to_str(ctx, obj));
        }

        array_t<fmt_arg> args{};
        array::init(args, g_cfg_sys.alloc);
        defer(array::free(args));

        for (u32 i = 0; i < strs.size; ++i)
            array::append(args, fmt::detail::make_arg<fmt_ctx>(strs[i].data));

        auto& buf = g_cfg_sys.buf;
        str::reset(buf); {
            str_buf_t str_buf{&buf};
            fmt::vformat_to(str_buf,
                            fmt_str,
                            fmt::basic_format_args<fmt_ctx>(args.data, args.size));
        }

        return str::c_str(g_cfg_sys.buf);
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

    static b8 cvar_set_string(u32 id, str::slice_t* value) {
        cvar_t* cvar{};
        if (!OK(cvar::get(id, &cvar)))
            scm::error(g_cfg_sys.ctx, "XXX: unable to find cvar");
        cvar->value.ptr = value->data;
        return true;
    }

    static scm::obj_t* log_warn(scm::ctx_t* ctx, scm::obj_t* arg) {
        log::warn(vlog(ctx, arg));
        return scm::nil(ctx);
    }

    static scm::obj_t* log_info(scm::ctx_t* ctx, scm::obj_t* arg) {
        log::info(vlog(ctx, arg));
        return scm::nil(ctx);
    }

    static scm::obj_t* log_alert(scm::ctx_t* ctx, scm::obj_t* arg) {
        log::alert(vlog(ctx, arg));
        return scm::nil(ctx);
    }

    static scm::obj_t* log_debug(scm::ctx_t* ctx, scm::obj_t* arg) {
        log::debug(vlog(ctx, arg));
        return scm::nil(ctx);
    }

    static scm::obj_t* log_error(scm::ctx_t* ctx, scm::obj_t* arg) {
        log::error(vlog(ctx, arg));
        return scm::nil(ctx);
    }

    static scm::obj_t* log_notice(scm::ctx_t* ctx, scm::obj_t* arg) {
        log::notice(vlog(ctx, arg));
        return scm::nil(ctx);
    }

    static scm::obj_t* current_user(scm::ctx_t* ctx, scm::obj_t* arg) {
        UNUSED(arg);
        return scm::make_user_ptr(ctx, context::top()->user);
    }

    static scm::obj_t* log_critical(scm::ctx_t* ctx, scm::obj_t* arg) {
        log::critical(vlog(ctx, arg));
        return scm::nil(ctx);
    }

    static scm::obj_t* log_emergency(scm::ctx_t* ctx, scm::obj_t* arg) {
        log::emergency(vlog(ctx, arg));
        return scm::nil(ctx);
    }

    static scm::obj_t* current_alloc(scm::ctx_t* ctx, scm::obj_t* arg) {
        UNUSED(arg);
        return scm::make_user_ptr(ctx, context::top()->alloc);
    }

    static scm::obj_t* syslog_create(scm::ctx_t* ctx, scm::obj_t* arg) {
        arg_map_t args{};
        symtab::init(args, g_cfg_sys.alloc);
        defer(symtab::free(args));
        make_arg_map(ctx, arg, args);

        log_level_t mask = log_level_t::debug;
        syslog_config_t config{};
        config.name = string::interned::fold("syslog"_ss);
        config.opts = {};

        auto ident_str = to_str(ctx, get_map_arg(args, 0));
        config.ident = (const s8*) string::interned::fold(ident_str).data;

        scm::obj_t* maskv;
        if (symtab::find(args, "#:mask"_ss, maskv)) {
            auto ll = find_log_level(to_str(ctx, maskv));
            if (ll == -1)
                scm::error(ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        auto opts = get_map_arg(args, 1);
        if (scm::type(ctx, opts) != scm::obj_type_t::pair)
            scm::error(ctx, "opts argument must be a list");
        while (true) {
            auto obj = scm::next_arg_no_chk(ctx, &opts);
            auto type = scm::type(ctx, obj);
            if (type == scm::obj_type_t::nil)
                break;
            else if (type != scm::obj_type_t::symbol)
                scm::error(ctx, "syslog opts list may only contain symbols");
            auto opt_value = find_syslog_opt_value(to_str(ctx, obj));
            if (opt_value == -1)
                scm::error(ctx, "invalid syslog opt flag");
            config.opts |= opt_value;
        }

        auto facility = find_syslog_facility_value(to_str(ctx,
                                                          get_map_arg(args, 2)));
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

    static scm::obj_t* current_logger(scm::ctx_t* ctx, scm::obj_t* arg) {
        UNUSED(arg);
        return scm::make_user_ptr(ctx, context::top()->logger);
    }

    static u32 localized_string(u32 id, str::slice_t* locale, str::slice_t* value) {
        string::localized::add(id, *locale, *value);
        return id;
    }

    static b8 localized_error(u32 id, str::slice_t* locale, str::slice_t* code, u32 str_id) {
        return OK(error::localized::add(id, str_id, *locale, *code));
    }

    static scm::obj_t* log_create_color(scm::ctx_t* ctx, scm::obj_t* arg) {
        arg_map_t args{};
        symtab::init(args, g_cfg_sys.alloc);
        defer(symtab::free(args));
        make_arg_map(ctx, arg, args);

        log_level_t mask = log_level_t::debug;
        spdlog_color_config_t config{};
        scm::obj_t* name;
        if (symtab::find(args, "#:name"_ss, name))
            config.name = string::interned::fold(to_str(ctx, name));
        else
            config.name = string::interned::fold("console"_ss);

        scm::obj_t* pattern;
        if (symtab::find(args, "#:pattern"_ss, pattern))
            config.pattern = string::interned::fold(to_str(ctx, pattern));

        scm::obj_t* maskv;
        if (symtab::find(args, "#:mask"_ss, maskv)) {
            auto ll = find_log_level(to_str(ctx, maskv));
            if (ll == -1)
                scm::error(ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        auto color_type = to_str(ctx, get_map_arg(args, 0));
        if (color_type == "out") {
            config.color_type = spdlog_color_type_t::out;
        } else if (color_type == "err") {
            config.color_type = spdlog_color_type_t::err;
        } else {
            scm::error(ctx, "invalid color-type value; expected: 'out or 'err");
        }

        logger_t* logger{};
        auto status = log::system::make(&logger,
                                        logger_type_t::spdlog,
                                        &config,
                                        mask);
        if (!OK(status))
            scm::error(ctx, "failed to create color logger");

        return scm::make_user_ptr(ctx, logger);
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

    static scm::obj_t* logger_append_child(scm::ctx_t* ctx, scm::obj_t* arg) {
        auto parent = scm::next_arg(ctx, &arg);
        if (scm::type(ctx, parent) != scm::obj_type_t::ptr)
            scm::error(ctx, "parent: expected pointer argument");
        auto child = scm::next_arg(ctx, &arg);
        if (scm::type(ctx, child) != scm::obj_type_t::ptr)
            scm::error(ctx, "child: expected pointer argument");
        log::append_child((logger_t*) scm::to_user_ptr(ctx, parent),
                          (logger_t*) scm::to_user_ptr(ctx, child));
        return scm::nil(ctx);
    }

    static scm::obj_t* current_command_line(scm::ctx_t* ctx, scm::obj_t* arg) {
        UNUSED(arg);
        const auto argc = context::top()->argc;
        const auto argv = context::top()->argv;
        scm::obj_t* objs[argc];
        for (u32 i = 0; i < argc; ++i)
            objs[i] = scm::make_string(ctx, argv[i]);
        return scm::make_list(ctx, &objs[0], argc);
    }

    static scm::obj_t* log_create_basic_file(scm::ctx_t* ctx, scm::obj_t* arg) {
        arg_map_t args{};
        symtab::init(args, g_cfg_sys.alloc);
        defer(symtab::free(args));
        make_arg_map(ctx, arg, args);

        log_level_t mask = log_level_t::debug;
        spdlog_basic_file_config_t config{};
        scm::obj_t* name;
        if (symtab::find(args, "#:name"_ss, name))
            config.name = string::interned::fold(to_str(ctx, name));
        else
            config.name = string::interned::fold("basic-file"_ss);

        scm::obj_t* pattern;
        if (symtab::find(args, "#:pattern"_ss, pattern))
            config.pattern = string::interned::fold(to_str(ctx, pattern));

        scm::obj_t* maskv;
        if (symtab::find(args, "#:mask"_ss, maskv)) {
            auto ll = find_log_level(to_str(ctx, maskv));
            if (ll == -1)
                scm::error(ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        auto file_name = to_str(ctx, get_map_arg(args, 0));
        path_t log_path{};
        adjust_log_path(log_path, slice::make(file_name));
        defer(path::free(log_path));
        config.file_name = string::interned::fold(log_path.str);

        logger_t* logger{};
        auto status = log::system::make(&logger,
                                        logger_type_t::spdlog,
                                        &config,
                                        mask);
        if (!OK(status))
            scm::error(ctx, "failed to create basic file logger");

        return scm::make_user_ptr(ctx, logger);
    }

    static scm::obj_t* log_create_daily_file(scm::ctx_t* ctx, scm::obj_t* arg) {
        arg_map_t args{};
        symtab::init(args, g_cfg_sys.alloc);
        defer(symtab::free(args));
        make_arg_map(ctx, arg, args);

        log_level_t mask = log_level_t::debug;
        spdlog_daily_file_config_t config{};
        scm::obj_t* name;
        if (symtab::find(args, "#:name"_ss, name))
            config.name = string::interned::fold(to_str(ctx, name));
        else
            config.name = string::interned::fold("daily-file"_ss);

        scm::obj_t* pattern;
        if (symtab::find(args, "#:pattern"_ss, pattern))
            config.pattern = string::interned::fold(to_str(ctx, pattern));

        scm::obj_t* maskv;
        if (symtab::find(args, "#:mask"_ss, maskv)) {
            auto ll = find_log_level(to_str(ctx, maskv));
            if (ll == -1)
                scm::error(ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        auto file_name = to_str(ctx, get_map_arg(args, 0));
        path_t log_path{};
        adjust_log_path(log_path, slice::make(file_name));
        defer(path::free(log_path));
        config.file_name = string::interned::fold(log_path.str);

        auto hour = get_map_arg(args, 1);
        config.hour = scm::to_fixnum(ctx, hour);

        auto minute = get_map_arg(args, 1);
        config.minute = scm::to_fixnum(ctx, minute);

        logger_t* logger{};
        auto status = log::system::make(&logger,
                                        logger_type_t::spdlog,
                                        &config,
                                        mask);
        if (!OK(status))
            scm::error(ctx, "failed to create daily file logger");

        return scm::make_user_ptr(ctx, logger);
    }

    static scm::obj_t* log_create_rotating_file(scm::ctx_t* ctx, scm::obj_t* arg) {
        arg_map_t args{};
        symtab::init(args, g_cfg_sys.alloc);
        defer(symtab::free(args));
        make_arg_map(ctx, arg, args);

        log_level_t mask = log_level_t::debug;
        spdlog_rotating_file_config_t config{};
        scm::obj_t* name;
        if (symtab::find(args, "#:name"_ss, name))
            config.name = string::interned::fold(to_str(ctx, name));
        else
            config.name = string::interned::fold("rotating-file"_ss);

        scm::obj_t* pattern;
        if (symtab::find(args, "#:pattern"_ss, pattern))
            config.pattern = string::interned::fold(to_str(ctx, pattern));

        scm::obj_t* maskv;
        if (symtab::find(args, "#:mask"_ss, maskv)) {
            auto ll = find_log_level(to_str(ctx, maskv));
            if (ll == -1)
                scm::error(ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        auto file_name = to_str(ctx, get_map_arg(args, 0));
        path_t log_path{};
        adjust_log_path(log_path, slice::make(file_name));
        defer(path::free(log_path));
        config.file_name = string::interned::fold(log_path.str);

        auto max_size = get_map_arg(args, 1);
        config.max_size  = scm::to_fixnum(ctx, max_size);

        auto max_files = get_map_arg(args, 2);
        config.max_files = scm::to_fixnum(ctx, max_files);

        logger_t* logger{};
        auto status = log::system::make(&logger,
                                        logger_type_t::spdlog,
                                        &config,
                                        mask);
        if (!OK(status))
            scm::error(ctx, "failed to create rotating file logger");

        return scm::make_user_ptr(ctx, logger);
    }

    static kernel_func_t s_kernel_funcs[] = {
        {"current-user",             current_user},
        {"current-alloc",            current_alloc},
        {"current-logger",           current_logger},
        {"current-command-line",     current_command_line},
        {"log-info",                 log_info},
        {"log-warn",                 log_warn},
        {"log-error",                log_error},
        {"log-alert",                log_alert},
        {"log-debug",                log_debug},
        {"log-notice",               log_notice},
        {"log-critical",             log_critical},
        {"log-emergency",            log_emergency},
        {"log-create-color",         log_create_color},
        {"log-create-daily-file",    log_create_daily_file},
        {"log-create-basic-file",    log_create_basic_file},
        {"log-create-rotating-file", log_create_rotating_file},
        {"logger-append-child",      logger_append_child},
        {"syslog-create",            syslog_create},
//        {"cvar-set!",                cvar_set},
//        {"cvar-ref",                 cvar_ref},
//        {"localized-string",         localized_string},
//        {"localized-error",          localized_error},
        {nullptr,                    nullptr}
    };

    namespace system {
        u0 fini() {
            scm::free(g_cfg_sys.ctx);
            str::free(g_cfg_sys.buf);
            memory::free(g_cfg_sys.alloc, g_cfg_sys.ctx);
        }

        scm::ctx_t* context() {
            return g_cfg_sys.ctx;
        }

        status_t init(const config_settings_t& settings, alloc_t* alloc) {
            g_cfg_sys.alloc     = alloc;
            g_cfg_sys.heap_size = settings.heap_size;
            g_cfg_sys.ctx       = (scm::ctx_t*) memory::alloc(g_cfg_sys.alloc, g_cfg_sys.heap_size);

            str::init(g_cfg_sys.buf, g_cfg_sys.alloc);
            str::reserve(g_cfg_sys.buf, 8192);

            scm::init(g_cfg_sys.ctx, g_cfg_sys.heap_size);
            {
                auto b8_type = ffi::param::make_type(param_cls_t::int_,
                                                     param_size_t::byte,
                                                     u8(scm::ffi_type_t::boolean));
                auto u32_type = ffi::param::make_type(param_cls_t::int_, param_size_t::dword);
                auto f32_type = ffi::param::make_type(param_cls_t::float_, param_size_t::dword);
//                auto ctx_type = ffi::param::make_type(param_cls_t::ptr,
//                                                      param_size_t::qword,
//                                                      u8(scm::ffi_type_t::context));
                auto obj_type = ffi::param::make_type(param_cls_t::ptr,
                                                      param_size_t::qword,
                                                      u8(scm::ffi_type_t::object));
                auto slice_type = ffi::param::make_type(param_cls_t::ptr,
                                                        param_size_t::qword,
                                                        u8(scm::ffi_type_t::string));

                {
                    auto proto = ffi::proto::make("cvar_ref"_ss);
                    auto ol    = ffi::overload::make("cvar_ref"_ss, obj_type, (u0*) cvar_ref);
                    ffi::overload::append(ol, ffi::param::make("id"_ss, u32_type));
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "cvar-ref"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto id_param  = ffi::param::make("id"_ss, u32_type);

                    auto proto = ffi::proto::make("cvar_set"_ss);
                    auto ol_flag   = ffi::overload::make("cvar_set_flag"_ss, b8_type, (u0*) cvar_set_flag);
                    ffi::overload::append(ol_flag, id_param);
                    ffi::overload::append(ol_flag, ffi::param::make("value"_ss, b8_type));
                    ffi::proto::append(proto, ol_flag);

                    auto ol_number = ffi::overload::make("cvar_set_number"_ss, b8_type, (u0*) cvar_set_number);
                    ffi::overload::append(ol_number, id_param);
                    ffi::overload::append(ol_number, ffi::param::make("value"_ss, f32_type));
                    ffi::proto::append(proto, ol_number);

                    auto ol_integer = ffi::overload::make("cvar_set_integer"_ss, b8_type, (u0*) cvar_set_integer);
                    ffi::overload::append(ol_integer, id_param);
                    ffi::overload::append(ol_integer, ffi::param::make("value"_ss, u32_type));
                    ffi::proto::append(proto, ol_integer);

                    auto ol_string = ffi::overload::make("cvar_set_string"_ss, b8_type, (u0*) cvar_set_string);
                    ffi::overload::append(ol_string, id_param);
                    ffi::overload::append(ol_string, ffi::param::make("value"_ss, slice_type));
                    ffi::proto::append(proto, ol_string);

                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "cvar-set!"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto proto = ffi::proto::make("localized_string"_ss);
                    auto ol    = ffi::overload::make("localized_string"_ss, u32_type, (u0*) localized_string);
                    ffi::overload::append(ol, ffi::param::make("id"_ss, u32_type));
                    ffi::overload::append(ol, ffi::param::make("locale"_ss, slice_type));
                    ffi::overload::append(ol, ffi::param::make("value"_ss, slice_type));
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "localized-string"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto proto = ffi::proto::make("localized_error"_ss);
                    auto ol    = ffi::overload::make("localized_error"_ss, u32_type, (u0*) localized_error);
                    ffi::overload::append(ol, ffi::param::make("id"_ss, u32_type));
                    ffi::overload::append(ol, ffi::param::make("locale"_ss, slice_type));
                    ffi::overload::append(ol, ffi::param::make("code"_ss, slice_type));
                    ffi::overload::append(ol, ffi::param::make("str_id"_ss, u32_type));
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "localized-error"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }
            }

            for (u32 i = 0; s_kernel_funcs[i].symbol != nullptr; ++i) {
                auto symbol = scm::make_symbol(g_cfg_sys.ctx, s_kernel_funcs[i].symbol);
                auto func = scm::make_native_func(g_cfg_sys.ctx, s_kernel_funcs[i].func);
                scm::set(g_cfg_sys.ctx, symbol, func);
            }

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
        auto file = fopen(path::c_str(path), "r");
        if (!file) return status_t::bad_input;
        auto gc = scm::save_gc(g_cfg_sys.ctx);
        defer(
            scm::restore_gc(g_cfg_sys.ctx, gc);
            fclose(file)
        );
        while (true) {
            auto expr = scm::read_fp(g_cfg_sys.ctx, file);
            if (!expr) break;
            *obj = scm::eval(g_cfg_sys.ctx, expr);
            scm::restore_gc(g_cfg_sys.ctx, gc);
        }
        return status_t::ok;
    }

    status_t eval(const u8* source, u32 len, scm::obj_t** obj) {
        auto gc = scm::save_gc(g_cfg_sys.ctx);
        defer(scm::restore_gc(g_cfg_sys.ctx, gc));
        auto file = ::fmemopen((u0*) source, len, "r");
        if (!file) return status_t::bad_input;
        defer(fclose(file));
        auto expr = scm::read_fp(g_cfg_sys.ctx, file);
        if (!expr) {
            *obj = scm::nil(g_cfg_sys.ctx);
            return status_t::ok;
        }
        *obj = scm::eval(g_cfg_sys.ctx, expr);
        return status_t::ok;
    }
}

