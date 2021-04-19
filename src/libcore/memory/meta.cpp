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

#include <basecode/core/hashtab.h>
#include <basecode/core/memory/meta.h>
#include <basecode/core/memory/system/slab.h>

namespace basecode::memory::meta {
    using alloc_info_table_t    = hashtab_t<u0*, alloc_info_t*>;

    struct system_t final {
        alloc_t*                alloc;
        alloc_t                 info_slab;
        alloc_info_array_t      infos;
        alloc_info_array_t      roots;
        alloc_info_table_t      infotab;
        b8                      init;
    };

    thread_local system_t       t_meta_sys{};

    namespace system {
        u0 fini() {
            t_meta_sys.init = {};
            for (auto info : t_meta_sys.infos)
                alloc_info::free(*info);
            array::free(t_meta_sys.infos);
            array::free(t_meta_sys.roots);
            hashtab::free(t_meta_sys.infotab);
            memory::fini(&t_meta_sys.info_slab);
        }

        u0 update(f32 dt) {
            for (auto info : t_meta_sys.infos) {
                if (info->mode == plot_mode_t::none)
                    continue;
                alloc_info::append_point(*info,
                                         dt,
                                         info->tracked->total_allocated);
            }
        }

        u0 init(alloc_t* alloc) {
            t_meta_sys.alloc = alloc;
            t_meta_sys.init  = true;
            array::init(t_meta_sys.roots, t_meta_sys.alloc);
            array::init(t_meta_sys.infos, t_meta_sys.alloc);
            hashtab::init(t_meta_sys.infotab, t_meta_sys.alloc);

            slab_config_t slab_config{};
            slab_config.backing   = t_meta_sys.alloc;
            slab_config.buf_size  = sizeof(alloc_info_t);
            slab_config.buf_align = alignof(alloc_info_t);
            slab_config.num_pages = DEFAULT_NUM_PAGES;
            memory::init(&t_meta_sys.info_slab,
                         alloc_type_t::slab,
                         &slab_config);

            track(alloc);
        }

        u0 track(alloc_t* alloc) {
            if (!t_meta_sys.init || !alloc)
                return;

            auto info = hashtab::find(t_meta_sys.infotab, (u0*) alloc);
            if (info)
                return;

            info = (alloc_info_t*) memory::alloc(&t_meta_sys.info_slab);
            alloc_info::init(*info, t_meta_sys.alloc);
            info->tracked = alloc;

            if (alloc->backing) {
                auto backing_info = hashtab::find(t_meta_sys.infotab,
                                                  (u0*) alloc->backing);
                if (backing_info) {
                    array::append(backing_info->children, info);
                    info->parent = backing_info;
                }
            } else {
                array::append(t_meta_sys.roots, info);
            }
            array::append(t_meta_sys.infos, info);
            hashtab::insert(t_meta_sys.infotab, (u0*) alloc, info);
        }

        u0 untrack(alloc_t* alloc) {
            if (!t_meta_sys.init || !alloc)
                return;

            auto info = hashtab::find(t_meta_sys.infotab, (u0*) alloc);
            if (!info)
                return;

            if (info->parent) {
                array::erase(info->parent->children, info);
                info->parent = {};
            } else {
                array::erase(t_meta_sys.roots, info);
            }

            array::erase(t_meta_sys.infos, info);
            memory::free(&t_meta_sys.info_slab, info);
            hashtab::remove(t_meta_sys.infotab, (u0*) alloc);
        }

        u0 stop_plot(alloc_info_t* info) {
            if (!info)
                return;
            alloc_info::stop_plot(*info);
        }

        const alloc_info_array_t& roots() {
            return t_meta_sys.roots;
        }

        u0 start_plot(alloc_info_t* info, plot_mode_t mode) {
            if (!info)
                return;
            alloc_info::start_plot(*info, mode);
        }
    }

    namespace alloc_info {
        u0 free(alloc_info_t& info) {
            info.tracked = {};
            stop_plot(info);
            array::free(info.children);
        }

        u0 stop_plot(alloc_info_t& info) {
            if (info.mode == plot_mode_t::none)
                return;
            switch (info.mode) {
                case plot_mode_t::rolled:
                    plot::rolled::free(info.plot.rolled);
                    break;
                case plot_mode_t::scrolled:
                    plot::scrolled::free(info.plot.scrolled);
                    break;
                default:
                    break;
            }
            info.mode = plot_mode_t::none;
       }

        u0 init(alloc_info_t& info, alloc_t* alloc) {
            info.mode    = plot_mode_t::none;
            info.alloc   = alloc;
            info.tracked = {};
            array::init(info.children, info.alloc);
        }

        u0 append_point(alloc_info_t& info, f32 x, f32 y) {
            if (info.mode == plot_mode_t::none)
                return;
            switch (info.mode) {
                case plot_mode_t::rolled:
                    info.plot.rolled.time += x;
                    plot::rolled::append_point(info.plot.rolled,
                                               info.plot.rolled.time,
                                               y);
                    break;
                case plot_mode_t::scrolled:
                    info.plot.scrolled.time += x;
                    plot::scrolled::append_point(info.plot.scrolled,
                                                 info.plot.scrolled.time,
                                                 y);
                    break;
                default:
                    break;
            }
        }

        u0 start_plot(alloc_info_t& info, plot_mode_t mode) {
            if (info.mode != plot_mode_t::none)
                return;
            info.mode = mode;
            switch (info.mode) {
                case plot_mode_t::rolled:
                    plot::rolled::init(info.plot.rolled,
                                       10.0f,
                                       2000,
                                       info.alloc);
                    break;
                case plot_mode_t::scrolled:
                    plot::scrolled::init(info.plot.scrolled,
                                         0,
                                         2000,
                                         info.alloc);
                    break;
                default:
                    break;
            }
        }
    }
}

