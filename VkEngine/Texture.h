#pragma once

class BaseTexture {
public:
	VkImageView getTextureImgView();
	VkSampler getTextureSampler();
	~BaseTexture();
protected:
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
	unsigned char* readImageFile(std::string texturePath, int* width,
		int* height);
	void createTextureSampler();
};

class Texture : public BaseTexture
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
private:
	void createTextureImage(unsigned char* pixels, int width, 
		int height);
	void createTextureImage(int width, int height, VkFormat format);
	void createTextureImageView();

};

class CubeMapTexture : public BaseTexture
{
public:
	CubeMapTexture(std::string texturePath);
};
