#include "stdafx.h"
#include "Renderer.h"
#include "PhysicalDevice.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "MaterialManager.h"
#include "DescriptorSetsFactory.h"
#include "MeshManager.h"
#include "TextureManager.h"
#include "Direction.h"
#include "LightSource.h"
#include "ApiUtils.h"

// function to feed a thread job
void threadRenderCode(Object* obj, vks::Frustum frustum, ThreadData* threadData, uint32_t frameBufferIndex, 
	uint32_t cmdBufferIndex,VkCommandBufferInheritanceInfo inheritanceInfo, VkDescriptorSet* descriptorSet);



Renderer::Renderer(RenderPass* renderPass, SwapChain* swapChain)
{
	this->swapChain = swapChain;
	this->renderPass = renderPass;
	this->multithreading = true;
	createDepthResources();
	createFramebuffers();	
	createSyncObjects();
}

void Renderer::setLights(std::vector<LightSource*> lights)
{
	this->lights = lights;
}

void Renderer::setObjects(std::vector<Object*> objects)
{
	this->objects = objects;
	this->findObjXthreadDivision();
	prepareThreadedRendering();
}

bool Renderer::renderScene()
{
	// Wait for fence to signal that all command buffers are ready
	vkWaitForFences(Device::get(), 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

	vkResetFences(Device::get(), 1, &inFlightFences[currentFrame]);

	// Prendo l'indice dell'immagine su cui disegnare dalla swapchain
	uint32_t imageIndex;
	if (!this->swapChain->acquireNextImage(imageAvailableSemaphores[currentFrame], &imageIndex)) {
		return false;
	}
	this->update_camera_infos();
	//Aggiorno tutti i commandBuffers

	this->updateCommandBuffer(imageIndex); 


	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	// Indico quale semaforo attendere e in che stato la pipeline deve fermarsi
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores; // indice semaforo = indice stage della pipeline
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &primaryCommandBuffers[imageIndex];
	// Indico quale è il semaforo che indica la fine dell'attesa
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(Device::getGraphicQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}
	// Presentazione del frame
	if (!swapChain->presentImage(imageIndex, &renderFinishedSemaphores[currentFrame])) {
		return false;
	}
	//vkQueueWaitIdle(presentQueue); not optimal time usage

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	return true;
}

Renderer::~Renderer()
{
	for (auto threadResource : this->per_thread_resources) {
		for (auto frameBufferCommandList : threadResource.commandBuffers) {
			vkFreeCommandBuffers(Device::get(), threadResource.commandPool, frameBufferCommandList.size(), frameBufferCommandList.data());
		}
		vkDestroyCommandPool(Device::get(), threadResource.commandPool, nullptr);
	}

	vkDestroyImageView(Device::get(), depth_buffer.depthImageView, nullptr);
	vkDestroyImage(Device::get(), depth_buffer.depthImage, nullptr);
	vkFreeMemory(Device::get(), depth_buffer.depthImageMemory, nullptr);

	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(Device::get(), framebuffer, nullptr);
	}
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(Device::get(), renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(Device::get(), imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(Device::get(), inFlightFences[i], nullptr);
	}}


void Renderer::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChain->getImageViews().size());
	// Un framebuffer per ogni immagine della swapchain
	for (size_t i = 0; i < swapChain->getImageViews().size(); i++) {
		std::array<VkImageView, 2> attachments = {
			swapChain->getImageViews()[i],
			depth_buffer.depthImageView
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = this->renderPass->get();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChain->getExtent().width;
		framebufferInfo.height = swapChain->getExtent().height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(Device::get(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void Renderer::createDepthResources() {
	VkFormat depthFormat = findDepthFormat(PhysicalDevice::get());
	createImage(PhysicalDevice::get(), Device::get(),
		swapChain->getExtent().width, swapChain->getExtent().height,
		depthFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depth_buffer.depthImage, depth_buffer.depthImageMemory);
	
	depth_buffer.depthImageView = createImageView(Device::get(), depth_buffer.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	
	transitionImageLayout(Device::get(), Device::getGraphicQueue(), Device::getGraphicCmdPool(),
		depth_buffer.depthImage, depthFormat, 
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void Renderer::prepareThreadedRendering()
{
	//allocazione buffer principali 1 per ogni frame
	primaryCommandBuffers.resize(swapChainFramebuffers.size());
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = Device::getGraphicCmdPool(); // using the standard Pool
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	for (int i = 0; i< swapChainFramebuffers.size();i++) {
		if (vkAllocateCommandBuffers(Device::get(), &allocInfo, &primaryCommandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate primary command buffer!");
		}
	}


	//allocazione pool e buffer secondari per ogni thread
	this->per_thread_resources.resize(numThreads);

	// for each thread
	for (uint32_t t = 0; t < this->numThreads; t++) {
		// 1 command pool
		Device::createCommandPool(PhysicalDevice::getQueueFamilies().graphicsFamily,
			&per_thread_resources[t].commandPool);
		// for each framebuffer...
		per_thread_resources[t].commandBuffers.resize(swapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = per_thread_resources[t].commandPool; // using the standard Pool
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		allocInfo.commandBufferCount = 1;

		for (uint32_t f = 0; f < swapChainFramebuffers.size(); f++) {
			per_thread_resources[t].commandBuffers[f].resize(objXthread);

			//... and for each object to draw, 1 command buffer.
			for (int i = 0; i < objXthread; i++) {
				if (vkAllocateCommandBuffers(Device::get(), &allocInfo, &per_thread_resources[t].commandBuffers[f][i]) != VK_SUCCESS) {
					throw std::runtime_error("failed to allocate primary command buffer!");
				}
			}
		}
	}
}

void Renderer::update_camera_infos()
{
	uniformBlockDefinition uniforms = {};
	uniforms.V_matrix = Direction::getCurrentCamera()->setCamera();
	uniforms.P_matrix = Direction::getCurrentCamera()->getProjection();
	uniforms.P_matrix[1][1] *= -1; // invert openGL Y sign
	uniforms.light_count = lights.size();
	for (int i = 0; i < lights.size(); i++) {
		uniforms.lights[i] = this->lights[i]->getData();
	}
	DescriptorSetsFactory::updateUniformBuffer(uniforms,this->currentFrame);

	this->frustum.update(uniforms.P_matrix * uniforms.V_matrix);
}

void Renderer::updateCommandBuffer(uint32_t frameBufferIndex)
{
	// Contains the list of secondary command buffers to be submitted
	std::vector<VkCommandBuffer> secondaryCmdBuffers;

	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.renderPass = renderPass->get();
	inheritanceInfo.framebuffer = swapChainFramebuffers[frameBufferIndex];
	// work is divided between the available threads
	vks::Frustum frustum_ = frustum;
	for (uint32_t t = 0, objIndex = 0; t < numThreads; t++)
	{
		for (uint32_t i = 0; i < objXthread && objIndex < objects.size(); i++, objIndex++)
		{
			VkDescriptorSet* set = nullptr;//DescriptorSetsFactory::getDescriptorSet(objects[objIndex]->getMatType());
			if (multithreading) {
				thread_pool.threads[t]->addJob([=] { threadRenderCode(objects[objIndex], frustum_, &per_thread_resources[t], frameBufferIndex, i, inheritanceInfo, set); });
			}
			else {
				threadRenderCode(objects[objIndex], frustum_, &per_thread_resources[t], frameBufferIndex, i, inheritanceInfo, set);
			}
		}
	}


	// begin main command recording
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	if (vkBeginCommandBuffer(primaryCommandBuffers[frameBufferIndex], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}
	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass->get();
	renderPassInfo.framebuffer = swapChainFramebuffers[frameBufferIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChain->getExtent();
	VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();
	// begin render pass
	vkCmdBeginRenderPass(primaryCommandBuffers[frameBufferIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	VkPipelineLayout playout = MaterialManager::getMaterial(MaterialType::SAMPLE)->getPipelineLayout();
	std::array<VkDescriptorSet, 2> desriptor_sets = {DescriptorSetsFactory::getStaticGlobalDescriptorSet(),DescriptorSetsFactory::getFrameDescriptorSet(frameBufferIndex)};
	vkCmdBindDescriptorSets(primaryCommandBuffers[frameBufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, 
		playout,
		0, 1, desriptor_sets.data(),
		0, nullptr);


	thread_pool.wait();
	// i draw only the command buffers related to visible objects
	for (uint32_t t = 0, objIndex = 0; t < numThreads; t++)
	{
		for (uint32_t i = 0;  i < objXthread && objIndex < objects.size(); i++, objIndex++)
		{
			if (objects[objIndex]->visible)
			{
				secondaryCmdBuffers.push_back(per_thread_resources[t].commandBuffers[frameBufferIndex][i]);
			}
		}
	}

	// Execute render commands from all secondary command buffers
	vkCmdExecuteCommands(primaryCommandBuffers[frameBufferIndex], secondaryCmdBuffers.size(), secondaryCmdBuffers.data());
	vkCmdEndRenderPass(primaryCommandBuffers[frameBufferIndex]);
	vkEndCommandBuffer(primaryCommandBuffers[frameBufferIndex]);
}

void Renderer::findObjXthreadDivision()
{
	// Trovo il numero di thread disponibili
	numThreads = std::thread::hardware_concurrency();
	// Se i thread superano il numero degli oggetti, 
	// riduco il numero di thread e imposto 1 oggetto per thread.
	if (objects.size() < numThreads) {
		numThreads = objects.size();
		objXthread = 1;
	}
	else {
		this->objXthread = objects.size() / numThreads;
	}
	//Imposto il numero di thread che la libreria deve utilizzare
	thread_pool.setThreadCount(numThreads);
	printf("\nRendering impostato su %i threads, %i oggetti ciascuno. Totale: %i oggetti.",numThreads, objXthread,objects.size());
}

/*
	This function assembles a command buffer for 1 object running on 1 thread
*/
void threadRenderCode(Object* obj, vks::Frustum frustum,ThreadData* threadData, uint32_t frameBufferIndex, uint32_t cmdBufferIndex,
	VkCommandBufferInheritanceInfo inheritanceInfo, VkDescriptorSet* descriptorSet)
{
	obj->visible = frustum.checkSphere(obj->getPos(), obj->getInfo().scale_factor );

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	beginInfo.pInheritanceInfo = &inheritanceInfo;

	VkCommandBuffer cmdBuffer = threadData->commandBuffers[frameBufferIndex][cmdBufferIndex];

	if (vkBeginCommandBuffer(cmdBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}
	//RenderPass has already been started by the main thread so here i just have to bind the needed data and draw.

	VkPipeline pipiline = MaterialManager::getMaterial(obj->getMatType())->getPipeline();
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipiline);

	VkBuffer vertexBuffers[] = { MeshManager::getMesh(obj->getMeshId())->getVkVertexBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(cmdBuffer, MeshManager::getMesh(obj->getMeshId())->getVkIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

	VkPipelineLayout pipelineLayout = MaterialManager::getMaterial(obj->getMatType())->getPipelineLayout();
	
	//vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descriptorSet, 0, nullptr);

	PushConstantBlock pushConsts = {};
	pushConsts.model_transform = obj->getMatrix();
	pushConsts.textureIndex = obj->getTextureId();
	vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConsts), &pushConsts);


	vkCmdDrawIndexed(cmdBuffer, static_cast<uint32_t>(MeshManager::getMesh(obj->getMeshId())->indices.size()), 1, 0, 0, 0);
	vkEndCommandBuffer(cmdBuffer);
}


void Renderer::createSyncObjects() {
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;


	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(Device::get(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(Device::get(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(Device::get(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}
