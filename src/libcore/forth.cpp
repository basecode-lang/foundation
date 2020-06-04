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

#include <basecode/core/forth.h>

namespace basecode::forth {
    s32 key(forth_t& forth) {
        UNUSED(forth);
        return 0;
    }

    u0 free(forth_t& forth) {
        stable_array::free(forth.words);
        intern::free(forth.intern);
        str::free(forth.buf);
        memory::free(forth.alloc, forth.stack);
    }

    u0 tick(forth_t& forth) {
        UNUSED(forth);
    }

    u32 word(forth_t& forth) {
        UNUSED(forth);
        return 0;
    }

    u0 comma(forth_t& forth) {
        UNUSED(forth);
    }

    u0 lbrac(forth_t& forth) {
        forth.state = state_t::immediate;
    }

    u0 rbrac(forth_t& forth) {
        forth.state = state_t::compile;
    }

    u0 hidden(forth_t& forth) {
        UNUSED(forth);
    }

    s64 number(forth_t& forth) {
        UNUSED(forth);
        return 0;
    }

    u0 immediate(forth_t& forth) {
        UNUSED(forth);
    }

    u0 emit(forth_t& forth, u8 ch) {
        UNUSED(forth);
        UNUSED(ch);
    }

    word_id create(forth_t& forth, u32 intern_id) {
        auto& word = stable_array::append(forth.words);
        word.intern_id  = intern_id;
        word.hidden     = false;
        word.immediate  = false;
        word.flag3      = false;
        word.flag4      = false;
        word.flag5      = false;
        word.flag6      = false;
        word.flag7      = false;
        word.flag8      = false;
        forth.latest    = forth.words.size;
        if (!forth.first)
            forth.first = forth.latest;
        return forth.latest;
    }

    word_header_t* find(forth_t& forth, u32 intern_id) {
        UNUSED(forth);
        UNUSED(intern_id);
        return nullptr;
    }

    status_t init(forth_t& forth, u32 data_stack_size, u32 return_stack_size, alloc_t* alloc) {
        u32 adjust{};
        forth.alloc           = alloc;
        forth.here            = {};
        forth.first           = forth.latest = 0;
        forth.data_stack_size = data_stack_size;
        forth.ret_stack_size  = return_stack_size;
        forth.stack           = memory::alloc(forth.alloc, (forth.data_stack_size + forth.ret_stack_size) * sizeof(u64), alignof(u64));
        forth.dsp             = (u64*) forth.stack + forth.data_stack_size;
        forth.rsp             = (u64*) memory::system::align_forward(forth.dsp + forth.ret_stack_size, alignof(u64), adjust);
        str::init(forth.buf, forth.alloc);
        str::reserve(forth.buf, 64);
        intern::init(forth.intern, forth.alloc);
        stable_array::init(forth.words, forth.alloc);
        return status_t::ok;
    }
}
