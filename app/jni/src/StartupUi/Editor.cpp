#include "ModelInfo.h"
#include "../Gui/UIScaling.h"
#include <imgui.h>

class ModelEditor {
private:
    casioemu::ModelInfo mi;
    int v;

    void RenderModelInfo() {
        // Calculate content width based on window width
        float contentWidth = ImGui::GetContentRegionAvail().x - 2 * UI::Scaling::padding;

        // Add padding
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(UI::Scaling::padding, UI::Scaling::padding));

        // Render fields with dynamic positioning
        auto RenderField = [&](const char* label, auto& value, auto renderFunc) {
            ImGui::Text("%s", label);
            ImGui::SameLine(UI::Scaling::labelWidth);
            ImGui::SetNextItemWidth(contentWidth - UI::Scaling::labelWidth);
            renderFunc(value);
        };

        // CSR Mask field
        RenderField("Csr Mask", v, [&](int& val) {
            if (ImGui::SliderInt("##csr_mask", &val, 0, 15)) {
                mi.csr_mask = val;
            }
        });

        // Is sample ROM field  
        RenderField("Is sample rom", mi.is_sample_rom, [](bool& val) {
            ImGui::Checkbox("##is_sample_rom", &val);
        });

        // Hardware ID field
        ImGui::Text("Hardware Id");

        ImGui::PopStyleVar();
    }

public:
    ModelEditor() {
        UI::Scaling::UpdateUIScale();
    }

    void Render() {
        // Update scaling on each frame
        UI::Scaling::UpdateUIScale();

        // Set window padding
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(UI::Scaling::padding, UI::Scaling::padding));

        if (ImGui::Begin("Model Editor", nullptr, ImGuiWindowFlags_None)) {
            float availWidth = ImGui::GetContentRegionAvail().x;
            float childWidth = availWidth * 0.7f; // Child window takes 70% of available width

            ImGui::SameLine(availWidth - childWidth - UI::Scaling::padding);

            if (ImGui::BeginChild("Model Info", ImVec2(childWidth, 0), true)) {
                RenderModelInfo();
            }
            ImGui::EndChild();
        }
        ImGui::End();

        ImGui::PopStyleVar();
    }
};
