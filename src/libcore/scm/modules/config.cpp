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
#include <basecode/core/hashtab.h>
#include <basecode/core/scm/types.h>
#include <basecode/core/scm/system.h>
#include <basecode/core/scm/modules/config.h>

namespace basecode::config {
    using var_table_t           = hashtab_t<str::slice_t, cvar_t>;

    struct system_t final {
        alloc_t*                alloc;
        scm::ctx_t*             ctx;
        var_table_t             vartab;
        scm::chained_handler_t  handlers;
        u32                     var_id;
    };

    system_t                    g_cfg_sys;

    static b8 localized_error(u32 id,
                              str::slice_t* locale,
                              str::slice_t* code,
                              u32 str_id) {
        return OK(error::localized::add(id, str_id, *locale, *code));
    }

    static u32 localized_string(u32 id,
                                str::slice_t* locale,
                                str::slice_t* value) {
        string::localized::add(id, *locale, *value);
        return id;
    }

    namespace system {
        namespace exports {
            using namespace scm::kernel;

            [[maybe_unused]] static proc_export_t s_exports[] = {
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

                {str::slice_t{}},
            };
        }

        u0 fini() {
            hashtab::free(g_cfg_sys.vartab);
        }

        scm::ctx_t* context() {
            return g_cfg_sys.ctx;
        }

        static b8 set_cvar(scm::ctx_t* ctx,
                           str::slice_t name,
                           scm::obj_t* value,
                           scm::obj_t* env) {
            UNUSED(env);

            if (name.length < 3
            ||  name[0] != '*'
            ||  name[name.length - 1] != '*') {
                return false;
            }

            cvar_t* var{};
            if (!OK(cvar::get(name, &var))) {
                scm::error(ctx,
                           "[config] cvar '{}' is undefined",
                           name);
            }

            cvar::set(var, value);

            return true;
        }

        static b8 define_cvar(scm::ctx_t* ctx,
                              str::slice_t name,
                              scm::obj_t* value,
                              scm::obj_t* env) {
            UNUSED(env);

            if (name.length < 3
            ||  name[0] != '*'
            ||  name[name.length - 1] != '*') {
                return false;
            }

            cvar_t* var{};
            if (OK(cvar::get(name, &var))) {
                scm::error(ctx,
                           "[config] cvar '{}' is already defined",
                           name);
            }

            cvar_type_t type{};

            switch (TYPE(value)) {
                case scm::obj_type_t::nil:
                case scm::obj_type_t::free:
                    scm::error(ctx,
                               "[config] cannot define cvar '{}' with nil object",
                               name);
                case scm::obj_type_t::ffi:
                case scm::obj_type_t::ptr:
                case scm::obj_type_t::pair:
                case scm::obj_type_t::prim:
                case scm::obj_type_t::port:
                case scm::obj_type_t::proc:
                case scm::obj_type_t::cfunc:
                case scm::obj_type_t::error:
                case scm::obj_type_t::lambda:
                case scm::obj_type_t::environment:
                    type = cvar_type_t::pointer;
                    break;
                case scm::obj_type_t::fixnum:
                    type = cvar_type_t::integer;
                    break;
                case scm::obj_type_t::flonum:
                    type = cvar_type_t::real;
                    break;
                case scm::obj_type_t::symbol:
                case scm::obj_type_t::string:
                case scm::obj_type_t::keyword:
                    type = cvar_type_t::string;
                    break;
                case scm::obj_type_t::boolean:
                    type = cvar_type_t::flag;
                    break;
            }

            if (!OK(cvar::add(name, type, &var))) {
                scm::error(ctx,
                           "[config] unable to define cvar '{}'",
                           name);
            }

            cvar::set(var, value);

            return true;
        }

        static scm::obj_t* get_cvar(scm::ctx_t* ctx,
                                    str::slice_t name,
                                    scm::obj_t* env) {
            UNUSED(env);

            if (name.length < 3
            ||  name[0] != '*'
            ||  name[name.length - 1] != '*') {
                return nullptr;
            }

            cvar_t* var{};
            if (!OK(cvar::get(name, &var))) {
                scm::error(ctx,
                           "[config] unable to find cvar '{}'",
                           name);
            }

            switch (var->type) {
                case cvar_type_t::flag:
                    return scm::make_bool(ctx, var->value.flag);
                case cvar_type_t::real:
                    return scm::make_flonum(ctx, var->value.real);
                case cvar_type_t::integer:
                    return scm::make_fixnum(ctx, var->value.integer);
                case cvar_type_t::string: {
                    auto s = string::interned::get_slice(var->value.integer);
                    return scm::make_string(ctx, *s);
                }
                case cvar_type_t::pointer:
                    return scm::make_user_ptr(ctx, (u0*) var->value.ptr);
                default:
                    scm::error(ctx, "[config] invalid cvar type");
            }

            return ctx->nil;
        }

        status_t init(const config_settings_t& settings, alloc_t* alloc) {
            g_cfg_sys.alloc = alloc;
            g_cfg_sys.ctx   = settings.ctx ? settings.ctx : scm::system::global_ctx();

            scm::kernel::create_exports(g_cfg_sys.ctx, exports::s_exports);
            g_cfg_sys.handlers = {
                .get = get_cvar,
                .set = set_cvar,
                .define = define_cvar,
                .get_enabled = true,
                .set_enabled = true,
                .define_enabled = true,
            };
            scm::set_next_handler(g_cfg_sys.ctx, &g_cfg_sys.handlers);

            hashtab::init(g_cfg_sys.vartab, g_cfg_sys.alloc);
            g_cfg_sys.var_id = 1;

            cvar_t* var{};
            auto status = config::cvar::add("*test-runner*"_ss,
                                            cvar_type_t::flag,
                                            &var);
            if (!OK(status))
                return status;
            cvar::set(var, settings.test_runner);

            status = config::cvar::add("*build-type*"_ss,
                                       cvar_type_t::string,
                                       &var);
            if (!OK(status))
                return status;
            cvar::set(var, settings.build_type);

            status = config::cvar::add("*platform*"_ss,
                                       cvar_type_t::string,
                                       &var);
            if (!OK(status))
                return status;
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
            cvar::set(var, platform);

            status = config::cvar::add("*product-name*"_ss,
                                       cvar_type_t::string,
                                       &var);
            if (!OK(status))
                return status;
            cvar::set(var, settings.product_name);

            status = config::cvar::add("*version-major*"_ss,
                                       cvar_type_t::integer,
                                       &var);
            if (!OK(status))
                return status;
            cvar::set(var, settings.version.major);

            status = config::cvar::add("*version-minor*"_ss,
                                       cvar_type_t::integer,
                                       &var);
            if (!OK(status))
                return status;
            cvar::set(var, settings.version.minor);

            status = config::cvar::add("*version-revision*"_ss,
                                       cvar_type_t::integer,
                                       &var);
            if (!OK(status))
                return status;
            cvar::set(var, settings.version.revision);

            return config::status_t::ok;
        }
    }

    namespace cvar {
        u0 clear() {
            scm::collect_garbage(g_cfg_sys.ctx);
        }

        status_t remove(str::slice_t name) {
            hashtab::remove(g_cfg_sys.vartab, name);
            return status_t::ok;
        }

        u0 set(cvar_t* var, scm::obj_t* value) {
            auto ctx = g_cfg_sys.ctx;
            switch (var->type) {
                case cvar_type_t::flag:
                    var->value.flag = IS_TRUE(value);
                    break;
                case cvar_type_t::real:
                    var->value.real = FLONUM(value);
                    break;
                case cvar_type_t::integer:
                    var->value.integer = FIXNUM(value);
                    break;
                case cvar_type_t::string:
                    var->value.integer = STRING_ID(value);
                    break;
                case cvar_type_t::pointer:
                    var->value.ptr = (u0*) value;
                    break;
                default:
                    scm::error(ctx, "[config] invalid cvar type");
            }
        }

        status_t get(str::slice_t name, cvar_t** var) {
            *var = hashtab::find(g_cfg_sys.vartab, name);
            if (!*var)
                return status_t::cvar_not_found;
            return status_t::ok;
        }

        status_t add(str::slice_t name, cvar_type_t type, cvar_t** var) {
            auto cvar = hashtab::emplace(g_cfg_sys.vartab, name);
            cvar->id        = g_cfg_sys.var_id++;
            cvar->name      = string::interned::fold(name);
            cvar->type      = type;
            cvar->value.ptr = {};
            *var = cvar;
            return status_t::ok;
        }
    }
}

