#pragma once
#include "EditorComponent.h"
class View3D :
	public EditorComponent
{
public:
	View3D(EditorUI* UI);
	virtual void draw() override;
	~View3D();
};

