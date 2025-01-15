#include "CallAnalysis.h"
#include "../Chipset/CPU.hpp"
#include "Hooks.h"
#include "ui.hpp"
#include "imgui/imgui.h"
#include "UIScaling.h"

struct CallAnalysis : public UIWindow {
    bool is_call_recoding = false;
    bool check_caller = false;
    char caller[260]{};
    uint32_t caller_v{};
    bool check_callee = false;
    char callee[260]{};
    uint32_t callee_v{};

    class FunctionCall {
    public:
        uint32_t pc{};
        uint32_t lr{};
        uint32_t xr0{};
        std::string stack{};
    };

    std::map<uint32_t, std::vector<FunctionCall>> funcs;
    std::vector<FunctionCall> viewing_calls;

    CallAnalysis() : UIWindow("Funcs") {
        SetupHook(on_call_function, [this](casioemu::CPU& sender, const FunctionEventArgs& ea) {
            OnCallFunction(sender, ea.pc, ea.lr);
        });
    }

    void OnCallFunction(casioemu::CPU& sender, uint32_t pc, uint32_t lr) {
        if (is_call_recoding) {
            if (check_caller && lr != caller_v) return;
            if (check_callee && pc != callee_v) return;

            FunctionCall fc{};
            fc.xr0 = (sender.reg_r[3] << 24) | (sender.reg_r[2] << 16) | 
                     (sender.reg_r[1] << 8) | (sender.reg_r[0]);
            fc.pc = pc;
            fc.lr = lr;
            fc.stack = sender.GetBacktrace();
            funcs[pc].push_back(fc);
        }
    }

    void RenderCore() override {
        UI::Scaling::UpdateUIScale();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(UI::Scaling::padding, UI::Scaling::padding));

        if (is_call_recoding) {
            if (ImGui::Button("停止", ImVec2(-1, UI::Scaling::buttonHeight))) {
                is_call_recoding = false;
            }

            ImGui::SameLine();
            if (ImGui::Button("清空", ImVec2(-1, UI::Scaling::buttonHeight))) {
                funcs.clear();
            }

            ImGui::Separator();
            RenderFunctionTable();
        }
        else {
            if (!viewing_calls.empty()) {
                RenderViewingCallsSection();
                ImGui::PopStyleVar();
                return;
            }

            RenderControlSection();
            ImGui::Separator();
            RenderFilterSection();
            ImGui::Separator();
            RenderFunctionTable();
        }

        ImGui::PopStyleVar();
    }

private:
    void RenderFunctionTable() {
        float tableHeight = ImGui::GetContentRegionAvail().y;
        ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | 
                               ImGuiTableFlags_RowBg | 
                               ImGuiTableFlags_Borders;

        if (ImGui::BeginTable("##records", 3, flags, ImVec2(0, tableHeight))) {
            ImGui::TableSetupScrollFreeze(0, 1);

            ImGui::TableSetupColumn("函数", ImGuiTableColumnFlags_WidthFixed, 
                                  UI::Scaling::minColumnWidth * 1.5f);
            ImGui::TableSetupColumn("调用计数", ImGuiTableColumnFlags_WidthFixed, 
                                  UI::Scaling::minColumnWidth * 1.5f);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableHeadersRow();

            int i = 0;
            for (auto& func : funcs) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%06x", func.first);

                ImGui::TableNextColumn();
                ImGui::Text("%lu", func.second.size());

                ImGui::TableNextColumn();
                ImGui::PushID(i++);
                if (ImGui::Button("展示所有调用记录", 
                    ImVec2(-1, UI::Scaling::buttonHeight * 0.8f))) {
                    viewing_calls = func.second;
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }

    void RenderViewingCallsSection() {
        if (ImGui::Button("关闭", ImVec2(-1, UI::Scaling::buttonHeight))) {
            viewing_calls.clear();
            return;
        }

        float tableHeight = ImGui::GetContentRegionAvail().y;
        ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | 
                               ImGuiTableFlags_RowBg | 
                               ImGuiTableFlags_Borders;

        if (ImGui::BeginTable("##records", 10, flags, ImVec2(0, tableHeight))) {
            ImGui::TableSetupScrollFreeze(0, 1);

            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, UI::Scaling::minColumnWidth * 0.8f);
            ImGui::TableSetupColumn("函数", ImGuiTableColumnFlags_WidthFixed, UI::Scaling::minColumnWidth * 1.5f);
            ImGui::TableSetupColumn("调用者", ImGuiTableColumnFlags_WidthFixed, UI::Scaling::minColumnWidth * 1.5f);
            ImGui::TableSetupColumn("R0", ImGuiTableColumnFlags_WidthFixed, UI::Scaling::minColumnWidth * 0.5f);
            ImGui::TableSetupColumn("R1", ImGuiTableColumnFlags_WidthFixed, UI::Scaling::minColumnWidth * 0.5f);
            ImGui::TableSetupColumn("R2", ImGuiTableColumnFlags_WidthFixed, UI::Scaling::minColumnWidth * 0.5f);
            ImGui::TableSetupColumn("R3", ImGuiTableColumnFlags_WidthFixed, UI::Scaling::minColumnWidth * 0.5f);
            ImGui::TableSetupColumn("ER0", ImGuiTableColumnFlags_WidthFixed, UI::Scaling::minColumnWidth);
            ImGui::TableSetupColumn("ER2", ImGuiTableColumnFlags_WidthFixed, UI::Scaling::minColumnWidth);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableHeadersRow();

            int i = 0;
            for (auto& func : viewing_calls) {
                ImGui::TableNextRow();
                RenderCallRecord(func, i++);
            }
            ImGui::EndTable();
        }
    }

    void RenderCallRecord(const FunctionCall& func, int index) {
        ImGui::TableNextColumn();
        ImGui::Text("%d", index);

        ImGui::TableNextColumn();
        ImGui::Text("%06x", func.pc);

        ImGui::TableNextColumn();
        ImGui::Text("%06x", func.lr);

        ImGui::TableNextColumn();
        ImGui::Text("%02x", func.xr0 & 0xff);

        ImGui::TableNextColumn();
        ImGui::Text("%02x", (func.xr0 >> 8) & 0xff);

        ImGui::TableNextColumn();
        ImGui::Text("%02x", (func.xr0 >> 16) & 0xff);

        ImGui::TableNextColumn();
        ImGui::Text("%02x", (func.xr0 >> 24) & 0xff);

        ImGui::TableNextColumn();
        ImGui::Text("%04x", func.xr0 & 0xffff);

        ImGui::TableNextColumn();
        ImGui::Text("%04x", (func.xr0 >> 16) & 0xffff);

        ImGui::TableNextColumn();
        ImGui::PushID(index);
        if (ImGui::Button("展示堆栈追踪", ImVec2(-1, UI::Scaling::buttonHeight * 0.8f))) {
            SDL_ShowSimpleMessageBox(0, "CasioEmuMsvc", func.stack.c_str(), 0);
        }
        ImGui::PopID();
    }

    void RenderControlSection() {
        if (ImGui::Button("开始记录", ImVec2(-1, UI::Scaling::buttonHeight))) {
            is_call_recoding = true;
            funcs.clear();
        }

        ImGui::SameLine();
        if (ImGui::Button("清空", ImVec2(-1, UI::Scaling::buttonHeight))) {
            funcs.clear();
        }
    }

    void RenderFilterSection() {
        ImGui::Text("过滤器:");

        float inputWidth = ImGui::GetContentRegionAvail().x * 0.7f;

        ImGui::Checkbox("被调用的函数是:", &check_callee);
        ImGui::SameLine();
        if (ImGui::InputText("##callee", callee, 260, 
            ImGuiInputTextFlags_CharsHexadecimal)) {
            callee_v = strtol(callee, 0, 16);
        }

        ImGui::Checkbox("调用者是:", &check_caller);
        ImGui::SameLine();
        if (ImGui::InputText("##caller", caller, 260, 
            ImGuiInputTextFlags_CharsHexadecimal)) {
            caller_v = strtol(caller, 0, 16);
        }
    }
};

UIWindow* CreateCallAnalysisWindow() {
    return new CallAnalysis();
}