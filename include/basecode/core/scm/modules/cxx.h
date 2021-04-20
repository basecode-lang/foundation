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

#include <basecode/core/log.h>
#include <basecode/core/error.h>
#include <basecode/core/string.h>
#include <basecode/core/cxx/cxx.h>
#include <basecode/core/scm/kernel.h>

namespace basecode::scm::module::cxx {
    enum class status_t : u32 {
        ok,
    };

    //
    // top-level api:
    // (cxx/make-program)                           -> program_t*
    // (cxx/add-module pgm file_name 'cpp20)        -> module_t*
    // (cxx/get-module pgm id)                      -> module_t*
    //
    // 'size can be: 'zero, 'byte, 'word, 'dword, 'qword
    // (cxx/integral/size-in-bits 'size)            -> u32
    // (cxx/integral/size-in-bytes 'size)           -> u32
    //
    // module api:
    // (cxx/get-scope mod idx)                      -> scope_t*
    //
    // scope api:
    // (cxx/pop-scope scope)
    // (cxx/push-scope scope)                       -> u32
    // (cxx/make-label scope name)                  -> u32
    //
    // (cxx/unary/-         scope expr_id)               -> u32
    // (cxx/unary/~         scope expr_id)               -> u32
    // (cxx/unary/!         scope expr_id)               -> u32
    // (cxx/unary/*         scope expr_id)               -> u32
    // (cxx/unary/&         scope expr_id)               -> u32
    // (cxx/unary/&&        scope expr_id)               -> u32
    // (cxx/unary/++        scope expr_id [#:prefix #t]) -> u32
    // (cxx/unary/--        scope expr_id [#:prefix #t]) -> u32
    //
    // (cxx/init/=          scope expr_id)               -> u32
    // (cxx/init/{}         scope . expr_ids)            -> u32
    //
    // (cxx/assign/+=       scope lhs_id rhs_id)         -> u32
    // (cxx/assign/<<=      scope lhs_id rhs_id)         -> u32
    // (cxx/assign/>>=      scope lhs_id rhs_id)         -> u32
    // (cxx/assign/|=       scope lhs_id rhs_id)         -> u32
    // (cxx/assign/^=       scope lhs_id rhs_id)         -> u32
    // (cxx/assign/-=       scope lhs_id rhs_id)         -> u32
    // (cxx/assign/*=       scope lhs_id rhs_id)         -> u32
    // (cxx/assign//=       scope lhs_id rhs_id)         -> u32
    // (cxx/assign/%=       scope lhs_id rhs_id)         -> u32
    // (cxx/assign/=        scope lhs_id rhs_id)         -> u32
    //
    // (cxx/binary/==       scope lhs_id rhs_id)         -> u32
    // (cxx/binary/<        scope lhs_id rhs_id)         -> u32
    // (cxx/binary/*        scope lhs_id rhs_id)         -> u32
    // (cxx/binary/!=       scope lhs_id rhs_id)         -> u32
    // (cxx/binary/<=       scope lhs_id rhs_id)         -> u32
    // (cxx/binary/>=       scope lhs_id rhs_id)         -> u32
    // (cxx/binary/+        scope lhs_id rhs_id)         -> u32
    // (cxx/binary/-        scope lhs_id rhs_id)         -> u32
    // (cxx/binary//        scope lhs_id rhs_id)         -> u32
    // (cxx/binary/%        scope lhs_id rhs_id)         -> u32
    // (cxx/binary/<<       scope lhs_id rhs_id)         -> u32
    // (cxx/binary/>>       scope lhs_id rhs_id)         -> u32
    // (cxx/binary/|        scope lhs_id rhs_id)         -> u32
    // (cxx/binary/||       scope lhs_id rhs_id)         -> u32
    // (cxx/binary/(cast)   scope lhs_id rhs_id)         -> u32
    // (cxx/binary/&        scope lhs_id rhs_id)         -> u32
    // (cxx/binary/^        scope lhs_id rhs_id)         -> u32
    // (cxx/binary/&&       scope lhs_id rhs_id)         -> u32
    // (cxx/binary/,        scope lhs_id rhs_id)         -> u32
    // (cxx/binary/:        scope lhs_id rhs_id)         -> u32
    // (cxx/binary/.        scope lhs_id rhs_id)         -> u32
    // (cxx/binary/::       scope lhs_id rhs_id)         -> u32
    // (cxx/binary/[]       scope lhs_id rhs_id)         -> u32

    // (cxx/expr/raw        scope source_str)            -> u32
    // (cxx/expr/ident      scope name)                  -> u32
    // (cxx/expr/list       scope . id)                  -> u32
    // (cxx/expr/var        scope type_id ident_id
    //                      init_expr_id flags)          -> u32
    //
    // (cxx/type/*          scope type_id)               -> u32
    // (cxx/type/u0         scope ident_id)              -> u32
    // (cxx/type/s8         scope ident_id)              -> u32
    // (cxx/type/u8         scope ident_id)              -> u32
    // (cxx/type/b8         scope ident_id)              -> u32
    // (cxx/type/&          scope ident_id)              -> u32
    // (cxx/type/f32        scope ident_id)              -> u32
    // (cxx/type/f64        scope ident_id)              -> u32
    // (cxx/type/s16        scope ident_id)              -> u32
    // (cxx/type/u16        scope ident_id)              -> u32
    // (cxx/type/s32        scope ident_id)              -> u32
    // (cxx/type/u32        scope ident_id)              -> u32
    // (cxx/type/s64        scope ident_id)              -> u32
    // (cxx/type/u64        scope ident_id)              -> u32
    // (cxx/type/bit-field  scope type_id bits)          -> u32
    // (cxx/type/[]         scope type_id size)          -> u32
    // (cxx/type/enum       scope block_id ident_id
    //                      flags)                       -> u32
    // (cxx/type/class      scope block_id ident_id
    //                      flags)                       -> u32
    // (cxx/type/union      scope block_id ident_id
    //                      flags)                       -> u32
    // (cxx/type/struct     scope block_id ident_id
    //                      flags)                       -> u32
    // (cxx/type/enum-class scope block_id ident_id
    //                      flags)                       -> u32
    // (cxx/type/func       scope block_id ret_type_id
    //                      ident_id params_list_id)     -> u32
    //
    // (cxx/lit/char        scope char)                  -> u32
    // (cxx/lit/string      scope str)                   -> u32
    // (cxx/lit/f32         scope value)                 -> u32
    // (cxx/lit/f64         scope value)                 -> u32
    // (cxx/lit/signed      scope value size radix)      -> u32
    // (cxx/lit/unsigned    scope value size radix)      -> u32
    //
    //
    // (cxx/stmt/pragma     scope raw)                   -> u32
    // (cxx/stmt/include    scope path)                  -> u32
    // (cxx/stmt/include<>  scope path)                  -> u32
    //
    // type: 'line, 'block
    // (cxx/stmt/comment    scope type value)            -> u32
    //
    // (cxx/stmt/if         scope predicate_id
    //                      true_expr_id false_expr_id
    //                      label_id)                    -> u32
    //
    // (cxx/stmt/for        scope predicate_id
    //                      expr_id init_expr_id
    //                      post_expr_id label_id)       -> u32
    //
    // (cxx/stmt/public     scope)                       -> u32
    // (cxx/stmt/private    scope)                       -> u32
    // (cxx/stmt/protected  scope)                       -> u32
    // (cxx/stmt/define     scope expr_id)               -> u32
    // (cxx/stmt/using-namespace scope expr_id)          -> u32
    // (cxx/stmt/empty      scope [#:label-id v])        -> u32
    // (cxx/stmt/break      scope [#:label-id v])        -> u32
    // (cxx/stmt/raw        scope source_str)            -> u32
    // (cxx/stmt/continue   scope [#:label-id v])        -> u32
    // (cxx/stmt/using      scope ident_id type_id)      -> u32
    // (cxx/stmt/expr       scope expr-id [#:label-id v])-> u32
    // (cxx/stmt/decl       scope expr_id [#:label-id v])-> u32
    // (cxx/stmt/goto       scope expr_id [#:label-id v])-> u32
    // (cxx/stmt/return     scope
    //                      [#:expr-id v]
    //                      [#:label-id v])              -> u32
    // (cxx/stmt/default    scope
    //                      [#:expr-id v]
    //                      [#:label-id v])              -> u32
    // (cxx/stmt/do         scope
    //                      predicate-id
    //                      expr-id [#:label-id v])      -> u32
    // (cxx/stmt/case       scope
    //                      predicate-id
    //                      expr-id
    //                      [#:label-id v])              -> u32
    // (cxx/stmt/while      scope
    //                      predicate-id
    //                      expr-id
    //                      [#:label-id v])              -> u32
    // (cxx/stmt/switch     scope
    //                      predicate-id
    //                      expr-id
    //                      [#:label-id v])              -> u32
    //
    //
    //
    //
    //
    //
    //
    // serialize api:
    // (cxx/serialize pgm output)               -> status
    //
    namespace system {
        u0 fini();

        status_t init(scm::ctx_t* ctx,
                      alloc_t* alloc = context::top()->alloc.main);
    }
}
