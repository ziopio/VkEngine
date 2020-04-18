#include "stdafx.h"
#include "TextureManager.h"


std::vector<Texture*> TextureManager::scene_textures;
std::unordered_map<std::string, unsigned> TextureManager::scene_textures_indices;
std::vector<Texture*> TextureManager::imgui_textures;
std::unordered_map<std::string, unsigned> TextureManager::imgui_textures_indices;

void TextureManager::init()
{
	scene_textures.push_back(new Texture()); // default / "place holder" texture
	scene_textures_indices["default"] = scene_textures.size() - 1;
}

void TextureManager::loadFontAtlasTexture(unsigned char * pixels, 
	int* width, int* height)
{
	imgui_textures.push_back(new Texture(pixels,width,height));
}

Texture * TextureManager::getImGuiTexture(int id)
{
	return imgui_textures[id];
}

void TextureManager::addTexture(std::string id, std::string texture_path)
{
	scene_textures.push_back(new Texture(texture_path));
	scene_textures_indices[id] = scene_textures.size() - 1;
}

void TextureManager::addImGuiTexture(unsigned char * pixels, int* width, int* height)
{
	imgui_textures.push_back(new Texture(pixels, width, height));
}

std::vector<std::string> TextureManager::listSceneTextures()
{
	std::vector<std::string> tex_ids;
	for (auto id : TextureManager::scene_textures_indices) {
		tex_ids.push_back(id.first);
	}
	return tex_ids;
}

void TextureManager::cleanUp()
{
	for (auto text : scene_textures) {
		delete text;
	}
	for (auto text : imgui_textures) {
		delete text;
	}
	scene_textures_indices.clear();
}
