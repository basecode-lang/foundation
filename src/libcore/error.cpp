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

#include <basecode/core/term.h>
#include <basecode/core/error.h>
#include <basecode/core/mutex.h>
#include <basecode/core/locale.h>
#include <basecode/core/string.h>
#include <basecode/core/hashtab.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode::error {
    using error_report_list_t   = array_t<error_report_t>;
    using localized_error_map_t = hashtab_t<locale_key_t, error_def_t*>;

    struct system_t final {
        alloc_t*                alloc;
        alloc_t*                error_slab;
        localized_error_map_t   errors;
        error_report_list_t     reports;
        term_t                  term;
        mutex_t                 lock;
    };

    system_t                    g_err_sys;

    static u0 format_report_header(const error_report_t& report,
                                   error_def_t* def,
                                   const s8* fmt_msg,
                                   str_buf_t& str_buf) {
        format::format_to(str_buf, "[{}] ", def->code);
        switch (report.level) {
            case error_report_level_t::warning:
                format::format_to(str_buf, "WARNING: ");
                break;
            case error_report_level_t::error:
                format::format_to(str_buf, "ERROR: ");
                break;
        }
        fmt::vformat_to(str_buf, fmt_msg, report.args);
        format::format_to(str_buf, "\n");
    }

    static u0 format_source_body(const error_report_t& report,
                                 const s8* fmt_msg,
                                 str_buf_t& str_buf) {
        s32 start_line = report.src_info.start.line - 4;
        if (start_line < 0)
            start_line = 0;
        s32 stop_line = report.src_info.end.line + 4;
        if (stop_line > report.buf->lines.size)
            stop_line = report.buf->lines.size;
        u32 start_pos = report.src_info.pos.start;
        for (s32 i = start_line; i < stop_line; ++i) {
            const auto line_number = i + 1;
            if (i >= report.src_info.start.line
            &&  i <= report.src_info.end.line) {
                const auto& line = report.buf->lines[i];
                format::format_to(str_buf, "{:8d}: ", line_number);

                const auto this_line_end = std::min(line.pos + line.len,
                                                    report.src_info.pos.end);
                term::colorize_range(g_err_sys.term,
                                     str_buf,
                                     term::color_t::yellow,
                                     term::color_t::blue,
                                     report.buf,
                                     i,
                                     start_pos,
                                     this_line_end);
                start_pos += line.len + 1;

                if (line_number > report.src_info.end.line) {
                    term::set_fg(g_err_sys.term, term::color_t::red);
                    term::refresh(g_err_sys.term, str_buf);
                    format::format_to(str_buf,
                                      "\n{:<{}}^ ",
                                      " ",
                                      10 + report.src_info.start.column);
                    fmt::vformat_to(str_buf, fmt_msg, report.args);
                    term::reset_all(g_err_sys.term);
                    term::refresh(g_err_sys.term, str_buf);
                }
            } else {
                const auto& line = buf::line(*report.buf, i);
                format::format_to(str_buf,
                                  "{:8d}: {}",
                                  line_number,
                                  line);
            }
            format::format_to(str_buf, "\n");
        }
    }

    namespace system {
        u0 fini() {
            array::free(g_err_sys.reports);
            hashtab::free(g_err_sys.errors);
            memory::system::free(g_err_sys.error_slab);
            mutex::free(g_err_sys.lock);
            term::free(g_err_sys.term);
        }

        status_t init(alloc_t* alloc) {
            g_err_sys.alloc = alloc;
            array::init(g_err_sys.reports, g_err_sys.alloc);
            hashtab::init(g_err_sys.errors, g_err_sys.alloc);
            mutex::init(g_err_sys.lock);
            slab_config_t slab_config{};
            slab_config.backing   = g_err_sys.alloc;
            slab_config.buf_size  = sizeof(error_def_t);
            slab_config.buf_align = alignof(error_def_t);
            slab_config.num_pages = 1;
            g_err_sys.error_slab = memory::system::make(alloc_type_t::slab, &slab_config);
            term::init(g_err_sys.term, g_err_sys.alloc);
            return status_t::ok;
        }
    }

    namespace report {
        u32 count() {
            scoped_lock_t lock(&g_err_sys.lock);
            return g_err_sys.reports.size;
        }

        b8 print(u32 id) {
            str_t buf{};
            str::init(buf, g_err_sys.alloc);
            if (!format(buf, id))
                return false;
            std::fwrite(buf.data, 1, buf.length, stderr);
            return true;
        }

        error_report_t* append() {
            scoped_lock_t lock(&g_err_sys.lock);
            return &array::append(g_err_sys.reports);
        }

        b8 format(str_t& buf, u32 id) {
            scoped_lock_t lock(&g_err_sys.lock);
            if (id > g_err_sys.reports.size - 1)
                return false;

            auto& report = g_err_sys.reports[id];
            error_def_t* def{};
            if (!OK(localized::find(report.id, &def))) {
                format::print(stderr, "error def not found: {}\n", report.id);
                return false;
            }

            str::slice_t* fmt_msg{};
            if (!OK(string::localized::find(def->lc_str_id, &fmt_msg))) {
                format::print(stderr, "localized string not found: {}\n", def->lc_str_id);
                return false;
            }

            str_buf_t str_buf{&buf};
            switch (report.type) {
                case error_report_type_t::default_: {
                    format_report_header(report, def, (const s8*) fmt_msg->data, str_buf);
                    break;
                }
                case error_report_type_t::source: {
                    format_report_header(report, def, (const s8*) fmt_msg->data, str_buf);
                    format_source_body(report, (const s8*) fmt_msg->data, str_buf);
                    break;
                }
                default:
                    break;
            }

            return true;
        }

        b8 print_range(u32 start_id, u32 end_id) {
            str_t buf{};
            str::init(buf, g_err_sys.alloc);
            if (!format_range(buf, start_id, end_id))
                return false;
            std::fwrite(buf.data, 1, buf.length, stderr);
            return true;
        }

        b8 format_range(str_t& buf, u32 start_id, u32 end_id) {
            scoped_lock_t lock(&g_err_sys.lock);
            if (start_id > g_err_sys.reports.size || end_id > g_err_sys.reports.size) {
                format::print(stderr, "range outside of bounds: {}-{}\n", start_id, end_id);
                return false;
            }

            if (start_id > end_id) {
                format::print(stderr, "start_id ({}) > end_if ({})\n", start_id, end_id);
                return false;
            }

            str_buf_t str_buf{&buf};
            for (u32 i = start_id; i < end_id; ++i) {
                auto& report = g_err_sys.reports[i];

                error_def_t* def{};
                if (!OK(localized::find(report.id, &def))) {
                    format::print(stderr, "error def not found: {}\n", report.id);
                    continue;
                }

                str::slice_t* fmt_msg{};
                if (!OK(string::localized::find(def->lc_str_id, &fmt_msg))) {
                    format::print(stderr, "localized string not found: {}\n", def->lc_str_id);
                    continue;
                }

                switch (report.type) {
                    case error_report_type_t::default_: {
                        format_report_header(report, def, (const s8*) fmt_msg->data, str_buf);
                        break;
                    }
                    case error_report_type_t::source: {
                        format_report_header(report, def, (const s8*) fmt_msg->data, str_buf);
                        format_source_body(report, (const s8*) fmt_msg->data, str_buf);
                        break;
                    }
                    default:
                        break;
                }
            }

            return true;
        }
    }

    namespace localized {
        status_t find(u32 id, error_def_t** def, const s8* locale, s32 len) {
            *def = nullptr;
            if (!locale) {
                *def = hashtab::find(g_err_sys.errors, locale::make_key(id));
            } else {
                const auto lc = slice::make(locale, len == -1 ? strlen(locale) : len);
                *def = hashtab::find(g_err_sys.errors, locale::make_key(id, lc));
            }
            return !*def ? status_t::localized_not_found : status_t::ok;
        }

        status_t add(u32 id, u32 str_id, const s8* locale, u32 locale_len, const s8* code, u32 code_len) {
            auto lc    = slice::make(locale, locale_len);
            auto key   = locale::make_key(id, lc);
            auto slice = hashtab::find(g_err_sys.errors, key);
            if (slice)
                return status_t::localized_dup_key;
            auto def = (error_def_t*) memory::alloc(g_err_sys.error_slab);
            def->code      = string::interned::fold(slice::make(code, code_len));
            def->locale    = lc;
            def->id        = id;
            def->lc_str_id = str_id;
            hashtab::insert(g_err_sys.errors, key, def);
            return status_t::ok;
        }
    }
}
