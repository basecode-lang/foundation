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

#pragma once

#include <basecode/gfx/gfx.h>
#include <basecode/core/timer.h>
#include <basecode/core/buf_pool.h>

namespace basecode::gfx::msg_stack {
    static inline b8 on_update(timer_t* timer, u0* user);

    inline u0 free(msg_stack_t& stack) {
        timer::stop(stack.timer);
        stack.timer = {};
        stack.sp    = msg_stack_depth;
    }

    inline u0 draw(msg_stack_t& stack) {
        if (stack.sp == msg_stack_depth)
            return;
        auto viewport = ImGui::GetMainViewport();
        auto pos      = viewport->Pos;
        auto draw_list = ImGui::GetForegroundDrawList();
        u32 count = std::min<u32>(msg_stack_depth - stack.sp, 4);
        const auto msg_size = count * (stack.font->FontSize + 4);
        const auto bottom = pos.y + viewport->Size.y - 5;
        const auto top = bottom - (4 * (stack.font->FontSize + 4));
        pos.y = bottom - msg_size;
        draw_list->AddRectFilled(ImVec2(pos.x + 10, top),
                                 ImVec2(viewport->Size.x - 10, bottom),
                                 IM_COL32(0, 0, 0, 200),
                                 12.0f,
                                 ImDrawFlags_RoundCornersAll);
        for (u32 fp = stack.sp; count; ++fp, --count) {
            const auto& entry = stack.data[fp];
            if (!entry.data || entry.alpha == 0)
                continue;
            draw_list->AddText(stack.font,
                               stack.font->FontSize,
                               ImVec2(pos.x + 14, pos.y),
                               (entry.alpha << IM_COL32_A_SHIFT) | entry.color,
                               (const s8*) entry.data,
                               (const s8*) entry.data + entry.length);
            draw_list->AddText(stack.font,
                               stack.font->FontSize,
                               ImVec2(pos.x + 16, pos.y - 2),
                               IM_COL32(0xff, 0xff, 0xff, entry.alpha),
                               (const s8*) entry.data,
                               (const s8*) entry.data + entry.length);
            pos.y += stack.font->FontSize + 4;
        }
    }

    template <String_Concept T>
    u0 push(msg_stack_t& stack,
            const T& msg,
            u32 color = IM_COL32(0xff, 0xff, 0xff, 0x00)) {
        auto& entry = stack.data[--stack.sp];
        entry.data   = buf_pool::retain(msg.length + 1);
        entry.alpha  = 250;
        entry.color  = color;
        entry.length = msg.length;
        std::memcpy(entry.data, msg.data, msg.length);
        entry.data[msg.length] = '\0';
    }

    inline u0 init(msg_stack_t& stack,
                   ImFont* font,
                   s64 ticks,
                   s64 period = MS_TO_NS(70)) {
        std::memset(stack.data, 0, sizeof(msg_t) * msg_stack_depth);
        stack.sp    = msg_stack_depth;
        stack.font  = font;
        stack.timer = timer::start(ticks, period, on_update, &stack);
    }

    static inline b8 on_update(timer_t* timer, u0* user) {
        auto& stack = *((msg_stack_t*) user);
        if (stack.sp == msg_stack_depth)
            return true;
        auto& entry = stack.data[31];
        if (entry.alpha >= 10) {
            entry.alpha -= 10;
        } else {
            buf_pool::release(entry.data);
            entry.data = {};
            if (stack.sp < 31) {
                std::memcpy(&stack.data[stack.sp + 1],
                            &stack.data[stack.sp],
                            (31 - stack.sp) * sizeof(msg_t));
            }
            ++stack.sp;
        }
        return true;
    }
}
