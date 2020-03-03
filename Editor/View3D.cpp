#include "View3D.h"
#include "EditorUI.h"
#include "ToolsPanel.h"
#include "Editor.h"
#include "Project.h"


View3D::View3D(EditorUI* UI) : EditorComponent(UI)
{
	this->name = "3D View";
	this->visible = true;
}

void View3D::draw(int w_width, int w_height)
{
	if (!visible) { return; }

	ImGui::SetNextWindowSize(ImVec2(w_width - w_width / 4 - tools_panel_width, w_height - w_height / 4 - main_menu_shift), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(tools_panel_width, main_menu_shift));
	ImGui::Begin(this->name, &visible, 
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar);
	ImGui::SetScrollHereY(0.5f);
	ImGui::BeginMenuBar();
	ImGui::Button("Go Full Window");
	ImGui::EndMenuBar();
	//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImVec2 oldDrawingPos = ImGui::GetCursorPos();
	ImVec2 frame_size = ImGui::GetWindowSize();
	frame_size.x -= oldDrawingPos.x;
	frame_size.y -= oldDrawingPos.y;
	ImVec2 width_padding = frame_size;
	frame_size.x = frame_size.y * ((float)w_width / (float)w_height);
	width_padding.x = (ImGui::GetWindowSize().x - frame_size.x) / 2;
	ImVec2 newDrawingPos = { width_padding.x, oldDrawingPos.y - oldDrawingPos.x };
	ImGui::SetCursorPos(newDrawingPos);
	ImGui::Image((ImTextureID)VIEW_3D_TEXTURE_CODE, frame_size,
		ImVec2(0, 0), ImVec2(1, 1),
		ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

	if (ImGui::IsWindowHovered() &&
		ImGui::IsWindowFocused()) {
		glm::vec2 absoluteMousePos = ImGui::GetMousePos();
		glm::vec2 relativeToWinMousePos = absoluteMousePos - (glm::vec2)ImGui::GetWindowPos();
		glm::vec2 relativeToRenderTargetPos = relativeToWinMousePos - (glm::vec2)newDrawingPos;

	
		
		if (relativeToRenderTargetPos.x >= 0 &&
			relativeToRenderTargetPos.y >= 0 &&
			relativeToRenderTargetPos.x <= frame_size.x &&
			relativeToRenderTargetPos.y <= frame_size.y) 
		{
			auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle, 0.f);
			ImGui::GetIO().MouseClickedPos[ImGuiMouseButton_Middle] = ImGui::GetMousePos();
			auto scene = vkengine::getScene(UI->getEditor()->loadedProject.get()->getActiveScene());
			auto cam = scene->getCamera(scene->current_camera);
			cam->rotate_FPS_style(delta);


			auto io = ImGui::GetIO();
			io.KeysDown[KeyType::KEY_W] ? cam->moveCameraForeward() : cam->stopCameraForeward();
			io.KeysDown[KeyType::KEY_A] ? cam->moveCameraLeft() : cam->stopCameraLeft();
			io.KeysDown[KeyType::KEY_S] ? cam->moveCameraBack() : cam->stopCameraBack();
			io.KeysDown[KeyType::KEY_D] ? cam->moveCameraRight() : cam->stopCameraRight();
		}
	}
	ImGui::End();
}


View3D::~View3D() = default;
