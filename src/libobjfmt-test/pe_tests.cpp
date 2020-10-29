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

#include <catch2/catch.hpp>
#include <basecode/core/defer.h>
#include <basecode/objfmt/objfmt.h>
#include <basecode/core/stopwatch.h>
#include <basecode/objfmt/container.h>

using namespace basecode;

static const u8 s_rot13_table[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
    23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
    55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 'N', 'O', 'P', 'Q', 'R', 'S',
    'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
    'J', 'K', 'L', 'M', 91, 92, 93, 94, 95, 96, 'n', 'o', 'p', 'q', 'r', 's',
    't', 'u', 'v', 'w', 'x', 'y', 'z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
    'j', 'k', 'l', 'm', 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150,
    151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166,
    167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182,
    183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198,
    199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214,
    215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230,
    231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246,
    247, 248, 249, 250, 251, 252, 253, 254, 255
};
//402000    = rot13_table
//403000    = GetStdHandle
//403008    = ReadFile
//403010    = WriteFile
//404000    = buffer
static const u8 s_rot13_code[] = {
    0x48,0x89,0x5c,0x24,0x18,                       //000000000401000: 48 89 5C 24 18          mov         qword ptr [rsp+18h],rbx
    0x57,                                           //000000000401005: 57                      push        rdi
    0x48,0x83,0xec,0x30,                            //000000000401006: 48 83 EC 30             sub         rsp,30h
    0xb9,0xf6,0xff,0xff,0xff,                       //00000000040100A: B9 F6 FF FF FF          mov         ecx,0FFFFFFF6h
    0xff,0x15,0xeb,0x1f,0x00,0x00,                  //00000000040100F: FF 15 00 00 00 00       call        qword ptr [__imp_GetStdHandle]
    0xb9,0xf5,0xff,0xff,0xff,                       //000000000401015: B9 F5 FF FF FF          mov         ecx,0FFFFFFF5h
    0x48,0x8b,0xd8,                                 //00000000040101A: 48 8B D8                mov         rbx,rax
    0xff,0x15,0xdd,0x1f,0x00,0x00,                  //00000000040101D: FF 15 00 00 00 00       call        qword ptr [__imp_GetStdHandle]
    0x48,0x8b,0xf8,                                 //000000000401023: 48 8B F8                mov         rdi,rax
    0xeb,0x4f,                                      //000000000401026: EB 4F                   jmp         0000000000000077
    0x83,0x64,0x24,0x48,0x00,                       //000000000401028: 83 64 24 48 00          and         dword ptr [rsp+48h],0
    0x85,0xd2,                                      //00000000040102D: 85 D2                   test        edx,edx
    0x74,0x28,                                      //00000000040102F: 74 28                   je          0000000000000059
    0x44,0x8b,0xc2,                                 //000000000401031: 44 8B C2                mov         r8d,edx
    0x48,0x8d,0x0d,0xc5,0x2f,0x00,0x00,             //000000000401034: 48 8D 0D 00 00 00 00    lea         rcx,[buffer]
    0x44,0x89,0x44,0x24,0x48,                       //00000000040103B: 44 89 44 24 48          mov         dword ptr [rsp+48h],r8d
    0x0f,0xb6,0x01,                                 //000000000401040: 0F B6 01                movzx       eax,byte ptr [rcx]
    0x4c,0x8d,0x0d,0xb6,0x0f,0x00,0x00,             //000000000401043: 4C 8D 0D 00 00 00 00    lea         r9,[rot13_table]
    0x42,0x8a,0x04,0x08,                            //00000000040104A: 42 8A 04 08             mov         al,byte ptr [rax+r9]
    0x88,0x01,                                      //00000000040104E: 88 01                   mov         byte ptr [rcx],al
    0x48,0xff,0xc1,                                 //000000000401050: 48 FF C1                inc         rcx
    0x49,0x83,0xe8,0x01,                            //000000000401053: 49 83 E8 01             sub         r8,1
    0x75,0xe7,                                      //000000000401057: 75 E7                   jne         0000000000000040
    0x48,0x83,0x64,0x24,0x20,0x00,                  //000000000401059: 48 83 64 24 20 00       and         qword ptr [rsp+20h],0
    0x4c,0x8d,0x4c,0x24,0x48,                       //00000000040105F: 4C 8D 4C 24 48          lea         r9,[rsp+48h]
    0x44,0x8b,0xc2,                                 //000000000401064: 44 8B C2                mov         r8d,edx
    0x48,0x8b,0xcf,                                 //000000000401067: 48 8B CF                mov         rcx,rdi
    0x48,0x8d,0x15,0x8f,0x2f,0x00,0x00,             //00000000040106A: 48 8D 15 00 00 00 00    lea         rdx,[buffer]
    0xff,0x15,0x99,0x1f,0x00,0x00,                  //000000000401071: FF 15 00 00 00 00       call        qword ptr [__imp_WriteFile]
    0x48,0x83,0x64,0x24,0x20,0x00,                  //000000000401077: 48 83 64 24 20 00       and         qword ptr [rsp+20h],0
    0x4c,0x8d,0x4c,0x24,0x40,                       //00000000040107D: 4C 8D 4C 24 40          lea         r9,[rsp+40h]
    0x41,0xb8,0x00,0x10,0x00,0x00,                  //000000000401082: 41 B8 00 10 00 00       mov         r8d,1000h
    0x48,0x8d,0x15,0x71,0x2f,0x00,0x00,             //000000000401088: 48 8D 15 00 00 00 00    lea         rdx,[buffer]
    0x48,0x8b,0xcb,                                 //00000000040108F: 48 8B CB                mov         rcx,rbx
    0xff,0x15,0x70,0x1f,0x00,0x00,                  //000000000401092: FF 15 00 00 00 00       call        qword ptr [__imp_ReadFile]
    0x8b,0x54,0x24,0x40,                            //000000000401098: 8B 54 24 40             mov         edx,dword ptr [rsp+40h]
    0x85,0xd2,                                      //00000000040109C: 85 D2                   test        edx,edx
    0x75,0x88,                                      //00000000040109E: 75 88                   jne         0000000000000028
    0x48,0x8b,0x5c,0x24,0x50,                       //0000000004010A0: 48 8B 5C 24 50          mov         rbx,qword ptr [rsp+50h]
    0x33,0xc0,                                      //0000000004010A5: 33 C0                   xor         eax,eax
    0x48,0x83,0xc4,0x30,                            //0000000004010A7: 48 83 C4 30             add         rsp,30h
    0x5f,                                           //0000000004010AB: 5F                      pop         rdi
    0xc3,                                           //0000000004010AC: C3                      ret
};

TEST_CASE("basecode::objfmt rot13 to PE/COFF exe") {
    using namespace objfmt;

    stopwatch_t timer{};
    stopwatch::start(timer);

    file_t rot13_pgm{};
    REQUIRE(OK(file::init(rot13_pgm)));
    defer(file::free(rot13_pgm));
    path::set(rot13_pgm.path, "rot13.exe");
    rot13_pgm.machine = machine::type_t::x86_64;

    /* .text section */ {
        auto text_rc = file::make_section(rot13_pgm,
                                          section::type_t::code);
        auto text = file::get_section(rot13_pgm, text_rc.id);
        section::data(text, s_rot13_code, sizeof(s_rot13_code));
        text->flags.code = true;
        text->flags.read = true;
        text->flags.exec = true;

        REQUIRE(OK(text_rc.status));
        REQUIRE(text);
    }

    /* .rdata section */ {
        auto rdata_rc = file::make_section(rot13_pgm,
                                           section::type_t::data);
        auto rdata = file::get_section(rot13_pgm, rdata_rc.id);
        section::data(rdata, s_rot13_table, sizeof(s_rot13_table));
        rdata->flags.data = true;
        rdata->flags.read = true;
        REQUIRE(OK(rdata_rc.status));
        REQUIRE(rdata);
    }

    /* .idata section */ {
        auto idata_rc = file::make_section(rot13_pgm,
                                           section::type_t::import);
        auto idata = file::get_section(rot13_pgm, idata_rc.id);
        idata->flags.data  = true;
        idata->flags.read  = true;

        const auto func_type = SYMBOL_TYPE(symbol::type::derived::function, symbol::type::base::null_);
        auto kernel32_sym_rc = file::make_symbol(rot13_pgm,
                                                 "kernel32.dll"_ss,
                                                 {.section = idata->id, .value = 1});
        auto read_file_sym_rc = file::make_symbol(rot13_pgm,
                                                  "ReadFile"_ss,
                                                  {.section = idata->id, .type = func_type, .value = 2});
        auto write_file_sym_rc = file::make_symbol(rot13_pgm,
                                                   "WriteFile"_ss,
                                                   {.section = idata->id, .type = func_type, .value = 3});
        auto get_std_handle_sym_rc = file::make_symbol(rot13_pgm,
                                                       "GetStdHandle"_ss,
                                                       {.section = idata->id, .type = func_type, .value = 4});
        auto kernel32_imp_rc = section::import_module(idata, kernel32_sym_rc.id);
        auto kernel32_imp    = section::get_import(idata, kernel32_imp_rc.id);
        import::add_symbol(kernel32_imp, get_std_handle_sym_rc.id);
        import::add_symbol(kernel32_imp, read_file_sym_rc.id);
        import::add_symbol(kernel32_imp, write_file_sym_rc.id);

        REQUIRE(OK(kernel32_sym_rc.status));
        REQUIRE(OK(read_file_sym_rc.status));
        REQUIRE(OK(write_file_sym_rc.status));
        REQUIRE(OK(get_std_handle_sym_rc.status));
        REQUIRE(OK(idata_rc.status));
        REQUIRE(idata);
        REQUIRE(OK(kernel32_imp_rc.status));
        REQUIRE(kernel32_imp);
        REQUIRE(kernel32_imp->symbols.size == 3);
        REQUIRE(kernel32_imp->symbols[0] == get_std_handle_sym_rc.id);
        REQUIRE(kernel32_imp->symbols[1] == read_file_sym_rc.id);
        REQUIRE(kernel32_imp->symbols[2] == write_file_sym_rc.id);
    }

    /* .bss section */ {
        auto bss_rc = file::make_section(rot13_pgm,
                                         section::type_t::uninit);
        auto bss = file::get_section(rot13_pgm, bss_rc.id);
        section::reserve(bss, 4096);
        bss->flags.data   = true;
        bss->flags.read   = true;
        bss->flags.write  = true;

        REQUIRE(OK(bss_rc.status));
        REQUIRE(bss);
        REQUIRE(bss->subclass.size == 4096);
    }

    container::session_t s{};
    container::session::init(s);
    defer(container::session::free(s));
    s.file                  = &rot13_pgm;
    s.type                  = container::type_t::pe;
    s.output_type           = container::output_type_t::exe;
    s.versions.linker.major = 6;
    s.versions.linker.minor = 0;
    s.versions.min_os.major = 4;
    s.versions.min_os.minor = 0;
    s.flags.console         = true;
//    s.opts.base_addr        = 0x140000000;
    REQUIRE(OK(container::write(s)));

    stopwatch::stop(timer);
    stopwatch::print_elapsed("objfmt write PE executable time"_ss, 40, timer);
}
