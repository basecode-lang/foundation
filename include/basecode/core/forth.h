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

#include <asmjit/asmjit.h>
#include <basecode/core/str.h>
#include <basecode/core/intern.h>
#include <basecode/core/stable_array.h>

namespace basecode {
    using word_id                   = u32;

    struct word_header_t final {
        u32                         intern_id;
        u8                          hidden:     1;
        u8                          immediate:  1;
        u8                          flag3:      1;
        u8                          flag4:      1;
        u8                          flag5:      1;
        u8                          flag6:      1;
        u8                          flag7:      1;
        u8                          flag8:      1;
    };

    using word_header_array_t       = stable_array_t<word_header_t>;

    enum class state_t : u8 {
        immediate,
        compile
    };

    struct forth_t final {
        alloc_t*                    alloc;
        u0*                         stack;
        u64*                        dsp;
        u64*                        rsp;
        u64*                        here;
        intern_t                    intern;
        str_t                       buf;
        word_header_array_t         words;
        word_id                     first;
        word_id                     latest;
        asmjit::JitRuntime          jit_rt;
        u32                         ret_stack_size;
        u32                         data_stack_size;
        state_t                     state;
    };

    namespace forth {
        enum class status_t : u8 {
            ok,
            error,
        };

        s32 key(forth_t& forth);

        u0 free(forth_t& forth);

        u0 tick(forth_t& forth);

        u32 word(forth_t& forth);

        u0 comma(forth_t& forth);

        u0 lbrac(forth_t& forth);

        u0 rbrac(forth_t& forth);

        u0 hidden(forth_t& forth);

        s64 number(forth_t& forth);

        u0 immediate(forth_t& forth);

        u0 emit(forth_t& forth, u8 ch);

        word_id create(forth_t& forth, u32 intern_id);

        word_header_t* find(forth_t& forth, u32 intern_id);

        status_t init(forth_t& forth, u32 data_stack_size = 2048, u32 return_stack_size = 2048, alloc_t* alloc = context::top()->alloc);
    }
}
