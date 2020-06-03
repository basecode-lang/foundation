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

#include <chibi/gc_heap.h>
#include <basecode/core/scm.h>
#include <basecode/core/memory/system/proxy.h>

namespace basecode::scm {
    struct system_t final {
        alloc_t*                    alloc;
        scm_t                       terp;
    };

    static system_t s_system;

    namespace system {
        status_t fini() {
            scm::free(s_system.terp);
            memory::system::free(s_system.alloc);
            return status_t::ok;
        }

        status_t init(alloc_t* alloc) {
            sexp_scheme_init();
            s_system.alloc = memory::proxy::make(alloc, "scm::sys"_ss);
            scm::init(s_system.terp, s_system.alloc);
            return scm::load_base(s_system.terp);
        }
    }

    u0 free(scm_t& scm) {
        sexp_destroy_context(scm.ctx);
        scm.parent = {};
        scm.ctx    = scm.env = {};
        scm.in     = scm.out = scm.err = {};
    }

    status_t load_base(scm_t& scm) {
        scm.env = sexp_load_standard_env(scm.ctx, sexp_context_env(scm.ctx), SEXP_SEVEN);
        return status_t::ok;
    }

    status_t write(scm_t& scm, sexp obj, sexp port) {
        sexp_gc_var1(tmp);
        sexp_gc_preserve1(scm.ctx, tmp);
        port = port ? port : scm.out;
        if (sexp_exceptionp(obj)) {
            sexp_print_exception(scm.ctx, obj, port);
        } else {
            tmp = sexp_env_bindings(scm.env);
            sexp_warn_undefs(scm.ctx, sexp_env_bindings(scm.env), tmp, obj);
            if (obj && sexp_exceptionp(obj)) {
                sexp_print_exception(scm.ctx, obj, port);
                if (obj != sexp_global(scm.ctx, SEXP_G_OOS_ERROR)) {
                    sexp_stack_trace(scm.ctx, port);
                }
            } else if (obj != SEXP_VOID) {
                sexp_write(scm.ctx, obj, port);
            }
        }
        sexp_gc_release1(scm.ctx);
        return status_t::ok;
    }

    status_t global_ref(scm_t& scm, sexp* obj, s32 slot) {
        *obj = sexp_global(scm.ctx, slot);
        return sexp_exceptionp(*obj) ? status_t::error : status_t::ok;
    }

    status_t parameter_ref(scm_t& scm, sexp* obj, sexp key) {
        *obj = sexp_parameter_ref(scm.ctx, key);
        return sexp_exceptionp(*obj) ? status_t::error : status_t::ok;
    }

    status_t load(scm_t& scm, sexp* obj, const path_t& path) {
        sexp_gc_var1(s);
        sexp_gc_preserve1(scm.ctx, s);
        s = sexp_c_string(scm.ctx, (const s8*) path.str.data, path.str.length);
        *obj = sexp_load(scm.ctx, s, nullptr);
        sexp_gc_release1(scm.ctx);
        return sexp_exceptionp(*obj) ? status_t::error : status_t::ok;
    }

    status_t parameter_set(scm_t& scm, sexp key, sexp value) {
        sexp_set_parameter(scm.ctx, scm.env, key, value);
        return status_t::ok;
    }

    status_t get_output_str(scm_t& scm, str_t& str, sexp port) {
        sexp_gc_var1(out_str);
        sexp_gc_preserve1(scm.ctx, out_str);
        if (sexp_buffered_flush(scm.ctx, port, true) != 0)
            return status_t::error;
        out_str = sexp_get_output_string(scm.ctx, port);
        sexp_cdr(sexp_port_cookie(port)) = SEXP_NULL;
        auto cstr = sexp_string_data(out_str);
        str::reset(str);
        str::append(str, cstr);
        sexp_gc_release1(scm.ctx);
        return status_t::ok;
    }

    status_t add_module_dir(scm_t& scm, const path_t& path, b8 append) {
        sexp_gc_var2(dir, res);
        sexp_gc_preserve2(scm.ctx, dir, res);
        dir = sexp_c_string(scm.ctx, (const s8*) path.str.data, path.str.length);
        res = sexp_add_module_directory(scm.ctx, dir, append ? SEXP_TRUE : SEXP_FALSE);
        sexp_gc_release2(scm.ctx);
        return sexp_exceptionp(res) ? status_t::error : status_t::ok;
    }

    status_t create_ports(scm_t& scm, sexp in, sexp out, sexp err, b8 no_close) {
        if (in) {
            scm.in = in;
            sexp_port_no_closep(in) = no_close;
            parameter_set(scm, sexp_global(scm.ctx, SEXP_G_CUR_IN_SYMBOL), in);
        }
        if (out) {
            sexp_port_no_closep(out) = no_close;
            parameter_set(scm, sexp_global(scm.ctx, SEXP_G_CUR_OUT_SYMBOL), out);
            scm.out = out;
        }
        if (err) {
            sexp_port_no_closep(err) = no_close;
            parameter_set(scm, sexp_global(scm.ctx, SEXP_G_CUR_ERR_SYMBOL), err);
            scm.err = err;
        }
        return status_t::ok;
    }

    status_t create_ports(scm_t& scm, FILE* in, FILE* out, FILE* err, b8 no_close) {
        sexp_load_standard_ports(scm.ctx, scm.env, in, out, err, no_close);
        if (in)  parameter_ref(scm, &scm.in,  sexp_global(scm.ctx, SEXP_G_CUR_IN_SYMBOL));
        if (out) parameter_ref(scm, &scm.out, sexp_global(scm.ctx, SEXP_G_CUR_OUT_SYMBOL));
        if (err) parameter_ref(scm, &scm.err, sexp_global(scm.ctx, SEXP_G_CUR_ERR_SYMBOL));
        return status_t::ok;
    }

    status_t init(scm_t& scm, alloc_t* alloc, u32 size, u32 max_size, scm_t* parent) {
        scm.alloc  = alloc;
        scm.parent = parent;
        scm.in     = scm.out = scm.err = {};
        scm.ctx    = sexp_make_eval_context(alloc, parent ? parent->ctx : nullptr, nullptr, nullptr, size, max_size);
        scm.env    = sexp_context_env(scm.ctx);
        return status_t::ok;
    }
}
