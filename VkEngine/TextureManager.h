#pragma once
#include "Texture.h"

#define MAX_TEXTURE_COUNT 32

class TextureManager
{
public:
	static void init();
	static void addTexture(std::string texture_path);
	static Texture* getTexture(int id);
	static void cleanUp();
private:
	static std::vector<Texture*> textures;
};

