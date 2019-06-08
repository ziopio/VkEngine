#include "EditorUI.h"
#include "../ImGui/imgui.h"


EditorUI::EditorUI()
{   
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();
}

void EditorUI::drawUI()
{
}


EditorUI::~EditorUI()
{
}

void EditorUI::loadFont()
{
}
