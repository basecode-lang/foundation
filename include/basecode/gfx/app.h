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

#include <basecode/gfx/gfx.h>
#include <basecode/core/str.h>
#include <basecode/core/path.h>

namespace basecode::gfx::app {
    namespace texture_atlas {
        u0 free(texture_atlas_t& atlas);

        u0 init(texture_atlas_t& atlas, alloc_t* alloc);

        status_t make_gpu_texture(texture_atlas_t& atlas);

        status_t load_bitmap(texture_atlas_t& atlas, const path_t& path);
    }

    u0 free(app_t& app);

    status_t run(app_t& app);

    status_t save_config(app_t& app);

    status_t load_config(app_t& app);

    texture_atlas_t* make_texture_atlas(app_t& app);

    status_t init(app_t& app, alloc_t* alloc = context::top()->alloc.main);
}
