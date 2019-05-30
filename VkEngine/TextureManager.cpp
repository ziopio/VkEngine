#include "stdafx.h"
#include "TextureManager.h"


std::vector<Texture*> TextureManager::textures;


void TextureManager::addTexture(std::string texture_path)
{
	textures.push_back(new Texture(texture_path));
}

Texture * TextureManager::getTexture(int id)
{
	return textures[id];
}

void TextureManager::loadAllTextures()
{
}

void TextureManager::cleanUp()
{
	for (auto text : textures) {
		delete text;
	}
}
