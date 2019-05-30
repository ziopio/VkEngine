#pragma once
#include "Texture.h"

#define MAX_TEXTURE_COUNT 1024

class TextureManager
{
public:
	static void addTexture(std::string texture_path);
	static Texture* getTexture(int id);
	static void loadAllTextures();
	static void cleanUp();
private:
	static std::vector<Texture*> textures;
};

