#pragma once
#include "Texture.h"


class TextureManager
{
public:
	static void init();	
	//User must free memory pointed by pixels
	static void loadFontAtlasTexture(unsigned char * pixels,int* width, int* height);
	static Texture* getFontAtlasTexture();
	static void addTexture(std::string texture_path);
	static Texture* getTexture(int id);
	static int getTextureCount();
	static void cleanUp();
private:
	static std::vector<Texture*> textures;
	static Texture* fontAtlas;
};

