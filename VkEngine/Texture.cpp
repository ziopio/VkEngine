#include "commons.h"
#include "Texture.h"
#include "ApiUtils.h"
#include "PhysicalDevice.h"
#include "Device.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Libraries/stb_image.h"

Texture::Texture()
{
	unsigned char pixels[] = { 255, 0., 255., 255. }; // purple default
	int width = 1, height = 1;
	this->Texture::Texture(pixels, &width, &height);
}

Texture::Texture(std::string texturePath)
{
	unsigned char* pixels;
	int width, height;
	pixels = this->readImageFile(texturePath, &width, &height);
	this->Texture::Texture(pixels, &width, &height);
	stbi_image_free(pixels);
}

Texture::Texture(unsigned char* pixels, int *width, int *height)
{
	this->createTextureImage(pixels,*width,*height);
	this->createTextureImageView();
	this->createTextureSampler();
}

Texture::Texture(int width, int height, VkFormat format)
{
	this->createTextureImage( width, height, format);
	this->createTextureImageView();
	this->createTextureSampler();
}


VkImageView Texture::getTextureImgView()
{
	return this->textureImageView;
}

VkSampler Texture::getTextureSampler()
{
	return this->textureSampler;
}


Texture::~Texture()
{
	vkDestroySampler(Device::get(), textureSampler, nullptr);
	vkDestroyImageView(Device::get(), textureImageView, nullptr);
	vkDestroyImage(Device::get(), textureImage, nullptr);
	vkFreeMemory(Device::get(), textureImageMemory, nullptr);
}

unsigned char* Texture::readImageFile(std::string texturePath, int * width, int * height)
{	// file loading
	int channels;
	unsigned char * pixels = stbi_load(texturePath.c_str(), width, height, &channels, STBI_rgb_alpha); // STBI_rgb_alpha RGBA forced
	if (!pixels) {
		throw std::runtime_error("failed to load texture image: " + texturePath);
	}
	return pixels;
}

void Texture::createTextureImage(int width, int height, VkFormat format)
{
	createImage(PhysicalDevice::get(), Device::get(), width, height,
		format, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		textureImage, textureImageMemory);
}


void Texture::createTextureImage(unsigned char * pixels, int width, int height)
{	// load image to an accessible stage buffer
	VkDeviceSize imageSize = width * height * 4 * sizeof(char);
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(PhysicalDevice::get(), Device::get(),
		imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);
	void* data;
	vkMapMemory(Device::get(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(Device::get(), stagingBufferMemory);

	createImage(PhysicalDevice::get(), Device::get(), width, height,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		textureImage, textureImageMemory);

	transitionImageLayout(Device::get(), Device::getGraphicQueue(), Device::getGraphicCmdPool(),
		textureImage,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	copyBufferToImage(Device::get(), Device::getGraphicQueue(), Device::getGraphicCmdPool(),
		stagingBuffer, textureImage,
		static_cast<uint32_t>(width), static_cast<uint32_t>(height));

	transitionImageLayout(Device::get(), Device::getGraphicQueue(), Device::getGraphicCmdPool(),
		textureImage,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); // shader accessible

	vkDestroyBuffer(Device::get(), stagingBuffer, nullptr);
	vkFreeMemory(Device::get(), stagingBufferMemory, nullptr);
}

void Texture::createTextureImageView() {
	this->textureImageView = createImageView(Device::get(),textureImage, 
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Texture::createTextureSampler()
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE; // usefull for percentage-close filtering for shadowmaps
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(Device::get(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}
