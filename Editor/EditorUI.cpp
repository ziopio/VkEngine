#include "EditorUI.h"
#include "Editor.h"
#include "../ImGui/imgui.h"
#include <iostream>
#include <ctime>

#define VIEW_3D_TEXTURE_CODE -1 // special case in gui fragment shader

#define W_WIDTH 1366
#define W_HEIGHT 768

static bool show_demo_window = true;

void setClipboardText(void* user_pointer, const char* text);
const char* getClipboardText(void* user_pointer);
//Takes data and pointers from ImDrawData and copies those to UiDrawData
UiDrawData out_put_draw_data(ImDrawData* data);

EditorUI::EditorUI(Editor* editor)
{   
	this->editor = editor;
	this->window = WindowManager::createWindow(W_WIDTH, W_HEIGHT, "Editor!!!");
	this->window->registerEventHandler(this);
	this->window->activateCharCallback();
	this->window->activateKeyCallBack();
	this->window->activateMouseButtonCallback();
	this->window->activateCursorPosCallback();
	this->window->activateScrollCallback();
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
	int f_width, f_height;
	this->window->getFrameBufferSize(&f_width,&f_height);
	this->updateImguiDisplay(f_width, f_height);
}

UiDrawData EditorUI::drawUI()
{
	this->pollInputs();
	ImGui::NewFrame();
	this->setCaptureFlags();

	int x, y;
	this->window->getWindowSize(&x, &y);

	if (show_demo_window) {
		ImGui::ShowDemoWindow(&show_demo_window);
	}
	// 3D View
	{
		ImGui::SetNextWindowSize(ImVec2(x / 2, y / 2), ImGuiCond_Once);
		ImGui::Begin("3D View");
		ImVec2 size = ImGui::GetWindowSize(); size.x -= 25; size.y -= 40;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Image((ImTextureID) VIEW_3D_TEXTURE_CODE, size,
			ImVec2(0, 0), ImVec2(1, 1), 
			ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImGui::End();
	}
	// Debug logging window
	{	
		ImGui::SetNextWindowPos(ImVec2(0, y - y / 4), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(x , y / 4), ImGuiCond_Always);
		ImGui::Begin("Validation Layers");
		ImGui::Text(debug_logs.c_str());
		ImGui::End();
	}
	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

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
	WindowManager::destroyWindow(this->window);
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

void EditorUI::updateImguiDisplay(int width, int height)
{
	int w_width, w_height;
	this->window->getWindowSize(&w_width, &w_height);
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)w_width, (float)w_height);
	if (io.DisplaySize.x > 0 && io.DisplaySize.y > 0)
	{
		io.DisplayFramebufferScale =
			ImVec2((float)width / io.DisplaySize.x,
			(float)height / io.DisplaySize.y);
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
	this->editor->resizeSwapChain(width, height);
	this->updateImguiDisplay(width, height);
}

void EditorUI::onKeyCallBack(KeyType key, int scancode, ActionType action, ModifierKeyType mods)
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
	std::cout << "Drop file callback" << std::endl;
}

VulkanInstanceInitInfo EditorUI::getInstanceExtInfo()
{
	VulkanInstanceInitInfo info = {};
	info.instanceExtensions = WindowManager::getRequiredInstanceExtensions4Vulkan(&info.instance_extension_count);
	info.enableValidation = true;
	return info;
}

void * EditorUI::getSurface(void * vulkan_instance)
{
	void* surface;
	this->window->createWindowSurface(vulkan_instance, &surface);
	return surface;
}

void EditorUI::getFrameBufferSize(int * width, int * height)
{
	this->window->getFrameBufferSize(width, height);
}

void EditorUI::printDebug(std::string msg)
{
	debug_logs.append(msg);
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
