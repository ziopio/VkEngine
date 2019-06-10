#include "stdafx.h"
#include "TextureManager.h"


std::vector<Texture*> TextureManager::textures;
Texture* TextureManager::fontAtlas;

void TextureManager::init()
{
	TextureManager::addTexture("VkEngine/Textures/default_texture.png");
}

void TextureManager::loadFontAtlasTexture(unsigned char * pixels, 
	int* width, int* height)
{
	TextureManager::fontAtlas = new Texture(pixels,width,height);
}

Texture * TextureManager::getFontAtlasTexture()
{
	return TextureManager::fontAtlas;
}

void TextureManager::addTexture(std::string texture_path)
{
	textures.push_back(new Texture(texture_path));
}

Texture * TextureManager::getTexture(int id)
{
	return textures[id];
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
	delete fontAtlas;
}
