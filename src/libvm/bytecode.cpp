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

    namespace alu {
        static str::slice_t s_names[] = {
            "add"_ss,
            "adc"_ss,
            "mul"_ss,
            "madd"_ss,
            "sbc"_ss,
            "sub"_ss,
            "msub"_ss,
            "div"_ss,
            "pow"_ss,
            "mod"_ss,
            "neg"_ss,
        };

        static u16 s_sizes[] = {
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
        };

        static u16 s_operands[] = {
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rn_imm24),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rn_imm24),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rn_imm24),
            MASK(operand::rd_rn_rm_ra) | MASK(operand::rd_rn_rm_imm12),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rn_imm24),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rn_imm24),
            MASK(operand::rd_rn_rm_ra) | MASK(operand::rd_rn_rm_imm12),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rn_imm24),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rn_imm24),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rn_imm24),
            MASK(operand::rd_rm),
        };

        static u16 s_cond_codes[] = {
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always)
        };

        static u16 s_num_types[] = {
            MASK(num_type::unsigned_) | MASK(num_type::signed_),
            MASK(num_type::unsigned_) | MASK(num_type::signed_),
            MASK(num_type::unsigned_) | MASK(num_type::signed_),
            MASK(num_type::unsigned_) | MASK(num_type::signed_),
            MASK(num_type::unsigned_) | MASK(num_type::signed_),
            MASK(num_type::unsigned_) | MASK(num_type::signed_),
            MASK(num_type::unsigned_) | MASK(num_type::signed_),
            MASK(num_type::unsigned_) | MASK(num_type::signed_),
            MASK(num_type::unsigned_) | MASK(num_type::signed_),
            MASK(num_type::unsigned_) | MASK(num_type::signed_),
            MASK(num_type::unsigned_),
        };

        u16 valid_sizes(u8 op) {
            return s_sizes[op];
        }

        str::slice_t name(u8 op) {
            return s_names[op];
        }

        u16 valid_operands(u8 op) {
            return s_operands[op];
        }

        u16 valid_cond_codes(u8 op) {
            return s_cond_codes[op];
        }

        u16 valid_number_types(u8 op) {
            return s_num_types[op];
        }
    }

    namespace cond {
        static str::slice_t s_names[] = {
            "clz"_ss,
            "ctz"_ss,
            "bfi"_ss,
            "bfc"_ss,
            "set"_ss,
            "clr"_ss,
            "neg"_ss,
            "inv"_ss,
        };

        static u16 s_sizes[] = {
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::byte),
            MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
        };

        static u16 s_operands[] = {
            MASK(operand::rd_rm),
            MASK(operand::rd_rm),
            MASK(operand::rd_rm_imm12),
            MASK(operand::rd_rm_imm12),
            MASK(operand::rd),
            MASK(operand::rd),
            MASK(operand::rd),
            MASK(operand::rd),
        };

        static u16 s_cond_codes[] = {
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::eq) | MASK(cond_code::ne) | MASK(cond_code::lt) | MASK(cond_code::le) | MASK(cond_code::gt) | MASK(cond_code::ge),
            MASK(cond_code::eq) | MASK(cond_code::ne) | MASK(cond_code::lt) | MASK(cond_code::le) | MASK(cond_code::gt) | MASK(cond_code::ge),
            MASK(cond_code::eq) | MASK(cond_code::ne) | MASK(cond_code::lt) | MASK(cond_code::le) | MASK(cond_code::gt) | MASK(cond_code::ge),
            MASK(cond_code::eq) | MASK(cond_code::ne) | MASK(cond_code::lt) | MASK(cond_code::le) | MASK(cond_code::gt) | MASK(cond_code::ge),
        };

        static u16 s_num_types[] = {
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
        };

        u16 valid_sizes(u8 op) {
            return s_sizes[op];
        }

        str::slice_t name(u8 op) {
            return s_names[op];
        }

        u16 valid_operands(u8 op) {
            return s_operands[op];
        }

        u16 valid_cond_codes(u8 op) {
            return s_cond_codes[op];
        }

        u16 valid_number_types(u8 op) {
            return s_num_types[op];
        }
    }

    namespace group {
        static str::slice_t s_names[] = {
            "alu"_ss,
            "cond"_ss,
            "system"_ss,
            "memory"_ss,
            "branch"_ss,
            "logical"_ss,
        };

        static u32 s_sizes[] = {
            11, 8, 7, 6, 7, 12,
        };

        u8 group_size(u8 group) {
            return s_sizes[group];
        }

        str::slice_t name(u8 group) {
            return s_names[group];
        }
    }

    namespace system {
        static str::slice_t s_names[] = {
            "nop"_ss,
            "hcf"_ss,
            "brk"_ss,
            "dmb"_ss,
            "dsb"_ss,
            "isb"_ss,
            "svc"_ss,
        };

        static u16 s_sizes[] = {
            MASK(size::qword),
            MASK(size::qword),
            MASK(size::qword),
            MASK(size::qword),
            MASK(size::qword),
            MASK(size::qword),
            MASK(size::qword),
        };

        static u16 s_operands[] = {
            MASK(operand::none),
            MASK(operand::none),
            MASK(operand::none),
            MASK(operand::imm32),
            MASK(operand::imm32),
            MASK(operand::imm32),
            MASK(operand::imm32) | MASK(operand::rd)
        };

        static u16 s_num_types[] = {
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_)
        };

        static u16 s_cond_codes[] = {
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always)
        };

        u16 valid_sizes(u8 op) {
            return s_sizes[op];
        }

        str::slice_t name(u8 op) {
            return s_names[op];
        }

        u16 valid_operands(u8 op) {
            return s_operands[op];
        }

        u16 valid_cond_codes(u8 op) {
            return s_cond_codes[op];
        }

        u16 valid_number_types(u8 op) {
            return s_num_types[op];
        }
    }

    namespace memory {
        static str::slice_t s_names[] = {
            "adr"_ss,
            "ldr"_ss,
            "str"_ss,
            "cas"_ss,
            "lda"_ss,
            "sta"_ss,
        };

        static u16 s_sizes[] = {
            MASK(size::qword),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
        };

        static u16 s_operands[] = {
            MASK(operand::rd_imm32),
            MASK(operand::rd_rm) | MASK(operand::rd_rn_rm_ra) | MASK(operand::rd_rn_imm24),
            MASK(operand::rd_rm) | MASK(operand::rd_rn_rm_ra) | MASK(operand::rd_rn_imm24),
            MASK(operand::rd_rm),
            MASK(operand::rd_rm) | MASK(operand::rd_rn_rm_ra) | MASK(operand::rd_rn_imm24),
            MASK(operand::rd_rm) | MASK(operand::rd_rn_rm_ra) | MASK(operand::rd_rn_imm24),
        };

        static u16 s_num_types[] = {
            MASK(num_type::signed_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
        };

        static u16 s_cond_codes[] = {
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
        };

        u16 valid_sizes(u8 op) {
            return s_sizes[op];
        }

        str::slice_t name(u8 op) {
            return s_names[op];
        }

        u16 valid_operands(u8 op) {
            return s_operands[op];
        }

        u16 valid_cond_codes(u8 op) {
            return s_cond_codes[op];
        }

        u16 valid_number_types(u8 op) {
            return s_num_types[op];
        }
    }

    namespace branch {
        static str::slice_t s_names[] = {
            "b"_ss,
            "bl"_ss,
            "br"_ss,
            "blr"_ss,
            "ret"_ss,
            "b"_ss,
            "cb"_ss,
            "tb"_ss,
        };

        static u16 s_sizes[] = {
            MASK(size::qword),
            MASK(size::qword),
            MASK(size::qword),
            MASK(size::qword),
            MASK(size::qword),
            MASK(size::qword),
            MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
        };

        static u16 s_operands[] = {
            MASK(operand::imm32),
            MASK(operand::imm32),
            MASK(operand::rd),
            MASK(operand::imm32),
            MASK(operand::none),
            MASK(operand::imm32),
            MASK(operand::imm32),
            MASK(operand::imm32)
        };

        static u16 s_num_types[] = {
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_)
        };

        static u16 s_cond_codes[] = {
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::eq) | MASK(cond_code::ne) | MASK(cond_code::lt) | MASK(cond_code::le) | MASK(cond_code::gt) | MASK(cond_code::ge),
            MASK(cond_code::eq) | MASK(cond_code::ne),
            MASK(cond_code::eq) | MASK(cond_code::ne),
        };

        u16 valid_sizes(u8 op) {
            return s_sizes[op];
        }

        str::slice_t name(u8 op) {
            return s_names[op];
        }

        u16 valid_operands(u8 op) {
            return s_operands[op];
        }

        u16 valid_cond_codes(u8 op) {
            return s_cond_codes[op];
        }

        u16 valid_number_types(u8 op) {
            return s_num_types[op];
        }
    }

    namespace logical {
        static str::slice_t s_names[] = {
            "mov"_ss,
            "lsl"_ss,
            "lsr"_ss,
            "rol"_ss,
            "rcl"_ss,
            "ror"_ss,
            "rcr"_ss,
            "or"_ss,
            "eor"_ss,
            "tst"_ss,
            "not"_ss,
            "and"_ss,
        };

        static u16 s_sizes[] = {
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
            MASK(size::qword) | MASK(size::dword) | MASK(size::word) | MASK(size::byte),
        };

        static u16 s_operands[] = {
            MASK(operand::rd_rn_rm) | MASK(operand::rd_imm32),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rm_imm12),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rm_imm12),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rm_imm12),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rm_imm12),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rm_imm12),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rm_imm12),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rm_imm12),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rm_imm12),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rm_imm12),
            MASK(operand::rd_rm),
            MASK(operand::rd_rn_rm) | MASK(operand::rd_rm_imm12),
        };

        static u16 s_num_types[] = {
            MASK(num_type::unsigned_) | MASK(num_type::zero_ext) | MASK(num_type::sign_ext),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
            MASK(num_type::unsigned_),
        };

        static u16 s_cond_codes[] = {
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
            MASK(cond_code::always),
        };

        u16 valid_sizes(u8 op) {
            return s_sizes[op];
        }

        str::slice_t name(u8 op) {
            return s_names[op];
        }

        u16 valid_operands(u8 op) {
            return s_operands[op];
        }

        u16 valid_cond_codes(u8 op) {
            return s_cond_codes[op];
        }

        u16 valid_number_types(u8 op) {
            return s_num_types[op];
        }
    }

    namespace num_type {
        static str::slice_t s_names[] = {
            "u"_ss,
            "s"_ss,
            "f"_ss,
            "zx"_ss,
            "sx"_ss,
        };

        str::slice_t name(u8 type) {
            return s_names[type];
        }
    }

    namespace operand {
        static str::slice_t s_names[] = {
            "none"_ss,
            "imm32"_ss,
            "rd, rm"_ss,
            "rd, rn, rm"_ss,
            "rm, imm32"_ss,
            "rd, imm32"_ss,
            "rd, rn, imm24"_ss,
            "rd, rn, rm, ra"_ss,
            "rd, imm24, imm12"_ss,
            "rd, rn, rm, imm12"_ss,
            "rd, rm, imm12"_ss,
            "rd"_ss,
        };

        static str::slice_t s_var_names[] = {
            "none"_ss,
            "imm32"_ss,
            "rd_rm"_ss,
            "rd_rn_rm"_ss,
            "rm_imm32"_ss,
            "rd_imm32"_ss,
            "rd_rn_imm24"_ss,
            "rd_rn_rm_ra"_ss,
            "rd_imm24_imm12"_ss,
            "rd_rn_rm_imm12"_ss,
            "rd_rm_imm12"_ss,
            "rd"_ss,
        };

        str::slice_t name(u8 type) {
            return s_names[type];
        }

        str::slice_t var_name(u8 type) {
            return s_var_names[type];
        }
    }

    namespace cond_code {
        static str::slice_t s_names[] = {
            "always"_ss,
            "eq"_ss,
            "ne"_ss,
            "lt"_ss,
            "le"_ss,
            "gt"_ss,
            "ge"_ss,
        };

        str::slice_t name(u8 code) {
            return s_names[code];
        }
    }
}
