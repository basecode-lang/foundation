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

#include <basecode/core/str.h>
#include <basecode/core/types.h>

#define UOP(g, op, s, cc, t, e) (inst_encoding_t((g), (op), (s), (cc), (t), (e)).data)
#define MASK(t)                 (u16(1U << (t)))
#define TST_MASK(m, t)          ((m) & u16(1U << (t)))

namespace basecode {
    struct gp_register_t;

//    union operand_encoding_t final {
//        u8                      imm12:      1;  // 12
//        u8                      imm24:      1;  // 24
//        u8                      imm32:      1;  // 32
//        u8                      rd:         1;  // 6
//        u8                      rn:         1;  // 6
//        u8                      rm:         1;  // 6
//        u8                      ra:         1;  // 6
//        u8                      post_inc:   1;
//    };

    struct operand_data_t final {
        gp_register_t*          rd;
        gp_register_t*          rn;
        gp_register_t*          rm;
        gp_register_t*          ra;
        u32                     imm12;
        u32                     imm24;
        u32                     imm32;
    };

    union inst_encoding_t final {
        struct {
            u32                 group:      3;
            u32                 micro_op:   4;
            u32                 oper_size:  2;
            u32                 cond_code:  3;
            u32                 num_type:   3;
            u32                 encoding:   4;
            u32                 pad:        13;
        }                       fields;
        u32                     data;

        inst_encoding_t(u8 group,
                        u8 micro_op,
                        u8 oper_size,
                        u8 cond_code,
                        u8 num_type,
                        u8 encoding) : fields({group,
                                               micro_op,
                                               oper_size,
                                               cond_code,
                                               num_type,
                                               encoding,
                                               0}) {
        }
    };

    struct instruction_t final {
        u64                     group:      3;
        u64                     micro_op:   4;
        u64                     oper_size:  2;
        u64                     cond_code:  3;
        u64                     num_type:   3;
        u64                     encoding:   4;
        u64                     data:       45;
    };
    static_assert(sizeof(instruction_t) <= 8, "instruction_t is now greater than 8 bytes!");

    namespace vm {
        namespace bytecode {
            namespace size {
                constexpr u8 qword      = 0;
                constexpr u8 dword      = 1;
                constexpr u8 word       = 2;
                constexpr u8 byte       = 3;

                str::slice_t name(u8 size);
            }

            namespace alu {
                constexpr u8 add        = 0;
                constexpr u8 adc        = 1;
                constexpr u8 mul        = 2;
                constexpr u8 madd       = 3;
                constexpr u8 sbc        = 4;
                constexpr u8 sub        = 5;    // N.B. sub & cmp are the *same*
                constexpr u8 cmp        = 5;    //      micro-op in the vm (signed)
                constexpr u8 msub       = 6;
                constexpr u8 div        = 7;
                constexpr u8 pow        = 8;
                constexpr u8 mod        = 9;
                constexpr u8 neg        = 10;

                u16 valid_sizes(u8 op);

                str::slice_t name(u8 op);

                u16 valid_operands(u8 op);

                u16 valid_cond_codes(u8 op);

                u16 valid_number_types(u8 op);
            }

            namespace cond {
                constexpr u8 clz        = 0;
                constexpr u8 ctz        = 1;
                constexpr u8 bfi        = 2;
                constexpr u8 bfc        = 3;
                constexpr u8 set        = 4;
                constexpr u8 clr        = 5;
                constexpr u8 neg        = 6;
                constexpr u8 inv        = 7;

                u16 valid_sizes(u8 op);

                str::slice_t name(u8 op);

                u16 valid_operands(u8 op);

                u16 valid_cond_codes(u8 op);

                u16 valid_number_types(u8 op);
            }

            namespace group {
                constexpr u8 alu        = 0;
                constexpr u8 cond       = 1;
                constexpr u8 system     = 2;
                constexpr u8 memory     = 3;
                constexpr u8 branch     = 4;
                constexpr u8 logical    = 5;

                u8 group_size(u8 group);

                str::slice_t name(u8 group);
            }

            namespace system {
                constexpr u8 nop        = 0;
                constexpr u8 hcf        = 1;
                constexpr u8 brk        = 2;
                constexpr u8 dmb        = 3;    // data memory barrier
                constexpr u8 dsb        = 4;    // data sync barrier
                constexpr u8 isb        = 5;    // inst. sync barrier
                constexpr u8 svc        = 6;

                u16 valid_sizes(u8 op);

                str::slice_t name(u8 op);

                u16 valid_operands(u8 op);

                u16 valid_cond_codes(u8 op);

                u16 valid_number_types(u8 op);
            }

            namespace memory {
                constexpr u8 adr        = 0;
                constexpr u8 ldr        = 1;
                constexpr u8 str        = 2;
                constexpr u8 cas        = 3;
                constexpr u8 lda        = 4;
                constexpr u8 sta        = 5;

                u16 valid_sizes(u8 op);

                str::slice_t name(u8 op);

                u16 valid_operands(u8 op);

                u16 valid_cond_codes(u8 op);

                u16 valid_number_types(u8 op);
            }

            namespace branch {
                constexpr u8 b          = 0;
                constexpr u8 bl         = 1;
                constexpr u8 br         = 2;
                constexpr u8 blr        = 3;
                constexpr u8 ret        = 4;
                constexpr u8 bcc        = 5;
                constexpr u8 cbcc       = 6;
                constexpr u8 tbcc       = 7;

                u16 valid_sizes(u8 op);

                str::slice_t name(u8 op);

                u16 valid_operands(u8 op);

                u16 valid_cond_codes(u8 op);

                u16 valid_number_types(u8 op);
            }

            namespace logical {
                constexpr u8 mov        = 0;
                constexpr u8 movn       = 1;
                constexpr u8 lsl        = 2;
                constexpr u8 lsr        = 3;
                constexpr u8 rol        = 4;
                constexpr u8 rcl        = 5;
                constexpr u8 ror        = 6;
                constexpr u8 rcr        = 7;
                constexpr u8 or_        = 8;
                constexpr u8 eor        = 9;
                constexpr u8 tst        = 10;
                constexpr u8 inv        = 11;   // N.B. like not
                constexpr u8 and_       = 12;

                u16 valid_sizes(u8 op);

                str::slice_t name(u8 op);

                u16 valid_operands(u8 op);

                u16 valid_cond_codes(u8 op);

                u16 valid_number_types(u8 op);
            }

            namespace num_type {
                constexpr u8 unsigned_  = 0;
                constexpr u8 signed_    = 1;
                constexpr u8 float_     = 2;
                constexpr u8 zero_ext   = 3;
                constexpr u8 sign_ext   = 4;

                str::slice_t name(u8 type);
            }

            namespace cond_code {
                constexpr u8 always     = 0;
                constexpr u8 eq         = 1;
                constexpr u8 ne         = 2;
                constexpr u8 lt         = 3;
                constexpr u8 le         = 4;
                constexpr u8 gt         = 5;
                constexpr u8 ge         = 6;

                str::slice_t name(u8 code);
            }

            namespace operand {
                constexpr u8 none           = 0;
                constexpr u8 imm32          = 1;
                constexpr u8 rd_rm          = 2;
                constexpr u8 rd_rn_rm       = 3;
                constexpr u8 rm_imm32       = 4;
                constexpr u8 rd_imm32       = 5;
                constexpr u8 rd_rn_imm24    = 6;
                constexpr u8 rd_rn_rm_ra    = 7;
                constexpr u8 rd_imm24_imm12 = 8;
                constexpr u8 rd_rn_rm_imm12 = 9;
                constexpr u8 rd_rm_imm12    = 10;
                constexpr u8 rd             = 11;

                str::slice_t name(u8 type);
            }
        }

        namespace register_file {
            constexpr u8 lr             = 0;
            constexpr u8 pc             = 1;
            constexpr u8 sp             = 2;
            constexpr u8 r0             = 3;
            constexpr u8 r1             = 4;
            constexpr u8 r2             = 5;
            constexpr u8 r3             = 6;
            constexpr u8 r4             = 7;
            constexpr u8 r5             = 8;
            constexpr u8 r6             = 9;
            constexpr u8 r7             = 10;
            constexpr u8 r8             = 11;
            constexpr u8 r9             = 12;
            constexpr u8 r10            = 13;
            constexpr u8 r11            = 14;
            constexpr u8 r12            = 15;
            constexpr u8 r13            = 16;
            constexpr u8 r14            = 17;
            constexpr u8 r15            = 18;
            constexpr u8 r16            = 19;
            constexpr u8 r17            = 20;
            constexpr u8 r18            = 21;
            constexpr u8 r19            = 22;
            constexpr u8 r20            = 23;
            constexpr u8 r21            = 24;
            constexpr u8 r22            = 25;
            constexpr u8 r23            = 26;
            constexpr u8 r24            = 27;
            constexpr u8 r25            = 28;
            constexpr u8 r26            = 29;
            constexpr u8 r27            = 30;
            constexpr u8 r28            = 31;
            constexpr u8 r29            = 32;
            constexpr u8 r30            = 33;
            constexpr u8 r31            = 34;
        }
    }
}
