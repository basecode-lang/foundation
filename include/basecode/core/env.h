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

#include <basecode/core/types.h>

namespace basecode::env {
    namespace system {
        u0 fini();

        env_t* get_root();

        env_t* get(str::slice_t name);

        status_t init(alloc_t* alloc = context::top()->alloc.main);

        env_t* make(str::slice_t name, env_t* parent = {}, s8** pairs = {});
    }

    env_value_t* set(env_t* env,
                     str::slice_t key,
                     const slice_array_t& value);

    status_t parse(env_t* env, s8** pairs);

    status_t load(env_t* env, const path_t& path);

    env_value_t* get(env_t* env, str::slice_t key);

    status_t expand(env_t* env, env_value_t* value, str_t& expanded);

    env_value_t* set(env_t* env, str::slice_t key, str::slice_t value);
}
