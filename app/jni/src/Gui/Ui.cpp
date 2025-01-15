#include "ui.hpp"
#include "../Chipset/Chipset.hpp"
#include "../Chipset/MMU.hpp"
#include "CallAnalysis.h"
#include "CasioData.h"
#include "CodeViewer.hpp"
#include "Editors.h"
#include "HwController.h"
#include "Injector.hpp"
#include "LabelViewer.h"
#include "MemBreakpoint.hpp"
#include "VariableWindow.h"
#include "WatchWindow.hpp"
#include "AddressWindow.h"
#include "UIScaling.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include <SDL.h>
#include <filesystem>
#include <cstdlib>

char* n_ram_buffer = 0;
casioemu::MMU* me_mmu = 0;

static SDL_Window* window = 0;
SDL_Renderer* renderer = 0;

CodeViewer* code_viewer = 0;
Injector* injector = 0;
MemBreakPoint* membp = 0;

std::vector<UIWindow*> windows{};

static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

ImVec2 CalculateButtonSize() {
    float width = UI::Scaling::buttonHeight * 3.0f;  // Tăng kích thước nút
    float height = UI::Scaling::buttonHeight * 1.2f; // Tăng chiều cao nút
    return ImVec2(width, height);
}

ImVec2 CalculateComboSize() {
    float width = UI::Scaling::windowWidth * 0.4f;  // Tăng kích thước combo box
    float height = UI::Scaling::buttonHeight * 1.2f;
    return ImVec2(width, height);
}

const ImWchar* GetPua() {
    static const ImWchar ranges[] = {
        0xE000, 0xE900, // PUA
        0,
    };
    return &ranges[0];
}

const ImWchar* GetKanji() {
    static const ImWchar ranges[] = {
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0xFFFD, 0xFFFD, // Invalid
        0x4e00, 0x9FAF, // CJK Ideograms
        0,
    };
    return &ranges[0];
}

void gui_loop() {
    if (!m_emu->Running())
        return;

    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Render các cửa sổ
    for (auto win : windows) {
        if (win->open) {
            ImGui::SetNextWindowSizeConstraints(
                ImVec2(250 * UI::Scaling::fontScale, 200 * UI::Scaling::fontScale),
                ImVec2(FLT_MAX, FLT_MAX)
            );
            win->Render();
        }
    }

    // Tạo overlay ở phía trên màn hình
    const float overlayHeight = UI::Scaling::buttonHeight * 2.5f;
    const float comboWidth = CalculateComboSize().x;
    const float buttonWidth = CalculateButtonSize().x;
    const float buttonSpacing = UI::Scaling::padding * 2.0f;
    const float minOverlayWidth = (buttonWidth * 3) + (buttonSpacing * 2) + comboWidth + UI::Scaling::padding * 6;

    ImGui::SetNextWindowBgAlpha(0.95f);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, overlayHeight));
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(minOverlayWidth, overlayHeight),  // Minimum size
        ImVec2(FLT_MAX, overlayHeight)           // Maximum size
    );

    if (ImGui::Begin("Overlay", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoScrollbar))
    {
        float windowWidth = ImGui::GetWindowWidth();

        // FPS Counter
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(50, 255, 50, 255));
        ImGui::SetCursorPos(ImVec2(UI::Scaling::padding * 2, UI::Scaling::padding));
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::PopStyleColor();

        // Windows Combo
        float comboX = UI::Scaling::padding * 8; // Điều chỉnh khoảng cách từ FPS counter
        float comboY = (overlayHeight - UI::Scaling::buttonHeight) / 2;
        ImGui::SetCursorPos(ImVec2(comboX, comboY));

        static UIWindow* current_filter = nullptr;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, 
            ImVec2(UI::Scaling::padding, UI::Scaling::padding));
        
        ImGui::SetNextItemWidth(comboWidth);
        if (ImGui::BeginCombo("##Windows", current_filter ? current_filter->name : "Select Window", 
            ImGuiComboFlags_HeightLarge)) {
            float popupMaxHeight = ImGui::GetIO().DisplaySize.y * 0.7f;
            ImGui::SetNextWindowSizeConstraints(
                ImVec2(0, 0), 
                ImVec2(FLT_MAX, popupMaxHeight)
            );

            for (auto* window : windows) {
                bool is_selected = (current_filter == window);
                if (ImGui::Selectable(window->name, is_selected)) {
                    current_filter = window;
                    window->open = true;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleVar();

        // Buttons
        float totalButtonsWidth = (buttonWidth * 3) + (buttonSpacing * 2);
        float buttonsStartX = windowWidth - totalButtonsWidth - UI::Scaling::padding * 2;
        float buttonsY = (overlayHeight - UI::Scaling::buttonHeight) / 2;

        ImGui::SetCursorPos(ImVec2(buttonsStartX, buttonsY));

        // Close All button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        if (ImGui::Button("Close All", CalculateButtonSize())) {
            for (auto& win : windows) {
                win->open = false;
            }
        }
        ImGui::PopStyleColor();

        // Restart button
        ImGui::SameLine(0, buttonSpacing);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.6f, 0.3f, 1.0f));
        if (ImGui::Button("Restart", CalculateButtonSize())) {
            gui_cleanup();
            char path[1024];
            uint32_t size = sizeof(path);
            if (SDL_GetBasePath() != nullptr) {
                strncpy(path, SDL_GetBasePath(), size);
                std::string command = std::string(path) + "casioemu";
                std::system(command.c_str());
                std::exit(0);
            }
        }
        ImGui::PopStyleColor();

        // Exit button
        ImGui::SameLine(0, buttonSpacing);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        if (ImGui::Button("Exit", CalculateButtonSize())) {
            if (m_emu) {
                m_emu->Shutdown();
            }
            std::exit(0);
        }
        ImGui::PopStyleColor();
    }
    ImGui::End();

    ImGui::Render();
    SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
}

int test_gui(bool* guiCreated, SDL_Window* wd, SDL_Renderer* rd) {
    window = wd;
    renderer = rd;
    if (renderer == nullptr) {
        SDL_Log("Error creating SDL_Renderer!");
        return 0;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.Fonts->AddFontDefault();

#if LANGUAGE == 2
    if (std::filesystem::exists("/system/fonts/NotoSansCJK-Regular.ttc")) {
        ImFontConfig config;
        config.MergeMode = true;
        config.SizePixels = 30.0f * UI::Scaling::fontScale;
        io.Fonts->AddFontFromFileTTF("/system/fonts/NotoSansCJK-Regular.ttc",
            30.0f * UI::Scaling::fontScale, &config, GetKanji());
    } else {
        printf("[Ui][Warn] No chinese font available!\n");
    }
#endif

    io.Fonts->Build();
    io.WantCaptureKeyboard = true;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    UI::Scaling::UpdateUIScale();

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    // Tối ưu style
    style.WindowRounding = 6.0f * UI::Scaling::fontScale;
    style.Colors[ImGuiCol_WindowBg].w = 0.95f;
    style.FrameRounding = 4.0f * UI::Scaling::fontScale;
    style.ScrollbarSize = 50.0f * UI::Scaling::fontScale;
    style.ScrollbarRounding = 6.0f * UI::Scaling::fontScale;
    style.ItemInnerSpacing = ImVec2(UI::Scaling::padding * 1.5f, UI::Scaling::padding * 1.5f);
    style.ItemSpacing = ImVec2(UI::Scaling::padding * 2.5f, UI::Scaling::padding * 2.5f);
    style.GrabMinSize = 40.0f * UI::Scaling::fontScale;

    // Tăng kích thước nút đóng/thu nhỏ cửa sổ
    style.TouchExtraPadding = ImVec2(10.0f, 10.0f);
    style.DisplayWindowPadding = ImVec2(30.0f, 30.0f);
    style.DisplaySafeAreaPadding = ImVec2(30.0f, 30.0f);

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);
    if (guiCreated)
        *guiCreated = true;

    while (!m_emu || !me_mmu)
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    
    // Khởi tạo các cửa sổ
    for (auto item : std::initializer_list<UIWindow*>{
        new VariableWindow(),
        new HwController(),
        new LabelViewer(),
        new CasioData(),
        new WatchWindow(),
        CreateCallAnalysisWindow(),
        CreateAddressWindow(),
        code_viewer = new CodeViewer(),
        injector = new Injector(),
        membp = new MemBreakPoint()
    })
        windows.push_back(item);

    for (auto item : GetEditors())
        windows.push_back(item);

    for (auto item : windows) {
        item->open = false;
    }

    return 0;
}

void gui_cleanup() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    for (auto& window : windows) {
        delete window;
    }
    windows.clear();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}