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

#include <basecode/core/bag.h>
#include <basecode/core/path.h>
#include <basecode/core/defer.h>
#include <basecode/core/queue.h>
#include <basecode/core/getopt.h>
#include <basecode/core/string.h>
#include <basecode/core/numbers.h>

namespace basecode::getopt {
    static arg_t* add_arg(getopt_t& opt,
                          u32 idx,
                          option_t* option,
                          b8 extra = false) {
        auto arg = extra ? &array::append(opt.extras) : &array::append(opt.args);
        arg->option = option;
        arg->type   = option ? option->type : arg_type_t::string;
        arg->pos    = idx;
        arg->extra  = extra;
        return arg;
    }

    static str::slice_t type_name(arg_type_t type) {
        switch (type) {
            case arg_type_t::none:          return "none"_ss;
            case arg_type_t::flag:          return "flag"_ss;
            case arg_type_t::file:          return "file"_ss;
            case arg_type_t::string:        return "string"_ss;
            case arg_type_t::integer:       return "integer"_ss;
            case arg_type_t::decimal:       return "decimal"_ss;
        }
    }

    static b8 arg_expects_value(const arg_t* arg) {
        return arg->type != arg_type_t::none && arg->type != arg_type_t::flag;
    }

    static status_t set_arg_value(arg_t* arg,
                                  str::slice_t value,
                                  bag_t<option_t*>& counts) {
        switch (arg->type) {
            case arg_type_t::file:
            case arg_type_t::string:
                arg->subclass.string = value;
                break;
            case arg_type_t::integer: {
                auto status = numbers::integer::parse(value,
                                                      arg->option->radix,
                                                      arg->subclass.integer);
                if (!OK(status))
                    return status_t::integer_conversion_error;
                break;
            }
            case arg_type_t::decimal: {
                auto status = numbers::fp::parse(value, arg->subclass.decimal);
                if (!OK(status))
                    return status_t::integer_conversion_error;
                break;
            }
            default: {
                break;
            }
        }
        if (arg->option)
            bag::insert(counts, arg->option);
        return status_t::ok;
    }

    static option_t* find_short_option(getopt_t& opt, s8 name) {
        if (name == 0)
            return nullptr;
        for (auto& option : opt.opts) {
            if (option.short_name == name)
                return &option;
        }
        return nullptr;
    }

    static u0 format_arg(const arg_t& arg, str_buf_t& sb, u32 idx) {
        format::format_to(sb, "{:02}: ", idx);
        if (arg.option) {
            if (arg.option->short_name != 0)
                format::format_to(sb, "{} ", arg.option->short_name);
            else
                format::format_to(sb, "  ");
            if (!slice::empty(arg.option->long_name))
                format::format_to(sb, "{:<16}", arg.option->long_name);
            else
                format::format_to(sb, "{:{}}", " ", 16);
            format::format_to(sb, "{:<8}", type_name(arg.type));
            format::format_to(sb, "{:>3} ", arg.option->min_required);
            format::format_to(sb, "{:>3} ", arg.option->max_allowed);
        } else {
            format::format_to(sb,
                              "- {:<16}{:<8}{:>3} {:>3} ",
                              "-",
                              type_name(arg.type),
                              -1,
                              -1);
        }
        if (arg_expects_value(&arg)) {
            switch (arg.type) {
                case arg_type_t::file:
                case arg_type_t::string:
                    format::format_to(sb, "{}", arg.subclass.string);
                    break;
                case arg_type_t::integer:
                    format::format_to(sb, "{:>17}", arg.subclass.integer);
                    break;
                case arg_type_t::decimal:
                    format::format_to(sb, "{:>17}", arg.subclass.decimal);
                    break;
                default:
                    break;
            }
        }
        format::format_to(sb, "\n");
    }

    static option_t* find_long_option(getopt_t& opt, str::slice_t name) {
        if (slice::empty(name))
            return nullptr;
        for (auto& option : opt.opts) {
            if (slice::empty(option.long_name))
                continue;
            if (strncmp((const s8*) name.data,
                        (const s8*) option.long_name.data,
                        option.long_name.length) == 0) {
                return &option;
            }
        }
        return nullptr;
    }

    u0 free(getopt_t& opt) {
        array::free(opt.args);
        array::free(opt.opts);
        array::free(opt.extras);
    }

    u0 format(getopt_t& opt) {
        str_t buf{};
        str::init(buf, opt.alloc);
        {
            str_buf_t sb{&buf};
            format::format_to(sb,
                              "Idx S Long            Type    MR  MA  Value\n");
            format::format_to(sb, "-------------------------------------------------------\n");
            u32 idx{};
            for (const auto& arg : opt.args) {
                format_arg(arg, sb, idx);
                ++idx;
            }
            format::format_to(sb, "\nExtra Argument\n");
            format::format_to(sb, "-------------------------------------------------------\n");
            for (const auto& arg : opt.extras) {
                format_arg(arg, sb, idx);
                ++idx;
            }
        }
        format::print("{}\n", buf);
    }

    status_t parse(getopt_t& opt) {
        if (array::empty(opt.opts))
            return status_t::unconfigured;

        str::slice_t args[opt.argc];
        for (u32 i = 0; i < opt.argc; ++i)
            args[i] = slice::make(opt.argv[i]);
        array::reserve(opt.args, opt.argc * 2);

        bag_t<option_t*> counts{};
        bag::init(counts, opt.alloc);

        queue_t<arg_t*> deferred{};
        queue::init(deferred, opt.alloc);

        defer(
            queue::free(deferred);
            bag::free(counts));

        auto present_new_arg = [&](const option_t* option,
                                   arg_t* arg,
                                   str::slice_t curr,
                                   u32& i,
                                   u32& j) -> status_t {
            if (!arg_expects_value(arg))
                return status_t::ok;

            if (j + 1 >= curr.length) {
                if (i >= opt.argc)
                    return status_t::expected_value;
                return set_arg_value(arg, args[++i], counts);
            }

            for (auto k = j + 1; k < curr.length; ++k) {
                if (!(curr[k] == '='))
                    continue;
                const auto len = curr.length - (k + 1);
                auto status = set_arg_value(
                    arg,
                    slice::make(curr.data + (k + 1), len),
                    counts);
                if (!OK(status))
                    return status;
                j += len;
                return status_t::ok;
            }

            queue::push_back(deferred, arg);
            return status_t::ok;
        };

        b8 extra_switch{};
        for (u32 i = 1; i < opt.argc; ++i) {
            const auto& curr = args[i];
            if (extra_switch) {
                auto arg = add_arg(opt, i, nullptr, true);
                auto status = set_arg_value(arg, curr, counts);
                if (!OK(status))
                    return status;
            } else {
                if (curr.length == 0) {
                    return status_t::invalid_argument;
                } else if (curr.length == 1) {
                    if (curr[0] == '-')
                        return status_t::invalid_argument;
                    if (queue::empty(deferred))
                        return status_t::invalid_argument;
                    auto status = set_arg_value(queue::pop_front(deferred),
                                                curr,
                                                counts);
                    if (!OK(status))
                        return status;
                    continue;
                } else if (curr == "--"_ss) {
                    extra_switch = true;
                    continue;
                } else if (curr[0] == '-') {
                    if (curr[1] == '-') {
                        const auto len = curr.length - 2;
                        auto opt_name = slice::make(curr.data + 2, len);
                        auto option = find_long_option(opt, opt_name);
                        if (!option)
                            return status_t::invalid_option;
                        u32 j{};
                        auto status = present_new_arg(
                            option,
                            add_arg(opt, i, option),
                            curr,
                            i,
                            j);
                        if (!OK(status))
                            return status;
                    } else {
                        for (u32 j = 1; j < curr.length; ++j) {
                            auto option = find_short_option(opt, curr[j]);
                            if (!option)
                                return status_t::invalid_option;
                            auto status = present_new_arg(
                                option,
                                add_arg(opt, i, option),
                                curr,
                                i,
                                j);
                            if (!OK(status))
                                return status;
                        }
                    }
                } else {
                    if (queue::empty(deferred))
                        return status_t::invalid_argument;
                    auto status = set_arg_value(queue::pop_front(deferred),
                                                curr,
                                                counts);
                    if (!OK(status))
                        return status;
                }
            }
        }

        for (auto& option : opt.opts) {
            if (option.min_required < 1)
                continue;
            auto count = bag::count(counts, &option);
            if (count != option.min_required)
                return status_t::missing_required_option;
            if (option.max_allowed > 0
            &&  count > option.max_allowed) {
                return status_t::count_exceeds_allowed;
            }
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
        option.radix        = _radix;
        option.long_name    = !slice::empty(_long_name) ?
            string::interned::fold(_long_name) :
            str::slice_t{};
        option.short_name   = _short_name;
        option.max_allowed  = _max_allowed;
        option.min_required = _min_required;
        option.value_name = !slice::empty(_value_name) ?
            string::interned::fold(_value_name) :
            str::slice_t{};
        option.description  = !slice::empty(_description) ?
            string::interned::fold(_description) :
            str::slice_t{};
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

    arg_t* find_arg(getopt_t& opt, s8 short_name) {
        for (auto& arg : opt.args) {
            if (arg.option && arg.option->short_name == short_name)
                return &arg;
        }
        return nullptr;
    }

    arg_t* find_arg(getopt_t& opt, str::slice_t long_name) {
        for (auto& arg : opt.args) {
            if (arg.option && arg.option->long_name == long_name)
                return &arg;
        }
        return nullptr;
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
