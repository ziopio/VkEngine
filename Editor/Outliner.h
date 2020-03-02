#pragma once
#include "EditorComponent.h"
#include <string>
#include <vector>

//enum NodeType {
//	OBJECT,
//	CAMERA,
//	LIGHT
//};
//
//struct SceneNode {
//	std::string name;
//	std::string id;
//	NodeType type;
//	std::vector<SceneNode> sub_nodes;
//};

class Outliner :
	public EditorComponent
{
public:
	Outliner(EditorUI* UI);
	virtual void draw(int w_width, int w_height) override;
	~Outliner();
private:
	//std::vector<SceneNode> scenes;
	std::string selected_element;
};

