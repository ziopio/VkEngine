#include "Outliner.h"
#include "EditorUI.h"
#include "Editor.h"
#include "Project.h"

#include <glm/gtc/type_ptr.hpp>


const float d_min = -1000.f, l_min= 0.0f, d_max = 1000.0;

static const ImGuiTreeNodeFlags base_flags =
	ImGuiTreeNodeFlags_DefaultOpen |
	ImGuiTreeNodeFlags_OpenOnArrow |
	ImGuiTreeNodeFlags_OpenOnDoubleClick |
	ImGuiTreeNodeFlags_SpanAvailWidth;
static const ImGuiTreeNodeFlags leaf_flags = base_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

void showName(vkengine::SceneElement* elem);
void showSceneProperties(vkengine::Scene3D* scene);
void showOjectProperties(vkengine::Scene3D* scene, unsigned obj_id);
void showLightProperties(vkengine::Scene3D* scene, unsigned light_id);
void showCameraProperties(vkengine::Scene3D* scene, unsigned cam_id);
void showVectorControls(std::string name, glm::vec3* vec);
//void showVectorControls(std::string name, glm::vec4* vec);

Outliner::Outliner(EditorUI * UI) : EditorComponent(UI)
{
	this->name = "Outliner";
	this->visible = true;
}


void Outliner::draw(int w_width, int w_height)
{
	static bool scene_open = true, obj_open = true, lights_open = true, cameras_open = true;
	if (!visible) { return; }

	auto scene = vkengine::getActiveScene();
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

	if (ImGui::TreeNodeEx(scene->name.c_str(), base_flags)) {
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			selected_element = 0;
			selected_elem_type = NodeType::SCENE;
		}

		char buff[32];
		if (ImGui::TreeNodeEx("Oggetti", base_flags)) {
			static unsigned context_menu_id;
			for (auto o : objs) {
				auto node_flag = leaf_flags | (o == selected_element ?
					ImGuiTreeNodeFlags_Selected : 0);
				ImGui::TreeNodeEx(std::to_string(o).c_str(), node_flag, scene->getObject(o)->name.c_str());
				if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
					selected_element = o;
					selected_elem_type = NodeType::OBJECT;
				}

				if (selected_element == o && ImGui::IsKeyDown(KeyType::KEY_DELETE) && ImGui::IsWindowFocused()) 
				{
					selected_element = -1;
					scene->removeObject(o);
					vkengine::reloadScene();
				}

				context_menu_id = o;
				ImGui::OpenPopupOnItemClick(std::to_string(context_menu_id).c_str(), ImGuiMouseButton_Right);
				if (ImGui::BeginPopupContextItem())
				{
					selected_element = context_menu_id;
					if (ImGui::Button("Copy")) {
						auto obj = scene->getObject(context_menu_id);
						vkengine::ObjectInitInfo info = {};
						info.name = obj->name + "_cpy";
						info.reflective = obj->reflective;
						info.mesh_name = obj->getMeshName();
						info.texture_name = obj->getTextureName();
						info.transformation = obj->getObjTransform();
						scene->addObject(info);
						vkengine::reloadScene();
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::Button("Delete")) {
						if (selected_element == context_menu_id) selected_element = -1;
						scene->removeObject(context_menu_id);
						vkengine::reloadScene();
					}
					ImGui::EndPopup();
				}
			}

			ImGui::TreePop();
		}
		if (ImGui::TreeNodeEx("Luci", base_flags)) {
			for (auto l : lights) {
				auto node_flag = leaf_flags | (l == selected_element ?
					ImGuiTreeNodeFlags_Selected : 0);
				ImGui::TreeNodeEx(std::to_string(l).c_str(), node_flag, scene->getLight(l)->name.c_str());
				if (ImGui::IsItemClicked()) {
					selected_element = l;
					selected_elem_type = NodeType::LIGHT;
				}				
				ImGui::OpenPopupOnItemClick(std::to_string(l).c_str(), ImGuiMouseButton_Right);
				if (ImGui::BeginPopupContextItem())
				{
					selected_element = l;
					if (ImGui::Button("Copy")) {
						auto lux = scene->getLight(l);
						auto data = lux->getData();
						vkengine::PointLightInfo info = {
						lux->name + "_cpy",
							{data.position.x,data.position.y,data.position.z},
							{data.color.x,data.color.y,data.color.z},
							data.power.w };
						scene->addLight(info);
						vkengine::reloadScene();
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::Button("Delete")) {
						if (selected_element == l) selected_element = -1;
						scene->removeLight(l);
						vkengine::reloadScene();
					}
					ImGui::EndPopup();
				}
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNodeEx("Telecamere", base_flags)) {
			for (auto c : cams) {
				auto node_flag = leaf_flags | (c == selected_element ?
					ImGuiTreeNodeFlags_Selected : 0);
				ImGui::TreeNodeEx(std::to_string(c).c_str(), node_flag, scene->getCamera(c)->name.c_str());
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

	if (selected_element != -1) {
		switch (selected_elem_type)
		{
		case NodeType::OBJECT: showOjectProperties(scene, selected_element); break;
		case NodeType::LIGHT: showLightProperties(scene, selected_element); break;
		case NodeType::CAMERA: showCameraProperties(scene, selected_element); break;
		case NodeType::SCENE: showSceneProperties(scene); break;
		default:
			break;
		}
	}

	ImGui::End();
}

Outliner::~Outliner() = default;

void showName(vkengine::SceneElement* elem) 
{
	static bool editing = false;
	static char* rename;
	rename = (char*)elem->name.c_str();
	if (editing) {
		if(ImGui::InputText("##rename", rename, 
			(elem->name.capacity() > 20 ? 20 : elem->name.capacity()),
			ImGuiInputTextFlags_EnterReturnsTrue))
		{
			editing = false;
		}
	}
	else {
		ImGui::Text(elem->name.c_str());	
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered()) {
			editing = true;
		}
	}

}

void showSceneProperties(vkengine::Scene3D* scene)
{
	ImGui::Text("Global Light");
	ImGui::SetNextItemWidth(50);
	ImGui::DragScalar("Power", ImGuiDataType_Float, &(scene->globalLight.power.w), 0.01f, &l_min, &d_max, "%f", 1.0f);
	ImGui::Text("Direction");
	showVectorControls("Direction", (glm::vec3* )&scene->globalLight.position);
}

void showOjectProperties(vkengine::Scene3D* scene, unsigned obj_id)
{
	auto obj = scene->getObject(obj_id);
	showName(obj);
	ImGui::Text("Position");
	showVectorControls("Position",  &obj->getObjTransform().position);
	ImGui::Text("Rotation XYZ");
	showVectorControls("Rotation", &obj->getObjTransform().eulerAngles);

	ImGui::DragScalar("Scale", ImGuiDataType_Float, 
		&obj->getObjTransform().scale_factor, 0.05f, &d_min, &d_max, "%0.2f", 1.0f);

	//showVectorControls("Scale",  &obj->getObjTransform().scale_vector);
	ImGui::Checkbox("Reflective", &obj->reflective);
}

void showLightProperties(vkengine::Scene3D* scene, unsigned light_id)
{
	auto light = scene->getLight(light_id);
	showName(light);
	ImGui::SetNextItemWidth(50);
	ImGui::DragScalar("Power", ImGuiDataType_Float, &(light->getData().power.w), 0.01f, &l_min, &d_max, "%f", 1.0f);
	showVectorControls("Position", (glm::vec3*)&light->getData().position);
	ImGui::ColorEdit3("Color", glm::value_ptr(*(glm::vec3*) & light->getData().color));
}

void showCameraProperties(vkengine::Scene3D* scene, unsigned cam_id)
{
	auto cam = scene->getCamera(cam_id);
	ImGui::Text(cam->name.c_str());
	ImGui::Text("Eye");
	showVectorControls("Eye", &cam->getViewSetup().position);
	ImGui::Text("Target");
	showVectorControls("PosTargetition", &cam->getViewSetup().target);
	ImGui::Text("Up");
	showVectorControls("Up", &cam->getViewSetup().upVector);
}

void showVectorControls(std::string name, glm::vec3* vec)
{
	const char* format = "%0.2f";
	ImGui::SetNextItemWidth(50);
	ImGui::DragScalar(("X##" + name).c_str(), ImGuiDataType_Float, &(*vec).x, 0.05f, &d_min, &d_max, format, 1.0f);
	ImGui::SameLine(); ImGui::SetNextItemWidth(50);
	ImGui::DragScalar(("Y##" + name).c_str(), ImGuiDataType_Float, &(*vec).y, 0.05f, &d_min, &d_max, format, 1.0f);
	ImGui::SameLine(); ImGui::SetNextItemWidth(50);
	ImGui::DragScalar(("Z##" + name).c_str(), ImGuiDataType_Float, &(*vec).z, 0.05f, &d_min, &d_max, format, 1.0f);
}
