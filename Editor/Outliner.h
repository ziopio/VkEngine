#pragma once
#include "EditorComponent.h"
#include <string>
#include <vector>

enum class NodeType {
	OBJECT,
	CAMERA,
	LIGHT,
	SCENE
};
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
	virtual void resetForNewScene();
	void resetSelection();
	~Outliner();
private:
	//std::vector<SceneNode> scenes;
	int selected_element = -1;
	NodeType selected_elem_type;
};

