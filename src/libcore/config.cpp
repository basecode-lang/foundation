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
#include <basecode/core/array.h>
#include <basecode/core/defer.h>
#include <basecode/core/error.h>
#include <basecode/core/format.h>
#include <basecode/core/config.h>
#include <basecode/core/symtab.h>
#include <basecode/core/string.h>
#include <basecode/core/filesys.h>
#include <basecode/core/str_array.h>
#include <basecode/core/log/system/spdlog.h>
#include <basecode/core/log/system/syslog.h>

namespace basecode::config {
    constexpr u32 max_cvar_count = 256;

    using arg_map_t             = symtab_t<fe_Object*>;

    struct system_t final {
        alloc_t*                alloc;
        fe_Context*             ctx;
        cvar_t                  vars[max_cvar_count];
        str_t                   buf;
        u32                     heap_size;
    };

    system_t                    g_cfg_sys;

    struct kernel_func_t final {
        const s8*               symbol;
        fe_CFunc                func;
    };

    struct named_flag_t final {
        const s8*               name;
        s32                     value;
    };

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

    static s32 find_syslog_facility_value(const str_t& name) {
        for (u32 i = 0; s_syslog_facilities[i].name != nullptr; ++i) {
            if (name == s_syslog_facilities[i].name) {
                return s_syslog_facilities[i].value;
            }
        }
        return -1;
    }

    static str_t& to_str(fe_Context* ctx, fe_Object* obj) {
        str::reset(g_cfg_sys.buf);
        g_cfg_sys.buf.length = fe_tostring(ctx,
                                           obj,
                                           (s8*) g_cfg_sys.buf.data,
                                           g_cfg_sys.buf.capacity);
        return g_cfg_sys.buf;
    }

    static fe_Object* get_map_arg(arg_map_t& args, u32 pos) {
        fe_Object* arg;
        ++pos;
        if (symtab::find(args, slice::make((const u8*) &pos, sizeof(u32)), arg))
            return arg;
        return nullptr;
    }

    static fe_Object* next_arg(fe_Context* ctx, fe_Object** arg) {
        fe_Object* a = *arg;
        if (fe_type(ctx, a) != FE_TPAIR)
            return fe_nil();
        *arg = fe_cdr(ctx, a);
        return fe_car(ctx, a);
    }

    static u32 make_arg_map(fe_Context* ctx, fe_Object* arg, arg_map_t& args) {
        u32 pos = 1;
        while (true) {
            auto obj = next_arg(ctx, &arg);
            auto type = fe_type(ctx, obj);
            if (type == FE_TNIL) {
                break;
            } else if (type == FE_TKEYWORD) {
                auto value_obj = next_arg(ctx, &arg);
                type = fe_type(ctx, value_obj);
                if (type == FE_TNIL)
                    fe_error(ctx, "keyword parameter requires value");
                else if (type == FE_TKEYWORD)
                    fe_error(ctx, "keyword parameter value cannot be another keyword");
                else
                    symtab::insert(args, to_str(ctx, obj), value_obj);
            } else {
                symtab::insert(args, slice::make((const u8*) &pos, sizeof(u32)), obj);
                ++pos;
            }
        }
        return pos;
    }

    static const s8* vlog(fe_Context* ctx, fe_Object* arg) {
        using fmt_ctx = fmt::format_context;
        using fmt_arg = fmt::basic_format_arg<fmt_ctx>;

        auto fmt_str_arg = to_str(ctx, fe_nextarg(ctx, &arg));
        const auto fmt_str = (const s8*) string::interned::fold(fmt_str_arg).data;

        str_array_t strs{};
        str_array::init(strs, g_cfg_sys.alloc);
        defer(str_array::free(strs));

        while (true) {
            auto obj = next_arg(ctx, &arg);
            if (fe_type(ctx, obj) == FE_TNIL)
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

    static fe_Object* log_warn(fe_Context* ctx, fe_Object* arg) {
        log::warn(vlog(ctx, arg));
        return fe_nil();
    }

    static fe_Object* log_info(fe_Context* ctx, fe_Object* arg) {
        log::info(vlog(ctx, arg));
        return fe_nil();
    }

    static fe_Object* log_alert(fe_Context* ctx, fe_Object* arg) {
        log::alert(vlog(ctx, arg));
        return fe_nil();
    }

    static fe_Object* log_debug(fe_Context* ctx, fe_Object* arg) {
        log::debug(vlog(ctx, arg));
        return fe_nil();
    }

    static fe_Object* log_error(fe_Context* ctx, fe_Object* arg) {
        log::error(vlog(ctx, arg));
        return fe_nil();
    }

    static fe_Object* log_notice(fe_Context* ctx, fe_Object* arg) {
        log::notice(vlog(ctx, arg));
        return fe_nil();
    }

    static fe_Object* log_critical(fe_Context* ctx, fe_Object* arg) {
        log::critical(vlog(ctx, arg));
        return fe_nil();
    }

    static fe_Object* log_emergency(fe_Context* ctx, fe_Object* arg) {
        log::emergency(vlog(ctx, arg));
        return fe_nil();
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

    static fe_Object* log_create_color(fe_Context* ctx, fe_Object* arg) {
        arg_map_t args{};
        symtab::init(args, g_cfg_sys.alloc);
        defer(symtab::free(args));
        make_arg_map(ctx, arg, args);

        log_level_t mask = log_level_t::debug;
        spdlog_color_config_t config{};
        fe_Object* name;
        if (symtab::find(args, "#:name"_ss, name))
            config.name = string::interned::fold(to_str(ctx, name));
        else
            config.name = string::interned::fold("console"_ss);

        fe_Object* pattern;
        if (symtab::find(args, "#:pattern"_ss, pattern))
            config.pattern = string::interned::fold(to_str(ctx, pattern));

        fe_Object* maskv;
        if (symtab::find(args, "#:mask"_ss, maskv)) {
            auto ll = find_log_level(to_str(ctx, maskv));
            if (ll == -1)
                fe_error(ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        auto color_type = to_str(ctx, get_map_arg(args, 0));
        if (color_type == "out") {
            config.color_type = spdlog_color_type_t::out;
        } else if (color_type == "err") {
            config.color_type = spdlog_color_type_t::err;
        } else {
            fe_error(ctx, "invalid color-type value; expected: 'out or 'err");
        }

        logger_t* logger{};
        auto status = log::system::make(&logger,
                                        logger_type_t::spdlog,
                                        &config,
                                        mask);
        if (!OK(status))
            fe_error(ctx, "failed to create color logger");

        return fe_ptr(ctx, logger);
    }

    static fe_Object* log_create_daily_file(fe_Context* ctx, fe_Object* arg) {
        arg_map_t args{};
        symtab::init(args, g_cfg_sys.alloc);
        defer(symtab::free(args));
        make_arg_map(ctx, arg, args);

        log_level_t mask = log_level_t::debug;
        spdlog_daily_file_config_t config{};
        fe_Object* name;
        if (symtab::find(args, "#:name"_ss, name))
            config.name = string::interned::fold(to_str(ctx, name));
        else
            config.name = string::interned::fold("daily-file"_ss);

        fe_Object* pattern;
        if (symtab::find(args, "#:pattern"_ss, pattern))
            config.pattern = string::interned::fold(to_str(ctx, pattern));

        fe_Object* maskv;
        if (symtab::find(args, "#:mask"_ss, maskv)) {
            auto ll = find_log_level(to_str(ctx, maskv));
            if (ll == -1)
                fe_error(ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        auto file_name = to_str(ctx, get_map_arg(args, 0));
        path_t log_path{};
        adjust_log_path(log_path, slice::make(file_name));
        defer(path::free(log_path));
        config.file_name = string::interned::fold(log_path.str);

        auto hour = get_map_arg(args, 1);
        if (fe_type(ctx, hour) != FE_TNUMBER)
            fe_error(ctx, "hour: expected number");
        config.hour = u32(fe_tonumber(ctx, hour));

        auto minute = get_map_arg(args, 1);
        if (fe_type(ctx, minute) != FE_TNUMBER)
            fe_error(ctx, "minute: expected number");
        config.minute = u32(fe_tonumber(ctx, minute));

        logger_t* logger{};
        auto status = log::system::make(&logger,
                                        logger_type_t::spdlog,
                                        &config,
                                        mask);
        if (!OK(status))
            fe_error(ctx, "failed to create daily file logger");

        return fe_ptr(ctx, logger);
    }

    static fe_Object* log_create_basic_file(fe_Context* ctx, fe_Object* arg) {
        arg_map_t args{};
        symtab::init(args, g_cfg_sys.alloc);
        defer(symtab::free(args));
        make_arg_map(ctx, arg, args);

        log_level_t mask = log_level_t::debug;
        spdlog_basic_file_config_t config{};
        fe_Object* name;
        if (symtab::find(args, "#:name"_ss, name))
            config.name = string::interned::fold(to_str(ctx, name));
        else
            config.name = string::interned::fold("basic-file"_ss);

        fe_Object* pattern;
        if (symtab::find(args, "#:pattern"_ss, pattern))
            config.pattern = string::interned::fold(to_str(ctx, pattern));

        fe_Object* maskv;
        if (symtab::find(args, "#:mask"_ss, maskv)) {
            auto ll = find_log_level(to_str(ctx, maskv));
            if (ll == -1)
                fe_error(ctx, "#:mask invalid log level symbol");
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
            fe_error(ctx, "failed to create basic file logger");

        return fe_ptr(ctx, logger);
    }

    static fe_Object* log_create_rotating_file(fe_Context* ctx, fe_Object* arg) {
        arg_map_t args{};
        symtab::init(args, g_cfg_sys.alloc);
        defer(symtab::free(args));
        make_arg_map(ctx, arg, args);

        log_level_t mask = log_level_t::debug;
        spdlog_rotating_file_config_t config{};
        fe_Object* name;
        if (symtab::find(args, "#:name"_ss, name))
            config.name = string::interned::fold(to_str(ctx, name));
        else
            config.name = string::interned::fold("rotating-file"_ss);

        fe_Object* pattern;
        if (symtab::find(args, "#:pattern"_ss, pattern))
            config.pattern = string::interned::fold(to_str(ctx, pattern));

        fe_Object* maskv;
        if (symtab::find(args, "#:mask"_ss, maskv)) {
            auto ll = find_log_level(to_str(ctx, maskv));
            if (ll == -1)
                fe_error(ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        auto file_name = to_str(ctx, get_map_arg(args, 0));
        path_t log_path{};
        adjust_log_path(log_path, slice::make(file_name));
        defer(path::free(log_path));
        config.file_name = string::interned::fold(log_path.str);

        auto max_size = get_map_arg(args, 1);
        if (fe_type(ctx, max_size) != FE_TNUMBER)
            fe_error(ctx, "max_size: expected number");
        config.max_size  = u32(fe_tonumber(ctx, max_size));

        auto max_files = get_map_arg(args, 2);
        if (fe_type(ctx, max_files)!= FE_TNUMBER)
            fe_error(ctx, "max_files: expected number");
        config.max_files = u32(fe_tonumber(ctx, max_files));

        logger_t* logger{};
        auto status = log::system::make(&logger,
                                        logger_type_t::spdlog,
                                        &config,
                                        mask);
        if (!OK(status))
            fe_error(ctx, "failed to create rotating file logger");

        return fe_ptr(ctx, logger);
    }

    static fe_Object* syslog_create(fe_Context* ctx, fe_Object* arg) {
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

        fe_Object* maskv;
        if (symtab::find(args, "#:mask"_ss, maskv)) {
            auto ll = find_log_level(to_str(ctx, maskv));
            if (ll == -1)
                fe_error(ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        auto opts = get_map_arg(args, 1);
        if (fe_type(ctx, opts) != FE_TPAIR)
            fe_error(ctx, "opts argument must be a list");
        while (true) {
            auto obj = next_arg(ctx, &opts);
            auto type = fe_type(ctx, obj);
            if (type == FE_TNIL)
                break;
            else if (type != FE_TSYMBOL)
                fe_error(ctx, "syslog opts list may only contain symbols");
            auto opt_value = find_syslog_opt_value(to_str(ctx, obj));
            if (opt_value == -1)
                fe_error(ctx, "invalid syslog opt flag");
            config.opts |= opt_value;
        }

        auto facility = find_syslog_facility_value(to_str(ctx, get_map_arg(args, 2)));
        if (facility == -1)
            fe_error(ctx, "invalid syslog facility value");
        config.facility = facility;

        logger_t* logger{};
        auto status = log::system::make(&logger,
                                        logger_type_t::syslog,
                                        &config,
                                        mask);
        if (!OK(status))
            fe_error(ctx, "failed to create syslog logger");

        return fe_ptr(ctx, logger);
    }

    static fe_Object* current_user(fe_Context* ctx, fe_Object* arg) {
        UNUSED(arg);
        return fe_ptr(ctx, context::top()->user);
    }

    static fe_Object* current_alloc(fe_Context* ctx, fe_Object* arg) {
        UNUSED(arg);
        return fe_ptr(ctx, context::top()->alloc);
    }

    static fe_Object* current_logger(fe_Context* ctx, fe_Object* arg) {
        UNUSED(arg);
        return fe_ptr(ctx, context::top()->logger);
    }

    static fe_Object* logger_append_child(fe_Context* ctx, fe_Object* arg) {
        auto parent = fe_nextarg(ctx, &arg);
        if (fe_type(ctx, parent) != FE_TPTR)
            fe_error(ctx, "parent: expected pointer argument");
        auto child = fe_nextarg(ctx, &arg);
        if (fe_type(ctx, child) != FE_TPTR)
            fe_error(ctx, "child: expected pointer argument");
        log::append_child((logger_t*) fe_toptr(ctx, parent), (logger_t*) fe_toptr(ctx, child));
        return fe_nil();
    }

    static fe_Object* current_command_line(fe_Context* ctx, fe_Object* arg) {
        UNUSED(arg);
        const auto argc = context::top()->argc;
        const auto argv = context::top()->argv;
        fe_Object* objs[argc];
        for (u32 i = 0; i < argc; ++i)
            objs[i] = fe_string(ctx, argv[i]);
        return fe_list(ctx, &objs[0], argc);
    }

    static fe_Object* cvar_set(fe_Context* ctx, fe_Object* arg) {
        auto id = fe_nextarg(ctx, &arg);
        if (fe_type(ctx, id) != FE_TNUMBER)
            fe_error(ctx, "id: expected a number");
        auto native_id = u32(fe_tonumber(ctx, id));

        cvar_t* cvar{};
        if (!OK(cvar::get(native_id, &cvar))) {
            fe_error(ctx, "XXX: unable to find cvar");
        }

        auto value = fe_nextarg(ctx, &arg);
        auto value_type = fe_type(ctx, value);
        switch (cvar->type) {
            case cvar_type_t::flag: {
                if (fe_istrue(ctx, value))
                    cvar->value.flag = true;
                else if (fe_isnil(ctx, value))
                    cvar->value.flag = false;
                else
                    fe_error(ctx, "invalid cvar value: flag must be #t or nil");
                break;
            }
            case cvar_type_t::real: {
                if (value_type != FE_TNUMBER)
                    fe_error(ctx, "invalid cvar value: must be a valid number");
                cvar->value.real = fe_tonumber(ctx, value);
                break;
            }
            case cvar_type_t::integer: {
                if (value_type != FE_TNUMBER)
                    fe_error(ctx, "invalid cvar value: must be a valid number");
                cvar->value.integer = u64(fe_tonumber(ctx, value));
                break;
            }
            case cvar_type_t::string: {
                if (value_type != FE_TSTRING && value_type != FE_TSYMBOL)
                    fe_error(ctx, "invalid cvar value: must be a string or symbol");
                auto str = to_str(ctx, value);
                cvar->value.ptr = string::interned::fold(str).data;
                break;
            }
            case cvar_type_t::pointer: {
                fe_error(ctx, "invalid cvar value: cannot directly set pointer type");
            }
            default: {
                fe_error(ctx, "invalid cvar type");
            }
        }

        return fe_bool(ctx, true);
    }

    static fe_Object* cvar_ref(fe_Context* ctx, fe_Object* arg) {
        auto id = fe_nextarg(ctx, &arg);
        if (fe_type(ctx, id) != FE_TNUMBER)
            fe_error(ctx, "id: expected a number");
        auto native_id = u32(fe_tonumber(ctx, id));

        cvar_t* cvar{};
        if (!OK(cvar::get(native_id, &cvar))) {
            fe_error(ctx, "XXX: unable to find cvar");
        }

        switch (cvar->type) {
            case cvar_type_t::flag:
                return fe_bool(ctx, cvar->value.flag);
            case cvar_type_t::real:
                return fe_number(ctx, cvar->value.real);
            case cvar_type_t::integer:
                return fe_number(ctx, cvar->value.integer);
            case cvar_type_t::string:
                return fe_string(ctx, (const s8*) cvar->value.ptr);
            case cvar_type_t::pointer:
                return fe_ptr(ctx, (u0*) cvar->value.ptr);
            default:
                fe_error(ctx, "invalid cvar type");
        }

        return fe_nil();
    }

    static fe_Object* localized_string(fe_Context* ctx, fe_Object* arg) {
        arg_map_t args{};
        symtab::init(args, g_cfg_sys.alloc);
        defer(symtab::free(args));
        make_arg_map(ctx, arg, args);

        auto id = get_map_arg(args, 0);
        if (fe_type(ctx, id) != FE_TNUMBER)
            fe_error(ctx, "id: expected number");

        auto locale = get_map_arg(args, 1);
        if (fe_type(ctx, locale) != FE_TSYMBOL)
            fe_error(ctx, "locale: expected symbol");
        auto locale_str = string::interned::fold(to_str(ctx, locale));

        auto value = get_map_arg(args, 2);
        if (fe_type(ctx, value) != FE_TSTRING)
            fe_error(ctx, "value: expected string");
        auto value_str = string::interned::fold(to_str(ctx, value));

        string::localized::add(u32(fe_tonumber(ctx, id)), locale_str, value_str);

        return id;
    }

    static fe_Object* localized_error(fe_Context* ctx, fe_Object* arg) {
        arg_map_t args{};
        symtab::init(args, g_cfg_sys.alloc);
        defer(symtab::free(args));
        make_arg_map(ctx, arg, args);

        auto id = get_map_arg(args, 0);
        if (fe_type(ctx, id) != FE_TNUMBER)
            fe_error(ctx, "id: expected number");

        auto locale = get_map_arg(args, 1);
        if (fe_type(ctx, locale) != FE_TSYMBOL)
            fe_error(ctx, "locale: expected symbol");
        auto locale_str = string::interned::fold(to_str(ctx, locale));

        auto code = get_map_arg(args, 2);
        if (fe_type(ctx, code) != FE_TSYMBOL)
            fe_error(ctx, "code: expected symbol");
        auto code_str = string::interned::fold(to_str(ctx, code));

        auto str_id = get_map_arg(args, 3);
        if (fe_type(ctx, str_id) != FE_TNUMBER)
            fe_error(ctx, "str_id: expected number");

        error::localized::add(u32(fe_tonumber(ctx, id)),
                              u32(fe_tonumber(ctx, str_id)),
                              locale_str,
                              code_str);

        return fe_bool(ctx, true);
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
        {"cvar-set!",                cvar_set},
        {"cvar-ref",                 cvar_ref},
        {"localized-string",         localized_string},
        {"localized-error",          localized_error},
        {nullptr,                    nullptr}
    };

    namespace system {
        u0 fini() {
            fe_close(g_cfg_sys.ctx);
            str::free(g_cfg_sys.buf);
            memory::free(g_cfg_sys.alloc, g_cfg_sys.ctx);
        }

        fe_Context* context() {
            return g_cfg_sys.ctx;
        }

        status_t init(const config_settings_t& settings, alloc_t* alloc) {
            g_cfg_sys.alloc     = alloc;
            g_cfg_sys.heap_size = settings.heap_size;
            g_cfg_sys.ctx       = (fe_Context*) memory::alloc(g_cfg_sys.alloc, g_cfg_sys.heap_size);

            str::init(g_cfg_sys.buf, g_cfg_sys.alloc);
            str::reserve(g_cfg_sys.buf, 8192);

            fe_open(g_cfg_sys.ctx, g_cfg_sys.heap_size);
            for (u32 i = 0; s_kernel_funcs[i].symbol != nullptr; ++i) {
                auto symbol = fe_symbol(g_cfg_sys.ctx, s_kernel_funcs[i].symbol);
                auto func = fe_cfunc(g_cfg_sys.ctx, s_kernel_funcs[i].func);
                fe_set(g_cfg_sys.ctx, symbol, func);
            }

            std::memset(g_cfg_sys.vars, 0, sizeof(cvar_t) * max_cvar_count);

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
            auto sym_name = format::format("cvar:{}", cvar->name);
            auto symbol = fe_symbol(g_cfg_sys.ctx, str::c_str(sym_name));
            fe_set(g_cfg_sys.ctx, symbol, fe_number(g_cfg_sys.ctx, cvar->id));
        }

        static u0 remove_binding(cvar_t* cvar) {
            auto sym_name = format::format("cvar:{}", cvar->name);
            auto symbol = fe_symbol(g_cfg_sys.ctx, str::c_str(sym_name));
            fe_set(g_cfg_sys.ctx, symbol, fe_nil());
        }

        u0 clear() {
            for (auto& cvar : g_cfg_sys.vars) {
                if (cvar.type == cvar_type_t::none)
                    continue;
                remove_binding(&cvar);
                cvar.type = cvar_type_t::none;
            }
            fe_collectgarbage(g_cfg_sys.ctx);
        }

        status_t remove(u32 id) {
            if (id > (max_cvar_count - 1))
                return status_t::cvar_id_out_of_range;
            auto cvar = &g_cfg_sys.vars[id];
            if (cvar->type == cvar_type_t::none)
                return status_t::cvar_not_found;
            remove_binding(cvar);
            cvar->type = cvar_type_t::none;
            fe_collectgarbage(g_cfg_sys.ctx);
            return status_t::ok;
        }

        status_t get(u32 id, cvar_t** var) {
            *var = nullptr;
            if (id > (max_cvar_count - 1))
                return status_t::cvar_id_out_of_range;
            auto cvar = &g_cfg_sys.vars[id];
            if (cvar->type == cvar_type_t::none)
                return status_t::cvar_not_found;
            *var = cvar;
            return status_t::ok;
        }

        status_t add(u32 id, const s8* name, cvar_type_t type, s32 len) {
            if (id > (max_cvar_count - 1))
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

    status_t eval(const path_t& path, fe_Object** obj) {
        auto file = fopen(path::c_str(path), "r");
        if (!file) return status_t::bad_input;
        auto gc = fe_savegc(g_cfg_sys.ctx);
        defer(
            fe_restoregc(g_cfg_sys.ctx, gc);
            fclose(file)
        );
        while (true) {
            auto expr = fe_readfp(g_cfg_sys.ctx, file);
            if (!expr) break;
            *obj = fe_eval(g_cfg_sys.ctx, expr);
            fe_restoregc(g_cfg_sys.ctx, gc);
        }
        return status_t::ok;
    }

    status_t eval(const u8* source, u32 len, fe_Object** obj) {
        auto gc = fe_savegc(g_cfg_sys.ctx);
        defer(fe_restoregc(g_cfg_sys.ctx, gc));
        auto file = ::fmemopen((u0*) source, len, "r");
        if (!file) return status_t::bad_input;
        defer(fclose(file));
        auto expr = fe_readfp(g_cfg_sys.ctx, file);
        if (!expr) {
            *obj = fe_nil();
            return status_t::ok;
        }
        *obj = fe_eval(g_cfg_sys.ctx, expr);
        return status_t::ok;
    }
}

