#pragma once

const int WIDTH = 1366;
const int HEIGHT = 768;

class Surface
{
public:
	Surface();
	~Surface();
	bool framebufferResized;
private:
	void createSurface();
};

