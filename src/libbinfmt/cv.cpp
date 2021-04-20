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

#include <basecode/binfmt/cv.h>

namespace basecode::binfmt::cv {
// skip pad bytes
//    for (;;) {
//        pdata = (PB) pfl->data + cb;
//        if (*(BYTE *) pdata < LF_PAD0)
//            break;
//        cb++;
//    }

    static str::slice_t s_sig_names[] = {
        [CV_SIGNATURE_C6      ] = "CV_SIGNATURE_C6"_ss,
        [CV_SIGNATURE_C7      ] = "CV_SIGNATURE_C7"_ss,
        [CV_SIGNATURE_C11     ] = "CV_SIGNATURE_C11"_ss,
        [CV_SIGNATURE_C13     ] = "CV_SIGNATURE_C13"_ss,
        [CV_SIGNATURE_RESERVED] = "CV_SIGNATURE_RESERVED"_ss,
    };

    static str::slice_t s_mach_names[] = {
        "8080"_ss,
        "8086"_ss,
        "80286"_ss,
        "80386"_ss,
        "80486"_ss,
        "Pentium"_ss,
        "Pentium Pro/Pentium II"_ss,
        "Pentium III"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "MIPS (Generic)"_ss,
        "MIPS16"_ss,
        "MIPS32"_ss,
        "MIPS64"_ss,
        "MIPS I"_ss,
        "MIPS II"_ss,
        "MIPS III"_ss,
        "MIPS IV"_ss,
        "MIPS V"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "M68000"_ss,
        "M68010"_ss,
        "M68020"_ss,
        "M68030"_ss,
        "M68040"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "Alpha 21064"_ss,
        "Alpha 21164"_ss,
        "Alpha 21164A"_ss,
        "Alpha 21264"_ss,
        "Alpha 21364"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "PPC 601"_ss,
        "PPC 603"_ss,
        "PPC 604"_ss,
        "PPC 620"_ss,
        "PPC w/FP"_ss,
        "PPC (Big Endian)"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "SH3"_ss,
        "SH3E"_ss,
        "SH3DSP"_ss,
        "SH4"_ss,
        "SHmedia"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "ARMv3 (CE)"_ss,
        "ARMv4 (CE)"_ss,
        "ARMv4T (CE)"_ss,
        "ARMv5 (CE)"_ss,
        "ARMv5T (CE)"_ss,
        "ARMv6 (CE)"_ss,
        "ARM (XMAC) (CE)"_ss,
        "ARM (WMMX) (CE)"_ss,
        "ARMv7 (CE)"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "Omni"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "Itanium"_ss,
        "Itanium (McKinley)"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "CEE"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "AM33"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "M32R"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "TriCore"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "x64"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "EBC"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "Thumb (CE)"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "ARM"_ss,
        "???"_ss,
        "ARM64"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "???"_ss,
        "D3D11_SHADER"_ss,
    };

    static str::slice_t s_lang_names[] = {
        "C"_ss,
        "C++"_ss,
        "FORTRAN"_ss,
        "MASM"_ss,
        "Pascal"_ss,
        "Basic"_ss,
        "COBOL"_ss,
        "LINK"_ss,
        "CVTRES"_ss,
        "CVTPGD"_ss,
        "C#"_ss,
        "Visual Basic"_ss,
        "ILASM"_ss,
        "Java"_ss,
        "JScript"_ss,
        "MSIL"_ss,
        "HLSL"_ss,
    };

    namespace sym {
        str::slice_t name(u16 type) {
            switch (type) {
                case S_PUB32:                                   return "S_PUB32"_ss;
                case S_GDATA32:                                 return "S_GDATA32"_ss;
                case S_GTHREAD32:                               return "S_GTHREAD32"_ss;
                case S_PROCREF:                                 return "S_PROCREF"_ss;
                case S_LPROCREF:                                return "S_LPROCREF"_ss;
                case S_GMANDATA:                                return "S_GMANDATA"_ss;
                case S_END:                                     return "S_END"_ss;
                case S_FRAMEPROC:                               return "S_FRAMEPROC"_ss;
                case S_OBJNAME:                                 return "S_OBJNAME"_ss;
                case S_THUNK32:                                 return "S_THUNK32"_ss;
                case S_BLOCK32:                                 return "S_BLOCK32"_ss;
                case S_LABEL32:                                 return "S_LABEL32"_ss;
                case S_REGISTER:                                return "S_REGISTER"_ss;
                case S_BPREL32:                                 return "S_BPREL32"_ss;
                case S_LPROC32:                                 return "S_LPROC32"_ss;
                case S_GPROC32:                                 return "S_GPROC32"_ss;
                case S_REGREL32:                                return "S_REGREL32"_ss;
                case S_COMPILE2:                                return "S_COMPILE2"_ss;
                case S_UNAMESPACE:                              return "S_UNAMESPACE"_ss;
                case S_TRAMPOLINE:                              return "S_TRAMPOLINE"_ss;
                case S_SECTION:                                 return "S_SECTION"_ss;
                case S_COFFGROUP:                               return "S_COFFGROUP"_ss;
                case S_EXPORT:                                  return "S_EXPORT"_ss;
                case S_CALLSITEINFO:                            return "S_CALLSITEINFO"_ss;
                case S_FRAMECOOKIE:                             return "S_FRAMECOOKIE"_ss;
                case S_COMPILE3:                                return "S_COMPILE3"_ss;
                case S_ENVBLOCK:                                return "S_ENVBLOCK"_ss;
                case S_LOCAL:                                   return "S_LOCAL"_ss;
                case S_DEFRANGE:                                return "S_DEFRANGE"_ss;
                case S_DEFRANGE_SUBFIELD:                       return "S_DEFRANGE_SUBFIELD"_ss;
                case S_DEFRANGE_REGISTER:                       return "S_DEFRANGE_REGISTER"_ss;
                case S_DEFRANGE_FRAMEPOINTER_REL:               return "S_DEFRANGE_FRAMEPOINTER_REL"_ss;
                case S_DEFRANGE_SUBFIELD_REGISTER:              return "S_DEFRANGE_SUBFIELD_REGISTER"_ss;
                case S_DEFRANGE_FRAMEPOINTER_REL_FULL_SCOPE:    return "S_DEFRANGE_FRAMEPOINTER_REL_FULL_SCOPE"_ss;
                case S_DEFRANGE_REGISTER_REL:                   return "S_DEFRANGE_REGISTER_REL"_ss;
                case S_LPROC32_ID:                              return "S_LPROC32_ID"_ss;
                case S_GPROC32_ID:                              return "S_GPROC32_ID"_ss;
                case S_BUILDINFO:                               return "S_BUILDINFO"_ss;
                case S_INLINESITE:                              return "S_INLINESITE"_ss;
                case S_INLINESITE_END:                          return "S_INLINESITE_END"_ss;
                case S_PROC_ID_END:                             return "S_PROC_ID_END"_ss;
                case S_FILESTATIC:                              return "S_FILESTATIC"_ss;
                case S_LPROC32_DPC:                             return "S_LPROC32_DPC"_ss;
                case S_LPROC32_DPC_ID:                          return "S_LPROC32_DPC_ID"_ss;
                case S_CALLEES:                                 return "S_CALLEES"_ss;
                case S_CALLERS:                                 return "S_CALLERS"_ss;
                case S_HEAPALLOCSITE:                           return "S_HEAPALLOCSITE"_ss;
                case S_FASTLINK:                                return "S_FASTLINK"_ss;
                case S_INLINEES:                                return "S_INLINEES"_ss;
                case S_CONSTANT:                                return "S_CONSTANT"_ss;
                case S_UDT:                                     return "S_UDT"_ss;
                case S_LDATA32:                                 return "S_LDATA32"_ss;
                case S_LTHREAD32:                               return "S_LTHREAD32"_ss;
                case S_LMANDATA:                                return "S_LMANDATA"_ss;
                case S_MANCONSTANT:                             return "S_MANCONSTANT"_ss;
                default:
                    break;
            }
            return "UNKNOWN"_ss;
        }
    }

    namespace leaf {
        str::slice_t name(u16 type) {
            switch (type) {
                case LF_ALIAS:              return "LF_ALIAS"_ss;
                case LF_BCLASS:             return "LF_BCLASS"_ss;
                case LF_BINTERFACE:         return "LF_BINTERFACE"_ss;
                case LF_VBCLASS:            return "LF_VBCLASS"_ss;
                case LF_IVBCLASS:           return "LF_IVBCLASS"_ss;
                case LF_VFUNCTAB:           return "LF_VFUNCTAB"_ss;
                case LF_STMEMBER:           return "LF_STMEMBER"_ss;
                case LF_METHOD:             return "LF_METHOD"_ss;
                case LF_MEMBER:             return "LF_MEMBER"_ss;
                case LF_NESTTYPE:           return "LF_NESTTYPE"_ss;
                case LF_ONEMETHOD:          return "LF_ONEMETHOD"_ss;
                case LF_ENUMERATE:          return "LF_ENUMERATE"_ss;
                case LF_INDEX:              return "LF_INDEX"_ss;
                case LF_POINTER:            return "LF_POINTER"_ss;
                case LF_MODIFIER:           return "LF_MODIFIER"_ss;
                case LF_PROCEDURE:          return "LF_PROCEDURE"_ss;
                case LF_MFUNCTION:          return "LF_MFUNCTION"_ss;
                case LF_LABEL:              return "LF_LABEL"_ss;
                case LF_ARGLIST:            return "LF_ARGLIST"_ss;
                case LF_FIELDLIST:          return "LF_FIELDLIST"_ss;
                case LF_ARRAY:              return "LF_ARRAY"_ss;
                case LF_CLASS:              return "LF_CLASS"_ss;
                case LF_STRUCTURE:          return "LF_STRUCTURE"_ss;
                case LF_INTERFACE:          return "LF_INTERFACE"_ss;
                case LF_UNION:              return "LF_UNION"_ss;
                case LF_ENUM:               return "LF_ENUM"_ss;
                case LF_TYPESERVER2:        return "LF_TYPESERVER2"_ss;
                case LF_VFTABLE:            return "LF_VFTABLE"_ss;
                case LF_VTSHAPE:            return "LF_VTSHAPE"_ss;
                case LF_BITFIELD:           return "LF_BITFIELD"_ss;
                case LF_FUNC_ID:            return "LF_FUNC_ID"_ss;
                case LF_MFUNC_ID:           return "LF_MFUNC_ID"_ss;
                case LF_BUILDINFO:          return "LF_BUILDINFO"_ss;
                case LF_SUBSTR_LIST:        return "LF_SUBSTR_LIST"_ss;
                case LF_STRING_ID:          return "LF_STRING_ID"_ss;
                case LF_UDT_SRC_LINE:       return "LF_UDT_SRC_LINE"_ss;
                case LF_UDT_MOD_SRC_LINE:   return "LF_UDT_MOD_SRC_LINE"_ss;
                case LF_METHODLIST:         return "LF_METHODLIST"_ss;
                case LF_PRECOMP:            return "LF_PRECOMP"_ss;
                case LF_ENDPRECOMP:         return "LF_ENDPRECOMP"_ss;
                default:
                    break;
            }
            return "UNKNOWN"_ss;
        }
    }

    static status_t read_ansi_symbols(cv_t& cv,
                                      io::file_t& file,
                                      u32 offset,
                                      u32 size);

    static status_t read_utf8_symbols(cv_t& cv,
                                      io::file_t& file,
                                      u32 offset,
                                      u32 size);

    status_t free(cv_t& cv) {
        UNUSED(cv);
        return status_t::ok;
    }

    str::slice_t sig_name(u32 sig) {
        return s_sig_names[sig];
    }

    str::slice_t machine_name(u16 machine) {
        return s_mach_names[machine];
    }

    str::slice_t language_name(u8 language) {
        return s_lang_names[language];
    }

    status_t init(cv_t& cv, alloc_t* alloc) {
        cv.alloc = alloc;
        cv.signature = {};
        cv.flags     = {};
        return status_t::ok;
    }

    static status_t read_ansi_symbols(cv_t& cv,
                                      io::file_t& file,
                                      u32 offset,
                                      u32 size) {
        UNUSED(cv);
        UNUSED(file);
        UNUSED(size);
        return status_t::ok;
    }

    static status_t read_utf8_symbols(cv_t& cv,
                                      io::file_t& file,
                                      u32 offset,
                                      u32 size) {
        u32 section_size = size;
        const auto section_end_pos = offset + section_size;
        while (true) {
            const auto pos     = FILE_POS();
            const auto buf_idx = pos - offset;

            if ((buf_idx & u32(3)) != 0) {
                // XXX: why seek past this subsection?
                auto skip_size = 4 - (buf_idx & u32(3));
                FILE_SEEK_FWD(skip_size);
                section_size -= skip_size;
            }

            if (pos >= section_end_pos)
                break;

            auto subsection_hdr = (subsection_header_t*) FILE_PTR();
            FILE_SEEK_FWD(sizeof(u32) * 2);
            section_size -= sizeof(u32) * 2 + subsection_hdr->len;

            format::print("Debug Subsection: {}\n", debug_subsection_name(subsection_hdr->type));
            switch (subsection_hdr->type) {
                case dbg_subsection_type_t::symbols: {
                    auto subsection_size = subsection_hdr->len;
                    while (subsection_size > 0) {
                        auto rec_hdr = (rec_header_t*) FILE_PTR();
                        const auto rec_size = rec_hdr->len + sizeof(u16);
                        FILE_SEEK_FWD(rec_size);
                        subsection_size -= rec_size;

                        const auto symtype_name = sym::name(rec_hdr->kind);
                        format::print("Symbol Type: {}\n", symtype_name);
                        switch (rec_hdr->kind) {
                            case cv::S_COMPILE3: {
                                auto sym = (sym::compile3_t*) rec_hdr;
                                format::print("    machine      = {}\n", machine_name(sym->machine));
                                format::print("    language     = {}\n", language_name(sym->flags.language));
                                format::print("    ver_fe_major = {}\n", sym->ver_fe_major);
                                format::print("    ver_fe_minor = {}\n", sym->ver_fe_minor);
                                format::print("    ver_fe_build = {}\n", sym->ver_fe_build);
                                format::print("    ver_major    = {}\n", sym->ver_major);
                                format::print("    ver_minor    = {}\n", sym->ver_minor);
                                format::print("    ver_build    = {}\n", sym->ver_build);
                                format::print("    ver_qfe      = {}\n", sym->ver_qfe);
                                auto slice = slice::make(sym->ver_str + 2);
                                format::print("    ver_str      = {}\n", slice);
                                break;
                            }
                            case cv::S_INLINESITE: {
                                auto sym = (sym::inline_site_t*) rec_hdr;
                                format::print("    inline site begin\n");
                                format::print("    inlinee      = {}\n", sym->inlinee);
                                break;
                            }
                            case cv::S_INLINESITE_END: {
                                format::print("    inline site end\n");
                                break;
                            }
                            case cv::S_CALLSITEINFO: {
                                auto sym = (sym::call_site_info_t*) rec_hdr;
                                format::print("    call site info\n");
                                format::print("    type_index   = {}\n", sym->type_index);
                                break;
                            }
                            case cv::S_HEAPALLOCSITE: {
                                auto sym = (sym::heap_alloc_site_t*) rec_hdr;
                                format::print("    heap alloc site info\n");
                                format::print("    type_index   = {}\n", sym->type_index);
                                break;
                            }
                            case cv::S_CONSTANT: {
                                auto sym = (sym::const_t*) rec_hdr;
                                format::print("    name         = {}\n", sym->name);
                                break;
                            }
                            case cv::S_UDT: {
                                auto sym = (sym::udt_t*) rec_hdr;
                                format::print("    name         = {}\n", sym->name);
                                format::print("    type_index   = {}\n", sym->type_index);
                                break;
                            }
                            case cv::S_LOCAL: {
                                auto sym = (sym::local_t*) rec_hdr;
                                format::print("    name         = {}\n", sym->name);
                                break;
                            }
                            case cv::S_GPROC32_ID:
                            case cv::S_LPROC32_ID:
                            case cv::S_LPROC32_DPC_ID: {
                                auto sym = (sym::proc_t*) rec_hdr;
                                format::print("    proc begin\n");
                                format::print("    name         = {}\n", sym->name);
                                break;
                            }
                            case cv::S_GPROC32:
                            case cv::S_LPROC32:
                            case cv::S_LPROC32_DPC: {
                                auto sym = (sym::proc_t*) rec_hdr;
                                format::print("    name         = {}\n", sym->name);
                                break;
                            }
                            case cv::S_PROC_ID_END: {
                                format::print("     proc end\n");
                                break;
                            }
                            case cv::S_FRAMEPROC: {
                                auto sym = (sym::frame_proc_t*) rec_hdr;
                                format::print("    frame_size    = {}\n", sym->frame_size);
                                break;
                            }
                            case cv::S_GDATA32:
                            case cv::S_LDATA32: {
                                auto sym = (sym::data_t*) rec_hdr;
                                format::print("    name         = {}\n", sym->name);
                                break;
                            }
                            case cv::S_BUILDINFO: {
                                auto sym = (sym::build_info_t*) rec_hdr;
                                format::print("    item_id      = {}\n", sym->id);
                                break;
                            }
                            default: {
                                break;
                            }
                        }
                    }
                    break;
                }
                case dbg_subsection_type_t::lines: {
                    auto subsection_size = subsection_hdr->len - sizeof(c13_lines_hdr_t);
                    auto lines_hdr = (c13_lines_hdr_t*) FILE_PTR();
                    format::print("    {:04x}:{:08x}-{:08x}, flags = {:04x}\n",
                                  u32(lines_hdr->seg),
                                  u32(lines_hdr->offset),
                                  u32(lines_hdr->offset + lines_hdr->len),
                                  u32(lines_hdr->flags));
                    b8 has_column = (lines_hdr->flags & CV_LINES_HAVE_COLUMNS);
                    FILE_SEEK_FWD(sizeof(c13_lines_hdr_t));
                    while (subsection_size > 0) {
                        auto block_hdr = (c13_file_block_t*) FILE_PTR();
                        subsection_size -= block_hdr->len;
                        FILE_SEEK_FWD(block_hdr->len);
                        for (u32 i = 0; i < block_hdr->num_lines; ++i) {
                            const auto& line = block_hdr->lines[i];
                            b8 special_line = line.line_num_start == 0xfeefee
                                              || line.line_num_start == 0xf00f00;
                            if (has_column) {
                                format::print("has_column!\n");
                            } else {
                                if ((i % 4) == 0) format::print("\n");
                                if (special_line) {
                                    format::print(" {:04x} {:08x}",
                                                  line.line_num_start,
                                                  line.offset + lines_hdr->offset);
                                } else {
                                    format::print("{:04x} {:08x}",
                                                  line.line_num_start,
                                                  line.offset + lines_hdr->offset);
                                }
                            }
                        }
                        format::print("\n");
                    }
                    break;
                }
                case dbg_subsection_type_t::string_table: {
                    s8* p = (s8*) FILE_PTR();
                    auto subsection_size = subsection_hdr->len;
                    u32 strtab_offset{};
                    while (subsection_size > 0) {
                        format::print("    {:08x} ", strtab_offset);
                        format::print("{}\n", p);
                        const auto len = strlen(p) + 1;
                        strtab_offset += len;
                        subsection_size -= len;
                        p += len;
                    }
                    FILE_SEEK_FWD(subsection_hdr->len);
                    break;
                }
                case dbg_subsection_type_t::file_chksms: {
                    // dumpsym7.cpp @ 1095
                    break;
                }
                // dumpsym7.cpp @ 1176
                case dbg_subsection_type_t::frame_data:
                case dbg_subsection_type_t::inlinee_lines:
                case dbg_subsection_type_t::cross_scope_imports:
                case dbg_subsection_type_t::cross_scope_exports:
                case dbg_subsection_type_t::il_lines:
                case dbg_subsection_type_t::func_mdtoken_map:
                case dbg_subsection_type_t::type_mdtoken_map:
                case dbg_subsection_type_t::merged_assembly_input:
                case dbg_subsection_type_t::coff_symbol_rva:
                default:
                    FILE_SEEK_FWD(subsection_hdr->len);
                    break;
            }
        }

        return status_t::ok;
    }

    str::slice_t debug_subsection_name(dbg_subsection_type_t type) {
        switch (type) {
            case dbg_subsection_type_t::ignore:
                return "IGNORE"_ss;
            case dbg_subsection_type_t::symbols:
                return "SYMBOLS"_ss;
            case dbg_subsection_type_t::lines:
                return "LINES"_ss;
            case dbg_subsection_type_t::string_table:
                return "STRING_TABLE"_ss;
            case dbg_subsection_type_t::file_chksms:
                return "FILE_CHECKSMS"_ss;
            case dbg_subsection_type_t::frame_data:
                return "FRAME_DATA"_ss;
            case dbg_subsection_type_t::inlinee_lines:
                return "INLINEE_LINES"_ss;
            case dbg_subsection_type_t::cross_scope_imports:
                return "CROSS_SCOPE_IMPORTS"_ss;
            case dbg_subsection_type_t::cross_scope_exports:
                return "CROSS_SCOPE_EXPORTS"_ss;
            case dbg_subsection_type_t::il_lines:
                return "IL_LINES"_ss;
            case dbg_subsection_type_t::func_mdtoken_map:
                return "FUNC_MDTOKEN_MAP"_ss;
            case dbg_subsection_type_t::type_mdtoken_map:
                return "TYPE_MDTOKEN_MAP"_ss;
            case dbg_subsection_type_t::merged_assembly_input:
                return "MERGED_ASSEMBLY_INPUT"_ss;
            case dbg_subsection_type_t::coff_symbol_rva:
                "COFF_SYMBOL_RVA"_ss;
        }
        return "UNKNOWN DEBUG SUBSECTION"_ss;
    }

    status_t read_type_data(cv_t& cv, io::file_t& file, u32 offset, u32 size) {
        FILE_PUSH_POS();
        defer(FILE_POP_POS());

        FILE_SEEK(offset);
        FILE_READ(u32, cv.signature);

        switch (cv.signature) {
            case CV_SIGNATURE_C7:
            case CV_SIGNATURE_C11:
                cv.flags.utf8_symbols = false;
                break;
            case CV_SIGNATURE_C13:
                cv.flags.utf8_symbols = true;
                break;
            default:
                return status_t::bad_cv_signature;
        }

        auto section_size = size - sizeof(cv.signature);
        while (section_size > 0) {
            auto rec_hdr = (rec_header_t*) FILE_PTR();
            const auto rec_size = sizeof(u16) + rec_hdr->len;
            section_size -= rec_size;
            FILE_SEEK_FWD(rec_size);

            const auto kind_name = leaf::name(rec_hdr->kind);
            format::print("Leaf Type: {}\n", kind_name);
            switch (rec_hdr->kind) {
                case cv::LF_ENUM: {
                    auto lf = (leaf::enum_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    format::print("    count        = {}\n", lf->count);
                    format::print("    base_type    = {}\n", lf->utype);
                    format::print("    field_type   = {}\n", lf->field);
                    format::print("    name         = {}\n", lf->name);
                    break;
                }
                case cv::LF_ALIAS: {
                    auto lf = (leaf::alias_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    format::print("    base_type    = {}\n", lf->utype);
                    format::print("    name         = {}\n", lf->name);
                    break;
                }
                case cv::LF_ARRAY: {
                    auto lf = (leaf::array_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    format::print("    elem_type    = {}\n", lf->elem_type);
                    format::print("    idx_type     = {}\n", lf->idx_type);
                    break;
                }
                case cv::LF_UNION: {
                    auto lf = (leaf::union_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    format::print("    count        = {}\n", lf->count);
                    format::print("    field_type   = {}\n", lf->field);
                    break;
                }
                case cv::LF_POINTER: {
                    auto lf = (leaf::pointer_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    format::print("    base_type    = {}\n", lf->utype);
                    break;
                }
                case cv::LF_CLASS:
                case cv::LF_INTERFACE:
                case cv::LF_STRUCTURE: {
                    auto lf = (leaf::class_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    format::print("    count        = {}\n", lf->count);
                    format::print("    field_type   = {}\n", lf->field);
                    format::print("    derived_type = {}\n", lf->derived);
                    format::print("    vshape       = {}\n", lf->vshape);
                    break;
                }
                case cv::LF_PROCEDURE: {
                    auto lf = (leaf::proc_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    format::print("    rv_type      = {}\n", lf->rv_type);
                    format::print("    call_type    = {}\n", lf->call_type);
                    format::print("    param_count  = {}\n", lf->param_count);
                    format::print("    arg_list     = {}\n", lf->arg_list);
                    break;
                }
                case cv::LF_BITFIELD: {
                    auto lf = (leaf::bit_field_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    format::print("    type         = {}\n", lf->type);
                    format::print("    length       = {}\n", lf->length);
                    format::print("    position     = {}\n", lf->position);
                    break;
                }
                case cv::LF_MFUNCTION: {
                    auto lf = (leaf::mfunc_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    format::print("    rv_type      = {}\n", lf->rv_type);
                    format::print("    class_type   = {}\n", lf->class_type);
                    format::print("    this_type    = {}\n", lf->this_type);
                    format::print("    call_type    = {}\n", lf->call_type);
                    format::print("    param_count  = {}\n", lf->param_count);
                    format::print("    arg_list     = {}\n", lf->arg_list);
                    format::print("    this_adj     = {}\n", lf->this_adjust);
                    break;
                }
                case cv::LF_MODIFIER: {
                    auto lf = (leaf::modifier_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    format::print("    type         = {}\n", lf->type);
                    break;
                }
                case cv::LF_METHODLIST: {
                    auto lf = (leaf::method_list_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    break;
                }
                case cv::LF_FIELDLIST: {
                    auto lf = (leaf::field_list_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    break;
                }
                case cv::LF_ARGLIST: {
                    auto lf = (leaf::arg_list_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    format::print("    count        = {}\n", lf->count);
                    break;
                }
                case cv::LF_FUNC_ID: {
                    auto lf = (leaf::func_id_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    format::print("    scope_id     = {}\n", lf->scope_id);
                    format::print("    type         = {}\n", lf->type);
                    format::print("    text         = {}\n", lf->name);
                    break;
                }
                case cv::LF_MFUNC_ID: {
                    auto lf = (leaf::mfunc_id_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    format::print("    parent_type  = {}\n", lf->parent_type);
                    format::print("    type         = {}\n", lf->type);
                    format::print("    text         = {}\n", lf->name);
                    break;
                }
                case cv::LF_STRING_ID: {
                    auto lf = (leaf::string_id_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    format::print("    item_id      = {}\n", lf->id);
                    format::print("    text         = {}\n", lf->name);
                    break;
                }
                case cv::LF_UDT_SRC_LINE: {
                    auto lf = (leaf::udt_src_line_t*) rec_hdr;
                    format::print("    leaf         = {}\n", lf->leaf);
                    format::print("    type         = {}\n", lf->type);
                    format::print("    src          = {}\n", lf->src);
                    format::print("    line         = {}\n", lf->line);
                    break;
                }
                default:
                    break;
            }
        }

        return status_t::ok;
    }

    status_t read_symbol_data(cv_t& cv, io::file_t& file, u32 offset, u32 size) {
        FILE_PUSH_POS();
        defer(FILE_POP_POS());

        FILE_SEEK(offset);
        FILE_READ(u32, cv.signature);

        switch (cv.signature) {
            case CV_SIGNATURE_C7:
            case CV_SIGNATURE_C11:
                cv.flags.utf8_symbols = false;
                break;
            case CV_SIGNATURE_C13:
                cv.flags.utf8_symbols = true;
                break;
            default:
                return status_t::bad_cv_signature;
        }

        status_t status;

        if (cv.flags.utf8_symbols) {
            status = read_utf8_symbols(cv,
                                       file,
                                       offset,
                                       size - sizeof(cv.signature));
        } else {
            status = read_ansi_symbols(cv,
                                       file,
                                       offset,
                                       size - sizeof(cv.signature));
        }

        return status;
    }
}
