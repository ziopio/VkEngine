#include "commons.h"
#include "Texture.h"
#include "ApiUtils.h"
#include "PhysicalDevice.h"
#include "Device.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Libraries/stb_image.h"

#include <filesystem>
namespace fs = std::filesystem;

Texture::Texture()
{
	unsigned char pixels[] = { 247, 231., 206., 255. }; // default
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

	VkCommandBuffer command = beginSingleTimeCommandBuffer(Device::get(), Device::getGraphicCmdPool());

	transitionImageLayout(command,
		textureImage,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	copyBufferToImage(command,
		stagingBuffer, textureImage,
		static_cast<uint32_t>(width), static_cast<uint32_t>(height));

	transitionImageLayout(command,
		textureImage,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL); // shader accessible

	submitAndWaitCommandBuffer(Device::get(), Device::getGraphicQueue(), Device::getGraphicCmdPool(), command);
	
	vkDestroyBuffer(Device::get(), stagingBuffer, nullptr);
	vkFreeMemory(Device::get(), stagingBufferMemory, nullptr);
}

void Texture::createTextureImageView() {
	this->textureImageView = createImageView(Device::get(),textureImage, 
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

void BaseTexture::createTextureSampler()
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

struct RawImage {
	unsigned char* pixels;
	int width, height;
};

CubeMapTexture::CubeMapTexture(std::string texturePath)
{
	constexpr const char* ext = ".png";
	const char* parts[6] = {"px","nx","py","ny","pz","nz"};
	RawImage faces[6] = {};
	VkDeviceSize buffer_size = 0;
	for (unsigned i = 0; i < 6; i++) 
	{
		faces[i].pixels = this->readImageFile(texturePath + "/" + parts[i] + ext, &faces[i].width, &faces[i].height);
		buffer_size += faces[i].width * faces[i].height * 4 * sizeof(char);
	}
	Buffer stage = {};
	createBuffer(PhysicalDevice::get(), Device::get(), buffer_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stage.vkBuffer,stage.vkMemory);	
	vkMapMemory(Device::get(), stage.vkMemory, 0, buffer_size, 0, &stage.mappedMemory);
	for (int i = 0; i < 6; i++)
	{
		memcpy((char*)stage.mappedMemory + (buffer_size / 6) * i, faces[i].pixels, buffer_size / 6);
	}
	vkUnmapMemory(Device::get(), stage.vkMemory);

	createImage(PhysicalDevice::get(), Device::get(), faces[0].width, faces[0].height,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		textureImage, textureImageMemory, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

	auto cmdBuff = beginSingleTimeCommandBuffer(Device::get(),Device::getGraphicCmdPool());

	transitionImageLayout(cmdBuff,
		textureImage,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6);

	std::vector<VkBufferImageCopy> regions(6);
	for (int face = 0; face < 6; face++)
	{
		regions[face] = {};
		regions[face].bufferOffset = (buffer_size / 6) * face;
		regions[face].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		regions[face].imageSubresource.mipLevel = 0;
		regions[face].imageSubresource.baseArrayLayer = face;
		regions[face].imageSubresource.layerCount = 1;
		regions[face].imageExtent = { 
			static_cast<uint32_t>(faces[face].width), 
			static_cast<uint32_t>(faces[face].height),
			1
		};
	}
	copyBufferToImage(cmdBuff, stage.vkBuffer, textureImage, &regions);

	transitionImageLayout(cmdBuff,
		textureImage,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6); // shader accessible

	submitAndWaitCommandBuffer(Device::get(),Device::getGraphicQueue(),Device::getGraphicCmdPool(),cmdBuff);
	createTextureSampler();

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = textureImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 6;

	if (vkCreateImageView(Device::get(), &viewInfo, nullptr, &this->textureImageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	vkDestroyBuffer(Device::get(), stage.vkBuffer, nullptr);
	vkFreeMemory(Device::get(), stage.vkMemory, nullptr);
	for (int i = 0; i < 6; i++)
	{
		stbi_image_free(faces[i].pixels);
	}
}

VkImageView BaseTexture::getTextureImgView()
{
	return this->textureImageView;
}

VkSampler BaseTexture::getTextureSampler()
{
	return this->textureSampler;
}

BaseTexture::~BaseTexture()
{
	vkDestroySampler(Device::get(), textureSampler, nullptr);
	vkDestroyImageView(Device::get(), textureImageView, nullptr);
	vkDestroyImage(Device::get(), textureImage, nullptr);
	vkFreeMemory(Device::get(), textureImageMemory, nullptr);
}

unsigned char* BaseTexture::readImageFile(std::string texturePath, int* width, int* height)
{	// file loading
	int channels;
	unsigned char* pixels = stbi_load(texturePath.c_str(), width, height, &channels, STBI_rgb_alpha); // STBI_rgb_alpha RGBA forced
	if (!pixels) {
		throw std::runtime_error("failed to load texture image: " + texturePath);
	}
	return pixels;
}
