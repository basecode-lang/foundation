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

#include <basecode/vm/bytecode.h>

namespace basecode::vm::bytecode {
    namespace size {
        static str::slice_t s_names[] = {
            "qw"_ss,
            "dw"_ss,
            "w"_ss,
            "b"_ss,
        };

        str::slice_t name(u8 size) {
            return s_names[size];
        }
    }

    namespace instructions {
        static str::slice_t s_names[] = {
        };

        str::slice_t name(u8 op) {
            return s_names[op];
        }
    }

    namespace register_file {
        static str::slice_t s_names[] = {
            "pc"_ss,
            "sp"_ss,
            "r1"_ss,
            "r2"_ss,
            "r3"_ss,
            "r4"_ss,
            "r5"_ss,
            "r6"_ss,
            "r7"_ss,
            "r8"_ss,
            "r9"_ss,
            "r10"_ss,
            "r11"_ss,
            "r12"_ss,
            "r13"_ss,
            "r14"_ss,
            "r15"_ss,
            "r16"_ss,
            "r17"_ss,
            "r18"_ss,
            "r19"_ss,
            "r20"_ss,
            "r21"_ss,
            "r22"_ss,
            "r23"_ss,
            "r24"_ss,
            "r25"_ss,
            "r26"_ss,
            "r27"_ss,
            "r28"_ss,
            "r29"_ss,
            "r30"_ss,
        };

        str::slice_t name(u8 reg) {
            return s_names[reg];
        }
    }
}
