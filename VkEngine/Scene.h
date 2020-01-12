#pragma once
#include <vector>

class Object;
class Camera;

class Scene
{
private:
	float clearColor[4];
	std::vector<Object*> objects;
	std::vector<Camera*> cameras;
public:
	Scene();
	void setClearColor(float clearColor[4]);
	void addCamera(Camera*);
	Camera* getCurrentCamera();
	void addObject(Object*);
	~Scene();
};

