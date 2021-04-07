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
#include <basecode/core/scm/modules/config.h>

namespace basecode::config {
    constexpr u32 max_cvar_size = 256;

    using arg_map_t             = symtab_t<scm::obj_t*>;

    struct system_t final {
        alloc_t*                alloc;
        scm::ctx_t*             ctx;
        scm::obj_t*             current_user;
        scm::obj_t*             current_alloc;
        scm::obj_t*             current_logger;
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

    static scm::obj_t* current_command_line() {
        const auto argc = context::top()->argc;
        const auto argv = context::top()->argv;
        scm::obj_t* objs[argc];
        for (u32 i = 0; i < argc; ++i)
            objs[i] = scm::make_string(g_cfg_sys.ctx, argv[i]);
        return scm::make_list(g_cfg_sys.ctx, &objs[0], argc);
    }

    static s32 find_log_level(str::slice_t name) {
        for (u32 i = 0; s_log_level_map[i].name != nullptr; ++i) {
            if (strncmp((const s8*) name.data, s_log_level_map[i].name, name.length) == 0)
                return s_log_level_map[i].value;
        }
        return -1;
    }

    static s32 find_syslog_opt_value(str::slice_t name) {
        for (u32 i = 0; s_syslog_opts[i].name != nullptr; ++i) {
            if (strncmp((const s8*) name.data, s_syslog_opts[i].name, name.length) == 0)
                return s_syslog_opts[i].value;
        }
        return -1;
    }

    static scm::obj_t* get_map_arg(arg_map_t& args, u32 pos) {
        scm::obj_t* arg;
        ++pos;
        if (symtab::find(args, slice::make((const u8*) &pos, sizeof(u32)), arg))
            return arg;
        return nullptr;
    }

    static s32 find_syslog_facility_value(str::slice_t name) {
        for (u32 i = 0; s_syslog_facilities[i].name != nullptr; ++i) {
            if (strncmp((const s8*) name.data, s_syslog_facilities[i].name, name.length) == 0) {
                return s_syslog_facilities[i].value;
            }
        }
        return -1;
    }

    static u32 make_arg_map(scm::ctx_t* ctx, scm::obj_t* arg, arg_map_t& args) {
        u32 pos = 1;
        while (true) {
            auto obj = scm::next_arg_no_chk(ctx, &arg);
            auto type = scm::type(obj);
            if (type == scm::obj_type_t::nil) {
                break;
            } else if (type == scm::obj_type_t::keyword) {
                auto value_obj = scm::next_arg_no_chk(ctx, &arg);
                type = scm::type(value_obj);
                if (type == scm::obj_type_t::nil)
                    scm::error(ctx, "keyword parameter requires value");
                else if (type == scm::obj_type_t::keyword)
                    scm::error(ctx, "keyword parameter value cannot be another keyword");
                else {
                    symtab::insert(args, scm::to_string(ctx, obj), value_obj);
                }
            } else {
                symtab::insert(args, slice::make((const u8*) &pos, sizeof(u32)), obj);
                ++pos;
            }
        }
        return pos;
    }

    static const s8* vlog(str::slice_t fmt_str, scm::obj_t* arg) {
        using fmt_ctx = fmt::format_context;
        using fmt_arg = fmt::basic_format_arg<fmt_ctx>;

        array_t<fmt_arg> objs{};
        array::init(objs, g_cfg_sys.alloc);
        defer(array::free(objs));

        while (true) {
            auto obj = scm::next_arg_no_chk(g_cfg_sys.ctx, &arg);
            if (scm::type(obj) == scm::obj_type_t::nil)
                break;
            array::append(objs,
                          fmt::detail::make_arg<fmt_ctx>(scm::printable_t{g_cfg_sys.ctx, obj}));
        }

        auto& buf = g_cfg_sys.buf;
        str::reset(buf); {
            str_buf_t str_buf{&buf};
            fmt::vformat_to(str_buf,
                            (std::string_view) fmt_str,
                            fmt::basic_format_args<fmt_ctx>(objs.data, objs.size));
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

    static u0 log_warn(str::slice_t* fmt_msg, scm::obj_t* rest) {
        log::warn(vlog(*fmt_msg, rest));
    }

    static u0 log_info(str::slice_t* fmt_msg, scm::obj_t* rest) {
        log::info(vlog(*fmt_msg, rest));
    }

    static u0 log_alert(str::slice_t* fmt_msg, scm::obj_t* rest) {
        log::alert(vlog(*fmt_msg, rest));
    }

    static u0 log_debug(str::slice_t* fmt_msg, scm::obj_t* rest) {
        log::debug(vlog(*fmt_msg, rest));
    }

    static u0 log_error(str::slice_t* fmt_msg, scm::obj_t* rest) {
        log::error(vlog(*fmt_msg, rest));
    }

    static u0 log_notice(str::slice_t* fmt_msg, scm::obj_t* rest) {
        log::notice(vlog(*fmt_msg, rest));
    }

    static u0 log_critical(str::slice_t* fmt_msg, scm::obj_t* rest) {
        log::critical(vlog(*fmt_msg, rest));
    }

    static u0 log_emergency(str::slice_t* fmt_msg, scm::obj_t* rest) {
        log::emergency(vlog(*fmt_msg, rest));
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

        auto ident_str = scm::to_string(ctx, get_map_arg(args, 0));
        config.ident = (const s8*) string::interned::fold(ident_str).data;

        scm::obj_t* maskv;
        if (symtab::find(args, "#:mask"_ss, maskv)) {
            auto ll = find_log_level(scm::to_string(ctx, maskv));
            if (ll == -1)
                scm::error(ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        auto opts = get_map_arg(args, 1);
        if (scm::type(opts) != scm::obj_type_t::pair)
            scm::error(ctx, "opts argument must be a list");
        while (true) {
            auto obj = scm::next_arg_no_chk(ctx, &opts);
            auto type = scm::type(obj);
            if (type == scm::obj_type_t::nil)
                break;
            else if (type != scm::obj_type_t::symbol)
                scm::error(ctx, "syslog opts list may only contain symbols");
            auto opt_value = find_syslog_opt_value(scm::to_string(ctx, obj));
            if (opt_value == -1)
                scm::error(ctx, "invalid syslog opt flag");
            config.opts |= opt_value;
        }

        auto facility = find_syslog_facility_value(scm::to_string(ctx,
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

    static scm::obj_t* log_create_color(scm::ctx_t* ctx, scm::obj_t* arg) {
        arg_map_t args{};
        symtab::init(args, g_cfg_sys.alloc);
        defer(symtab::free(args));
        make_arg_map(ctx, arg, args);

        log_level_t mask = log_level_t::debug;
        spdlog_color_config_t config{};
        scm::obj_t* name;
        if (symtab::find(args, "#:name"_ss, name))
            config.name = string::interned::fold(scm::to_string(ctx, name));
        else
            config.name = string::interned::fold("console"_ss);

        scm::obj_t* pattern;
        if (symtab::find(args, "#:pattern"_ss, pattern))
            config.pattern = string::interned::fold(scm::to_string(ctx, pattern));

        scm::obj_t* maskv;
        if (symtab::find(args, "#:mask"_ss, maskv)) {
            auto ll = find_log_level(scm::to_string(ctx, maskv));
            if (ll == -1)
                scm::error(ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        auto color_type = scm::to_string(ctx, get_map_arg(args, 0));
        if (color_type == "out"_ss) {
            config.color_type = spdlog_color_type_t::out;
        } else if (color_type == "err"_ss) {
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

    static scm::obj_t* logger_append_child(scm::ctx_t* ctx, scm::obj_t* arg) {
        auto parent = scm::next_arg(ctx, &arg);
        if (scm::type(parent) != scm::obj_type_t::ptr)
            scm::error(ctx, "parent: expected pointer argument");
        auto child = scm::next_arg(ctx, &arg);
        if (scm::type(child) != scm::obj_type_t::ptr)
            scm::error(ctx, "child: expected pointer argument");
        log::append_child((logger_t*) scm::to_user_ptr(ctx, parent),
                          (logger_t*) scm::to_user_ptr(ctx, child));
        return scm::nil(ctx);
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
            config.name = scm::to_string(ctx, name);
        else
            config.name = string::interned::fold("daily-file"_ss);

        scm::obj_t* pattern;
        if (symtab::find(args, "#:pattern"_ss, pattern))
            config.pattern = scm::to_string(ctx, pattern);

        scm::obj_t* maskv;
        if (symtab::find(args, "#:mask"_ss, maskv)) {
            auto ll = find_log_level(scm::to_string(ctx, maskv));
            if (ll == -1)
                scm::error(ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        auto file_name = scm::to_string(ctx, get_map_arg(args, 0));
        path_t log_path{};
        adjust_log_path(log_path, file_name);
        defer(path::free(log_path));
        config.file_name = string::interned::fold(log_path.str);

        auto hour = get_map_arg(args, 1);
        config.hour = scm::to_fixnum(hour);

        auto minute = get_map_arg(args, 1);
        config.minute = scm::to_fixnum(minute);

        logger_t* logger{};
        auto status = log::system::make(&logger,
                                        logger_type_t::spdlog,
                                        &config,
                                        mask);
        if (!OK(status))
            scm::error(ctx, "failed to create daily file logger");

        return scm::make_user_ptr(ctx, logger);
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
            config.name = scm::to_string(ctx, name);
        else
            config.name = string::interned::fold("basic-file"_ss);

        scm::obj_t* pattern;
        if (symtab::find(args, "#:pattern"_ss, pattern))
            config.pattern = scm::to_string(ctx, pattern);

        scm::obj_t* maskv;
        if (symtab::find(args, "#:mask"_ss, maskv)) {
            auto ll = find_log_level(scm::to_string(ctx, maskv));
            if (ll == -1)
                scm::error(ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        auto file_name = scm::to_string(ctx, get_map_arg(args, 0));
        path_t log_path{};
        adjust_log_path(log_path, file_name);
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

    static scm::obj_t* log_create_rotating_file(scm::ctx_t* ctx, scm::obj_t* arg) {
        arg_map_t args{};
        symtab::init(args, g_cfg_sys.alloc);
        defer(symtab::free(args));
        make_arg_map(ctx, arg, args);

        log_level_t mask = log_level_t::debug;
        spdlog_rotating_file_config_t config{};
        scm::obj_t* name;
        if (symtab::find(args, "#:name"_ss, name))
            config.name = scm::to_string(ctx, name);
        else
            config.name = string::interned::fold("rotating-file"_ss);

        scm::obj_t* pattern;
        if (symtab::find(args, "#:pattern"_ss, pattern))
            config.pattern = scm::to_string(ctx, pattern);

        scm::obj_t* maskv;
        if (symtab::find(args, "#:mask"_ss, maskv)) {
            auto ll = find_log_level(scm::to_string(ctx, maskv));
            if (ll == -1)
                scm::error(ctx, "#:mask invalid log level symbol");
            mask = log_level_t(ll);
        }

        auto file_name = scm::to_string(ctx, get_map_arg(args, 0));
        path_t log_path{};
        adjust_log_path(log_path, file_name);
        defer(path::free(log_path));
        config.file_name = string::interned::fold(log_path.str);

        auto max_size = get_map_arg(args, 1);
        config.max_size  = scm::to_fixnum(max_size);

        auto max_files = get_map_arg(args, 2);
        config.max_files = scm::to_fixnum(max_files);

        logger_t* logger{};
        auto status = log::system::make(&logger,
                                        logger_type_t::spdlog,
                                        &config,
                                        mask);
        if (!OK(status))
            scm::error(ctx, "failed to create rotating file logger");

        return scm::make_user_ptr(ctx, logger);
    }

    static u32 localized_string(u32 id, str::slice_t* locale, str::slice_t* value) {
        string::localized::add(id, *locale, *value);
        return id;
    }

    static b8 localized_error(u32 id, str::slice_t* locale, str::slice_t* code, u32 str_id) {
        return OK(error::localized::add(id, str_id, *locale, *code));
    }

    static kernel_func_t s_kernel_funcs[] = {
        {"log-create-color",         log_create_color},
        {"log-create-daily-file",    log_create_daily_file},
        {"log-create-basic-file",    log_create_basic_file},
        {"log-create-rotating-file", log_create_rotating_file},
        {"logger-append-child",      logger_append_child},
        {"syslog-create",            syslog_create},
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
            g_cfg_sys.alloc          = alloc;
            g_cfg_sys.heap_size      = settings.heap_size;
            g_cfg_sys.ctx            = (scm::ctx_t*) memory::alloc(g_cfg_sys.alloc,
                                                                   g_cfg_sys.heap_size);
            str::init(g_cfg_sys.buf, g_cfg_sys.alloc);
            str::reserve(g_cfg_sys.buf, 8192);

            scm::init(g_cfg_sys.ctx, g_cfg_sys.heap_size, g_cfg_sys.alloc);
            g_cfg_sys.current_user   = scm::nil(g_cfg_sys.ctx);
            g_cfg_sys.current_alloc  = scm::nil(g_cfg_sys.ctx);
            g_cfg_sys.current_logger = scm::nil(g_cfg_sys.ctx);

            {
                auto b8_type = ffi::param::make_type(param_cls_t::int_,
                                                     param_size_t::byte,
                                                     u8(scm::ffi_type_t::boolean));
                auto u0_type = ffi::param::make_type(param_cls_t::void_, param_size_t::none);
                auto u32_type = ffi::param::make_type(param_cls_t::int_, param_size_t::dword);
                auto f32_type = ffi::param::make_type(param_cls_t::float_, param_size_t::dword);
//                auto ctx_type = ffi::param::make_type(param_cls_t::ptr,
//                                                      param_size_t::qword,
//                                                      u8(scm::ffi_type_t::context));
                auto obj_type = ffi::param::make_type(param_cls_t::ptr,
                                                      param_size_t::qword,
                                                      u8(scm::ffi_type_t::object));
                auto list_type = ffi::param::make_type(param_cls_t::ptr,
                                                      param_size_t::qword,
                                                      u8(scm::ffi_type_t::list));
                auto slice_type = ffi::param::make_type(param_cls_t::ptr,
                                                        param_size_t::qword,
                                                        u8(scm::ffi_type_t::string));

                {
                    auto proto = ffi::proto::make("cvar_ref"_ss);
                    auto ol    = ffi::overload::make("cvar_ref"_ss,
                                                     obj_type,
                                                     (u0*) cvar_ref);
                    ffi::overload::append(ol, ffi::param::make("id"_ss, u32_type));
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "cvar-ref"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto id_param  = ffi::param::make("id"_ss, u32_type);

                    auto proto = ffi::proto::make("cvar_set"_ss);
                    auto ol_flag   = ffi::overload::make("cvar_set_flag"_ss,
                                                         b8_type,
                                                         (u0*) cvar_set_flag);
                    ffi::overload::append(ol_flag, id_param);
                    ffi::overload::append(ol_flag, ffi::param::make("value"_ss, b8_type));
                    ffi::proto::append(proto, ol_flag);

                    auto ol_number = ffi::overload::make("cvar_set_number"_ss,
                                                         b8_type,
                                                         (u0*) cvar_set_number);
                    ffi::overload::append(ol_number, id_param);
                    ffi::overload::append(ol_number, ffi::param::make("value"_ss, f32_type));
                    ffi::proto::append(proto, ol_number);

                    auto ol_integer = ffi::overload::make("cvar_set_integer"_ss,
                                                          b8_type,
                                                          (u0*) cvar_set_integer);
                    ffi::overload::append(ol_integer, id_param);
                    ffi::overload::append(ol_integer, ffi::param::make("value"_ss, u32_type));
                    ffi::proto::append(proto, ol_integer);

                    auto ol_string = ffi::overload::make("cvar_set_string"_ss,
                                                         b8_type,
                                                         (u0*) cvar_set_string);
                    ffi::overload::append(ol_string, id_param);
                    ffi::overload::append(ol_string, ffi::param::make("value"_ss, slice_type));
                    ffi::proto::append(proto, ol_string);

                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "cvar-set!"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto proto = ffi::proto::make("localized_string"_ss);
                    auto ol    = ffi::overload::make("localized_string"_ss,
                                                     u32_type,
                                                     (u0*) localized_string);
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
                    auto ol    = ffi::overload::make("localized_error"_ss,
                                                     u32_type,
                                                     (u0*) localized_error);
                    ffi::overload::append(ol, ffi::param::make("id"_ss, u32_type));
                    ffi::overload::append(ol, ffi::param::make("locale"_ss, slice_type));
                    ffi::overload::append(ol, ffi::param::make("code"_ss, slice_type));
                    ffi::overload::append(ol, ffi::param::make("str_id"_ss, u32_type));
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "localized-error"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto proto = ffi::proto::make("log_info"_ss);
                    auto ol    = ffi::overload::make("log_info"_ss,
                                                     u0_type,
                                                     (u0*) log_info);
                    ffi::overload::append(ol, ffi::param::make("fmt_msg"_ss, slice_type));
                    ffi::overload::append(ol, ffi::param::make("rest"_ss, list_type));
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "log-info"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto proto = ffi::proto::make("log_warn"_ss);
                    auto ol    = ffi::overload::make("log_warn"_ss,
                                                     u0_type,
                                                     (u0*) log_warn);
                    ffi::overload::append(ol, ffi::param::make("fmt_msg"_ss, slice_type));
                    ffi::overload::append(ol, ffi::param::make("rest"_ss, list_type));
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "log-warn"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto proto = ffi::proto::make("log_error"_ss);
                    auto ol    = ffi::overload::make("log_error"_ss,
                                                     u0_type,
                                                     (u0*) log_error);
                    ffi::overload::append(ol, ffi::param::make("fmt_msg"_ss, slice_type));
                    ffi::overload::append(ol, ffi::param::make("rest"_ss, list_type));
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "log-error"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto proto = ffi::proto::make("log_alert"_ss);
                    auto ol    = ffi::overload::make("log_alert"_ss,
                                                     u0_type,
                                                     (u0*) log_alert);
                    ffi::overload::append(ol, ffi::param::make("fmt_msg"_ss, slice_type));
                    ffi::overload::append(ol, ffi::param::make("rest"_ss, list_type));
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "log-alert"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto proto = ffi::proto::make("log_debug"_ss);
                    auto ol    = ffi::overload::make("log_debug"_ss,
                                                     u0_type,
                                                     (u0*) log_debug);
                    ffi::overload::append(ol, ffi::param::make("fmt_msg"_ss, slice_type));
                    ffi::overload::append(ol, ffi::param::make("rest"_ss, list_type));
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "log-debug"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto proto = ffi::proto::make("log_notice"_ss);
                    auto ol    = ffi::overload::make("log_notice"_ss,
                                                     u0_type,
                                                     (u0*) log_notice);
                    ffi::overload::append(ol, ffi::param::make("fmt_msg"_ss, slice_type));
                    ffi::overload::append(ol, ffi::param::make("rest"_ss, list_type));
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "log-notice"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto proto = ffi::proto::make("log_critical"_ss);
                    auto ol    = ffi::overload::make("log_critical"_ss,
                                                     u0_type,
                                                     (u0*) log_critical);
                    ffi::overload::append(ol, ffi::param::make("fmt_msg"_ss, slice_type));
                    ffi::overload::append(ol, ffi::param::make("rest"_ss, list_type));
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "log-critical"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto proto = ffi::proto::make("log_emergency"_ss);
                    auto ol    = ffi::overload::make("log_emergency"_ss,
                                                     u0_type,
                                                     (u0*) log_emergency);
                    ffi::overload::append(ol, ffi::param::make("fmt_msg"_ss, slice_type));
                    ffi::overload::append(ol, ffi::param::make("rest"_ss, list_type));
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "log-emergency"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto proto = ffi::proto::make("current_user"_ss);
                    auto ol    = ffi::overload::make("current_user"_ss,
                                                     obj_type,
                                                     (u0*) current_user);
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "current-user"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto proto = ffi::proto::make("current_alloc"_ss);
                    auto ol    = ffi::overload::make("current_alloc"_ss,
                                                     obj_type,
                                                     (u0*) current_alloc);
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "current-alloc"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto proto = ffi::proto::make("current_logger"_ss);
                    auto ol    = ffi::overload::make("current_logger"_ss,
                                                     obj_type,
                                                     (u0*) current_logger);
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "current-logger"),
                             scm::make_ffi(g_cfg_sys.ctx, proto));
                }

                {
                    auto proto = ffi::proto::make("current_command_line"_ss);
                    auto ol    = ffi::overload::make("current_command_line"_ss,
                                                     obj_type,
                                                     (u0*) current_command_line);
                    ffi::proto::append(proto, ol);
                    scm::set(g_cfg_sys.ctx,
                             scm::make_symbol(g_cfg_sys.ctx, "current-command-line"),
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

