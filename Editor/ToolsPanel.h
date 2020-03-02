#pragma once
#include "EditorComponent.h"

constexpr const unsigned tools_panel_width = 50;

class ToolsPanel : public EditorComponent
{
public:
	ToolsPanel(EditorUI* UI);
	virtual void draw(int w_width, int w_height) override;
	~ToolsPanel();
};

