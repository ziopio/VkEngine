#include "Outliner.h"
#include "EditorUI.h"
#include "Editor.h"
#include "Project.h"

Outliner::Outliner(EditorUI * UI) : EditorComponent(UI)
{
	this->name = "Outliner";
	this->visible = true;
}

void Outliner::draw(int w_width, int w_height)
{
	if (!visible) { return; }

	auto scene_id = UI->getEditor()->loadedProject.get()->getActiveScene();

	auto scene = vkengine::getScene(scene_id);

	auto elements = scene.getAllElements();

	// Drawing
	// Outliner
	ImGui::SetNextWindowSize(ImVec2(w_width / 4, w_height / 8 * 3 - main_menu_shift), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(w_width - w_width/4, main_menu_shift));
	ImGui::Begin(this->name, &visible,
		ImGuiWindowFlags_NoCollapse | 
		ImGuiWindowFlags_NoMove);

	if (ImGui::TreeNode(scene_id.c_str())) {
		ImGuiTreeNodeFlags base_flags = 
			ImGuiTreeNodeFlags_OpenOnArrow | 
			ImGuiTreeNodeFlags_OpenOnDoubleClick | 
			ImGuiTreeNodeFlags_SpanAvailWidth;
		ImGuiTreeNodeFlags leaf_flags = base_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		for (auto e : elements) {
			auto node_flag = leaf_flags | (e->getId() == selected_element ? 
				ImGuiTreeNodeFlags_Selected : 0);
			ImGui::TreeNodeEx(e->getId().c_str(), node_flag, e->name.c_str());
			if (ImGui::IsItemClicked())
				selected_element = e->getId();
		}
		ImGui::TreePop();
	}
	ImGui::End();

	//Properties panel
	const char* p_name = "Properties";
	ImGui::SetNextWindowSize(ImVec2(w_width / 4, w_height / 8 * 3), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(w_width - w_width / 4, main_menu_shift + w_height / 8 * 3 - main_menu_shift));
	ImGui::Begin(p_name, &visible,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove);

	if (selected_element != "") {
		// TODO
	}

	ImGui::End();
}

Outliner::~Outliner() = default;
