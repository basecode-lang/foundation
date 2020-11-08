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

#include <basecode/core/types.h>

#define LF_PAD(n) (0xf0 + (n))

namespace basecode::binfmt::cv {
    struct guid_t final {
        u32                     data1;
        u16                     data2;
        u16                     data3;
        u8                      data4[8];
    };

    struct rec_header_t final {
        u16                     len;
        u16                     kind;
    };

    namespace machine {
        constexpr u32 x86_64        = 0xd0;
    }

    namespace type {
        enum class ptr_kind_t : u8 {
            Near16                  = 0x00,
            Far16                   = 0x01,
            Huge16                  = 0x02,
            BasedOnSegment          = 0x03,
            BasedOnValue            = 0x04,
            BasedOnSegmentValue     = 0x05,
            BasedOnAddress          = 0x06,
            BasedOnSegmentAddress   = 0x07,
            BasedOnType             = 0x08,
            BasedOnSelf             = 0x09,
            Near32                  = 0x0a,
            Far32                   = 0x0b,
            Near64                  = 0x0c
        };

        enum class ptr_mode_t : u8 {
            Pointer                 = 0x00,
            LValueReference         = 0x01,
            PointerToDataMember     = 0x02,
            PointerToMemberFunction = 0x03,
            RValueReference         = 0x04
        };

        enum class ptr_mods_t : u8 {
            None                    = 0x00,
            Flat32                  = 0x01,
            Volatile                = 0x02,
            Const                   = 0x04,
            Unaligned               = 0x08,
            Restrict                = 0x10,
        };

        enum class ptr_flags_t : u8 {
            WinRTSmartPointer       = 0x01,
            LValueRefThisPointer    = 0x02,
            RValueRefThisPointer    = 0x04
        };

        namespace member {
            constexpr u32 LF_BCLASS             = (0x1400);
            constexpr u32 LF_BINTERFACE         = (0x151a);
            constexpr u32 LF_VBCLASS            = (0x1401);
            constexpr u32 LF_IVBCLASS           = (0x1402);
            constexpr u32 LF_VFUNCTAB           = (0x1409);
            constexpr u32 LF_STMEMBER           = (0x150e);
            constexpr u32 LF_METHOD             = (0x150f);
            constexpr u32 LF_MEMBER             = (0x150d);
            constexpr u32 LF_NESTTYPE           = (0x1510);
            constexpr u32 LF_ONEMETHOD          = (0x1511);
            constexpr u32 LF_ENUMERATE          = (0x1502);
            constexpr u32 LF_INDEX              = (0x1404);
        }

        namespace leaf {
            constexpr u32 LF_POINTER            = 0x1002;
            constexpr u32 LF_MODIFIER           = 0x1001;
            constexpr u32 LF_PROCEDURE          = 0x1008;
            constexpr u32 LF_MFUNCTION          = 0x1009;
            constexpr u32 LF_LABEL              = 0x000e;
            constexpr u32 LF_ARGLIST            = 0x1201;
            constexpr u32 LF_FIELDLIST          = 0x1203;
            constexpr u32 LF_ARRAY              = 0x1503;
            constexpr u32 LF_CLASS              = 0x1504;
            constexpr u32 LF_STRUCTURE          = 0x1505;
            constexpr u32 LF_INTERFACE          = 0x1519;
            constexpr u32 LF_UNION              = 0x1506;
            constexpr u32 LF_ENUM               = 0x1507;
            constexpr u32 LF_TYPESERVER2        = 0x1515;
            constexpr u32 LF_VFTABLE            = 0x151d;
            constexpr u32 LF_VTSHAPE            = 0x000a;
            constexpr u32 LF_BITFIELD           = 0x1205;
            constexpr u32 LF_FUNC_ID            = 0x1601; // pdb: ipi
            constexpr u32 LF_MFUNC_ID           = 0x1602; // pdb: ipi
            constexpr u32 LF_BUILDINFO          = 0x1603; // pdb: ipi
            constexpr u32 LF_SUBSTR_LIST        = 0x1604; // pdb: ipi
            constexpr u32 LF_STRING_ID          = 0x1605; // pdb: ipi
            constexpr u32 LF_UDT_SRC_LINE       = 0x1606; // pdb: ipi
            constexpr u32 LF_UDT_MOD_SRC_LINE   = 0x1607; // pdb: ipi
            constexpr u32 LF_METHODLIST         = 0x1206;
            constexpr u32 LF_PRECOMP            = 0x1509;
            constexpr u32 LF_ENDPRECOMP         = 0x0014;
        }

        // 0x1002
        struct ptr_t final {
            u32                     referent_type;
            u32                     attributes;
            u32                     unused:     8;
            u32                     flags:      3;
            u32                     size:       8;
            u32                     mods:       5;
            u32                     mode:       3;
            u32                     kind:       5;
            // member ptr info?
        };
    }

    namespace symbol {
        constexpr u32 S_PUB32                                = 0x110e;      // public stream
//        constexpr u32 S_GDATA32                                           // global
        constexpr u32 S_GTHREAD32                            = 0x1113;      //
        constexpr u32 S_PROCREF                              = 0x1125;      //
        constexpr u32 S_LPROCREF                             = 0x1127;      //
        constexpr u32 S_GMANDATA                             = 0x111d;      //
        constexpr u32 S_END                                  = 0x0006;
        constexpr u32 S_FRAMEPROC                            = 0x1012;
        constexpr u32 S_OBJNAME                              = 0x1101;
        constexpr u32 S_THUNK32                              = 0x1102;
        constexpr u32 S_BLOCK32                              = 0x1103;
        constexpr u32 S_LABEL32                              = 0x1105;
        constexpr u32 S_REGISTER                             = 0x1106;
        constexpr u32 S_BPREL32                              = 0x110b;
        constexpr u32 S_LPROC32                              = 0x110f;
        constexpr u32 S_GPROC32                              = 0x1110;
        constexpr u32 S_REGREL32                             = 0x1111;
        constexpr u32 S_COMPILE2                             = 0x1116;
        constexpr u32 S_UNAMESPACE                           = 0x1124;
        constexpr u32 S_TRAMPOLINE                           = 0x112c;
        constexpr u32 S_SECTION                              = 0x1136;
        constexpr u32 S_COFFGROUP                            = 0x1137;
        constexpr u32 S_EXPORT                               = 0x1138;
        constexpr u32 S_CALLSITEINFO                         = 0x1139;
        constexpr u32 S_FRAMECOOKIE                          = 0x113a;
        constexpr u32 S_COMPILE3                             = 0x113c;
        constexpr u32 S_ENVBLOCK                             = 0x113d;
        constexpr u32 S_LOCAL                                = 0x113e;
        constexpr u32 S_DEFRANGE                             = 0x113f;
        constexpr u32 S_DEFRANGE_SUBFIELD                    = 0x1140;
        constexpr u32 S_DEFRANGE_REGISTER                    = 0x1141;
        constexpr u32 S_DEFRANGE_FRAMEPOINTER_REL            = 0x1142;
        constexpr u32 S_DEFRANGE_SUBFIELD_REGISTER           = 0x1143;
        constexpr u32 S_DEFRANGE_FRAMEPOINTER_REL_FULL_SCOPE = 0x1144;
        constexpr u32 S_DEFRANGE_REGISTER_REL                = 0x1145;
        constexpr u32 S_LPROC32_ID                           = 0x1146;
        constexpr u32 S_GPROC32_ID                           = 0x1147;
        constexpr u32 S_BUILDINFO                            = 0x114c;
        constexpr u32 S_INLINESITE                           = 0x114d;
        constexpr u32 S_INLINESITE_END                       = 0x114e;
        constexpr u32 S_PROC_ID_END                          = 0x114f;
        constexpr u32 S_FILESTATIC                           = 0x1153;
        constexpr u32 S_LPROC32_DPC                          = 0x1155;
        constexpr u32 S_LPROC32_DPC_ID                       = 0x1156;
        constexpr u32 S_CALLEES                              = 0x115a;
        constexpr u32 S_CALLERS                              = 0x115b;
        constexpr u32 S_HEAPALLOCSITE                        = 0x115e;
        constexpr u32 S_FASTLINK                             = 0x1167;
        constexpr u32 S_INLINEES                             = 0x1168;
        constexpr u32 S_CONSTANT                             = 0x1107;  // module or global
        constexpr u32 S_UDT                                  = 0x1108;  //
        constexpr u32 S_LDATA32                              = 0x110c;  //
        constexpr u32 S_LTHREAD32                            = 0x1112;  //
        constexpr u32 S_LMANDATA                             = 0x111c;  //
        constexpr u32 S_MANCONSTANT                          = 0x112d;  //
    }
}
