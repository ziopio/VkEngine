#pragma once
#include "Texture.h"


class TextureManager
{
public:
	static void init();	
	//User must free memory pointed by pixels
	static void loadFontAtlasTexture(unsigned char * pixels,int* width, int* height);
	static Texture* getDynamicTexture(int id);
	static Texture* getFontAtlasTexture();
	static void addTexture(std::string texture_path);
	static void addDynamicTexture(int width, int heigth, VkFormat format);
	static Texture* getTexture(int id);
	static int getTextureCount();
	static void cleanUp();
private:

	static std::vector<Texture*> textures;
	static Texture* fontAtlas;
	static std::vector<Texture*> dynamicTextures;
};

