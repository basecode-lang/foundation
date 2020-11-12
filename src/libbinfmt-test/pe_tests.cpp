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
#include <basecode/binfmt/io.h>
#include <basecode/binfmt/binfmt.h>
#include <basecode/core/stopwatch.h>
#include "test.h"

using namespace basecode;

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
    0xff,0x15,0x40,0x20,0x00,0x00,                  //00000000040100F: FF 15 00 00 00 00       call        qword ptr [__imp_GetStdHandle]
    0xb9,0xf5,0xff,0xff,0xff,                       //000000000401015: B9 F5 FF FF FF          mov         ecx,0FFFFFFF5h
    0x48,0x8b,0xd8,                                 //00000000040101A: 48 8B D8                mov         rbx,rax
    0xff,0x15,0x32,0x20,0x00,0x00,                  //00000000040101D: FF 15 00 00 00 00       call        qword ptr [__imp_GetStdHandle]
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
    0xff,0x15,0xee,0x1f,0x00,0x00,                  //000000000401071: FF 15 00 00 00 00       call        qword ptr [__imp_WriteFile]
    0x48,0x83,0x64,0x24,0x20,0x00,                  //000000000401077: 48 83 64 24 20 00       and         qword ptr [rsp+20h],0
    0x4c,0x8d,0x4c,0x24,0x40,                       //00000000040107D: 4C 8D 4C 24 40          lea         r9,[rsp+40h]
    0x41,0xb8,0x00,0x10,0x00,0x00,                  //000000000401082: 41 B8 00 10 00 00       mov         r8d,1000h
    0x48,0x8d,0x15,0x71,0x2f,0x00,0x00,             //000000000401088: 48 8D 15 00 00 00 00    lea         rdx,[buffer]
    0x48,0x8b,0xcb,                                 //00000000040108F: 48 8B CB                mov         rcx,rbx
    0xff,0x15,0xc5,0x1f,0x00,0x00,                  //000000000401092: FF 15 00 00 00 00       call        qword ptr [__imp_ReadFile]
    0x8b,0x54,0x24,0x40,                            //000000000401098: 8B 54 24 40             mov         edx,dword ptr [rsp+40h]
    0x85,0xd2,                                      //00000000040109C: 85 D2                   test        edx,edx
    0x75,0x88,                                      //00000000040109E: 75 88                   jne         0000000000000028
    0x48,0x8b,0x5c,0x24,0x50,                       //0000000004010A0: 48 8B 5C 24 50          mov         rbx,qword ptr [rsp+50h]
    0x33,0xc0,                                      //0000000004010A5: 33 C0                   xor         eax,eax
    0x48,0x83,0xc4,0x30,                            //0000000004010A7: 48 83 C4 30             add         rsp,30h
    0x5f,                                           //0000000004010AB: 5F                      pop         rdi
    0xc3,                                           //0000000004010AC: C3                      ret
};

TEST_CASE("basecode::binfmt rot13 to PE/COFF exe") {
    using namespace binfmt;

    stopwatch_t timer{};
    stopwatch::start(timer);

    module_t* mod{};
    REQUIRE(OK(system::make_module(module_type_t::object, 10, &mod)));
    REQUIRE(system::get_module(10) != nullptr);

    /* .text section */ {
        auto text_rc = module::make_section(*mod,
                                            section::type_t::code,
                                            {
                                              .flags = {
                                                  .code = true,
                                                  .exec = true,
                                                  .alloc = true,
                                              }
                                          });
        auto set_data_rc = section::data(*mod,
                                         text_rc.id,
                                         s_rot13_code,
                                         sizeof(s_rot13_code));
        REQUIRE(OK(set_data_rc.status));
        REQUIRE(OK(text_rc.status));
    }

    /* .rdata section */ {
        auto rdata_rc = module::make_section(*mod,
                                             section::type_t::data,
                                             {
                                               .flags = {
                                                   .code = false,
                                                   .init = true,
                                                   .alloc = true,
                                               }
                                           });
        auto set_data_rc = section::data(*mod,
                                         rdata_rc.id,
                                         s_rot13_table,
                                         sizeof(s_rot13_table));
        REQUIRE(OK(set_data_rc.status));
        REQUIRE(OK(rdata_rc.status));
    }

    /* .idata section */ {
        auto idata_rc = module::make_section(*mod,
                                             section::type_t::import,
                                             {
                                                .flags = {
                                                    .code = false,
                                                    .init = true,
                                                    .write = true,
                                                    .alloc = true,
                                                }
                                           });
        auto kernel32_sym_rc = module::make_symbol(*mod,
                                                   "kernel32.dll"_ss,
                                                   {
                                                     .value = 1,
                                                     .section = idata_rc.id,
                                                     .type   = symbol::type_t::file,
                                                     .scope  = symbol::scope_t::local
                                                 });
        auto read_file_sym_rc = module::make_symbol(*mod,
                                                    "ReadFile"_ss,
                                                    {
                                                      .value = 2,
                                                      .section = idata_rc.id,
                                                      .type = symbol::type_t::function,
                                                      .scope = symbol::scope_t::global,
                                                  });
        auto write_file_sym_rc = module::make_symbol(*mod,
                                                     "WriteFile"_ss,
                                                     {
                                                       .value = 3,
                                                       .section = idata_rc.id,
                                                       .type = symbol::type_t::function,
                                                       .scope = symbol::scope_t::global,
                                                   });
        auto get_std_handle_sym_rc = module::make_symbol(*mod,
                                                         "GetStdHandle"_ss,
                                                         {
                                                           .value = 4,
                                                           .section = idata_rc.id,
                                                           .type = symbol::type_t::function,
                                                           .scope = symbol::scope_t::global,
                                                       });
        auto kernel32_imp_rc = section::import_module(*mod, idata_rc.id, kernel32_sym_rc.id);
        auto kernel32_imp    = section::get_import(*mod, idata_rc.id, kernel32_imp_rc.id);
        import::add_symbol(kernel32_imp, get_std_handle_sym_rc.id);
        import::add_symbol(kernel32_imp, read_file_sym_rc.id);
        import::add_symbol(kernel32_imp, write_file_sym_rc.id);

        REQUIRE(OK(kernel32_sym_rc.status));
        REQUIRE(OK(read_file_sym_rc.status));
        REQUIRE(OK(write_file_sym_rc.status));
        REQUIRE(OK(get_std_handle_sym_rc.status));
        REQUIRE(OK(idata_rc.status));
        REQUIRE(OK(kernel32_imp_rc.status));
        REQUIRE(kernel32_imp);
        REQUIRE(kernel32_imp->symbols.size == 3);
        REQUIRE(kernel32_imp->symbols[0] == get_std_handle_sym_rc.id);
        REQUIRE(kernel32_imp->symbols[1] == read_file_sym_rc.id);
        REQUIRE(kernel32_imp->symbols[2] == write_file_sym_rc.id);
    }

    /* .bss section */ {
        auto bss_rc = module::make_section(*mod,
                                           section::type_t::data,
                                           {
                                            .flags = {
                                                .code   = false,
                                                .init   = false,
                                                .write  = true,
                                                .alloc  = true,
                                            }
                                         });
        auto bss = module::get_section(*mod, bss_rc.id);
        section::reserve(*mod, bss_rc.id, 4096);

        REQUIRE(OK(bss_rc.status));
        REQUIRE(bss);
        REQUIRE(bss->subclass.size == 4096);
    }

    io::session_t s{};
    io::session::init(s);
    defer(io::session::free(s));

    auto rot13_exe_path = "rot13.exe"_path;
    defer(path::free(rot13_exe_path));
    auto rot13_exe_file = io::session::add_file(s,
                                                mod,
                                                rot13_exe_path,
                                                machine::type_t::x86_64,
                                                io::type_t::pe,
                                                io::file_type_t::exe);
    rot13_exe_file->versions.linker.major = 6;
    rot13_exe_file->versions.linker.minor = 0;
    rot13_exe_file->versions.min_os.major = 4;
    rot13_exe_file->versions.min_os.minor = 0;
    rot13_exe_file->flags.console = true;

    REQUIRE(OK(io::write(s)));

    stopwatch::stop(timer);
    stopwatch::print_elapsed("binfmt write PE executable time"_ss, 40, timer);
}
