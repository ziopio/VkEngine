#include "ToolsPanel.h"
#include "EditorUI.h"
#include "Editor.h"
#include "Project.h"

constexpr const char* add_obj_modal = "Add new Object";
constexpr const char* add_lux_modal = "Add new Light";
constexpr const char* select_mesh_modal = "Choose Object Mesh";
constexpr const char* select_texture_modal = "Choose Object Texture ";

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
	std::string id = "default";
	if (ImGui::Button("+OBJ",ImVec2(0,0))) {
		ImGui::OpenPopup(add_obj_modal);
	}
	if (ImGui::BeginPopupModal(add_obj_modal, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static std::string selected_mesh = "cube.obj", selected_texture = "default";
		static char obj_name[20] = "Sample";
		//static char new_obj_id[32] = "obj_id";
		//ImGui::InputText("Unique ID", new_obj_id, sizeof(new_obj_id));
		ImGui::InputText("Name", obj_name, sizeof(obj_name));
		ImGui::Separator();
		ImGui::Text("The object transformations are fixed for now...");
		ImGui::Separator();

		if (ImGui::Button("Choose Mesh"))
			ImGui::OpenPopup(select_mesh_modal);
		ImGui::SameLine();
		ImGui::Text(selected_mesh.c_str());
		if (ImGui::Button("Choose Texture"))
			ImGui::OpenPopup(select_texture_modal);
		ImGui::SameLine();
		ImGui::Text(selected_texture.c_str());

		if (ImGui::BeginPopupModal(select_mesh_modal))
		{
			ImGui::Text("Meshes loaded are listed here");
			auto vec = vkengine::listLoadedMesh();
			static int  index = -1;
			for (int i = 0; i < vec.size(); i++) {
				if (ImGui::Selectable(vec[i].c_str(), index == i))
					selected_mesh = vec[i];
			}
			if (ImGui::Button("Ok"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		if (ImGui::BeginPopupModal(select_texture_modal))
		{
			ImGui::Text("Textures loaded are listed here");
			auto vec = vkengine::listLoadedTextures();
			static int  index = -1;
			for (int i = 0; i < vec.size(); i++) {
				if (ImGui::Selectable(vec[i].c_str(), index == i))
					selected_texture = vec[i];
			}
			if (ImGui::Button("Ok"))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		if (ImGui::Button("OK", ImVec2(120, 0))) 
		{
			auto scene = vkengine::getActiveScene();
			vkengine::ObjTransformation transform = {};
			transform.scale_factor = 1.f;
			transform.eulerAngles = { 0,1,0 };
			vkengine::ObjectInitInfo obj_info = {};
			obj_info.name = obj_name;
			obj_info.texture_name = selected_texture;
			obj_info.mesh_name = selected_mesh;
			obj_info.transformation = transform;

			scene->addObject(obj_info);
			vkengine::reloadScene();

			ImGui::CloseCurrentPopup();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	if (ImGui::Button("+LUX", ImVec2(0, 0))) {
		ImGui::OpenPopup(add_lux_modal);
	}
	if (ImGui::BeginPopupModal(add_lux_modal, NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char light_name[20] = "point";
		ImGui::InputText("Name", light_name, sizeof(light_name));
		ImGui::Separator();
		if (ImGui::Button("OK", ImVec2(120, 0))) 
		{ 
			auto scene = vkengine::getActiveScene();
			vkengine::PointLightInfo l = {
			light_name,{0,1,0},{1,1,1},1.f
			};
			scene->addLight(l);
			ImGui::CloseCurrentPopup(); 
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	ImGui::End();
}

ToolsPanel::~ToolsPanel() = default;
