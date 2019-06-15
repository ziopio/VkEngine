#include "EditorUI.h"
#include "Editor.h"
#include "../ImGui/imgui.h"
#include <ctime>

static bool show_demo_window = true;
static bool show_another_window = true;
static std::string debug_logs = "";
float clear_color[3] = {0,0,0};

void setClipboardText(void* user_pointer, const char* text);
const char* getClipboardText(void* user_pointer);
//Takes data and pointers from ImDrawData and copies those to UiDrawData
UiDrawData out_put_draw_data(ImDrawData* data);

EditorUI::EditorUI(Editor* editor)
{   
	this->editor = editor;
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();
	this->mapWindowInput2ImGui();
	int w_width, w_height, f_width, f_height;
	this->editor->getWindow()->getWindowSize(&w_width,&w_height);
	this->editor->getFrameBufferSize(&f_width,&f_height);
	this->updateFrameSize(w_width,w_height,f_width,f_height);
}

UiDrawData EditorUI::drawUI()
{
	this->pollInputs();
	ImGui::NewFrame();
	this->setCaptureFlags();
	if (show_demo_window) {
		ImGui::ShowDemoWindow(&show_demo_window);
	}
	// Debug logging window
	{	
		int x, y;
		this->editor->getWindow()->getWindowSize(&x,&y);
		ImGui::SetNextWindowPos(ImVec2(0, y - y / 4), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(x , y / 4), ImGuiCond_Always);
		ImGui::Begin("Validation Layers");
		ImGui::Text(debug_logs.c_str());
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
}

void EditorUI::updateFrameSize(int w_width, int w_height, int frame_width, int frame_height)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)w_width, (float)w_height);
	if (io.DisplaySize.x > 0 && io.DisplaySize.y > 0)
	{
		io.DisplayFramebufferScale =
			ImVec2((float)frame_width / io.DisplaySize.x,
			(float)frame_height / io.DisplaySize.y);
	}
}

void EditorUI::updateMousePos(double xpos, double ypos)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2((float)xpos, (float)ypos);
}

void EditorUI::updateMouseButton(int button, int action, int mods)
{
	ImGuiIO& io = ImGui::GetIO();
	if (action == ActionType::PRESS && button >= 0 
		&& button < IM_ARRAYSIZE(mouseButtonsHasBeenPressed))
		mouseButtonsHasBeenPressed[button] = true;
}

void EditorUI::updateKeyboard(int key, int scancode, int action, int mods)
{
	bool pressed = false;
	ImGuiIO& io = ImGui::GetIO();
	if (action == ActionType::PRESS) {
		pressed = true;
	}
	io.KeysDown[key] = pressed;
	// Modifiers are not reliable across systems
	io.KeyCtrl = io.KeysDown[KeyType::KEY_LEFT_CTRL] || io.KeysDown[KeyType::KEY_RIGHT_CTRL];
	io.KeyShift = io.KeysDown[KeyType::KEY_LEFT_SHIFT] || io.KeysDown[KeyType::KEY_RIGHT_SHIFT];
	io.KeyAlt = io.KeysDown[KeyType::KEY_LEFT_ALT] || io.KeysDown[KeyType::KEY_RIGHT_ALT];
	io.KeySuper = io.KeysDown[KeyType::KEY_LEFT_SUPER] || io.KeysDown[KeyType::KEY_RIGHT_SUPER];
}

void EditorUI::updateChar(unsigned int character)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(character);
}

void EditorUI::updateScroll(double xoffset, double yoffset)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheel += yoffset;
	io.MouseWheelH += xoffset;
}

void EditorUI::updateCursor()
{
	auto win = this->editor->getWindow();
	ImGuiIO& io = ImGui::GetIO();
	if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) || 
		win->getInputMode(InputMode::CURSOR) == InputModeValueType::CURSOR_DISABLED)
		return;

	ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
	if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
	{
		// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
		win->setInputMode(InputMode::CURSOR, InputModeValueType::CURSOR_HIDDEN);
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
		win->setCursor(newCursor);
		win->setInputMode(InputMode::CURSOR, InputModeValueType::CURSOR_NORMAL);
	}
}

void EditorUI::setDeltaTime(double delta_time)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = (float)delta_time > 0 ? (float)delta_time : 0;
}

void EditorUI::showDebugString(std::string debug_info)
{
	debug_logs.append(debug_info);
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
	io.ClipboardUserData = this->editor;
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
		io.MouseDown[i] = mouseButtonsHasBeenPressed[i] || 
			this->editor->getWindow()->getMouseButton(i) != 0;
		mouseButtonsHasBeenPressed[i] = false;
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

void setClipboardText(void * user_pointer, const char * text)
{
	auto editor = static_cast<Editor*>(user_pointer);
	editor->getWindow()->setClipboardText(text);
}

const char * getClipboardText(void * user_pointer)
{
	auto editor = static_cast<Editor*>(user_pointer);	
	return editor->getWindow()->getClipboardText();
}

UiDrawData out_put_draw_data(ImDrawData* data)
{
	UiDrawData ui_data = {};
	ui_data.display_pos = data->DisplayPos;
	ui_data.display_size = data->DisplaySize;
	ui_data.totalIdxCount = data->TotalIdxCount;
	ui_data.totalVtxCount = data->TotalVtxCount;
	ui_data.frame_buffer_scale = data->FramebufferScale;
	for (int i = 0; i < data->CmdListsCount; i++) {
		ImDrawList* list = data->CmdLists[i];
		UiDrawList myList = {};
		myList.vertexBuffer = list->VtxBuffer.Data;
		myList.vertexBufferSize = list->VtxBuffer.Size;
		myList.indexBuffer = list->IdxBuffer.Data;
		myList.indexBufferSize = list->IdxBuffer.Size;
		for (ImDrawCmd cmd : list->CmdBuffer) {
			UiDrawCmd my_cmd = {};
			my_cmd.elementCount = cmd.ElemCount;
			my_cmd.clipRectangle = cmd.ClipRect;
			my_cmd.textureID = reinterpret_cast<uint32_t>(cmd.TextureId);
			myList.drawCommands.push_back(my_cmd);
		}
		ui_data.drawLists.push_back(myList);
	}
	return ui_data;
}
