#pragma once
#include "EditorComponent.h"
class MainMenuBar :
    public EditorComponent
{
public:
    MainMenuBar(EditorUI* ui);

    virtual void draw(int w_width, int w_height) override;
    ~MainMenuBar();
};

