#pragma once
class Texture
{
public:
	// default texture loading
	Texture();
	//Texture loading from file
	Texture(std::string texturePath);
	//User must free memory pointed by pixels
	Texture(unsigned char* pixels, int * width, int * height);
	// Creates an empty texture in the Device memory
	Texture(int width, int height, VkFormat format);
	VkImageView getTextureImgView();
	VkSampler getTextureSampler();
	~Texture();
	
private:
	unsigned char* readImageFile(std::string texturePath,	int * width, 
		int * height);
	void createTextureImage(unsigned char* pixels, int width, 
		int height);
	void createTextureImage(int width, int height, VkFormat format);
	void createTextureImageView();
	void createTextureSampler();
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
};

