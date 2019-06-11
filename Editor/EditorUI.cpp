#include "EditorUI.h"
#include "Editor.h"
#include "../ImGui/imgui.h"

static bool show_demo_window = true;

void setClipboardText(void* user_pointer, const char* text);
const char* getClipboardText(void* user_pointer);
//Takes data and pointer from ImDrawData and copies to UiDrawData
UiDrawData out_put_draw_data(ImDrawData* data);

EditorUI::EditorUI(Editor* editor)
{   
	this->editor = editor;
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
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

	ImGui::NewFrame();
	ImGui::ShowDemoWindow(&show_demo_window);

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
}

void EditorUI::updateMouseButton(int button, int action, int mods)
{
}

void EditorUI::updateCursor()
{
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
	ui_data.elemtCounts.reserve(data->CmdListsCount);
	ui_data.clipRectangles.reserve(data->CmdListsCount);
	ui_data.vertexBuffers.reserve(data->CmdListsCount);
	ui_data.vertexBuffersSizes.reserve(data->CmdListsCount);
	ui_data.indexBuffers.reserve(data->CmdListsCount);
	ui_data.indexBuffersSizes.reserve(data->CmdListsCount);
	for (int i = 0; i < data->CmdListsCount; i++) {
		ImDrawList* list = data->CmdLists[i];
		ui_data.elemtCounts.push_back(list->CmdBuffer[i].ElemCount);
		ui_data.clipRectangles.push_back(list->CmdBuffer[i].ClipRect);
		ui_data.vertexBuffers.push_back((void*)(list->VtxBuffer.Data));
		ui_data.vertexBuffersSizes.push_back(list->VtxBuffer.Size);
		ui_data.indexBuffers.push_back((void*)(list->IdxBuffer.Data));
		ui_data.indexBuffersSizes.push_back(list->IdxBuffer.Size);
	}
	return ui_data;
}
