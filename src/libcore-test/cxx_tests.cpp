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
#include <basecode/core/cxx/types.h>
#include <basecode/core/profiler/system.h>
#include <basecode/core/profiler/stopwatch.h>
#include <basecode/core/memory/proxy_system.h>
#include <basecode/core/memory/dlmalloc_system.h>

using namespace basecode;
using namespace basecode::cxx;

TEST_CASE("basecode::cxx create program_t") {
    stopwatch_t build_time{};
    stopwatch::start(build_time);

    cxx::program_t pgm{};
    cxx::program::init(pgm);
    defer({
        cxx::program::free(pgm);
        memory::proxy::reset();
    });
    REQUIRE(pgm.storage.alloc);
    REQUIRE(pgm.modules.alloc);
    REQUIRE(pgm.storage.index.alloc);
    REQUIRE(pgm.storage.index.size == 1);
    REQUIRE(pgm.modules.size == 0);
    REQUIRE(pgm.intern.alloc);

    stopwatch::stop(build_time);
    stopwatch::print_elapsed("total build time"_ss, 40, stopwatch::elapsed(build_time));
}

TEST_CASE("basecode::cxx create module_t") {
    stopwatch_t build_time{};
    stopwatch::start(build_time);

    cxx::program_t pgm{};
    cxx::program::init(pgm);
    defer({
        cxx::program::free(pgm);
        memory::proxy::reset();
    });

    const auto expected_filename = "test.cpp"_ss;
    const auto expected_revision = cxx::revision_t::cpp20;

    auto& mod = cxx::program::add_module(pgm, expected_filename, expected_revision);
    REQUIRE(mod.id != 0);
    REQUIRE(mod.filename_id != 0);
    REQUIRE(mod.revision == expected_revision);
    REQUIRE(mod.program == &pgm);

    const auto& root_scope = module::get_scope(mod, mod.root_scope_idx);
    REQUIRE(root_scope.id != 0);
    REQUIRE(root_scope.parent_idx == 0);
    REQUIRE(root_scope.stack.alloc);
    REQUIRE(root_scope.stack.size == 0);
    REQUIRE(root_scope.types.alloc);
    REQUIRE(root_scope.types.size == 0);
    REQUIRE(root_scope.labels.alloc);
    REQUIRE(root_scope.labels.size == 0);
    REQUIRE(root_scope.children.alloc);
    REQUIRE(root_scope.children.size == 0);
    REQUIRE(root_scope.statements.alloc);
    REQUIRE(root_scope.statements.size == 0);
    REQUIRE(root_scope.identifiers.alloc);
    REQUIRE(root_scope.identifiers.size == 0);

    stopwatch::stop(build_time);
    stopwatch::print_elapsed("total build time"_ss, 40, stopwatch::elapsed(build_time));
}

TEST_CASE("basecode::cxx create identifier within scope") {
    stopwatch_t build_time{};
    stopwatch::start(build_time);

    cxx::program_t pgm{};
    cxx::program::init(pgm);
    defer({
        cxx::program::free(pgm);
        memory::proxy::reset();
    });

    const auto expected_filename = "test.cpp"_ss;
    const auto expected_revision = cxx::revision_t::cpp20;

    auto& mod = cxx::program::add_module(pgm, expected_filename, expected_revision);
    auto& top_level = module::get_scope(mod, mod.root_scope_idx);

    const auto expected_ident = "int"_ss;

    auto id = cxx::scope::expr::ident(top_level, expected_ident);
    REQUIRE(id != 0);

    s32 idx = array::contains(top_level.identifiers, id);
    REQUIRE(idx != -1);

    auto intern_result = intern::get(pgm.intern, top_level.interns[idx]);
    REQUIRE(OK(intern_result.status));
    REQUIRE(intern_result.id == top_level.interns[idx]);
    REQUIRE(intern_result.slice == expected_ident);

    stopwatch::stop(build_time);
    stopwatch::print_elapsed("total build time"_ss, 40, stopwatch::elapsed(build_time));
}

TEST_CASE("basecode::cxx declare s32 type within scope") {
    stopwatch_t build_time{};
    stopwatch::start(build_time);

    cxx::program_t pgm{};
    cxx::program::init(pgm);
    defer({
        cxx::program::free(pgm);
        memory::proxy::reset();
    });

    const auto expected_ident = "int"_ss;
    const auto expected_filename = "test.cpp"_ss;
    const auto expected_revision = cxx::revision_t::cpp20;

    auto& mod = cxx::program::add_module(pgm, expected_filename, expected_revision);
    auto& top_level = module::get_scope(mod, mod.root_scope_idx);

    auto int_ident_id = cxx::scope::expr::ident(top_level, expected_ident);
    auto int_type_id = cxx::scope::type::s32_(top_level, int_ident_id);
    REQUIRE(int_type_id != 0);

    stopwatch::stop(build_time);
    stopwatch::print_elapsed("total build time"_ss, 40, stopwatch::elapsed(build_time));
}

TEST_CASE("basecode::cxx example program") {
    alloc_t region_alloc{};
    dl_config_t region_config{};
    region_config.heap_size = 512 * 1024;
    memory::init(&region_alloc, alloc_type_t::dlmalloc, &region_config);
    defer({
        memory::release(&region_alloc, false);
        memory::proxy::reset(false);
    });
    auto region_proxy = memory::proxy::make(&region_alloc, "512kb region"_ss);

    stopwatch_t build_time{};
    stopwatch::start(build_time);

    cxx::program_t pgm{};
    cxx::program::init(pgm, region_proxy);
    defer(cxx::program::free(pgm));

    const auto expected_main_ident = "main"_ss;
    const auto expected_filename = "test.cpp"_ss;
    const auto expected_int_ident = "int"_ss;
    const auto expected_char_ident = "char"_ss;
    const auto expected_revision = cxx::revision_t::cpp20;

    auto& mod = cxx::program::add_module(pgm, expected_filename, expected_revision);
    auto& top_level = module::get_scope(mod, mod.root_scope_idx);

    auto int_ident_id       = scope::expr::ident(top_level, expected_int_ident);
    auto int_type_id        = scope::type::s32_(top_level, int_ident_id);
    auto char_ident_id      = scope::expr::ident(top_level, expected_char_ident);
    auto char_type_id       = scope::type::s8_(top_level, char_ident_id);
    auto char_ptr_id        = scope::type::ptr(top_level, char_type_id);
    auto char_ptr_ptr_id    = scope::type::ptr(top_level, char_ptr_id);

    //
    // int main(int argc, const char** argv) {
    //    int x{};
    //    int y = 10;
    //    x = 50;
    //    return x * y;
    // }
    auto main_ident_id = scope::expr::ident(top_level, expected_main_ident);
    auto& main_scope = module::get_scope(mod, scope::push(top_level));

    auto x_ident_id = scope::expr::ident(main_scope, "x"_ss);
    auto y_ident_id = scope::expr::ident(main_scope, "y"_ss);

    auto lit_10_id = scope::lit::signed_(
        main_scope,
        10,
        cxx::integral_size_t::dword);
    auto x_var_id = scope::expr::var(main_scope, int_type_id, x_ident_id);
    auto y_var_id = scope::expr::var(
        main_scope,
        int_type_id,
        y_ident_id,
        scope::expr::init::direct(main_scope, lit_10_id));
    scope::stmt::decl(main_scope, x_var_id);
    scope::stmt::decl(main_scope, y_var_id);

    auto lit_50_id = cxx::scope::lit::signed_(
        main_scope,
        50,
        cxx::integral_size_t::dword);
    scope::stmt::expr(
        main_scope,
        scope::expr::assign::direct(main_scope, x_var_id, lit_50_id));

    auto& true_scope = module::get_scope(mod, scope::push(main_scope));
    {
        scope::stmt::return_(true_scope, y_var_id);
        scope::pop(main_scope);
    }
    auto& false_scope = module::get_scope(mod, scope::push(main_scope));
    auto& true_scope1 = module::get_scope(mod, scope::push(false_scope));
    auto else_if_id = scope::stmt::if_(
        false_scope,
        scope::expr::binary::eq(false_scope, x_var_id, lit_10_id),
        true_scope1.id);
    scope::stmt::return_(true_scope1, x_var_id);
    scope::pop(false_scope);

    scope::stmt::if_(
        main_scope,
        scope::expr::binary::eq(true_scope, y_var_id, lit_50_id),
        true_scope.id,
        else_if_id);
    scope::pop(main_scope);

    scope::stmt::raw(main_scope, R"(printf("%d, %d\n", x, y);)"_ss);
    scope::stmt::empty(main_scope);
    auto label_id = scope::label(main_scope, "all_done"_ss);
    scope::stmt::return_(
        main_scope,
        scope::expr::binary::mul(main_scope, x_var_id, y_var_id),
        label_id);

    u32 params_list[] = {
        scope::expr::var(
            main_scope,
            int_type_id,
            scope::expr::ident(main_scope, "argc"_ss),
            0,
            cxx::var::none),
        scope::expr::var(
            main_scope,
            char_ptr_ptr_id,
            scope::expr::ident(main_scope, "argv"_ss),
            0,
            cxx::var::const_)
    };
    auto main_params_list_id = scope::expr::list(top_level, params_list, 2);
    scope::stmt::comment::line(top_level, " this is a test comment"_ss);
    scope::stmt::empty(top_level);
    scope::stmt::pp::include_system(top_level, "cstdio"_ss);
    scope::stmt::empty(top_level);
    scope::stmt::def(
        top_level,
        scope::type::func(
            top_level,
            main_scope.id,
            int_type_id,
            main_ident_id,
            main_params_list_id));
    scope::pop(top_level);

    REQUIRE(OK(cxx::program::finalize(pgm)));

    stopwatch::stop(build_time);
    stopwatch::print_elapsed("total build time"_ss, 40, stopwatch::elapsed(build_time));

    stopwatch_t serialize_time{};
    stopwatch::start(serialize_time);

    cxx::serializer_t s{};
    cxx::serializer::init(s, pgm, region_proxy);

    auto status = cxx::serializer::serialize(s);
    stopwatch::stop(serialize_time);
    stopwatch::print_elapsed("total serialize time"_ss, 40, stopwatch::elapsed(serialize_time));

    format::print(stderr, "{}\n", program::status_name(status));

    auto proxies = memory::proxy::active();
    defer(array::free(proxies));
    for (auto proxy : proxies) {
        format::print(stderr, "{:<32} {:>10}\n", memory::proxy::name(proxy), proxy->total_allocated);
    }

    REQUIRE(OK(status));
}
