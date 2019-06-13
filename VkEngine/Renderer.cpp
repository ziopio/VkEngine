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
void threadRenderCode(Object* obj, vks::Frustum frustum, 
	ThreadData* threadData, uint32_t frameBufferIndex, uint32_t cmdBufferIndex, 
	VkCommandBufferInheritanceInfo inheritanceInfo, std::array<VkDescriptorSet,2> descriptorSet);

Renderer::Renderer(RenderPass* renderPass, SwapChain* swapChain)
{
	this->swapChain = swapChain;
	this->renderPass = renderPass;
	this->multithreading = true;
	createDepthResources();
	createFramebuffers();
	createSyncObjects();
	this->findObjXthreadDivision();
	prepareThreadedRendering();
}

unsigned Renderer::getNextFrameBufferIndex()
{
	return this->last_imageIndex >= swapChain->getImageViews().size() - 1 ?
		0 : this->last_imageIndex + 1;
}

void Renderer::setLights(std::vector<LightSource> lights)
{
	this->lights = lights;
}

void Renderer::setObjects(std::vector<Object> objects)
{
	for (auto threadResource : this->per_thread_resources) {
		vkDestroyCommandPool(Device::get(), threadResource.commandPool, nullptr);
	}
	vkDestroyCommandPool(Device::get(), this->mainThreadSecondaryCmdPool, nullptr);
	vkDestroyCommandPool(Device::get(), this->primaryCommandPool, nullptr);
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
	this->last_imageIndex = imageIndex;
	this->update_camera_infos(imageIndex);
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
	submitInfo.pCommandBuffers = &primaryCmdBuffers[imageIndex];
	// Indico quale � il semaforo che indica la fine dell'attesa
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
	//vkQueueWaitIdle(Device::getPresentQueue()); //not optimal time usage

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	return true;
}

Renderer::~Renderer()
{
	this->objects.clear();
	this->lights.clear();

	for (auto threadResource : this->per_thread_resources) {
		vkDestroyCommandPool(Device::get(), threadResource.commandPool, nullptr);
	}
	this->per_thread_resources.clear();

	vkDestroyCommandPool(Device::get(), this->mainThreadSecondaryCmdPool, nullptr);
	mainThreadSecondaryCmdBuffers.clear();
	vkDestroyCommandPool(Device::get(), this->primaryCommandPool, nullptr);
	primaryCmdBuffers.clear();

	vkDestroyImageView(Device::get(), depth_buffer.depthImageView, nullptr);
	vkDestroyImage(Device::get(), depth_buffer.depthImage, nullptr);
	vkFreeMemory(Device::get(), depth_buffer.depthImageMemory, nullptr);

	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(Device::get(), framebuffer, nullptr);
	}
	swapChainFramebuffers.clear();
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(Device::get(), renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(Device::get(), imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(Device::get(), inFlightFences[i], nullptr);
	}
	renderFinishedSemaphores.clear();
	imageAvailableSemaphores.clear();
	inFlightFences.clear();
}



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
	{
		//Creazione della pool per i buffer di comando principali
		Device::createCommandPool(PhysicalDevice::getQueueFamilies().graphicsFamily, &this->primaryCommandPool);
		//allocazione buffer principali 1 per ogni frame
		primaryCmdBuffers.resize(swapChainFramebuffers.size());
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = this->primaryCommandPool; // using the standard Pool
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		for (int i = 0; i < swapChainFramebuffers.size(); i++) {
			if (vkAllocateCommandBuffers(Device::get(), &allocInfo, &primaryCmdBuffers[i]) 
				!= VK_SUCCESS) 
			{
				throw std::runtime_error("failed to allocate primary command buffer!");
			}
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
				if (vkAllocateCommandBuffers(Device::get(), &allocInfo, 
					&per_thread_resources[t].commandBuffers[f][i]) != VK_SUCCESS) 
				{
					throw std::runtime_error("failed to allocate primary command buffer!");
				}
			}
		}
	}
	//Secondary buffer and pool for gui draw comands
	// 1 command pool
	Device::createCommandPool(PhysicalDevice::getQueueFamilies().graphicsFamily,
		&mainThreadSecondaryCmdPool);
	mainThreadSecondaryCmdBuffers.resize(swapChainFramebuffers.size());
	//1 command buffer
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mainThreadSecondaryCmdPool; // using the standard Pool
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandBufferCount = swapChainFramebuffers.size();
	if (vkAllocateCommandBuffers(Device::get(), &allocInfo, mainThreadSecondaryCmdBuffers.data()) 
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to allocate primary command buffer!");
	}
}

void Renderer::update_camera_infos(uint32_t frameBufferIndex)
{
	uniformBlockDefinition uniforms = {};
	uniforms.V_matrix = Direction::getCurrentCamera()->setCamera();
	uniforms.P_matrix = Direction::getCurrentCamera()->getProjection();
	uniforms.P_matrix[1][1] *= -1; // invert openGL Y sign
	uniforms.light_count = lights.size();
	for (int i = 0; i < lights.size(); i++) {
		uniforms.lights[i] = this->lights[i].getData();
	}
	DescriptorSetsFactory::updateUniformBuffer(uniforms, frameBufferIndex);

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
	std::array<VkDescriptorSet,2> descriptor_sets = 
	{ 
		DescriptorSetsFactory::getStaticGlobalDescriptorSet(),
		DescriptorSetsFactory::getFrameDescriptorSet(frameBufferIndex) 
	};
	for (uint32_t t = 0, objIndex = 0; t < numThreads; t++)
	{
		for (uint32_t i = 0; i < objXthread && objIndex < objects.size(); i++, objIndex++)
		{
			if (multithreading) {
				thread_pool.threads[t]->addJob([=] { threadRenderCode(&objects[objIndex], frustum_, &per_thread_resources[t], 
					frameBufferIndex, i, inheritanceInfo, descriptor_sets); });
			}
			else {
				threadRenderCode(&objects[objIndex], frustum_, &per_thread_resources[t], frameBufferIndex, i, 
					inheritanceInfo, descriptor_sets);
			}
		}
	}

	// begin main command recording
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	if (vkBeginCommandBuffer(primaryCmdBuffers[frameBufferIndex], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { 0.2f, 0.2f, 0.2f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass->get();
	renderPassInfo.framebuffer = swapChainFramebuffers[frameBufferIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChain->getExtent();
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();
	// begin render pass
	vkCmdBeginRenderPass(primaryCmdBuffers[frameBufferIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	thread_pool.wait();
	// i draw only the command buffers related to visible objects
	for (uint32_t t = 0, objIndex = 0; t < numThreads; t++)
	{
		for (uint32_t i = 0;  i < objXthread && objIndex < objects.size(); i++, objIndex++)
		{
			if (objects[objIndex].visible)
			{
				secondaryCmdBuffers.push_back(per_thread_resources[t].commandBuffers[frameBufferIndex][i]);
			}
		}
	}
	// ImGui rendering
	if (MeshManager::getImGuiMesh(frameBufferIndex)->getIdxCount() > 0) 
	{
		this->recordImGuiDrawCmds(frameBufferIndex, inheritanceInfo);
		secondaryCmdBuffers.push_back(this->mainThreadSecondaryCmdBuffers[frameBufferIndex]);
	}

	// Execute render commands from all secondary command buffers
	if (secondaryCmdBuffers.size() > 0) {
		vkCmdExecuteCommands(primaryCmdBuffers[frameBufferIndex], secondaryCmdBuffers.size(), secondaryCmdBuffers.data());
	}
	vkCmdEndRenderPass(primaryCmdBuffers[frameBufferIndex]);
	vkEndCommandBuffer(primaryCmdBuffers[frameBufferIndex]);
}

void Renderer::recordImGuiDrawCmds(uint32_t frameBufferIndex, VkCommandBufferInheritanceInfo inheritanceInfo)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = &inheritanceInfo;

	if (vkBeginCommandBuffer(mainThreadSecondaryCmdBuffers[frameBufferIndex], &beginInfo) 
		!= VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	vkCmdBindPipeline(mainThreadSecondaryCmdBuffers[frameBufferIndex],
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		MaterialManager::getMaterial(MaterialType::UI)->getPipeline());

	auto pipelineLayout = 
		MaterialManager::getMaterial(MaterialType::UI)->getPipelineLayout();
	auto descrSet = DescriptorSetsFactory::getImGuiDescriptorSet();
	vkCmdBindDescriptorSets(mainThreadSecondaryCmdBuffers[frameBufferIndex],
		VK_PIPELINE_BIND_POINT_GRAPHICS,pipelineLayout,0,1,
		&descrSet,0,nullptr);

	GuiMesh* imgui = MeshManager::getImGuiMesh(frameBufferIndex);
	VkBuffer vertexBuffer = imgui->getVkVertexBuffer();
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(mainThreadSecondaryCmdBuffers[frameBufferIndex]
		, 0, 1, &vertexBuffer, offsets);
	vkCmdBindIndexBuffer(mainThreadSecondaryCmdBuffers[frameBufferIndex],
		imgui->getVkIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

	UiDrawData data = imgui->getData();
	ImGuiPushConstantBlock pushBlock = {};
	pushBlock.uScale = 2.0f / data.display_size;
	pushBlock.uTranslate = -1.0f - data.display_pos * pushBlock.uScale;
	pushBlock.tex_ID = 0;
	vkCmdPushConstants(mainThreadSecondaryCmdBuffers[frameBufferIndex],
		pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
		0, sizeof(ImGuiPushConstantBlock), &pushBlock);

	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = data.display_size.x * data.frame_buffer_scale.x;
	viewport.height = data.display_size.y * data.frame_buffer_scale.y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(mainThreadSecondaryCmdBuffers[frameBufferIndex],
		0, 1, &viewport);

	uint32_t idx_offset = 0;
	uint32_t vtx_offset = 0;
	for (auto draw_list : data.drawLists) {
		for ( auto cmd : draw_list.drawCommands) {
			// Project scissor/clipping rectangles into framebuffer space
			glm::vec4 clip_rect;
			clip_rect = (cmd.clipRectangle - glm::vec4(data.display_pos,data.display_pos) )
				* glm::vec4(data.display_size, data.display_size);
			// Apply scissor/clipping rectangle
			VkRect2D scissor;
			scissor.offset.x = std::max((int32_t)(cmd.clipRectangle.x), 0);
			scissor.offset.y = std::max((int32_t)(cmd.clipRectangle.y), 0);
			scissor.extent.width = (uint32_t)(cmd.clipRectangle.z - cmd.clipRectangle.x);
			scissor.extent.height = (uint32_t)(cmd.clipRectangle.w - cmd.clipRectangle.y);
			vkCmdSetScissor(mainThreadSecondaryCmdBuffers[frameBufferIndex],
				0, 1, &scissor);
			vkCmdDrawIndexed(mainThreadSecondaryCmdBuffers[frameBufferIndex],
				cmd.elementCount, 1, idx_offset, vtx_offset, 0);

			idx_offset += cmd.elementCount;
		}
		vtx_offset += draw_list.vertexBufferSize;
	}

	VkResult result = vkEndCommandBuffer(mainThreadSecondaryCmdBuffers[frameBufferIndex]);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Command buffer ending failed");
	}
}

void Renderer::findObjXthreadDivision()
{
	// Trovo il numero di thread disponibili
	numThreads = std::thread::hardware_concurrency();
	// Se i thread superano il numero degli oggetti, 
	// riduco il numero di thread e imposto 1 oggetto per thread.
	if (objects.size() < numThreads) {
		numThreads = 1;
		objXthread = objects.size();
	}
	else {
		this->objXthread = objects.size() / numThreads;
	}
	//Imposto il numero di thread che la libreria deve utilizzare
	thread_pool.setThreadCount(numThreads);
	std::cout << "\nRendering impostato su " << numThreads 
		<<" threads, " << objXthread << " oggetti ciascuno. Totale: " 
		<< objects.size() << " oggetti." << std::endl;
}

/*
	This function assembles a command buffer for 1 object running on 1 thread
*/
void threadRenderCode(Object* obj, vks::Frustum frustum,ThreadData* threadData, uint32_t frameBufferIndex, uint32_t cmdBufferIndex,
	VkCommandBufferInheritanceInfo inheritanceInfo, std::array<VkDescriptorSet,2> descriptorSets)
{
	obj->visible = frustum.checkSphere(obj->getPos(), obj->getInfo().scale_factor );

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
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

	VkPipelineLayout playout = MaterialManager::getMaterial(obj->getMatType())->getPipelineLayout();
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		playout,
		0,descriptorSets.size(), descriptorSets.data(),
		0, nullptr);

	PushConstantBlock pushConsts = {};
	pushConsts.model_transform = obj->getMatrix();
	pushConsts.textureIndex = obj->getTextureId();
	vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConsts), &pushConsts);


	vkCmdDrawIndexed(cmdBuffer, static_cast<uint32_t>(MeshManager::getMesh(obj->getMeshId())->getIdxCount()), 1, 0, 0, 0);
	VkResult result = vkEndCommandBuffer(cmdBuffer);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Command buffer ending failed");
	}
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
