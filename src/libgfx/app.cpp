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

#include <thread>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <imgui_freetype.h>
#include <basecode/gfx/app.h>
#include <basecode/core/timer.h>
#include <basecode/core/profiler.h>
#include <basecode/core/memory/meta.h>
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

    static u0 glfw_win_iconify(GLFWwindow* window, s32 iconified);

    static u0 glfw_win_maximize(GLFWwindow* window, s32 maximized);

    static u0 glfw_win_resize(GLFWwindow* window, s32 width, s32 height);

    u0 free(app_t& app) {
        str::free(app.scratch);
    }

    status_t run(app_t& app) {
#ifdef _WIN32
        SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#endif
        glfwSetErrorCallback(glfw_error);
        if (!glfwInit())
            return status_t::glfw_init_failure;

#if defined(IMGUI_IMPL_OPENGL_ES2)
        const s8* glsl_version = "#version 100";
#elif __APPLE__
        const s8* glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
        const s8* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

        glfwWindowHint(GLFW_MAXIMIZED, app.window.maximized != 0);
        app.window.backing = glfwCreateWindow(app.window.width,
                                              app.window.height,
                                              "Temp Name",
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
        //io.IniFilename = _ini_path.string().c_str();
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

        io.FontDefault = io.Fonts->AddFontFromFileTTF("../share/fonts/SEGOEUI.TTF",
                                                      24);
        ImGuiFreeType::BuildFontAtlas(io.Fonts);

        s32 dw{};
        s32 dh{};

        while (!glfwWindowShouldClose(app.window.backing)) {
            glfwPollEvents();

            if (glfwGetWindowAttrib(app.window.backing, GLFW_ICONIFIED)) {
                std::this_thread::yield();
                continue;
            }

            const auto ticks = profiler::get_time()
                               * profiler::calibration_mult();
            timer::update(ticks);
            memory::meta::system::update(ticks);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            if (app.on_render) {
                if (!app.on_render())
                    break;
            }
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

    static u0 glfw_win_close(GLFWwindow* window) {
        UNUSED(window);
    }

    static u0 glfw_error(s32 error, const s8* description) {
        UNUSED(error);
        UNUSED(description);
    }

    static u0 glfw_win_pos(GLFWwindow* window, s32 x, s32 y) {
        UNUSED(window);
        UNUSED(x);
        UNUSED(y);
    }

    static u0 glfw_win_focus(GLFWwindow* window, s32 focused) {
        UNUSED(window);
        UNUSED(focused);
    }

    static u0 glfw_win_iconify(GLFWwindow* window, s32 iconified) {
        UNUSED(window);
        UNUSED(iconified);
    }

    static u0 glfw_win_maximize(GLFWwindow* window, s32 maximized) {
        UNUSED(window);
        UNUSED(maximized);
    }

    static u0 glfw_win_resize(GLFWwindow* window, s32 width, s32 height) {
        UNUSED(window);
        UNUSED(width);
        UNUSED(height);
    }
}
