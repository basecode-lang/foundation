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
        [CV_SIGNATURE_C6      ]   = "CV_SIGNATURE_C6"_ss,
        [CV_SIGNATURE_C7      ]   = "CV_SIGNATURE_C7"_ss,
        [CV_SIGNATURE_C11     ]   = "CV_SIGNATURE_C11"_ss,
        [CV_SIGNATURE_C13     ]   = "CV_SIGNATURE_C13"_ss,
        [CV_SIGNATURE_RESERVED]   = "CV_SIGNATURE_RESERVED"_ss,
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
    }

    static status_t read_ansi_symbols(cv_t& cv, io::file_t& file) {
        UNUSED(cv);
        UNUSED(file);
        return status_t::ok;
    }

    static status_t read_utf8_symbols(cv_t& cv, io::file_t& file) {
        u32 section_size = cv.size - sizeof(u32);
        const auto section_end_pos = cv.offset + section_size;
        u32 subsection_type{};
        u32 subsection_size{};
        while (true) {
            const auto pos     = FILE_POS();
            const auto buf_idx = pos - cv.offset;

            if ((buf_idx & u32(3)) != 0) {
                // why seek past this subsection?
                subsection_size = 4 - (buf_idx & u32(3));
                FILE_SEEK_FWD(subsection_size);
                section_size -= subsection_size;
            }

            if (pos >= section_end_pos)
                break;

            FILE_READ(u32, subsection_type);
            FILE_READ(u32, subsection_size);
            section_size -= sizeof(u32) * 2 + subsection_size;

            u8 scratch[65535];
            auto rec_hdr = (rec_header_t*) &scratch;

            auto sst = debug_subsection_type_t(subsection_type);
            format::print("Debug Subsection: {}\n", debug_subsection_name(sst));
            switch (sst) {
                case debug_subsection_type_t::symbols: {
                    while (subsection_size > 0) {
                        FILE_READ(u16, rec_hdr->len);
                        FILE_READ_STR(scratch + sizeof(u16), rec_hdr->len);

                        subsection_size -= rec_hdr->len + sizeof(u16);

                        const auto symtype_name = sym::name(rec_hdr->kind);
                        format::print("Symbol Type: {}\n", symtype_name);
                        switch (rec_hdr->kind) {
                            case cv::S_COMPILE3: {
                                auto sym = (sym::compile3_t*) rec_hdr;
                                format::print("    machine      = {}\n", sym->machine);
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
                            case cv::S_CONSTANT: {
                                auto sym = (sym::const_t*) rec_hdr;
                                format::print("    name         = {}\n", sym->name);
                                break;
                            }
                            case cv::S_UDT: {
                                auto sym = (sym::udt_t*) rec_hdr;
                                format::print("    name         = {}\n", sym->name);
                                break;
                            }
                            case cv::S_LOCAL: {
                                auto sym = (sym::local_t*) rec_hdr;
                                format::print("    name         = {}\n", sym->name);
                                break;
                            }
                            case cv::S_GDATA32:
                            case cv::S_LDATA32: {
                                auto sym = (sym::data_t*) rec_hdr;
                                format::print("    name         = {}\n", sym->name);
                                break;
                            }
                            default: {
                                break;
                            }
                        }
                    }
                    break;
                }
                case debug_subsection_type_t::lines:
                case debug_subsection_type_t::string_table:
                case debug_subsection_type_t::file_chksms:
                case debug_subsection_type_t::frame_data:
                case debug_subsection_type_t::inlinee_lines:
                case debug_subsection_type_t::cross_scope_imports:
                case debug_subsection_type_t::cross_scope_exports:
                case debug_subsection_type_t::il_lines:
                case debug_subsection_type_t::func_mdtoken_map:
                case debug_subsection_type_t::type_mdtoken_map:
                case debug_subsection_type_t::merged_assembly_input:
                case debug_subsection_type_t::coff_symbol_rva:
                default:
                    FILE_SEEK_FWD(subsection_size);
                    break;
            }
        }

        return status_t::ok;
    }

    status_t free(cv_t& cv) {
        UNUSED(cv);
        return status_t::ok;
    }

    str::slice_t sig_name(u32 sig) {
        return s_sig_names[sig];
    }

    status_t read(cv_t& cv, io::file_t& file) {
        FILE_PUSH_POS();
        defer(FILE_POP_POS());

        FILE_SEEK(cv.offset);
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
            status = read_utf8_symbols(cv, file);
        } else {
            status = read_ansi_symbols(cv, file);
        }

        format::print("CV SIG: {}\n", sig_name(cv.signature));

        return status;
    }

    status_t init(cv_t& cv, u32 offset, u32 size, alloc_t* alloc) {
        cv.alloc = alloc;
        cv.signature = {};
        cv.flags     = {};
        cv.offset    = offset;
        cv.size      = size;
        return status_t::ok;
    }

    str::slice_t debug_subsection_name(debug_subsection_type_t type) {
        switch (type) {
            case debug_subsection_type_t::ignore:
                return "IGNORE"_ss;
            case debug_subsection_type_t::symbols:
                return "SYMBOLS"_ss;
            case debug_subsection_type_t::lines:
                return "LINES"_ss;
            case debug_subsection_type_t::string_table:
                return "STRING_TABLE"_ss;
            case debug_subsection_type_t::file_chksms:
                return "FILE_CHECKSMS"_ss;
            case debug_subsection_type_t::frame_data:
                return "FRAME_DATA"_ss;
            case debug_subsection_type_t::inlinee_lines:
                return "INLINEE_LINES"_ss;
            case debug_subsection_type_t::cross_scope_imports:
                return "CROSS_SCOPE_IMPORTS"_ss;
            case debug_subsection_type_t::cross_scope_exports:
                return "CROSS_SCOPE_EXPORTS"_ss;
            case debug_subsection_type_t::il_lines:
                return "IL_LINES"_ss;
            case debug_subsection_type_t::func_mdtoken_map:
                return "FUNC_MDTOKEN_MAP"_ss;
            case debug_subsection_type_t::type_mdtoken_map:
                return "TYPE_MDTOKEN_MAP"_ss;
            case debug_subsection_type_t::merged_assembly_input:
                return "MERGED_ASSEMBLY_INPUT"_ss;
            case debug_subsection_type_t::coff_symbol_rva:
                "COFF_SYMBOL_RVA"_ss;
        }
        return "UNKNOWN DEBUG SUBSECTION"_ss;
    }
}
