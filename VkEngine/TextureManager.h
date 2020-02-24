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
	static void addTexture(std::string id, std::string texture_path);
	static void addDynamicTexture(int width, int heigth, VkFormat format);
	static Texture* getTexture(std::string id); // by string id
	static Texture* getTexture(unsigned int index); // by position
	// Used to get the index in the texture array (both for CPU and GPU side data)
	static unsigned int getTextureIndex(std::string id);
	static int getTextureCount();
	static void cleanUp();
private:
	static std::vector<Texture*> textures;
	// This maps the texutres IDs with their position in the vector
	static std::unordered_map<std::string, int> textures_indices;
	static Texture* fontAtlas;
	static std::vector<Texture*> dynamicTextures;
};

