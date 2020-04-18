#pragma once
#include "Texture.h"

class TextureManager
{
public:
	static void init();
	//User must free memory pointed by pixels
	static void loadFontAtlasTexture(unsigned char * pixels, int* width, int* height);
	static Texture* getImGuiTexture(int id);
	static void addTexture(std::string id, std::string texture_path);
	static void addImGuiTexture(unsigned char * pixels, int* width, int* height);
	// by string id
	static inline Texture* getSceneTexture(std::string id) 
	{ return scene_textures[scene_textures_indices[id]]; };
	// by position
	static inline Texture* getSceneTexture(unsigned int index) 
	{ return scene_textures[index]; };
	// Used to get the index in the texture array (both for CPU and GPU side data)
	static inline unsigned int getSceneTextureIndex(std::string id) { return scene_textures_indices[id]; };
	static std::vector<std::string> listSceneTextures();
	static inline unsigned countSceneTextures() { return scene_textures.size(); }
	// by position
	static inline Texture* getImGuiTexture(unsigned int index)
	{ return scene_textures[index]; };
	static inline unsigned countImGuiTextures() { return imgui_textures.size(); }
	static void cleanUp();
private:
	static std::vector<Texture*> scene_textures;
	// This maps the texutres IDs with their position in the vector
	static std::unordered_map<std::string, unsigned> scene_textures_indices;
	static Texture* fontAtlas;
	static std::vector<Texture*> imgui_textures;	
	static std::unordered_map<std::string, unsigned> imgui_textures_indices;
};

