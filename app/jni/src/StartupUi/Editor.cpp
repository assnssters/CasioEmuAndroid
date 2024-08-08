#include "ModelInfo.h"
#include <imgui.h>
class ModelEditor {
	casioemu::ModelInfo mi;
	int v;

public:
	void Render() {
		if (ImGui::Begin("Model Editor")) {
			ImGui::SameLine(500);
			if (ImGui::BeginChild("Model Info"))
			{
				ImGui::Text("Csr Mask");
				ImGui::SameLine(100);
				v = mi.csr_mask;
				if (ImGui::SliderInt("##a", &v, 0, 15)) {

				}
				ImGui::Text("Is sample rom");
				ImGui::SameLine(100);
				ImGui::Checkbox("##b", &mi.is_sample_rom);
				ImGui::Text("Hardware Id");
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}
};