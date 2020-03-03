#include "Outliner.h"
#include "EditorUI.h"
#include "Editor.h"
#include "Project.h"


const float d_min = -1000.f, d_max = 1000.0;

static ImGuiTreeNodeFlags base_flags =
	ImGuiTreeNodeFlags_OpenOnArrow |
	ImGuiTreeNodeFlags_OpenOnDoubleClick |
	ImGuiTreeNodeFlags_SpanAvailWidth;
static ImGuiTreeNodeFlags leaf_flags = base_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

void showOjectProperties(std::string scene, std::string obj_id);
void showLightProperties(std::string scene, std::string light_id);
void showCameraProperties(std::string scene, std::string cam_id);
void showVectorControls(std::string name, glm::vec3* vec);

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
	auto objs = scene->listObjects();
	auto cams = scene->listCameras();
	auto lights = scene->listLights();

	// Drawing
	// Outliner
	ImGui::SetNextWindowSize(ImVec2(w_width / 4, w_height / 8 * 3 - main_menu_shift), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(w_width - w_width / 4, main_menu_shift));
	ImGui::Begin(this->name, &visible,
		ImGuiWindowFlags_NoCollapse | 
		ImGuiWindowFlags_NoMove);

	ImGui::SetNextItemOpen(true);
	if (ImGui::TreeNode(scene_id.c_str())) {
		ImGui::SetNextItemOpen(true);
		if (ImGui::TreeNode("Oggetti")) {
			for (auto o : objs) {
				auto node_flag = leaf_flags | (o == selected_element ?
					ImGuiTreeNodeFlags_Selected : 0);
				ImGui::TreeNodeEx(o.c_str(), node_flag, scene->getObject(o)->name.c_str());
				if (ImGui::IsItemClicked()) {
					selected_element = o;
					selected_elem_type = NodeType::OBJECT;
				}
			}
			ImGui::TreePop();
		}
		ImGui::SetNextItemOpen(true);
		if (ImGui::TreeNode("Luci")) {
			for (auto l : lights) {
				auto node_flag = leaf_flags | (l == selected_element ?
					ImGuiTreeNodeFlags_Selected : 0);
				ImGui::TreeNodeEx(l.c_str(), node_flag, scene->getLight(l)->name.c_str());
				if (ImGui::IsItemClicked()) {
					selected_element = l;
					selected_elem_type = NodeType::LIGHT;
				}
			}
			ImGui::TreePop();
		}
		ImGui::SetNextItemOpen(true);
		if (ImGui::TreeNode("Telecamere")) {
			for (auto c : cams) {
				auto node_flag = leaf_flags | (c == selected_element ?
					ImGuiTreeNodeFlags_Selected : 0);
				ImGui::TreeNodeEx(c.c_str(), node_flag, scene->getCamera(c)->name.c_str());
				if (ImGui::IsItemClicked()) {
					selected_element = c;
					selected_elem_type = NodeType::CAMERA;
				}
			}
			ImGui::TreePop();
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
		switch (selected_elem_type)
		{
		case NodeType::OBJECT: showOjectProperties(scene_id, selected_element); break;
		case NodeType::LIGHT: showLightProperties(scene_id, selected_element); break;
		case NodeType::CAMERA: showCameraProperties(scene_id, selected_element); break;
		default:
			break;
		}
	}

	ImGui::End();
}

Outliner::~Outliner() = default;

void showOjectProperties(std::string scene, std::string obj_id)
{
	auto obj = vkengine::getScene(scene)->getObject(obj_id);
	ImGui::Text(obj->name.c_str()); 
	showVectorControls("Position", &obj->getPos());
	showVectorControls("Scale", &obj->getScale());
}

void showLightProperties(std::string scene, std::string light_id)
{
	auto light = vkengine::getScene(scene)->getLight(light_id);
	ImGui::Text(light->name.c_str());
	showVectorControls("Position", (glm::vec3*)&light->getData().position);
}

void showCameraProperties(std::string scene, std::string cam_id)
{
	auto cam = vkengine::getScene(scene)->getCamera(cam_id);
	ImGui::Text(cam->name.c_str()); 
	showVectorControls("Eye", &cam->getViewSetup().position);

	showVectorControls("Target", &cam->getViewSetup().target);

	showVectorControls("Up", &cam->getViewSetup().upVector);
}

void showVectorControls(std::string name, glm::vec3* vec)
{
	ImGui::Text(name.c_str());
	ImGui::DragScalar((name + " X").c_str(), ImGuiDataType_Float, &(*vec).x, 0.05f, &d_min, &d_max, "%f", 1.0f);
	ImGui::DragScalar((name + " Y").c_str(), ImGuiDataType_Float, &(*vec).y, 0.05f, &d_min, &d_max, "%f", 1.0f);
	ImGui::DragScalar((name + " Z").c_str(), ImGuiDataType_Float, &(*vec).z, 0.05f, &d_min, &d_max, "%f", 1.0f);
}
