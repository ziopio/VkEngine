#pragma once
class Texture
{
public:
	Texture(std::string texturePath);
	//User must free memory pointed by pixels
	Texture(unsigned char* pixels, int * width, int * height);
	VkImageView getTextureImgView();
	VkSampler getTextureSampler();
	~Texture();
	
private:
	unsigned char* readImageFile(std::string texturePath,	int * width, 
		int * height);
	void createTextureImage(unsigned char* pixels, int width, 
		int height);
	void createTextureImageView();
	void createTextureSampler();
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
};

