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
#include <basecode/core/getopt.h>
#include <basecode/core/format.h>
#include <basecode/core/string.h>

namespace basecode::getopt {
    static const option_t* find_short_option(const getopt_t& opt, const s8 name) {
        if (name == 0)
            return nullptr;
        for (const auto& option : opt.opts) {
            if (option.short_name == name)
                return &option;
        }
        return nullptr;
    }

    static const option_t* find_long_option(const getopt_t& opt, str::slice_t name) {
        if (slice::empty(name))
            return nullptr;
        for (const auto& option : opt.opts) {
            if (option.long_name == name)
                return &option;
        }
        return nullptr;
    }

    static u0 add_arg(getopt_t& opt, u32 idx, const option_t* option, b8 extra = false) {
        auto& arg = array::append(opt.args);
        arg.option = option;
        arg.type   = option ? option->type : arg_type_t::string;
        arg.pos    = idx;
        arg.extra  = extra;
        switch (arg.type) {
            case arg_type_t::none:
                break;
            case arg_type_t::flag:
                break;
            case arg_type_t::file:
                break;
            case arg_type_t::string:
                break;
            case arg_type_t::integer:
                break;
            case arg_type_t::decimal:
                break;
        }
    }

    u0 free(getopt_t& opt) {
        array::free(opt.args);
        array::free(opt.opts);
        array::free(opt.extras);
    }

    status_t parse(getopt_t& opt) {
        if (array::empty(opt.opts))
            return status_t::unconfigured;
        str::slice_t args[opt.argc];
        for (u32 i = 0; i < opt.argc; ++i)
            args[i] = slice::make(opt.argv[i]);
        u32 i{};
        b8 extra_switch{};
        for (const auto& a : args) {
            if (extra_switch) {
                add_arg(opt, i, nullptr, true);
            } else {
                if (a == "--"_ss) {
                    extra_switch = true;
                } else if (a.length > 1) {
                    if (a[0] == '-') {
                        if (a[1] == '-') {
                            auto opt_name = slice::make(a.data + 2,
                                                        a.length - 2);
                            auto option = find_long_option(opt, opt_name);
                            if (!option)
                                return status_t::invalid_option;
                        } else {
                            u32 j{};
                            while (j < a.length) {
                                auto option = find_short_option(opt, a[j]);
                                if (!option)
                                    return status_t::invalid_option;
                                add_arg(opt, i, option, true);
                            }
                        }
                    }
                } else {
                }
            }
            ++i;
        }
        return status_t::ok;
    }

    status_t option_builder_t::build() {
        if (getopt::find_short_option(*_opt, _short_name)
            ||  getopt::find_long_option(*_opt, _long_name)) {
            return status_t::duplicate_option;
        }
        auto& option = array::append(_opt->opts);
        option.type         = _type;
        option.long_name    = !slice::empty(_long_name) ? string::interned::fold(_long_name) : str::slice_t{};
        option.short_name   = _short_name;
        option.max_allowed  = _max_allowed;
        option.min_required = _min_required;
        option.value_name = !slice::empty(_value_name) ? string::interned::fold(_value_name) : str::slice_t{};
        option.description  = !slice::empty(_description) ? string::interned::fold(_description) : str::slice_t{};
        if (_type == arg_type_t::file)
            _opt->file_option = &option;
        return status_t::ok;
    }

    u0 format_help(getopt_t& opt, str_t& buf) {
        path_t self{};
        path::init(self);
        path::set(self, opt.argv[0]);
        defer(path::free(self));

        const auto self_stem = path::stem(self);
        str_buf_t sb{&buf};
        format::format_to(sb, "usage: {}", self_stem);
        if (opt.opts.size > 0) {
            format::format_to(sb,
                              " [option{}]",
                              opt.opts.size > 1 ? "..." : "");
        }
        if (opt.file_option) {
            format::format_to(sb,
                              " [file]{}",
                              opt.file_option->min_required > 1
                                || opt.file_option->max_allowed > 0 ? "..." : "");
        }
        format::format_to(sb, "\n");
        if (!slice::empty(opt.description))
            format::format_to(sb, "{}\n", opt.description);
        format::format_to(sb, "\n");
        if (!array::empty(opt.opts)) {
            format::format_to(sb, "Options:\n");
            for (const auto& option : opt.opts) {
                const auto start = sb.size();
                if (option.short_name != 0 && !slice::empty(option.long_name)) {
                    format::format_to(sb,
                                      "  -{}, --{}",
                                      option.short_name,
                                      option.long_name);
                } else if (option.short_name != 0) {
                    format::format_to(sb,
                                      "  -{}",
                                      option.short_name);
                } else if (!slice::empty(option.long_name)) {
                    format::format_to(sb,
                                      "       --{}",
                                      option.long_name);
                }
                if (!slice::empty(option.value_name)) {
                    format::format_to(sb,
                                      "={}",
                                      option.value_name);
                }
                format::format_to(sb,
                                  "{:<{}}",
                                  " ",
                                  32 - (sb.size() - start));
                if (option.min_required > 0) {
                    format::format_to(sb, "required");
                    if (option.min_required > 1 || option.max_allowed > 0) {
                        format::format_to(sb,
                                          " (at least {}; up to {}) ",
                                          option.min_required,
                                          option.max_allowed);
                    } else if (option.max_allowed > 0) {
                        format::format_to(sb,
                                          " (up to {}) ",
                                          option.max_allowed);
                    }
                    format::format_to(sb, ": ");
                }
                if (!slice::empty(option.description)) {
                    format::format_to(sb,
                                      "{}",
                                      option.description);
                }
                format::format_to(sb, "\n");
            }
        }
    }

    option_builder_t make_option(getopt_t& opt) {
        return option_builder_t(&opt);
    }

    option_builder_t& option_builder_t::type(arg_type_t type) {
        _type = type;
        return *this;
    }

    option_builder_t& option_builder_t::max_allowed(u32 value) {
        _max_allowed = value;
        return *this;
    }

    option_builder_t& option_builder_t::min_required(u32 value) {
        _min_required = value;
        return *this;
    }

    option_builder_t& option_builder_t::short_name(s8 short_name) {
        _short_name = short_name;
        return *this;
    }

    u0 program_description(getopt_t& opt, str::slice_t description) {
        opt.description = slice::empty(description) ?
            str::slice_t{} :
            string::interned::fold(description);
    }

    option_builder_t& option_builder_t::long_name(str::slice_t long_name) {
        _long_name = long_name;
        return *this;
    }

    status_t init(getopt_t& opt, s32 argc, const s8** argv, alloc_t* alloc) {
        opt.alloc       = alloc;
        opt.argc        = argc;
        opt.argv        = argv;
        opt.file_option = {};
        array::init(opt.args, opt.alloc);
        array::init(opt.opts, opt.alloc);
        array::init(opt.extras, opt.alloc);
        return status_t::ok;
    }

    option_builder_t& option_builder_t::value_name(str::slice_t value_name) {
        _value_name = value_name;
        return *this;
    }

    option_builder_t& option_builder_t::description(str::slice_t description) {
        _description = description;
        return *this;
    }
}
