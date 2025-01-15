#include "AddressWindow.h"
#include "Hooks.h"
#include "UIScaling.h"

struct AddressInfo {
    uint32_t address;
    uint8_t value;
    bool locked;
    AddressInfo(uint32_t addr, uint8_t val, bool lock) : address(addr), value(val), locked(lock) {}
};

class AddressWindow : public UIWindow {
public:
    AddressWindow() : UIWindow("Address Manager") {
        SetupHooks();
    }

    void RenderCore() override {
        UI::Scaling::UpdateUIScale();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(UI::Scaling::padding, UI::Scaling::padding));

        RenderTitle();
        ImGui::Separator();
        RenderAddressTable();
        ImGui::Separator();
        RenderAddAddressControls();

        ImGui::PopStyleVar();
    }

private:
    std::vector<AddressInfo> addresses;
    uint32_t newAddress = 0;
    char addressInput[9] = "00000000";  // For hex input

    void RenderTitle() {
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Manage Addresses");
        ImGui::Spacing();
    }

    void RenderAddressTable() {
        float tableHeight = ImGui::GetContentRegionAvail().y * 0.7f;
        ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | 
                               ImGuiTableFlags_RowBg | 
                               ImGuiTableFlags_Borders;

        if (ImGui::BeginTable("Addresses", 4, flags, ImVec2(0, tableHeight))) {
            ImGui::TableSetupScrollFreeze(0, 1);
            
            ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed, 
                                  UI::Scaling::minColumnWidth * 1.5f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 
                                  UI::Scaling::minColumnWidth);
            ImGui::TableSetupColumn("Lock", ImGuiTableColumnFlags_WidthFixed, 
                                  UI::Scaling::minColumnWidth);
            ImGui::TableSetupColumn("Delete", ImGuiTableColumnFlags_WidthStretch);
            
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < addresses.size(); ++i) {
                ImGui::PushID(static_cast<int>(i));
                auto& info = addresses[i];

                ImGui::TableNextRow();
                
                // Address Column
                ImGui::TableNextColumn();
                ImGui::Text("0x%08X", info.address);

                // Value Column
                ImGui::TableNextColumn();
                uint8_t value = info.locked ? info.value : m_emu->chipset.mmu.ReadData(info.address);
                if (ImGui::InputScalar("##value", ImGuiDataType_U8, &value, 0, 0, "%02X", 
                    ImGuiInputTextFlags_CharsHexadecimal)) {
                    info.value = value;
                    UpdateMemoryValue(info.address, info.value);
                }

                // Lock Column
                ImGui::TableNextColumn();
                bool locked = info.locked;
                if (ImGui::Checkbox("##lock", &locked)) {
                    info.locked = locked;
                }

                // Delete Column
                ImGui::TableNextColumn();
                if (ImGui::Button("Delete##del", ImVec2(-1, UI::Scaling::buttonHeight * 0.8f))) {
                    addresses.erase(addresses.begin() + i);
                    i--;
                }

                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }

    void RenderAddAddressControls() {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 0.4f, 1.0f));
        ImGui::Text("Add New Address");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        float inputWidth = ImGui::GetContentRegionAvail().x * 0.7f;
        ImGui::SetNextItemWidth(inputWidth);
        
        if (ImGui::InputText("##address", addressInput, sizeof(addressInput), 
            ImGuiInputTextFlags_CharsHexadecimal)) {
            newAddress = strtoul(addressInput, nullptr, 16);
        }
        
        ImGui::SameLine();
        ImGui::Text("0x");
        
        if (ImGui::Button("Add Address", ImVec2(-1, UI::Scaling::buttonHeight))) {
            if (newAddress != 0) {
                bool exists = false;
                for (const auto& info : addresses) {
                    if (info.address == newAddress) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    addresses.emplace_back(newAddress, 0, false);
                    memset(addressInput, '0', 8);
                    addressInput[8] = '\0';
                    newAddress = 0;
                }
            }
        }
    }

    void UpdateMemoryValue(uint32_t address, uint8_t value) {
        m_emu->chipset.mmu.WriteData(address, value);
    }

    void SetupHooks() {
        SetupHook(on_memory_write, [this](casioemu::MMU& mmu, MemoryEventArgs& args) {
            for (const auto& info : addresses) {
                if (info.locked && info.address == args.offset) {
                    args.handled = true;
                }
            }
        });

        SetupHook(on_memory_read, [this](casioemu::MMU& mmu, MemoryEventArgs& args) {
            for (const auto& info : addresses) {
                if (info.locked && info.address == args.offset) {
                    args.value = info.value;
                    args.handled = true;
                }
            }
        });
    }
};

UIWindow* CreateAddressWindow() {
    return new AddressWindow();
}