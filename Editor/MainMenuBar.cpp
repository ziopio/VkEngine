#include "MainMenuBar.h"
#include "EditorUI.h"
#include "Editor.h"
#include "Project.h"


constexpr const char* scene_create_popup = "Add New Scene";

MainMenuBar::MainMenuBar(EditorUI* UI) : EditorComponent(UI)
{
	this->name = "MainMenuBar";
	this->visible = true;
}

void MainMenuBar::draw(int w_width, int w_height)
{
	if (!visible) return;

	static bool add_scene = false;
	static bool show_demo_window = false;

	ImGui::BeginMainMenuBar();
	if (vkengine::hasRayTracing()) {
		ImGui::Checkbox("Ray Tracing: ", vkengine::rayTracing());
	}
	else { ImGui::Text("Ray Tracing: device not capable :P"); }
	ImGui::SameLine(ImGui::GetWindowWidth() - 400);
	ImGui::SetNextItemWidth(120);
	std::vector<const char*> scene_ids = vkengine::list_scenes(); scene_ids.push_back("+ ADD NEW");
	static const char* item_current = scene_ids[0];            // Here our selection is a single pointer stored outside the object.
	if (ImGui::BeginCombo("Selected Scene", vkengine::getActiveScene()->name.c_str(), ImGuiComboFlags_NoArrowButton))
	{
		for (int n = 0; n < scene_ids.size(); n++)
		{
			bool is_selected = (item_current == scene_ids[n]);
			if (n == scene_ids.size() - 1) {
				if (ImGui::Selectable(scene_ids[n], is_selected)) {
					add_scene = true;
				}
			}
			else {
				if (ImGui::Selectable(vkengine::getScene(scene_ids[n])->name.c_str(), is_selected)) {
					item_current = scene_ids[n];
					vkengine::loadScene(item_current);
					this->UI->showNewScene();
				}
			}

			if (is_selected) {
				ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
			}
		}
		ImGui::EndCombo();
	}
	if (ImGui::Button("X"))
	{
		vkengine::removeScene(vkengine::getActiveScene()->getId());
		this->UI->showNewScene();
	}
	
	if (add_scene) {
		ImGui::OpenPopup(scene_create_popup); add_scene = false;
	}
	if (ImGui::BeginPopupModal(scene_create_popup, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static std::string selected_mesh = "plane.obj", selected_texture = "default";
		static char scene_name[20] = "Sample";
		ImGui::InputText("Name", scene_name, sizeof(scene_name));
		ImGui::Separator();

		const char* title = "ID Error";
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			vkengine::createScene(scene_name, scene_name);
			this->UI->showNewScene();
			item_current = scene_name;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (ImGui::Button("SAVE")) this->UI->getEditor()->loadedProject.get()->save();
	ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
	if (show_demo_window) {
		ImGui::ShowDemoWindow(&show_demo_window);
	}
	ImGui::EndMainMenuBar();
}

MainMenuBar::~MainMenuBar() = default;
