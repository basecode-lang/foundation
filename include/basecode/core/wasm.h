// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
// V I R T U A L  M A C H I N E  P R O J E C T
//
// Copyright (C) 2020 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <basecode/core/types.h>

namespace basecode {
    namespace wasm {
        namespace types {
            constexpr u8 i32        = 0x7f;
            constexpr u8 i64        = 0x7e;
            constexpr u8 f32        = 0x7d;
            constexpr u8 f64        = 0x7c;
            constexpr u8 ref        = 0x70;
            constexpr u8 ext_ref    = 0x6f;
        }

        namespace instructions {
            constexpr u8 unreachable    = 0x00;
            constexpr u8 nop            = 0x01;
            constexpr u8 block          = 0x02;
            constexpr u8 loop           = 0x03;
            constexpr u8 if_            = 0x04;
            constexpr u8 else_          = 0x05;
            constexpr u8 br             = 0x0c;
            constexpr u8 br_if          = 0x0d;
            constexpr u8 br_table       = 0x0e;
            constexpr u8 return_        = 0x0f;
            constexpr u8 call           = 0x10;
            constexpr u8 calli          = 0x11;
            constexpr u8 null           = 0xd0;
            constexpr u8 is_null        = 0xd1;
            constexpr u8 ref_func       = 0xd2;
            constexpr u8 drop           = 0x1a;
            constexpr u8 select         = 0x1b;
            constexpr u8 select_t       = 0x1c;
            constexpr u8 local_get      = 0x20;
            constexpr u8 local_set      = 0x21;
            constexpr u8 local_tee      = 0x22;
            constexpr u8 global_get     = 0x23;
            constexpr u8 global_set     = 0x24;
            constexpr u8 end            = 0x08;
            constexpr u8 block_type     = 0x40;
        }
    }
}
