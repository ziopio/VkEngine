#pragma once
class Texture
{
public:
	Texture(std::string texturePath);
	VkImageView getTextureImgView();
	VkSampler getTextureSampler();
	~Texture();
	
private:
	void createTextureImage(std::string texturePath);
	void createTextureImageView();
	void createTextureSampler();
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
};

