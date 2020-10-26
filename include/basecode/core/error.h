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

#pragma once

#include <basecode/core/buf.h>
#include <basecode/core/format.h>
#include <basecode/core/src_loc.h>

namespace basecode {
    struct error_def_t final {
        str::slice_t            code;
        str::slice_t            locale;
        u32                     id;
        u32                     lc_str_id;
    };

    constexpr u32 max_report_args_count = 4;

    enum class error_report_level_t : u8 {
        warning,
        error,
    };

    enum class error_report_type_t : u8 {
        default_,
        source,
    };

    struct error_report_t final {
        buf_t*                  buf;
        fmt_arg_t               args[max_report_args_count];
        time_t                  ts;
        source_info_t           src_info;
        u32                     id;
        u32                     args_size;
        error_report_type_t     type;
        error_report_level_t    level;
    };

    namespace error {
        enum class status_t : u8 {
            ok                  = 0,
            localized_dup_key,
            localized_not_found,
        };

        namespace system {
            u0 fini();

            status_t init(alloc_t* alloc = context::top()->alloc);
        }

        namespace report {
            namespace internal {
                template <typename T>
                u0 add_arg(error_report_t* report, const T& arg) {
                    if (report->args_size == max_report_args_count - 1) return;
                    report->args[report->args_size++] = fmt::detail::make_arg<fmt_ctx_t>(arg);
                }
            }

            u32 count();

            b8 print(u32 id);

            error_report_t* append();

            b8 format(str_t& buf, u32 id);

            b8 print_range(u32 start_id, u32 end_id);

            b8 format_range(str_t& buf, u32 start_id, u32 end_id);

            template <typename ...Args>
            u0 add(u32 id, error_report_level_t level, Args&&... args) {
                auto report = append();
                report->type      = error_report_type_t::default_;
                report->level     = level;
                report->buf       = {};
                report->src_info  = {};
                report->id        = id;
                report->ts        = std::time(nullptr);
                report->args_size = 0;
                (internal::add_arg(report, args), ...);
            }

            template <typename ...Args>
            u0 add_src(u32 id, error_report_level_t level, buf_t* buf, const source_info_t& src_info, Args&&... args) {
                auto report = append();
                report->type      = error_report_type_t::source;
                report->level     = level;
                report->buf       = buf;
                report->src_info  = src_info;
                report->id        = id;
                report->ts        = std::time(nullptr);
                report->args_size = 0;
                (internal::add_arg(report, args), ...);
            }
        }

        namespace localized {
            status_t find(u32 id, error_def_t** def, const s8* locale = {}, s32 len = -1);

            status_t find(u32 id, error_def_t** def, const String_Concept auto& locale = {}) {
                if (locale.length == 0)
                    return find(id, def);
                else
                    return find(id, def, (const s8*) locale.data, locale.length);
            }

            status_t add(u32 id, u32 str_id, const s8* locale, u32 locale_len, const s8* code, u32 code_len);

            status_t add(u32 id, u32 str_id, const String_Concept auto& locale, const String_Concept auto& code) {
                return add(id, str_id, (const s8*) locale.data, locale.length, (const s8*) code.data, code.length);
            }
        }
    }
}

