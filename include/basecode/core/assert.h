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

#include <basecode/core/format_fwd.h>

#define BC_ASSERT_MSG(cond, msg, ...)   SAFE_SCOPE(                                                                     \
	if (!(cond)) {                                                                                                      \
        assert_handler("Assertion Failure",                                                                             \
                       #cond,                                                                                           \
                       __FILE__,                                                                                        \
                       __LINE__,                                                                                        \
                       msg,                                                                                             \
                       ##__VA_ARGS__);                                                                                  \
		DEBUG_TRAP();                                                                                                   \
	})
#define BC_ASSERT(cond)         BC_ASSERT_MSG(cond, nullptr)
#define BC_ASSERT_NOT_NULL(ptr) BC_ASSERT_MSG((ptr) != NULL, #ptr " must not be nullptr")

namespace basecode {
    u0 format_assert(const s8* prefix,
                     const s8* condition,
                     const s8* file,
                     s32 line,
                     const s8* msg,
                     const fmt_args_t& args);

    template <typename... Args>
    u0 assert_handler(const s8* prefix,
                      const s8* condition,
                      const s8* file,
                      s32 line,
                      const s8* msg,
                      const Args& ... args) {
        format_assert(prefix,
                      condition,
                      file,
                      line,
                      msg,
                      fmt::make_format_args(args...));
    }
}
