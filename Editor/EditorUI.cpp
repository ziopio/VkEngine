#include "EditorUI.h"
#include "Editor.h"
#include "Project.h"
#include "EditorComponent.h"
#include "MainMenuBar.h"
#include "View3D.h"
#include "Outliner.h"
#include "ToolsPanel.h"
#include "../ImGui/imgui.h"
#include <string>
#include <iostream>
#include <sstream> 
#include <ctime>


#define DEFAULT_WIDTH 1366
#define DEFAULT_HEIGHT 768
static constexpr float DEF_ASPECT_RATIO = 16.f / 9.f;


void setClipboardText(void* user_pointer, const char* text);
const char* getClipboardText(void* user_pointer);
//Takes data and pointers from ImDrawData and copies those to UiDrawData
vkengine::UiDrawData out_put_draw_data(ImDrawData* data);


EditorUI::EditorUI(Editor* editor)
{   
	this->editor = editor;
	this->window = WindowManager::createWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT, win_title);
	this->window->registerEventHandler(this);
	this->window->activateCharCallback();
	this->window->activateKeyCallBack();
	this->window->activateMouseButtonCallback();
	this->window->activateCursorPosCallback();
	this->window->activateScrollCallback();
	this->window->activateDropCallback();
	// High Level UI Components
	//3D view with current scene
	this->editorComponents.push_back(new View3D(this));
	this->editorComponents.push_back(new Outliner(this));
	this->editorComponents.push_back(new ToolsPanel(this));
	this->editorComponents.push_back(new MainMenuBar(this));
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	this->setUpImGuiStyle();
	this->mapWindowInput2ImGui();
	this->updateImguiDisplay();
}

vkengine::UiDrawData EditorUI::drawUI()
{
	this->pollInputs();
	ImGui::NewFrame();
	this->setCaptureFlags();

	int w_width, w_height;
	this->window->getWindowSize(&w_width, &w_height);

	std::ostringstream s(win_title); 
	s << win_title << " FPS: " << ImGui::GetIO().Framerate;
	this->window->changeTitle(s.str().c_str());

	// Exit check
	if (this->getWindow()->windowShouldClose()) this->editor->terminate = true;
	
	for (auto comp : editorComponents)
	{
		comp->draw(w_width, w_height);
	}

	// Debug logging window
	{					
		ImGui::SetNextWindowPos(ImVec2(0, w_height - w_height / 4), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(w_width, w_height / 4), ImGuiCond_Always);
		ImGui::Begin("bottomW", NULL, ImGuiWindowFlags_NoTitleBar);
		if (ImGui::BeginTabBar("MyTabBar", 0))
		{
			if (ImGui::BeginTabItem("Log"))
			{
				for (auto log : debug_logs) {
					ImGui::TextWrapped(log.c_str());
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::SetScrollHereY(1.0f);
		ImGui::End();
	}
	ImGui::EndFrame();
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();
	return out_put_draw_data(draw_data);
}

EditorUI::~EditorUI()
{
	ImGui::DestroyContext();
	WindowManager::destroyWindow(this->window);
	for (auto comp : this->editorComponents) {
		delete comp;
	}
}

void EditorUI::updateCursor()
{
	ImGuiIO& io = ImGui::GetIO();
	if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || 
		window->getInputMode(InputMode::CURSOR) == InputModeValueType::CURSOR_DISABLED)
		return;

	ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
	if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
	{
		// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
		window->setInputMode(InputMode::CURSOR, InputModeValueType::CURSOR_HIDDEN);
	}
	else
	{
		CursorType newCursor;
		switch (imgui_cursor)
		{
			case ImGuiMouseCursor_Arrow: newCursor = CursorType::ARROW_CURSOR; break;
				// When hovering over InputText, etc.
			case ImGuiMouseCursor_TextInput: newCursor = CursorType::IBEAM_CURSOR; break;
				// When hovering over an horizontal border
			case ImGuiMouseCursor_ResizeNS:  newCursor = CursorType::VRESIZE_CURSOR; break;
				// When hovering over a vertical border or a column
			case ImGuiMouseCursor_ResizeEW: newCursor = CursorType::HRESIZE_CURSOR; break;
			// (Unused by Dear ImGui functions. Use for e.g. hyperlinks)
			case ImGuiMouseCursor_Hand: newCursor = CursorType::HAND_CURSOR; break;	
				 //When hovering over the bottom-left corner of a window	
			case ImGuiMouseCursor_ResizeNESW: newCursor = CursorType::CROSSHAIR_CURSOR; break;
				// When hovering over the bottom-right corner of a window
			case ImGuiMouseCursor_ResizeNWSE: newCursor = CursorType::CROSSHAIR_CURSOR; break;
			default: newCursor = CursorType::ARROW_CURSOR; break;
		}
		// Show OS mouse cursor
		window->setCursor(newCursor);
		window->setInputMode(InputMode::CURSOR, InputModeValueType::CURSOR_NORMAL);
	}
}

void EditorUI::updateImguiDisplay()
{
	int w_width, w_height;
	this->window->getWindowSize(&w_width, &w_height);
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)w_width, (float)w_height);
	if (io.DisplaySize.x > 0 && io.DisplaySize.y > 0)
	{
		io.DisplayFramebufferScale =
			ImVec2((float)w_width / (float)io.DisplaySize.x,
			(float)w_height / (float)io.DisplaySize.y);
	}
}

void EditorUI::setDeltaTime(double delta_time)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = (float)delta_time > 0 ? (float)delta_time : 0;
}

void EditorUI::mapWindowInput2ImGui()
{// Setup back-end capabilities flags
	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
	io.BackendPlatformName = "vulkan";

	// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
	io.KeyMap[ImGuiKey_Tab] =   KeyType::KEY_TAB ;
	io.KeyMap[ImGuiKey_LeftArrow] =   KeyType::KEY_LEFT ;
	io.KeyMap[ImGuiKey_RightArrow] =   KeyType::KEY_RIGHT ;
	io.KeyMap[ImGuiKey_UpArrow] =   KeyType::KEY_UP ;
	io.KeyMap[ImGuiKey_DownArrow] =   KeyType::KEY_DOWN ;
	io.KeyMap[ImGuiKey_PageUp] =   KeyType::KEY_PAGE_UP ;
	io.KeyMap[ImGuiKey_PageDown] =   KeyType::KEY_PAGE_DOWN ;
	io.KeyMap[ImGuiKey_Home] =   KeyType::KEY_HOME ;
	io.KeyMap[ImGuiKey_End] =   KeyType::KEY_END ;
	io.KeyMap[ImGuiKey_Insert] =   KeyType::KEY_INSERT ;
	io.KeyMap[ImGuiKey_Delete] =   KeyType::KEY_DELETE ;
	io.KeyMap[ImGuiKey_Backspace] =   KeyType::KEY_BACKSPACE ;
	io.KeyMap[ImGuiKey_Space] =   KeyType::KEY_SPACE ;
	io.KeyMap[ImGuiKey_Enter] =   KeyType::KEY_ENTER ;
	io.KeyMap[ImGuiKey_Escape] =   KeyType::KEY_ESCAPE ;
	io.KeyMap[ImGuiKey_A] =   KeyType::KEY_A ;
	io.KeyMap[ImGuiKey_C] =   KeyType::KEY_C ;
	io.KeyMap[ImGuiKey_V] =   KeyType::KEY_V ;
	io.KeyMap[ImGuiKey_X] =   KeyType::KEY_X ;
	io.KeyMap[ImGuiKey_Y] =   KeyType::KEY_Y ;
	io.KeyMap[ImGuiKey_Z] =   KeyType::KEY_Z ;

	io.SetClipboardTextFn = setClipboardText;
	io.GetClipboardTextFn = getClipboardText;
	io.ClipboardUserData = this;
//#if defined(_WIN32)
//	io.ImeWindowHandle = (void*)glfwGetWin32Window(g_Window);
//#endif
}

void EditorUI::pollInputs()
{
	ImGuiIO& io = ImGui::GetIO();
	for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
	{
		// If a mouse press event came, always pass it as 
		// "mouse held this frame", so we don't miss click-release events 
		// that are shorter than 1 frame.
		io.MouseDown[i] = mouseButtonsHaveBeenPressed[i] || 
			window->getMouseButton(i) != 0;
		mouseButtonsHaveBeenPressed[i] = false;
	}
	this->updateCursor();
}

void EditorUI::setCaptureFlags()
{
	ImGuiIO& io = ImGui::GetIO();
	this->_wantCaptureKeyboard = io.WantCaptureKeyboard;
	this->_wantTextInput = io.WantTextInput;
	this->_wantSetMousePos = io.WantSetMousePos;
	this->_wantCaptureMouse = io.WantCaptureMouse;
}

FontAtlas EditorUI::getDefaultFontAtlas()
{
	ImGuiIO& io = ImGui::GetIO();
	FontAtlas f;
	io.Fonts->GetTexDataAsRGBA32(&f.pixels, &f.width, &f.height);
	return f;
}


void EditorUI::onFrameBufferResizeCallBack(int width, int height)
{
	// width and height are not used due to a side effect of using GLFW on Windows
	// basically they are correct ...
	// but become too old when window is re-opened after minimization giving 0 0
	this->editor->resizeSwapChain();
	this->updateImguiDisplay();
}

void EditorUI::onKeyCallBack(KeyType key, int scancode, ActionType action, ModifierKeyType mods)
{
	bool pressed = false;
	ImGuiIO& io = ImGui::GetIO();
	if (action == ActionType::PRESS || action == ActionType::REPEAT) {
		pressed = true;
	}

	io.KeysDown[key] = pressed;
	// Modifiers are not reliable across systems
	io.KeyCtrl = io.KeysDown[KeyType::KEY_LEFT_CTRL] || io.KeysDown[KeyType::KEY_RIGHT_CTRL];
	io.KeyShift = io.KeysDown[KeyType::KEY_LEFT_SHIFT] || io.KeysDown[KeyType::KEY_RIGHT_SHIFT];
	io.KeyAlt = io.KeysDown[KeyType::KEY_LEFT_ALT] || io.KeysDown[KeyType::KEY_RIGHT_ALT];
	io.KeySuper = io.KeysDown[KeyType::KEY_LEFT_SUPER] || io.KeysDown[KeyType::KEY_RIGHT_SUPER];
}
void EditorUI::onCharCallback(unsigned int code_point)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(code_point);
}

void EditorUI::onCursorPosCallback(double xpos, double ypos)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2((float)xpos, (float)ypos);
}

void EditorUI::onMouseButtonCallback(MouseButtonType button, ActionType action, ModifierKeyType mods)
{
	ImGuiIO& io = ImGui::GetIO();
	if (action == ActionType::PRESS && button >= 0
		&& button < IM_ARRAYSIZE(mouseButtonsHaveBeenPressed))
		mouseButtonsHaveBeenPressed[button] = true;
}

void EditorUI::onScrollCallback(double xoffset, double yoffset)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheel += yoffset;
	io.MouseWheelH += xoffset;
}

void EditorUI::onDropCallback(int count, const char ** paths)
{
	for (int i = 0; i < count; i++)
		this->debug_logs.push_back(paths[i]);	
}


vkengine::VulkanInstanceInitInfo EditorUI::getInstanceExtInfo()
{
	vkengine::VulkanInstanceInitInfo info = {};
	info.instanceExtensions = WindowManager::getRequiredInstanceExtensions4Vulkan(&info.instance_extension_count);
	return info;
}

void * EditorUI::getSurface(void * vulkan_instance)
{
	if (!this->surface)
		this->window->createWindowSurface(vulkan_instance, &this->surface);
	return this->surface;
}

void EditorUI::getFrameBufferSize(int * width, int * height)
{
	this->window->getFrameBufferSize(width, height);
}

void EditorUI::printDebug(std::string msg)
{
	debug_logs.push_back(msg);
	if (debug_logs.size() > 100) debug_logs.pop_front();
}

void EditorUI::waitEvents()
{
	WindowManager::waitEvents();
}


void setClipboardText(void * user_pointer, const char * text)
{
	auto UI = static_cast<EditorUI*>(user_pointer);
	return UI->getWindow()->setClipboardText(text);
}

const char * getClipboardText(void * user_pointer)
{
	auto UI = static_cast<EditorUI*>(user_pointer);
	return UI->getWindow()->getClipboardText();
}

vkengine::UiDrawData out_put_draw_data(ImDrawData* data)
{
	vkengine::UiDrawData ui_data = {};
	ui_data.display_pos = data->DisplayPos;
	ui_data.display_size = data->DisplaySize;
	ui_data.totalIdxCount = data->TotalIdxCount;
	ui_data.totalVtxCount = data->TotalVtxCount;
	ui_data.frame_buffer_scale = data->FramebufferScale;
	for (int i = 0; i < data->CmdListsCount; i++) {
		ImDrawList* list = data->CmdLists[i];
		vkengine::UiDrawList myList = {};
		myList.vertexBuffer = list->VtxBuffer.Data;
		myList.vertexBufferSize = list->VtxBuffer.Size;
		myList.indexBuffer = list->IdxBuffer.Data;
		myList.indexBufferSize = list->IdxBuffer.Size;
		for (ImDrawCmd cmd : list->CmdBuffer) {
			vkengine::UiDrawCmd my_cmd = {};
			my_cmd.elementCount = cmd.ElemCount;
			my_cmd.clipRectangle = cmd.ClipRect;
			my_cmd.textureID = reinterpret_cast<uint32_t>(cmd.TextureId);
			myList.drawCommands.push_back(my_cmd);
		}
		ui_data.drawLists.push_back(myList);
	}
	return ui_data;
}

void EditorUI::setUpImGuiStyle()
{
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	ImGuiStyle& style = ImGui::GetStyle();

	// light style from Pacome Danhiez (user itamago)
	//style.Alpha = 1.0f;
	//style.FrameRounding = 3.0f;
	//style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	//style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	//style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
	//style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	//style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
	//style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.19f);
	//style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
	//style.Colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
	//style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	//style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	//style.Colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
	//style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
	//style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
	//style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	//style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	//style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
	//style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
	//style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	//style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	//style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	//style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	//style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	//style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	//style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	//style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	//style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	//style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	//style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	//style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	//style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	//style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	//style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	//style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	//style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	//style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	//style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	//style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	//style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	//style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}