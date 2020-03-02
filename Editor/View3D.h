#pragma once
#include "EditorComponent.h"

class View3D :
	public EditorComponent
{
public:
	View3D(EditorUI* UI);
	virtual void draw(int w_width, int w_height) override;
	~View3D();
};

