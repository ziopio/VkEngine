#include "View3D.h"
#include "EditorUI.h"
#include "../ImGui/imgui.h"



View3D::View3D(EditorUI* UI) : EditorComponent(UI)
{
	this->name = "View3D";
	this->visible = true;
}

void View3D::draw()
{
	if (!visible) { return; }
	int w_width, w_height;
	this->UI->getWindow()->getWindowSize(&w_width, &w_height);
	



	ImGui::SetNextWindowSize(ImVec2(w_width / 2, w_height / 2), ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(0, 20));
	ImGui::Begin("3D View", &visible, 
		ImGuiWindowFlags_MenuBar |
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
		glm::vec2 renderTargetPosition = relativeToWinMousePos - (glm::vec2)newDrawingPos;

		if (renderTargetPosition.x >= 0 &&
			renderTargetPosition.y >= 0 &&
			renderTargetPosition.x <= frame_size.x &&
			renderTargetPosition.y <= frame_size.y) {
			//std::stringstream ss;
			//ss << "RenderTarget HIT! " << renderTargetPosition.x
			//	<< " " << renderTargetPosition.y << std::endl;
			//this->UI->printDebug(ss.str());
		}
	}
	ImGui::End();
}


View3D::~View3D()
{
}
