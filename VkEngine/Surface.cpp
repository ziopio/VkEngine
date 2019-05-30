#include "stdafx.h"
#include "Instance.h"
#include "Surface.h"



Surface::Surface()
{

}

Surface::~Surface()
{
	//vkDestroySurfaceKHR(Instance::get(), surface, nullptr);
}

void Surface::createSurface()
{
	//if (glfwCreateWindowSurface(Instance::get(), window, nullptr, &surface) != VK_SUCCESS) {
	//	throw std::runtime_error("failed to create window surface!");
	//}
}
