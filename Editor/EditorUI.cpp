#include "EditorUI.h"
#include "Editor.h"
#include "../ImGui/imgui.h"
#include <ctime>

static bool show_demo_window = true;
static bool show_another_window = true;
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
	//ImGuiIO& io = ImGui::GetIO();
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
	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;
		ImGui::Begin("Hello, world!");                         // Create a window called "Hello, world!" and append into it.
		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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
