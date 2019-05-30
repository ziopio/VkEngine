#include "stdafx.h"
#include "Device.h"
#include "MaterialManager.h"
#include "RenderPass.h"


std::vector<Material*> MaterialManager::materials;
SwapChain*  MaterialManager::swapchain;
RenderPass* MaterialManager::renderPass;
bool MaterialManager::ready;


void MaterialManager::init(SwapChain * swapchain, RenderPass * renderPass)
{
	MaterialManager::swapchain = swapchain;
	MaterialManager::renderPass = renderPass;
	loadMaterials();
	ready = true;
}

Material* MaterialManager::getMaterial(MaterialType material)
{
	if (!ready) {
		throw std::runtime_error("Material Manager not initialized!");
	}
	return MaterialManager::materials[material];
}


void MaterialManager::destroyAllMaterials()
{
	for (auto mat : MaterialManager::materials) {
		delete mat;
	}
	MaterialManager::materials.clear();
	ready = false;
}



void MaterialManager::loadMaterials()
{
	MaterialManager::materials.push_back(new Material(MaterialType::SAMPLE, swapchain, renderPass));

	MaterialManager::materials.push_back(new Material(MaterialType::PHONG, swapchain, renderPass));
}
