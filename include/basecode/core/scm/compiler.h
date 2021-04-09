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

#include <basecode/core/scm/vm.h>
#include <basecode/core/scm/bytecode.h>

namespace basecode::scm::compiler {
    u0 free(compiler_t& comp);

    u0 reset(compiler_t& comp);

    u0 init(compiler_t& comp, vm_t* vm, alloc_t* alloc);

    compile_result_t compile(compiler_t& comp, const context_t& c);

    context_t make_context(bb_t& bb, ctx_t* ctx, obj_t* obj, obj_t* env, b8 top_level = false);
}
