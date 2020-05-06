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

#include <basecode/core/cxx/cxx.h>
#include <basecode/core/memory/system/proxy.h>

namespace basecode::cxx::module {
    u0 free(module_t& module) {
        module.root_scope_idx = {};
        for (auto& scope : module.scopes)
            scope::free(scope);
        array::free(module.scopes);
    }

    status_t finalize(module_t& module) {
        for (auto& scope : module.scopes) {
            auto status = scope::finalize(scope);
            if (!OK(status))
                return status;
        }
        return status_t::ok;
    }

    scope_t& get_scope(module_t& module, u32 scope_idx) {
        return module.scopes[scope_idx];
    }

    u0 init(module_t& module, program_t& pgm, str::slice_t& filename, cxx::revision_t rev, alloc_t* alloc) {
        module.program = &pgm;
        module.revision = rev;

        cursor_t cursor{};
        bass::seek_current(pgm.storage, cursor);
        bass::new_record(cursor, element::header::module, 4);
        bass::write_field(cursor, element::field::parent, pgm.id);
        bass::write_field(cursor, element::field::revision, (u32) rev);
        module.id = cursor.id;

        array::init(module.scopes, alloc);

        auto& root = array::append(module.scopes);
        root.idx = module.scopes.size - 1;
        module.root_scope_idx = root.idx;

        scope::init(&pgm, &module, root, {}, alloc);

        module.filename_id = scope::lit::str(root, filename);
        bass::write_field(cursor, element::field::lit, module.filename_id);
        bass::write_field(cursor, element::field::child, root.id);
    }
}
