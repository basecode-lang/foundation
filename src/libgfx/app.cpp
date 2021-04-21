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

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <imgui_freetype.h>
#include <basecode/gfx/app.h>
#include <basecode/core/log.h>
#include <basecode/core/timer.h>
#include <basecode/core/filesys.h>
#include <basecode/core/profiler.h>
#include <basecode/core/scm/types.h>
#include <basecode/core/scm/system.h>
#include <basecode/core/memory/meta.h>
#include <basecode/core/scm/modules/config.h>
#include <basecode/gfx/imgui/imgui_impl_glfw.h>
#include <basecode/gfx/imgui/imgui_impl_opengl3.h>

#ifdef _WIN32
#   include <shellscalingapi.h>
#endif

namespace basecode::app {
    static u0 glfw_key(GLFWwindow* window,
                       s32 key,
                       s32 scan_code,
                       s32 action,
                       s32 mods);

    static u0 glfw_win_close(GLFWwindow* window);

    static u0 glfw_error(s32 error, const s8* description);

    static u0 glfw_win_pos(GLFWwindow* window, s32 x, s32 y);

    static u0 glfw_win_focus(GLFWwindow* window, s32 focused);

    static status_t get_config_path(app_t& app, path_t& path);

    static u0 glfw_win_iconify(GLFWwindow* window, s32 iconified);

    static u0 glfw_win_maximize(GLFWwindow* window, s32 maximized);

    static status_t get_config_file_path(app_t& app, path_t& path);

    static u0 glfw_win_resize(GLFWwindow* window, s32 width, s32 height);

    u0 free(app_t& app) {
        str::free(app.scratch);
    }

    status_t run(app_t& app) {
        BC_ASSERT_NOT_NULL(app.on_render);

        if (!OK(load_config(app))) {
            log::error("load_config failed");
            return status_t::load_config_error;
        }

#ifdef _WIN32
        SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#endif

        glfwSetErrorCallback(glfw_error);
        if (!glfwInit())
            return status_t::glfw_init_failure;

#if defined(IMGUI_IMPL_OPENGL_ES2)
        const s8* glsl_version = "#version 100";
#else
        const s8* glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
#endif

        glfwWindowHint(GLFW_MAXIMIZED, app.window.maximized != 0);
        app.window.backing = glfwCreateWindow(app.window.width,
                                              app.window.height,
                                              (const s8*) app.title.data,
                                              {},
                                              {});
        if (!app.window.backing)
            return status_t::glfw_init_failure;

        glfwSetWindowUserPointer(app.window.backing, &app.window);
        glfwSetWindowSizeLimits(app.window.backing,
                                app.window.min_width,
                                app.window.min_height,
                                GLFW_DONT_CARE,
                                GLFW_DONT_CARE);
        glfwSetKeyCallback(app.window.backing, glfw_key);
        glfwSetWindowPosCallback(app.window.backing, glfw_win_pos);
        glfwSetWindowSizeCallback(app.window.backing, glfw_win_resize);
        glfwSetWindowFocusCallback(app.window.backing, glfw_win_focus);
        glfwSetWindowCloseCallback(app.window.backing, glfw_win_close);
        glfwSetWindowIconifyCallback(app.window.backing, glfw_win_iconify);
        glfwSetWindowMaximizeCallback(app.window.backing, glfw_win_maximize);
        glfwMakeContextCurrent(app.window.backing);
        glfwSwapInterval(1);

        if (gl3wInit() != 0)
            return status_t::gl3w_init_failure;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGui::StyleColorsClassic();
        auto& io = ImGui::GetIO();

        {
            path_t config_path{};
            path::init(config_path);
            if (OK(get_config_path(app, config_path))) {
                path_t ini_file{};
                path::init(ini_file, "imgui.ini"_ss);
                path::append(config_path, ini_file);
                io.IniFilename = path::c_str(config_path);
                path::free(ini_file);
            }
            path::free(config_path);
        }

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();
        auto& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        ImGui_ImplGlfw_InitForOpenGL(app.window.backing, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        io.FontDefault = io.Fonts->AddFontFromFileTTF(
            "../share/fonts/SEGOEUI.TTF",
            PT_TO_PX(18));

        ImWchar icons_range[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
        ImFontConfig icons_config;
        icons_config.MergeMode  = true;
        icons_config.PixelSnapH = true;
        io.Fonts->AddFontFromFileTTF(
            "../share/fonts/" FONT_ICON_FILE_NAME_FAS,
            PT_TO_PX(18),
            &icons_config,
            icons_range);

        ImGuiFreeType::BuildFontAtlas(io.Fonts);

        s32 dw{};
        s32 dh{};
        f64 timer_mult = profiler::calibration_mult();

        while (!glfwWindowShouldClose(app.window.backing)) {
            glfwPollEvents();

            const auto ticks = profiler::get_time() * timer_mult;
            timer::update(ticks);
            memory::meta::system::update(io.DeltaTime);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            if (!app.on_render(app))
                break;
            ImGui::Render();

            glfwGetFramebufferSize(app.window.backing, &dw, &dh);
            glViewport(0, 0, dw, dh);
            glClearColor(app.bg_color.x,
                         app.bg_color.y,
                         app.bg_color.z,
                         app.bg_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                auto backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }

            glfwSwapBuffers(app.window.backing);
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        if (!OK(save_config(app))) {
            log::error("save_config failed");
        }

        glfwDestroyWindow(app.window.backing);
        glfwTerminate();

        return status_t::ok;
    }

    static u0 glfw_key(GLFWwindow* window,
                       s32 key,
                       s32 scan_code,
                       s32 action,
                       s32 mods) {
        UNUSED(window);
        UNUSED(key);
        UNUSED(scan_code);
        UNUSED(action);
        UNUSED(mods);
    }

    status_t init(app_t& app, alloc_t* alloc) {
        app.alloc     = alloc;
        app.bg_color  = {0.45f, 0.55f, 0.60f, 1.00f};
        str::init(app.scratch, app.alloc);
        str::reserve(app.scratch, 64);
        return status_t::ok;
    }

    status_t load_config(app_t& app) {
        path_t path{};
        path::init(path);
        defer(path::free(path)); {
            auto status = get_config_file_path(app, path);
            if (!OK(status))
                return status;
        }
        scm::obj_t* obj{};
        auto status = scm::system::eval(path, &obj);
        if (!OK(status))
            return status_t::load_config_error;
        cvar_t* var{};
        if (OK(config::cvar::get("*gfx-win-x*"_ss, &var))) {
            app.window.x = var->value.integer;
        }
        if (OK(config::cvar::get("*gfx-win-y*"_ss, &var))) {
            app.window.y = var->value.integer;
        }
        if (OK(config::cvar::get("*gfx-win-width*"_ss, &var))) {
            app.window.width = var->value.integer;
        }
        if (OK(config::cvar::get("*gfx-win-height*"_ss, &var))) {
            app.window.height = var->value.integer;
        }
        if (OK(config::cvar::get("*gfx-win-maximized*"_ss, &var))) {
            app.window.maximized = var->value.flag;
        }
        if (OK(config::cvar::get("*gfx-win-min-width*"_ss, &var))) {
            app.window.min_width = var->value.integer;
        }
        if (OK(config::cvar::get("*gfx-win-min-height*"_ss, &var))) {
            app.window.min_height = var->value.integer;
        }
        return status_t::ok;
    }

    status_t save_config(app_t& app) {
        path_t path{};
        path::init(path);
        defer(path::free(path));
        auto status = get_config_file_path(app, path);
        if (!OK(status))
            return status;
        auto source = format::format(R"(
    (begin
        (define *gfx-win-x*             {})
        (define *gfx-win-y*             {})
        (define *gfx-win-width*         {})
        (define *gfx-win-height*        {})
        (define *gfx-win-maximized*     {})
        (define *gfx-win-min-width*     {})
        (define *gfx-win-min-height*    {}))
)",
        app.window.x,
        app.window.y,
        app.window.width,
        app.window.height,
        app.window.maximized ? "#t" : "#f",
        app.window.min_width,
        app.window.min_height);
        auto file = fopen(path::c_str(path), "w");
        defer(fclose(file));
        format::print(file, "{}", source);
        return status_t::ok;
    }

    static u0 glfw_win_close(GLFWwindow* window) {
        log::info("glfw window close");
    }

    static u0 glfw_error(s32 error, const s8* description) {
        log::error("glfw error: {} ({})", description, error);
    }

    static u0 glfw_win_pos(GLFWwindow* window, s32 x, s32 y) {
        auto win = (window_t*) glfwGetWindowUserPointer(window);
        win->x = x;
        win->y = y;
    }

    static u0 glfw_win_focus(GLFWwindow* window, s32 focused) {
        auto win = (window_t*) glfwGetWindowUserPointer(window);
        win->focused = true;
    }

    static status_t get_config_path(app_t& app, path_t& path) {
        auto status = filesys::places::user::config(path);
        if (!OK(status))
            return status_t::save_config_error;
        path_t product_path{};
        path::init(product_path, app.short_name);
        path::append(path, product_path);
        path::free(product_path);
        if (!OK(filesys::exists(path))) {
            if (!OK(filesys::_mkdir(path, true)))
                return status_t::save_config_error;
        }
        return status_t::ok;
    }

    static u0 glfw_win_iconify(GLFWwindow* window, s32 iconified) {
        auto win = (window_t*) glfwGetWindowUserPointer(window);
        win->focused   = false;
        win->iconified = true;
    }

    static u0 glfw_win_maximize(GLFWwindow* window, s32 maximized) {
        auto win = (window_t*) glfwGetWindowUserPointer(window);
        win->maximized = true;
        win->iconified = false;
    }

    static status_t get_config_file_path(app_t& app, path_t& path) {
        auto status = get_config_path(app, path);
        if (!OK(status))
            return status;
        path_t config_file{};
        path::init(config_file, "gfx.scm"_ss);
        path::append(path, config_file);
        path::free(config_file);
        return status_t::ok;
    }

    static u0 glfw_win_resize(GLFWwindow* window, s32 width, s32 height) {
        auto win = (window_t*) glfwGetWindowUserPointer(window);
        win->width = width;
        win->height = height;
    }
}
