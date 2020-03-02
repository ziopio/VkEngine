#include "ToolsPanel.h"
#include "EditorUI.h"

ToolsPanel::ToolsPanel(EditorUI * UI) : EditorComponent(UI)
{
	this->name = "Tools";
	this->visible = true;
}

void ToolsPanel::draw(int w_width, int w_height)
{
	if (!visible) { return; }

	// Drawing

	ImGui::SetNextWindowSize(ImVec2(tools_panel_width, w_height / 4 * 3 - main_menu_shift), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0, main_menu_shift));
	ImGui::Begin(this->name, nullptr, ImGuiWindowFlags_NoCollapse);



	ImGui::End();
}

ToolsPanel::~ToolsPanel() = default;
