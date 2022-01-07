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

#include <basecode/core/variant.h>

namespace basecode::error {
    namespace system {
        u0 fini();

        status_t init(alloc_t* alloc = context::top()->alloc.main);
    }

    namespace report {
        namespace internal {
            template <typename T>
            u0 add_arg(error_report_t* report, const T& arg) {
                variant::append(report->args, arg);
            }
        }

        u32 count();

        inline b8 ok() {
            return count() == 0;
        }

        b8 print(u32 id);

        error_report_t* append();

        template <typename ...Args>
        u0 add_src(Error_Id auto id,
                   error_report_level_t level,
                   buf_t* buf,
                   const source_info_t& src_info,
                   Args&&... args) {
            auto report = append();
            report->ts        = std::time(nullptr);
            report->id        = u32(id);
            report->buf       = buf;
            report->type      = error_report_type_t::source;
            report->level     = level;
            report->src_info  = src_info;
            (internal::add_arg(report, args), ...);
        }

        b8 format(str_t& buf, u32 id);

        b8 print_range(u32 start_id, u32 end_id);

        b8 format_range(str_t& buf, u32 start_id, u32 end_id);

        template <typename ...Args>
        u0 add(Error_Id auto id, error_report_level_t level, Args&&... args) {
            auto report = append();
            report->ts        = std::time(nullptr);
            report->id        = u32(id);
            report->buf       = {};
            report->type      = error_report_type_t::default_;
            report->level     = level;
            report->src_info  = {};
            (internal::add_arg(report, args), ...);
        }
    }

    namespace localized {
        status_t add(u32 id,
                     u32 str_id,
                     const s8* locale,
                     u32 locale_len,
                     const s8* code,
                     u32 code_len);

        status_t add(u32 id,
                     u32 str_id,
                     const String_Concept auto& locale,
                     const String_Concept auto& code) {
            return add(id,
                       str_id,
                       (const s8*) locale.data,
                       locale.length,
                       (const s8*) code.data,
                       code.length);
        }

        status_t find(u32 id,
                      error_def_t** def,
                      const s8* locale = {},
                      s32 len = -1);

        status_t find(u32 id,
                      error_def_t** def,
                      const String_Concept auto& locale = {}) {
            if (locale.length == 0)
                return find(id, def);
            else
                return find(id, def, (const s8*) locale.data, locale.length);
        }
    }
}

