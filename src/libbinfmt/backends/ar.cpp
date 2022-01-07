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

namespace basecode::binfmt::ar {
    struct header_t final {
        s8                      name[16];
        s8                      date[12];
        s8                      uid[6];
        s8                      gid[6];
        s8                      mode[8];
        s8                      size[10];
    };

    struct sym_offset_t final {
        str::slice_t        name;
        u32                 offset;
    };

    static status_t parse_headers(file_t& file);

    static status_t parse_symbol_table(file_t& file);

    static status_t parse_ecoff_symbol_table(file_t& file,
                                             member_t& symbol_table);

    static status_t parse_long_name_ref(file_t& file, member_t& member);

    static status_t parse_headers(file_t& file) {
        auto& module = *file.module;
        u64_bytes_t thunk_u64{};
        s8          marker[2];

        FILE_READ(u64, thunk_u64.value);
        if (memcmp(thunk_u64.bytes, "!<arch>\n", 8) != 0)
            return status_t::read_error;

        while (!FILE_EOF()) {
            auto hdr_offset = FILE_POS();
            auto hdr = (header_t*) FILE_PTR();
            FILE_SEEK_FWD(sizeof(header_t));
            FILE_READ(s8[2], marker);

            if (marker[0] != 0x60 || marker[1] != 0x0a)
                return status_t::read_error;

            auto member = module::make_member(module,
                                              slice::make((u8*) hdr, 16));
            member->buf.data      = FILE_PTR();
            member->offset.data   = FILE_POS();
            member->offset.header = hdr_offset;

            numbers::status_t parse_rc;

            const auto date_slice = slice::make(hdr->date, 12);
            parse_rc = numbers::integer::parse(date_slice,
                                               10,
                                               member->date);
            if (!OK(parse_rc))
                return status_t::read_error;

            const auto uid_slice = slice::make(hdr->uid, 6);
            parse_rc = numbers::integer::parse(uid_slice,
                                               10,
                                               member->uid);
            if (!OK(parse_rc))
                return status_t::read_error;

            const auto gid_slice = slice::make(hdr->gid, 6);
            parse_rc = numbers::integer::parse(gid_slice,
                                               10,
                                               member->gid);
            if (!OK(parse_rc))
                return status_t::read_error;

            const auto mode_slice = slice::make(hdr->mode, 8);
            parse_rc = numbers::integer::parse(mode_slice,
                                               8,
                                               member->mode);
            if (!OK(parse_rc))
                return status_t::read_error;

            const auto size_slice = slice::make(hdr->size, 10);
            parse_rc = numbers::integer::parse(size_slice,
                                               10,
                                               member->buf.length);
            if (!OK(parse_rc))
                return status_t::read_error;

            FILE_SEEK_FWD(member->buf.length);
            while (!FILE_EOF() && FILE_PEEK(0) == '\n')
                FILE_NEXT();
        }

        return status_t::ok;
    }

    static status_t parse_symbol_table(file_t& file) {
        auto& module = *file.module;
        auto sc = module.as_archive();

        if (array::empty(sc->members))
            return status_t::ok;

        auto& symbol_table = sc->members[0];
        if (symbol_table.name[0] != '/'
        &&  symbol_table.name[1] != ' ') {
            return status_t::ok;
        }

        sc->long_names = 1;

        if (sc->members.size > 1) {
            const auto& ecoff_table = sc->members[1];
            if (ecoff_table.name[0] == '/'
            &&  ecoff_table.name[1] == ' ') {
                auto status = parse_ecoff_symbol_table(file, sc->members[1]);
                if (!OK(status))
                    return status;
            }
        }

        auto symtab_sect = module::make_default_symbol_table(module);
        auto& symtab = symtab_sect->as_symtab();
        auto& strtab = module.strtab->as_strtab();

        hashtab_t<u32, u32> offset_member_idx{};
        hashtab::init(offset_member_idx);

        array_t<sym_offset_t> sym_offsets{};
        array::init(sym_offsets);

        FILE_PUSH_POS();
        FILE_SEEK(symbol_table.offset.data);

        defer(
            FILE_POP_POS();
            array::free(sym_offsets);
            hashtab::free(offset_member_idx));

        u32 num_symbols{};
        u8 ch{};

        FILE_READ(u32, num_symbols);
        num_symbols = endian_swap_dword(num_symbols);
        array::resize(sym_offsets, num_symbols);

        for (u32 i = 0; i < num_symbols; ++i) {
            auto& sym = sym_offsets[i];
            FILE_READ(u32, sym.offset);
            sym.offset = endian_swap_dword(sym.offset);
        }

        u32  idx{};
        auto symbol_offset = FILE_POS();
        while (num_symbols) {
            FILE_READ(u8, ch);
            if (ch == 0) {
                auto& sym_offs = sym_offsets[idx++];
                sym_offs.name = slice::make(FILE_PTR_OFFS(symbol_offset),
                                            FILE_POS() - symbol_offset - 1);
                auto sym_name_offs = string_table::append(strtab, sym_offs.name);
                symbol_opts_t opts{
                    .value = sym_offs.offset,
                    .type = symbol::type_t::none,
                    .scope = symbol::scope_t::local
                };
                symbol_table::make_symbol(symtab,
                                          sym_name_offs,
                                          opts);
                hashtab::insert(sc->index, sym_offs.name, 0U);
                symbol_offset = FILE_POS();
                --num_symbols;
            }
        }

        while (!FILE_EOF() && FILE_PEEK(0) == 0)
            FILE_NEXT();

        for (u32 i = 0; i < sc->members.size; ++i) {
            hashtab::insert(offset_member_idx,
                            sc->members[i].offset.header,
                            i);
        }

        bitset::resize(sc->bitmap,
                       sc->index.size * sc->members.size);

        for (const auto& pair : sc->index)
            sc->index[pair.bucket_idx] = idx * sc->members.size;

        for (u32 i = 0; i < sym_offsets.size; ++i) {
            auto& sym_offs = sym_offsets[i];
            const auto bm_offset = *hashtab::find(sc->index, sym_offs.name);
            auto member_idx = *hashtab::find(offset_member_idx, sym_offs.offset);
            bitset::write(sc->bitmap, bm_offset + member_idx, true);
        }

        return status_t::ok;
    }

    static status_t parse_ecoff_symbol_table(file_t& file,
                                             member_t& symbol_table) {
        FILE_PUSH_POS();
        defer(FILE_POP_POS());

        auto& module = *file.module;
        auto sc = file.module->as_archive();
        sc->coff_table = 2;

        auto strtab_sect = module::make_default_string_table(module);
        section_opts_t opts{};
        opts.link = strtab_sect;
        sc->extended_symtab = module::make_section(module,
                                                   section::type_t::symtab);

        FILE_SEEK(symbol_table.offset.data);

        u32 num_members{};
        FILE_READ(u32, num_members);

        u32 mem_offsets[num_members];
        for (u32 i = 0; i < num_members; ++i)
            FILE_READ(u32&, mem_offsets[i]);

        u32 num_symbols{};
        FILE_READ(u32, num_symbols);

        u16 sym_indexes[num_symbols];
        for (u32 i = 0; i < num_symbols; ++i)
            FILE_READ(u16&, sym_indexes[i]);

        u8 ch{};
        while (num_symbols) {
            FILE_READ(u8, ch);
            if (ch == 0)
                --num_symbols;
        }

        return status_t::ok;
    }

    static status_t parse_long_name_ref(file_t& file, member_t& member) {
        if (member.name[0] != '/' || !isdigit(member.name[1]))
            return status_t::ok;

        auto& module = *file.module;
        auto sc = module.as_archive();

        auto& long_names = sc->members[sc->long_names];

        u32 offset{};
        const auto name_slice = slice::make(member.name.data + 1, 15);
        auto status = numbers::integer::parse(name_slice, 10, offset);
        if (!OK(status))
            return status_t::read_error;

        member.name.data = long_names.buf.data + offset;
        auto p = member.name.data;
        while (*p != '\n') p++;
        member.name.length  = p - member.name.data;

        return status_t::ok;
    }

    namespace internal {
        static u0 fini();

        static status_t read(file_t& file);

        static status_t write(file_t& file);

        static status_t init(alloc_t* alloc);

        io_system_t             g_ar_backend {
            .init                   = init,
            .fini                   = fini,
            .read                   = read,
            .write                  = write,
            .type                   = format_type_t::ar
        };

        struct archive_system_t final {
            alloc_t*                alloc;
        };

        archive_system_t            g_ar_sys{};

        static u0 fini() {
        }

        static status_t read(file_t& file) {
            file.module = system::make_module(module_type_t::archive);

            auto status = file::map_existing(file);
            if (!OK(status))
                return status_t::read_error;

            status = parse_headers(file);
            if (!OK(status))
                return status;

            status = parse_symbol_table(file);
            if (!OK(status))
                return status;

            auto sc = file.module->as_archive();
            for (auto& member : sc->members) {
                status = parse_long_name_ref(file, member);
                if (!OK(status))
                    return status;
            }

            return status_t::ok;
        }

        static status_t write(file_t& file) {
            return status_t::ok;
        }

        static status_t init(alloc_t* alloc) {
            g_ar_sys.alloc = alloc;
            return status_t::ok;
        }
    }

    io_system_t* system() {
        return &internal::g_ar_backend;
    }
}
