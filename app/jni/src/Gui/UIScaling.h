#pragma once
#include <imgui.h>
#include <algorithm>
#include <SDL.h>
#include <cmath>

namespace UI {
    struct Scaling {
        static float fontScale;
        static float padding;
        static float buttonHeight;
        static float minColumnWidth;
        static float labelWidth;
        static float windowWidth;
        static float windowHeight;
        static float aspectRatio;

        static void UpdateUIScale() {
            #ifdef __ANDROID__
            ImGuiIO& io = ImGui::GetIO();
            windowWidth = io.DisplaySize.x;
            windowHeight = io.DisplaySize.y;
            aspectRatio = windowWidth / windowHeight;

            int displayDensity = 1;
            SDL_DisplayMode displayMode;
            if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0) {
                float diagonalPixels = sqrt(pow(displayMode.w, 2) + pow(displayMode.h, 2));
                if (diagonalPixels > 2500) {
                    displayDensity = 4;
                } else if (diagonalPixels > 2000) {
                    displayDensity = 3;
                } else if (diagonalPixels > 1500) {
                    displayDensity = 2;
                } else {
                    displayDensity = 1;
                }
            }

            float widthScale = windowWidth / 1920.0f;
            float heightScale = windowHeight / 1080.0f;
            fontScale = std::min(widthScale, heightScale) * displayDensity;
            fontScale = std::clamp(fontScale, 0.8f, 2.5f);

            io.FontGlobalScale = fontScale;

            padding = std::max(8.0f * fontScale, 6.0f);
            buttonHeight = std::max(40.0f * fontScale, 35.0f);
            minColumnWidth = std::max(60.0f * fontScale * aspectRatio, 50.0f);
            labelWidth = std::max(100.0f * fontScale * aspectRatio, 80.0f);

            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowPadding = ImVec2(padding, padding);
            style.FramePadding = ImVec2(padding * 0.8f, padding * 0.8f);
            style.ItemSpacing = ImVec2(padding * aspectRatio, padding);
            style.ItemInnerSpacing = ImVec2(padding * aspectRatio, padding);
            style.TouchExtraPadding = ImVec2(padding * 0.4f, padding * 0.4f);

            style.WindowRounding = 4.0f * fontScale;
            style.ScrollbarSize = std::max(20.0f * fontScale, 15.0f);
            style.GrabMinSize = std::max(30.0f * fontScale, 25.0f);
            style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
            style.MouseCursorScale = 1.5f * fontScale;
            style.TabRounding = 4.0f * fontScale;
            style.FrameRounding = 4.0f * fontScale;
            style.ScrollbarRounding = 4.0f * fontScale;
            style.GrabRounding = 4.0f * fontScale;

            if (aspectRatio > 1.5f) {
                style.ItemSpacing.x *= 1.2f;
                style.WindowPadding.x *= 1.2f;
            }
            #endif
        }
    };
}