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

namespace basecode {
    struct source_loc_t final {
        u32                     line    {};
        u32                     column  {};
    };

    struct source_pos_t final {
        u32                     end     {};
        u32                     start   {};
    };

    struct source_info_t final {
        source_pos_t            pos;
        source_loc_t            start;
        source_loc_t            end;
    };
}

FORMAT_TYPE(basecode::source_info_t,
            format_to(ctx.out(),
                      "[S: {:>06}, E: {:>06}, [SL: {:>04}, SC: {:>03}, EL: {:>04}, EC: {:>04}]]",
                      data.pos.start,
                      data.pos.end,
                      data.start.line,
                      data.start.column,
                      data.end.line,
                      data.end.column));
