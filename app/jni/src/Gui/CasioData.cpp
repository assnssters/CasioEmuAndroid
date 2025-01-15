#include "CasioData.h"
#include "../Models.h"
#include "CwiiHelp.h"
#include "Ui.hpp"
#include "UIScaling.h"
#include "imgui/imgui.h"

void CasioData::RenderCore() {
    // Update UI scaling at start of frame
    UI::Scaling::UpdateUIScale();

    auto hwid = m_emu->hardware_id;
    if (hwid != casioemu::HardwareId::HW_CLASSWIZ && hwid != casioemu::HardwareId::HW_CLASSWIZ_II) {
        return;
    }

    auto ram = casioemu::GetRamBaseAddr(hwid);
    auto mode = me_mmu->ReadData(casioemu::GetModeOffset(hwid));
    
    if (mode == 0xd) {
        static char formula[200]{};
        static int row{}, col{};

        // Scale input text widget
        ImGui::PushItemWidth(-UI::Scaling::padding);
        float inputHeight = UI::Scaling::buttonHeight * 1.2f;
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##input", formula, 200)) {
            // Handle input
        }
        ImGui::PopItemWidth();

        // Add padding
        ImGui::Dummy(ImVec2(0.0f, UI::Scaling::padding));

        // Cell position text
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%c%d", 
            static_cast<char>('A' + col), row);
        ImGui::TextUnformatted(buffer);

        std::string ss_data[45][5]{};
        auto sptr = n_ram_buffer - ram + casioemu::GetAppOffset(hwid);
        auto ptr = sptr;
        auto sz = (size_t)0;
        auto iptr = ptr;

        if (hwid == casioemu::HardwareId::HW_CLASSWIZ) {
            iptr = n_ram_buffer - ram + 0xDBE8;
        } else {
            iptr = n_ram_buffer - ram + 0xC3FC;
        }

        // Data processing logic remains same...
        for (size_t i = 0; i < 45 * 5 * 2; i += 2) {
            auto fmla = iptr[i];
            auto type = iptr[i + 1];
            if ((type & 0x80) == 0x80) {
                sz += casioemu::GetVariableSize(hwid);
                if ((type & 0x40) != 0x40) {
                    auto a = i >> 1;
                    auto r = a % 45;
                    auto c = a / 45;
                    ss_data[r][c] = cwii::StringizeCwiiNumber(ptr);
                }
                ptr = sptr + sz;
                if (sz > 2452)
                    break;
            }
        }

        // Process formulas
        for (size_t i = 0; i < 45 * 5 * 2; i += 2) {
            auto fmla = iptr[i];
            auto type = iptr[i + 1];
            if (fmla) {
                sz += fmla;
                if ((type & 0x40) == 0x40) {
                    auto a = i >> 1;
                    auto r = a % 45;
                    auto c = a / 45;
                    std::string s{ptr, (size_t)fmla};
                    ss_data[r][c] = s;
                }
                ptr = sptr + sz;
                if (sz > 2452)
                    break;
            }
        }

        // Scaled table
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(UI::Scaling::padding, UI::Scaling::padding));
        
        float tableHeight = ImGui::GetContentRegionAvail().y;
        if (ImGui::BeginTable("Spreadsheet", 5, 
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner,
            ImVec2(0.0f, tableHeight))) {
            
            // Setup scaled columns
            for (int i = 0; i < 5; i++) {
                ImGui::TableSetupColumn(std::string(1, 'A' + i).c_str(), 
                    ImGuiTableColumnFlags_WidthFixed,
                    UI::Scaling::minColumnWidth);
            }
            
            ImGui::TableHeadersRow();

            // Render cells with proper scaling
            for (size_t i = 0; i < 45; i++) {
                ImGui::TableNextRow(0, UI::Scaling::buttonHeight);
                for (size_t j = 0; j < 5; j++) {
                    ImGui::TableNextColumn();
                    ImGui::PushID(static_cast<int>(i * 5 + j + 20));
                    
                    auto& cellData = ss_data[i][j];
                    bool isSelected = (row == i && col == j);
                    
                    if (ImGui::Selectable(cellData.c_str(), isSelected, 
                        ImGuiSelectableFlags_SpanAllColumns)) {
                        row = i;
                        col = j;
                    }
                    
                    ImGui::PopID();
                }
            }
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
    }
}
