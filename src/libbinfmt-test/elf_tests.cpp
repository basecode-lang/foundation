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

#include <catch.hpp>
#include <basecode/binfmt/io.h>
#include <basecode/core/error.h>
#include <basecode/binfmt/binfmt.h>
#include "test.h"

using namespace basecode;

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

TEST_CASE("basecode::binfmt ELF read obj file") {
    using namespace binfmt;

    io::session_t s{};
    io::session::init(s);

    auto backend_obj_path = R"(C:\temp\test\elf\backend.cpp.o)"_path;
    defer(
        path::free(backend_obj_path);
        io::session::free(s);
        error::report::print_range(0, error::report::count());
        );

    auto backend_obj = io::session::add_file(s,
                                             backend_obj_path,
                                             io::type_t::elf,
                                             io::file_type_t::obj);
    REQUIRE(backend_obj);
    REQUIRE(!backend_obj->module);

    REQUIRE(OK(io::read(s)));

    REQUIRE(backend_obj->module);
}

TEST_CASE("basecode::binfmt ELF round trip obj file") {
    using namespace binfmt;

    io::session_t read_session{};
    io::session::init(read_session);

    io::session_t write_session{};
    io::session::init(write_session);

    auto source_obj_path = R"(C:\temp\test\elf\backend.cpp.o)"_path;
    auto dest_obj_path   = R"(backend.cpp.o)"_path;
    defer(
        path::free(source_obj_path);
        path::free(dest_obj_path);
        io::session::free(read_session);
        io::session::free(write_session);
        error::report::print_range(0, error::report::count());
         );

    auto source_obj = io::session::add_file(read_session,
                                            source_obj_path,
                                            io::type_t::elf,
                                            io::file_type_t::obj);
    REQUIRE(source_obj);
    REQUIRE(!source_obj->module);
    REQUIRE(OK(io::read(read_session)));
    REQUIRE(source_obj->module);

    auto dest_obj = io::session::add_file(write_session,
                                          source_obj->module,
                                          dest_obj_path,
                                          source_obj->machine,
                                          source_obj->bin_type,
                                          source_obj->file_type);

    REQUIRE(dest_obj);
    REQUIRE(OK(io::write(write_session)));
}

TEST_CASE("basecode::binfmt ELF write rot13_elf.exe file") {
    using namespace binfmt;

    module_t mod{};
    REQUIRE(OK(module::init(mod, module_type_t::object, 20)));
    defer(module::free(mod);
              error::report::print_range(0, error::report::count());
    );

    // 1. create the default string table
    auto strtab_sect = module::make_default_string_table(mod);
    REQUIRE(strtab_sect);
    REQUIRE(error::report::ok());

    auto read_file_str      = section::add_string(strtab_sect, "ReadFile"_ss);
    auto write_file_str     = section::add_string(strtab_sect, "WriteFile"_ss);
    auto get_std_handle_str = section::add_string(strtab_sect, "GetStdHandle"_ss);

    REQUIRE(read_file_str > 0);
    REQUIRE(write_file_str > read_file_str);
    REQUIRE(get_std_handle_str > write_file_str);
    REQUIRE(error::report::ok());

    // 2. create the default symbol table
    auto symtab_sect = module::make_default_symbol_table(mod);
    REQUIRE(symtab_sect);
    REQUIRE(error::report::ok());

    auto read_file_sym = section::add_symbol(symtab_sect, read_file_str);
    REQUIRE(read_file_sym);
    read_file_sym->type  = symbol::type_t::function;
    read_file_sym->value = 2;
    read_file_sym->scope = symbol::scope_t::global;

    auto write_file_sym = section::add_symbol(symtab_sect, write_file_str);
    REQUIRE(write_file_sym);
    write_file_sym->type  = symbol::type_t::function;
    write_file_sym->value = 3;
    write_file_sym->scope = symbol::scope_t::global;

    auto get_std_handle_sym = section::add_symbol(symtab_sect, get_std_handle_str);
    REQUIRE(get_std_handle_sym);
    get_std_handle_sym->type  = symbol::type_t::function;
    get_std_handle_sym->value = 4;
    get_std_handle_sym->scope = symbol::scope_t::global;

    /* .text section */ {
        auto text_sect = module::make_text(mod, (u8*) s_rot13_code, sizeof(s_rot13_code));
        REQUIRE(text_sect);
        REQUIRE(error::report::ok());

        read_file_sym->section      = text_sect;
        write_file_sym->section     = text_sect;
        get_std_handle_sym->section = text_sect;

        REQUIRE(error::report::ok());
    }

    /* .rdata section */ {
        auto rodata_sect = module::make_rodata(mod, (u8*) s_rot13_table, sizeof(s_rot13_table));
        REQUIRE(rodata_sect);
        REQUIRE(error::report::ok());
    }

    /* .bss section */ {
        auto bss_sect = module::make_bss(mod, 4096);
        REQUIRE(bss_sect);
        REQUIRE(bss_sect->size == 4096);
    }

    io::session_t s{};
    io::session::init(s);

    auto rot13_exe_path = "rot13_elf.exe"_path;
    defer(
        io::session::free(s);
        path::free(rot13_exe_path);
        );
    auto rot13_exe_file = io::session::add_file(s,
                                                &mod,
                                                rot13_exe_path,
                                                machine::type_t::x86_64,
                                                io::type_t::elf,
                                                io::file_type_t::exe);
    rot13_exe_file->versions.linker.major = 6;
    rot13_exe_file->versions.linker.minor = 0;
    rot13_exe_file->versions.min_os.major = 4;
    rot13_exe_file->versions.min_os.minor = 0;
    rot13_exe_file->flags.console = true;

    REQUIRE(OK(io::write(s)));
}
