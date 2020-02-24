#include "stdafx.h"
#include "TextureManager.h"


std::vector<Texture*> TextureManager::textures;
std::unordered_map<std::string, int> TextureManager::textures_indices;
std::vector<Texture*> TextureManager::dynamicTextures;
Texture* TextureManager::fontAtlas;

void TextureManager::init()
{
	textures.push_back(new Texture());
}

void TextureManager::loadFontAtlasTexture(unsigned char * pixels, 
	int* width, int* height)
{
	TextureManager::fontAtlas = new Texture(pixels,width,height);
}

Texture * TextureManager::getDynamicTexture(int id)
{
	return dynamicTextures[id];
}

Texture * TextureManager::getFontAtlasTexture()
{
	if (TextureManager::fontAtlas == nullptr) {
		return TextureManager::textures[0];
	}
	return TextureManager::fontAtlas;
}

void TextureManager::addTexture(std::string id, std::string texture_path)
{
	textures.push_back(new Texture(texture_path));
	textures_indices[id] = textures.size() - 1;
}

void TextureManager::addDynamicTexture(int width, int heigth, VkFormat format)
{
	dynamicTextures.push_back(new Texture(width, heigth, format));
}

Texture * TextureManager::getTexture(std::string id)
{
	return textures[textures_indices[id]];
}

Texture * TextureManager::getTexture(unsigned int index)
{
	return textures[index];
}

unsigned int TextureManager::getTextureIndex(std::string id)
{
	return textures_indices[id];
}

int TextureManager::getTextureCount()
{
	return textures.size();
}

void TextureManager::cleanUp()
{
	for (auto text : textures) {
		delete text;
	}
	textures_indices.clear();
	delete fontAtlas;
}
