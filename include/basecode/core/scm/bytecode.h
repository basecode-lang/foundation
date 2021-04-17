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
#include <basecode/core/scm/emitter.h>
#include <basecode/core/scm/basic_block.h>

namespace basecode::scm::bytecode {
    bb_t& leave(bb_t& bb);

    bb_t& enter(bb_t& bb, u32 locals);

    u0 free_stack(bb_t& bb, u32 words);

    u0 todo(bb_t& bb, str::slice_t msg);

    compile_result_t ffi(compiler_t& comp,
                         const context_t& c,
                         obj_t* sym,
                         obj_t* form,
                         obj_t* args);

    compile_result_t prim(compiler_t& comp,
                          const context_t& c,
                          obj_t* sym,
                          obj_t* form,
                          obj_t* args,
                          prim_type_t type);

    compile_result_t apply(compiler_t& comp,
                           const context_t& c,
                           obj_t* sym,
                           obj_t* form,
                           obj_t* args);

    compile_result_t lambda(compiler_t& comp,
                            const context_t& c,
                            obj_t* form,
                            obj_t* args);

    compile_result_t cmp_op(compiler_t& comp,
                            const context_t& c,
                            prim_type_t type,
                            obj_t* args);

    compile_result_t arith_op(compiler_t& comp,
                              const context_t& c,
                              op_code_t op_code,
                              obj_t* args);

    compile_result_t call_back(compiler_t& comp,
                               const context_t& c,
                               obj_t* sym,
                               obj_t* form,
                               obj_t* args);

    compile_result_t comp_proc(compiler_t& comp,
                               const context_t& c,
                               proc_t* proc);

    compile_result_t define_macro(compiler_t& comp,
                                  const context_t& c,
                                  obj_t* args);

    u0 alloc_stack(bb_t& bb, u32 words, var_t** base_addr);

    compile_result_t lookup(compiler_t& comp, const context_t& c);

    compile_result_t self_eval(compiler_t& comp, const context_t& c);

    compile_result_t qt(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t qq(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t uq(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t uqs(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t car(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t cdr(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t or_(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t if_(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t set(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t atom(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t eval(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t list(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t cons(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t and_(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t not_(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t begin(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t error(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t while_(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t define(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t set_car(compiler_t& comp, const context_t& c, obj_t* args);

    compile_result_t set_cdr(compiler_t& comp, const context_t& c, obj_t* args);
}
