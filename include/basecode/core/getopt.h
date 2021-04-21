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

#pragma once

#include <basecode/core/str.h>
#include <basecode/core/array.h>

namespace basecode {
    struct option_t final {
        str::slice_t            long_name;
        str::slice_t            value_name;
        str::slice_t            description;
        s32                     max_allowed;
        s32                     min_required;
        u8                      radix;
        s8                      short_name;
        arg_type_t              type;
    };

    union arg_subclass_t final {
        b8                      flag;
        str::slice_t            string;
        s64                     integer;
        f64                     decimal;
    };

    struct arg_t final {
        option_t*               option;
        arg_subclass_t          subclass;
        u32                     pos;
        arg_type_t              type;
        b8                      extra;
    };

    struct getopt_t final {
        alloc_t*                alloc;
        const s8**              argv;
        option_t*               file_option;
        str::slice_t            description;
        arg_array_t             args;
        option_array_t          opts;
        arg_array_t             extras;
        s32                     argc;
    };

    namespace getopt {
        class option_builder_t final {
            getopt_t*               _opt;
            str::slice_t            _long_name      {};
            str::slice_t            _value_name     {};
            str::slice_t            _description    {};
            s32                     _min_required   {-1};
            s32                     _max_allowed    {-1};
            s8                      _short_name     {};
            u8                      _radix          {10};
            arg_type_t              _type           {};

        public:
            option_builder_t(getopt_t* opt) : _opt(opt) {}

            status_t build();

            option_builder_t& type(arg_type_t type);

            option_builder_t& max_allowed(u32 value);

            option_builder_t& min_required(u32 value);

            option_builder_t& short_name(s8 short_name);

            option_builder_t& radix(Radix_Concept auto value) {
                _radix = value;
                return *this;
            }

            option_builder_t& long_name(str::slice_t long_name);

            option_builder_t& value_name(str::slice_t value_name);

            option_builder_t& description(str::slice_t description);
        };

        u0 free(getopt_t& opt);

        u0 format(getopt_t& opt);

        status_t parse(getopt_t& opt);

        u0 format_help(getopt_t& opt, str_t& buf);

        option_builder_t make_option(getopt_t& opt);

        arg_t* find_arg(getopt_t& opt, s8 short_name);

        arg_t* find_arg(getopt_t& opt, str::slice_t long_name);

        status_t init(getopt_t& opt,
                      s32 argc,
                      const s8** argv,
                      alloc_t* alloc = context::top()->alloc.main);

        u0 program_description(getopt_t& opt, str::slice_t description);
    }
}

namespace basecode::hash {
    inline u64 hash64(option_t* const& key) {
        return murmur::hash64((const u0*) key, sizeof(u0*));
    }
}
