#include "raytracing.h"
#include "Renderer.h"
#include "RenderPass.h"
#include "Pipeline.h"
#include "Device.h"
#include "TextureManager.h"
#include "MeshManager.h"
#include "SwapChain.h"
#include "PhysicalDevice.h"
#include "ApiUtils.h"
#include "vk_extensions.h"
#include "commons.h"

using namespace vkengine;


std::vector<BottomLevelAS> RayTracer::BLASs;
std::vector<TopLevelAS> RayTracer::TLASs;
VkPipeline RayTracer::rayTracingPipeline;
Buffer RayTracer::shaderBindingTable;
Buffer RayTracer::sceneBuffer;

uint64_t getBufferDeviceAddress(VkBuffer buffer)
{
	VkBufferDeviceAddressInfoKHR bufferDeviceAI = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	bufferDeviceAI.buffer = buffer;
	return vkGetBufferDeviceAddressKHR(Device::get(), &bufferDeviceAI);
}

Buffer createScratchBuffer(VkDeviceSize size) {
	VkBuffer scratchBuffer;
	VkDeviceMemory scratchMem;
	createBuffer(PhysicalDevice::get(), Device::get(), size,
		VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, scratchBuffer, scratchMem);
	// Get its device address
	VkBufferDeviceAddressInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	bufferInfo.buffer = scratchBuffer;
	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(Device::get(), &bufferInfo);
	return Buffer{ scratchBuffer,scratchMem, scratchAddress };
}

VkDeviceSize getAsBuildSize(AccelerationStructure AS, VkAccelerationStructureMemoryRequirementsTypeKHR req_type) {
	VkAccelerationStructureMemoryRequirementsInfoKHR memoryReqInfo{
				  VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR };
	memoryReqInfo.type = req_type;
	memoryReqInfo.accelerationStructure = AS.accelerationStructure;
	memoryReqInfo.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
	VkMemoryRequirements2 reqMem{ VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
	vkGetAccelerationStructureMemoryRequirementsKHR(Device::get(), &memoryReqInfo, &reqMem);

	return reqMem.memoryRequirements.size;
}

AccelerationStructureGeometry mesh3DToASGeometryKHR(const Mesh3D * model)
{
	// Setting up the creation info of acceleration structure
	VkAccelerationStructureCreateGeometryTypeInfoKHR gCreate = {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR};
	gCreate.geometryType = VkGeometryTypeKHR::VK_GEOMETRY_TYPE_TRIANGLES_KHR; // could be AABBs or Instances
	gCreate.indexType = VkIndexType::VK_INDEX_TYPE_UINT32;
	gCreate.vertexFormat = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
	gCreate.maxPrimitiveCount = model->getIdxCount() / 3; // how many triangles
	gCreate.maxVertexCount = model->getVertexCount();
	gCreate.allowsTransforms = VK_FALSE; // Not adding transformation matrices

	VkDeviceAddress vertexAddr = getBufferDeviceAddress(model->getVkVertexBuffer());
	VkDeviceAddress indexAddr = getBufferDeviceAddress(model->getVkIndexBuffer());

	// We use triangles but other "geometries" are supported like AABBs (for collision detection) or INSTANCES (for TopLevel AS)
	VkAccelerationStructureGeometryTrianglesDataKHR triangles = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR };
	triangles.vertexFormat = gCreate.vertexFormat;
	triangles.vertexData = { vertexAddr };
	triangles.vertexStride = sizeof(Vertex3D);
	triangles.indexType = gCreate.indexType;
	triangles.indexData = { indexAddr };
	triangles.transformData = {};

	// Setting up the build info of the acceleration
	VkAccelerationStructureGeometryKHR asGeom = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
	asGeom.geometryType = gCreate.geometryType;
	asGeom.flags = VkGeometryFlagBitsKHR::VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
	asGeom.geometry.triangles = triangles;

	//Offset in memory could be non-zero
	VkAccelerationStructureBuildOffsetInfoKHR offset = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
	offset.firstVertex = 0;
	offset.primitiveCount = gCreate.maxPrimitiveCount;
	offset.primitiveOffset = 0;
	offset.transformOffset = 0;

	AccelerationStructureGeometry ASG;
	ASG.info = gCreate;
	ASG.geometry = asGeom;
	ASG.offset = offset;
	return ASG;
}

AccelerationStructure createAcceleration(VkAccelerationStructureCreateInfoKHR & accel_)
{
	AccelerationStructure AS;
	// 1. Create the acceleration structure
	if (vkCreateAccelerationStructureKHR(Device::get(), &accel_, nullptr, 
		&AS.accelerationStructure) != VK_SUCCESS){
		throw std::runtime_error("raytracing: failed  creation of acceleration Structure!");
	}

	// 2. Find memory requirements
	VkAccelerationStructureMemoryRequirementsInfoKHR memInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR };
	memInfo.accelerationStructure = AS.accelerationStructure;
	memInfo.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
	memInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR;
	VkMemoryRequirements2 memReqs{ VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
	vkGetAccelerationStructureMemoryRequirementsKHR(Device::get(), &memInfo, &memReqs);

	VkMemoryAllocateFlagsInfo memFlagInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR};
	memFlagInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

	// 3. Allocate memory
	VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocInfo.allocationSize = memReqs.memoryRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(PhysicalDevice::get(),
			memReqs.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	allocInfo.pNext = &memFlagInfo;
	if (vkAllocateMemory(Device::get(), &allocInfo, nullptr, &AS.memory) != VK_SUCCESS) {
		throw std::runtime_error("raytracing: failed to allocate AS memory!");
	}

	// 4. Bind memory with acceleration structure
	VkBindAccelerationStructureMemoryInfoKHR bind{ VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR };
	bind.accelerationStructure = AS.accelerationStructure;
	bind.memory = AS.memory;
	bind.memoryOffset = 0;

	if (vkBindAccelerationStructureMemoryKHR(Device::get(), 1, &bind) != VK_SUCCESS) {
		throw std::runtime_error("raytracing: failed to bind AS memory!");
	}

	return AS;
}

void RayTracer::buildBottomLevelAS()
{
	// Mesh to geometry traslation and setup of the BLAS vector
	for (auto& mesh : MeshManager::getMeshLibrary())
	{
		// for simplicity we define one blas for each mesh
		BottomLevelAS blas = {};
		// each mesh is a geometry
		auto ASG = mesh3DToASGeometryKHR(mesh);
		blas.gCreateinfos.push_back(ASG.info);
		blas.geometries.push_back(ASG.geometry);
		blas.offsets.push_back(ASG.offset);
		BLASs.push_back(blas);
	}
	std::vector<VkDeviceSize> originalSizes;
	// For each blas we create its AccelerationStructure and query its memory requirements
	VkDeviceSize maxScratch{ 0 }; // we want to find the worst case scratch size we could need
	for (auto& blas : BLASs) {
		/////// BLAS CREATION (vulkan object)
		VkAccelerationStructureCreateInfoKHR asCreateInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
		asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		asCreateInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
		asCreateInfo.maxGeometryCount = (uint32_t)blas.gCreateinfos.size();
		asCreateInfo.pGeometryInfos = blas.gCreateinfos.data();
		// Create an acceleration structure identifier and allocate memory to
		// store the resulting structure data
		blas.as = createAcceleration(asCreateInfo);
		// SCRATCH MEMORY ESTIMATION
		// Estimate the amount of scratch memory required to build the BLAS, and
		// update the size of the scratch buffer that will be allocated to
		// sequentially build all BLASes		
		VkDeviceSize scratchSize = getAsBuildSize(blas.as, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR);
		maxScratch = std::max(maxScratch, scratchSize);
		//Memory size of the future build result
		VkDeviceSize objectSize = getAsBuildSize(blas.as, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR);
		originalSizes.push_back(objectSize);
	}

	// SCRATCH BUFFER CREATION
	Buffer scratchBuffer = createScratchBuffer(maxScratch);

	//EXTRA
	// Query for the size of compacted BLASs
	VkQueryPoolCreateInfo qpci{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
	qpci.queryCount = (uint32_t)BLASs.size();
	qpci.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR;
	VkQueryPool queryPool;
	vkCreateQueryPool(Device::get(), &qpci, nullptr, &queryPool);
	int queryCtr = 0;

	// Record CMDs to build the final BLAS, 
	// We use 1 cmdBuffer x blas to allow the driver to allow system interuption 
	// and avoid a TDR (Timeout Detection and Recovery) if the job was to heavy
	std::vector<VkCommandBuffer> cmdBuffers;
	cmdBuffers.reserve(BLASs.size());
	for (auto & blas : BLASs) {
		auto cmdBuffer = beginSingleTimeCommandBuffer(Device::get(), Device::getGraphicCmdPool());
		cmdBuffers.push_back(cmdBuffer);


		VkAccelerationStructureBuildGeometryInfoKHR bottomASInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		bottomASInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		bottomASInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
		bottomASInfo.update = VK_FALSE; // we are building, not updating
		bottomASInfo.srcAccelerationStructure = VK_NULL_HANDLE;
		bottomASInfo.dstAccelerationStructure = blas.as.accelerationStructure;
		bottomASInfo.geometryArrayOfPointers = VK_FALSE;
		bottomASInfo.geometryCount = (uint32_t)blas.geometries.size();
		VkAccelerationStructureGeometryKHR* as_geometries = blas.geometries.data();
		bottomASInfo.ppGeometries = &as_geometries;
		bottomASInfo.scratchData.deviceAddress = scratchBuffer.deviceAddr;
		// Vulkan wants an array of pointers to offsets... :(
		std::vector<VkAccelerationStructureBuildOffsetInfoKHR*> pBuildOffset(blas.offsets.size());
		for (size_t i = 0; i < blas.offsets.size(); i++) {
			pBuildOffset[i] = &blas.offsets[i];
		}

		// BUILD
		vkCmdBuildAccelerationStructureKHR(cmdBuffer, 1,&bottomASInfo, pBuildOffset.data());

		// Since the scratch buffer is reused across builds, we need a barrier to ensure one build
		// is finished before starting the next one | NOTE: BLAS building on GPU cannot be done in parallel (stated by NVIDIA)
		VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
		barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
		barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
		vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);

		vkCmdWriteAccelerationStructuresPropertiesKHR(cmdBuffer, 1, &blas.as.accelerationStructure,
			VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, queryPool, queryCtr++);
	}
	submitAndWaitCommandBuffers(Device::get(), Device::getGraphicQueue(), Device::getGraphicCmdPool(), cmdBuffers);
	cmdBuffers.clear();

	//BLAS COMPACTION - phase
	std::vector<AccelerationStructure> okBoomers(BLASs.size());
	{
		auto cmdBuff = beginSingleTimeCommandBuffer(Device::get(), Device::getGraphicCmdPool());		
		cmdBuffers.push_back(cmdBuff);
		// Get the size result back
		std::vector<VkDeviceSize> compactSizes(BLASs.size());
		vkGetQueryPoolResults(Device::get(), queryPool, 0, (uint32_t)compactSizes.size(), compactSizes.size() * sizeof(VkDeviceSize),
			compactSizes.data(), sizeof(VkDeviceSize), VK_QUERY_RESULT_WAIT_BIT);

		// Compacting
		uint32_t totOriginalSize{ 0 }, totCompactSize{ 0 };
		for (int i = 0; i < BLASs.size(); i++)
		{
			std::cout << "Compacting BLAS from " << originalSizes[i] << " to " << compactSizes[i] << " bytes" << std::endl;
			totOriginalSize += (uint32_t)originalSizes[i];
			totCompactSize += (uint32_t)compactSizes[i];

			// Creating a compact version of the AS
			VkAccelerationStructureCreateInfoKHR asCreateInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
			asCreateInfo.compactedSize = compactSizes[i];
			asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
			asCreateInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;;
			auto newBlas = createAcceleration(asCreateInfo);

			// Copy the original BLAS to a compact version
			VkCopyAccelerationStructureInfoKHR copyInfo{ VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR };
			copyInfo.src = BLASs[i].as.accelerationStructure;
			copyInfo.dst = newBlas.accelerationStructure;
			copyInfo.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR;
			vkCmdCopyAccelerationStructureKHR(cmdBuff, &copyInfo);
			okBoomers[i] = BLASs[i].as;
			BLASs[i].as = newBlas;
		}
		std::cout << "BLASs total size went from " << totOriginalSize << " to " << totCompactSize << " bytes, saving "
			<< (float)(totOriginalSize - totCompactSize) / totOriginalSize * 100 << "% of memory." << std::endl;
	}

	submitAndWaitCommandBuffers(Device::get(),Device::getGraphicQueue(),Device::getGraphicCmdPool(), cmdBuffers);

	//Destroy the query for the blas size
	vkDestroyQueryPool(Device::get(),queryPool, nullptr);
	// Destroying previous BLAS versions
	for (auto oldAS : okBoomers) {
		vkDestroyAccelerationStructureKHR(Device::get(), oldAS.accelerationStructure, nullptr);
		vkFreeMemory(Device::get(), oldAS.memory, nullptr);
	}
	// We can destroy our scratch buffer
	vkDestroyBuffer(Device::get(), scratchBuffer.vkBuffer, nullptr);
	vkFreeMemory(Device::get(), scratchBuffer.vkMemory, nullptr);
}

void RayTracer::buildTopLevelAS(Scene3D * scene, TopLevelAS* tlas)
{
	tlas->instances.reserve((scene->get_object_num()));
	for (auto & obj_id : scene->listObjects()) {
		Object3D* obj = scene->getObject(obj_id);
		unsigned mesh_id = MeshManager::getMeshID(obj->getMeshName());

		VkAccelerationStructureDeviceAddressInfoKHR addressInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
		addressInfo.accelerationStructure = BLASs[mesh_id].as.accelerationStructure;
		VkDeviceAddress blasAddress = vkGetAccelerationStructureDeviceAddressKHR(Device::get(), &addressInfo);

		TLAS_Instance instance = {};
		instance.customID = obj_id; // return by gl_InstaceID
		instance.blasAddr = blasAddress;
		instance.hitGroupId = 0;  // We will use the same hit group for all objects
		instance.matrix = obj->getMatrix();  // Position of the instance
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		tlas->instances.push_back(instance);
	}
	// This AS does not contain geometry data so CreateGeometryTypeInfo is set to VK_GEOMETRY_TYPE_INSTANCES_KHR
	VkAccelerationStructureCreateGeometryTypeInfoKHR instanceCreate{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR };
	instanceCreate.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	instanceCreate.maxPrimitiveCount = static_cast<uint32_t>(tlas->instances.size()); // max instances 
	instanceCreate.allowsTransforms = VK_TRUE;

	VkAccelerationStructureCreateInfoKHR asCreateInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
	asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR; // TOP LEVEL
	asCreateInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	asCreateInfo.maxGeometryCount = 1; // Must be one to comply specification
	asCreateInfo.pGeometryInfos = &instanceCreate;

	tlas->as = createAcceleration(asCreateInfo);

	// Compute the amount of scratch memory required by the acceleration structure builder
	VkDeviceSize scratchSize = getAsBuildSize(tlas->as, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR);

	//SCRATCH BUFFER CREATION
	tlas->scratchBuffer = createScratchBuffer(scratchSize);

	// For each instance, build the corresponding instance descriptor
	std::vector<VkAccelerationStructureInstanceKHR> geometryInstances;
	geometryInstances.reserve(tlas->instances.size());
	for (const auto& inst : tlas->instances)
	{
		geometryInstances.push_back(inst.to_VkAcInstanceKHR());
	}
	// We must load the AS instances in GPU memory
	// SIZE
	tlas->bufferSize = geometryInstances.size() * sizeof(VkAccelerationStructureInstanceKHR);
	//--------------------------------------------------------------------------------------------------------------
	// STAGE buffer loading
	createBuffer(PhysicalDevice::get(), Device::get(), tlas->bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		tlas->stagebuffer.vkBuffer, tlas->stagebuffer.vkMemory);
	vkMapMemory(Device::get(), tlas->stagebuffer.vkMemory,0, tlas->bufferSize, 0, &tlas->stagebuffer.mappedMemory);
	memcpy(tlas->stagebuffer.mappedMemory, geometryInstances.data(), tlas->bufferSize);
	//Final TLAS instance buffer loading
	createBuffer(PhysicalDevice::get(), Device::get(), tlas->bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		tlas->instanceBuffer.vkBuffer, tlas->instanceBuffer.vkMemory);
	//--------------------------------------------------------------------------------------------------------------
	// Copy data in GPU memory
	VkCommandBuffer cmdBuffer = beginSingleTimeCommandBuffer(Device::get(), Device::getGraphicCmdPool());
	VkBufferCopy copyRegion = {};
	copyRegion.size = tlas->bufferSize;
	vkCmdCopyBuffer(cmdBuffer, tlas->stagebuffer.vkBuffer, tlas->instanceBuffer.vkBuffer, 1, &copyRegion);
	//We put a barrier to make all as_structure build operations to wait for previous commands to end their write operations
	VkMemoryBarrier barrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER};
	barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	// The barrier defines the type of the operations. But GPU work is in pipeline fascion so we must set the stages affected by the barrier
	vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 
		0, 1, &barrier, 0, nullptr, 0, nullptr);
	//--------------------------------------------------------------------------------------------------------------
	//FINALLY we have the data in device local memory safely written ready for the TLAS build
	VkAccelerationStructureGeometryDataKHR gData = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR };
	gData.instances.arrayOfPointers = VK_FALSE;
	gData.instances.data.deviceAddress = getBufferDeviceAddress(tlas->instanceBuffer.vkBuffer);
	VkAccelerationStructureGeometryKHR topASGeometry = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
	topASGeometry.geometryType = VkGeometryTypeKHR::VK_GEOMETRY_TYPE_INSTANCES_KHR;
	topASGeometry.geometry = gData;

	VkAccelerationStructureBuildGeometryInfoKHR topASInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
	topASInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	topASInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	topASInfo.update = VK_FALSE;
	topASInfo.srcAccelerationStructure = VK_NULL_HANDLE;
	topASInfo.dstAccelerationStructure = tlas->as.accelerationStructure;
	topASInfo.geometryArrayOfPointers = VK_FALSE;
	topASInfo.geometryCount = 1;
	VkAccelerationStructureGeometryKHR * pTopGeom = &topASGeometry;
	topASInfo.ppGeometries = &pTopGeom;
	topASInfo.scratchData.deviceAddress = tlas->scratchBuffer.deviceAddr;

	VkAccelerationStructureBuildOffsetInfoKHR offsets = {static_cast<uint32_t>(tlas->instances.size()),0,0,0};
	const VkAccelerationStructureBuildOffsetInfoKHR* pOffsets = &offsets;

	vkCmdBuildAccelerationStructureKHR(cmdBuffer, 1, &topASInfo, &pOffsets);

	std::vector<VkCommandBuffer> buffers = { cmdBuffer };
	submitAndWaitCommandBuffers(Device::get(), Device::getGraphicQueue(), Device::getGraphicCmdPool(), buffers);
	//--------------------------------------------------------------------------------------------------------------

	tlas->instanceBuffer.deviceAddr = getBufferDeviceAddress(tlas->instanceBuffer.vkBuffer);

}

void RayTracer::recordCmdUpdateTopLevelAS(VkCommandBuffer& cmd_buf, TopLevelAS * tlas)
{
	// COPY to GPU
	VkBufferCopy region = { 0, 0, tlas->bufferSize };
	vkCmdCopyBuffer(cmd_buf, tlas->stagebuffer.vkBuffer, tlas->instanceBuffer.vkBuffer, 1, &region);
	VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
		0, 1, &barrier, 0, nullptr, 0, nullptr);

	VkAccelerationStructureGeometryDataKHR geometry{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR };
	geometry.instances.arrayOfPointers = VK_FALSE;
	geometry.instances.data.deviceAddress = tlas->instanceBuffer.deviceAddr;
	VkAccelerationStructureGeometryKHR topASGeometry{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
	topASGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	topASGeometry.geometry = geometry;

	const VkAccelerationStructureGeometryKHR* pGeometry = &topASGeometry;

	VkAccelerationStructureBuildGeometryInfoKHR topASInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
	topASInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	topASInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
	topASInfo.update = VK_TRUE; //VK_TRUE parameter to trigger the update
	topASInfo.srcAccelerationStructure = tlas->as.accelerationStructure; //the existing TLAS being passed and updated in place
	topASInfo.dstAccelerationStructure = tlas->as.accelerationStructure;
	topASInfo.geometryArrayOfPointers = VK_FALSE;
	topASInfo.geometryCount = 1;
	topASInfo.ppGeometries = &pGeometry;
	topASInfo.scratchData.deviceAddress = tlas->scratchBuffer.deviceAddr;

	uint32_t                                         nbInstances = (uint32_t)tlas->instances.size();
	VkAccelerationStructureBuildOffsetInfoKHR        buildOffsetInfo = { nbInstances, 0, 0, 0 };
	const VkAccelerationStructureBuildOffsetInfoKHR* pBuildOffsetInfo = &buildOffsetInfo;

	// Build the TLAS
	vkCmdBuildAccelerationStructureKHR(cmd_buf, 1, &topASInfo, &pBuildOffsetInfo);

	// Command structure readings from the shader to wait for writing in build phase
	VkMemoryBarrier postUpdate{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
	postUpdate.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	postUpdate.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
	vkCmdPipelineBarrier(cmd_buf, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		0, 1, &postUpdate, 0, nullptr, 0, nullptr);

}

void RayTracer::createSceneBuffer(vkengine::Scene3D* scene)
{
	//VkDeviceSize allocation_size = sizeof(SceneObjRtDescBlock) * scene->get_object_num();
	VkDeviceSize allocation_size = sizeof(SceneObjRtDescBlock) * scene->getCurrentObjectCapacity();
	VkDeviceSize minAlignement = PhysicalDevice::getProperties().properties.limits.minStorageBufferOffsetAlignment;
	VkDeviceSize padding = minAlignement - (allocation_size % minAlignement);
	allocation_size = (allocation_size + padding) * SwapChainMng::get()->getImageCount();

	createBuffer(PhysicalDevice::get(), Device::get(), allocation_size,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		sceneBuffer.vkBuffer, sceneBuffer.vkMemory);

	vkMapMemory(Device::get(), sceneBuffer.vkMemory, 0, allocation_size, 0, &sceneBuffer.mappedMemory);
}

void RayTracer::updateCmdBuffer(std::vector<VkCommandBuffer> &cmdBuffers, std::vector<FrameAttachment> &storageImages, unsigned frameIndex)
{
	// begin main command recording
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	if (vkBeginCommandBuffer(cmdBuffers[frameIndex], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	RayTracer::recordCmdUpdateTopLevelAS(cmdBuffers[frameIndex], &TLASs[frameIndex]);

	auto Playout = PipelineFactory::pipeline_layouts[PIPELINE_LAYOUT_RAY_TRACING];
	std::vector<VkDescriptorSet> descrSets;
	for (auto& set : Playout.descriptors.static_sets) {
		descrSets.push_back(set.set);
	}
	for (auto& setlist : Playout.descriptors.frame_dependent_sets) {
		descrSets.push_back(setlist[frameIndex].set);
	}
	vkCmdBindPipeline(cmdBuffers[frameIndex], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rayTracingPipeline);

	vkCmdBindDescriptorSets(cmdBuffers[frameIndex], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, Playout.layout,
		 0, descrSets.size(), descrSets.data(), 0, nullptr);

	// Size of a program identifier
	VkDeviceSize progSize = PhysicalDevice::getPhysicalDeviceRayTracingProperties().shaderGroupBaseAlignment;  
	VkDeviceSize rayGenOffset = 0u * progSize;  // Start at the beginning of the shaderBindingTable
	VkDeviceSize missOffset = 1u * progSize;  // Jump over raygen
	VkDeviceSize missStride = progSize;
	VkDeviceSize hitGroupOffset = 3u * progSize;  // Jump over the previous shaders
	VkDeviceSize hitGroupStride = progSize;

	// since 3 are the shader groups of 1 shader all the same size
	VkDeviceSize tableSize = progSize * 3;

	const VkStridedBufferRegionKHR raygenShaderBindingTable = { shaderBindingTable.vkBuffer, rayGenOffset,
																 progSize, tableSize };
	const VkStridedBufferRegionKHR missShaderBindingTable = { shaderBindingTable.vkBuffer, missOffset,
															   progSize, tableSize };
	const VkStridedBufferRegionKHR hitShaderBindingTable = { shaderBindingTable.vkBuffer, hitGroupOffset,
															  progSize, tableSize };
	const VkStridedBufferRegionKHR callableShaderBindingTable = {}; // not used


	// it's basically a compute task, the invocation resembles a CUDA call
	auto extent = SwapChainMng::get()->getExtent();

	transitionImageLayout(cmdBuffers[frameIndex],
		storageImages[frameIndex].image, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

	vkCmdTraceRaysKHR(cmdBuffers[frameIndex], &raygenShaderBindingTable, &missShaderBindingTable, &hitShaderBindingTable,
		&callableShaderBindingTable, extent.width, extent.height, 1); 

	// Usually a Renderpass takes care of changing the image layout, 
	// here we don't have one so we must take care of this
	transitionImageLayout(cmdBuffers[frameIndex],
		storageImages[frameIndex].image, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkEndCommandBuffer(cmdBuffers[frameIndex]);
}

void RayTracer::createRayTracingPipeline()
{	
	Shader rayGen("VkEngine/Shaders/raytracing_simple/rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR);
	Shader rayMiss("VkEngine/Shaders/raytracing_simple/rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR);
	Shader rayShadowMiss("VkEngine/Shaders/raytracing_simple/shadow.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR);
	Shader rayAnyHit_0("VkEngine/Shaders/raytracing_simple/0_rahit.spv", VK_SHADER_STAGE_ANY_HIT_BIT_KHR);
	Shader rayAnyHit_1("VkEngine/Shaders/raytracing_simple/1_rahit.spv", VK_SHADER_STAGE_ANY_HIT_BIT_KHR);
	Shader rayClosestHit("VkEngine/Shaders/raytracing_simple/rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
	std::array<VkPipelineShaderStageCreateInfo, 6> stages{ rayGen.getStage(), 
		rayMiss.getStage(), rayShadowMiss.getStage(), 
		rayClosestHit.getStage(), rayAnyHit_0.getStage(), rayAnyHit_1.getStage() };

	/* Shaders are gathered in groups
		RAYGEN	-> those who generate
		MISS	-> those who handle miss [rmiss and shadow.rmiss]
		HIT		-> those involved in hit payload generation
	*/
	std::vector<VkRayTracingShaderGroupCreateInfoKHR >shaderGroups;
	/*
		Setup ray tracing shader groups
	*/
	// rayGen group
	VkRayTracingShaderGroupCreateInfoKHR raygenGroupCI{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
	raygenGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	raygenGroupCI.generalShader = 0; // raygen is general
	raygenGroupCI.closestHitShader = VK_SHADER_UNUSED_KHR;
	raygenGroupCI.anyHitShader = VK_SHADER_UNUSED_KHR;
	raygenGroupCI.intersectionShader = VK_SHADER_UNUSED_KHR;
	shaderGroups.push_back(raygenGroupCI);
	// Miss group: miss + shadow_miss
	VkRayTracingShaderGroupCreateInfoKHR missGroupCI{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
	missGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	missGroupCI.generalShader = 1; // miss is counted as general
	missGroupCI.closestHitShader = VK_SHADER_UNUSED_KHR;
	missGroupCI.anyHitShader = VK_SHADER_UNUSED_KHR;
	missGroupCI.intersectionShader = VK_SHADER_UNUSED_KHR;
	shaderGroups.push_back(missGroupCI);
	missGroupCI.generalShader = 2;
	shaderGroups.push_back(missGroupCI);
	// Hit Group: Closest Hit + AnyHit
	VkRayTracingShaderGroupCreateInfoKHR closesHitGroupCI{ VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR };
	closesHitGroupCI.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR; // could also be procedural instead
	closesHitGroupCI.generalShader = VK_SHADER_UNUSED_KHR;
	closesHitGroupCI.closestHitShader = 3; // Triangle hit
	closesHitGroupCI.anyHitShader = 4;
	closesHitGroupCI.intersectionShader = VK_SHADER_UNUSED_KHR;
	shaderGroups.push_back(closesHitGroupCI);
	// Hit Group: shadow anyhit
	closesHitGroupCI.closestHitShader = VK_SHADER_UNUSED_KHR; // No closest hit in shadow rays
	closesHitGroupCI.anyHitShader = 5;
	shaderGroups.push_back(closesHitGroupCI);

	// Assemble the shader stages and recursion depth info into the ray tracing pipeline
	VkRayTracingPipelineCreateInfoKHR rayPipelineInfo{VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR};
	// layout already defined by PipelineFactory
	rayPipelineInfo.layout = PipelineFactory::pipeline_layouts[PIPELINE_LAYOUT_RAY_TRACING].layout;
	rayPipelineInfo.stageCount = static_cast<uint32_t>(stages.size());  // Stages are shaders
	rayPipelineInfo.pStages = stages.data();
	rayPipelineInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());  // 1-raygen, n-miss, n-(hit[+anyhit+intersect])
	rayPipelineInfo.pGroups = shaderGroups.data();
	rayPipelineInfo.maxRecursionDepth = 2; // 1 forces a Miss call if a trace call happens after the first hit
	rayPipelineInfo.libraries = { VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR };

	if (vkCreateRayTracingPipelinesKHR(Device::get(), nullptr, 1, & rayPipelineInfo, nullptr, &RayTracer::rayTracingPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create Ray-Tracing Pipeline!");
	}
}

void RayTracer::createShaderBindingTable()
{
	// 6 shader groups: rgen, rmiss, rmiss_shadow, (rchit & rahit_0), rahit_1
	auto groupCount = 5;               
	// Size of a program identifier
	uint32_t groupHandleSize = PhysicalDevice::getPhysicalDeviceRayTracingProperties().shaderGroupHandleSize;
	// Size of shader alignment
	uint32_t baseAlignment = PhysicalDevice::getPhysicalDeviceRayTracingProperties().shaderGroupBaseAlignment;

	// Fetch all the shader handles used in the pipeline, so that they can be written in the SBT
	uint32_t sbtSize = groupCount * baseAlignment;
	std::vector<uint8_t> shaderHandleStorage(sbtSize);
	vkGetRayTracingShaderGroupHandlesKHR(Device::get(), RayTracer::rayTracingPipeline, 
		0, groupCount, sbtSize,	shaderHandleStorage.data());
	// Write the handles in the SBT
	createBuffer(PhysicalDevice::get(), Device::get(), sbtSize,  
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		shaderBindingTable.vkBuffer, shaderBindingTable.vkMemory);

	// Write the handles in the SBT	
	void* mapped;
	vkMapMemory(Device::get(), shaderBindingTable.vkMemory, 0, sbtSize, 0, &mapped);
	auto* pData = reinterpret_cast<uint8_t*>(mapped);
	for (uint32_t g = 0; g < groupCount; g++)
	{
		memcpy(pData, shaderHandleStorage.data() + g * groupHandleSize, groupHandleSize);  // raygen
		pData += baseAlignment;
	}
	vkUnmapMemory(Device::get(), shaderBindingTable.vkMemory);
}

void RayTracer::updateRTPipelineResources(vkengine::Scene3D* scene)
{
	auto bundle = PipelineFactory::pipeline_layouts[PIPELINE_LAYOUT_RAY_TRACING].descriptors;
	std::vector<VkWriteDescriptorSet> writes;
	std::vector<VkDescriptorBufferInfo> vertexBuffersInfos{ 
		SUPPORTED_MESH_COUNT,
		{MeshManager::getMesh(0)->getVkVertexBuffer(), 0, VK_WHOLE_SIZE} };
	std::vector<VkDescriptorBufferInfo> indexBuffersInfos{
		SUPPORTED_MESH_COUNT,
		{MeshManager::getMesh(0)->getVkIndexBuffer(), 0, VK_WHOLE_SIZE} };
	std::vector<VkDescriptorImageInfo> textureSamplersInfos(
		SUPPORTED_TEXTURE_COUNT, // Pre-fill with default texture
		{ TextureManager::getSceneTexture(0)->getTextureSampler(),
		TextureManager::getSceneTexture(0)->getTextureImgView(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });


	{
		// Fill all vertex and all index buffers
		for (int i = 0; i < MeshManager::countLoadedMeshes(); i++) {
			vertexBuffersInfos[i] = { MeshManager::getMesh(i)->getVkVertexBuffer(), 0, VK_WHOLE_SIZE };
			indexBuffersInfos[i] = { MeshManager::getMesh(i)->getVkIndexBuffer(), 0, VK_WHOLE_SIZE };
		}
		VkWriteDescriptorSet vertexDescWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		vertexDescWrite.dstSet = bundle.static_sets[0].set;
		vertexDescWrite.dstBinding = 0;
		vertexDescWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		vertexDescWrite.descriptorCount = vertexBuffersInfos.size();
		vertexDescWrite.pBufferInfo = vertexBuffersInfos.data();
		writes.push_back(vertexDescWrite);

		VkWriteDescriptorSet indexDescWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		indexDescWrite.dstSet = bundle.static_sets[0].set;
		indexDescWrite.dstBinding = 1;
		indexDescWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		indexDescWrite.descriptorCount = indexBuffersInfos.size();
		indexDescWrite.pBufferInfo = indexBuffersInfos.data();
		writes.push_back(indexDescWrite);

		//Get all texture samplers
		for (int i=0; i < TextureManager::countSceneTextures(); i++) {
			textureSamplersInfos[i] = {
				TextureManager::getSceneTexture(i)->getTextureSampler(),
				TextureManager::getSceneTexture(i)->getTextureImgView(),
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL 
			};
		}

		VkWriteDescriptorSet texturesDescWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		texturesDescWrite.dstSet = bundle.static_sets[0].set;
		texturesDescWrite.dstBinding = 2;
		texturesDescWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		texturesDescWrite.descriptorCount = textureSamplersInfos.size();
		texturesDescWrite.pImageInfo = textureSamplersInfos.data();
		writes.push_back(texturesDescWrite);
	}

	std::vector<VkWriteDescriptorSetAccelerationStructureKHR> accStructureWrites(bundle.frame_dependent_sets[0].size());
	std::vector<VkDescriptorImageInfo> imageDescriptors(bundle.frame_dependent_sets[0].size());
	std::vector<VkDescriptorBufferInfo> sceneBuffInfo(bundle.frame_dependent_sets[0].size());
	for (int i = 0; i < bundle.frame_dependent_sets[0].size(); i++)
	{
		VkWriteDescriptorSet accStructWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		accStructWrite.dstSet = bundle.frame_dependent_sets[0][i].set;
		accStructWrite.dstBinding = 0;
		accStructWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		accStructWrite.descriptorCount = 1;
		// The specialized acceleration structure descriptor has to be chained
		accStructureWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		accStructureWrites[i].accelerationStructureCount = 1;
		accStructureWrites[i].pAccelerationStructures = &TLASs[i].as.accelerationStructure;
		accStructWrite.pNext = &accStructureWrites[i];
		writes.push_back(accStructWrite);

		VkWriteDescriptorSet storageImgDescSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		storageImgDescSet.dstSet = bundle.frame_dependent_sets[0][i].set;
		storageImgDescSet.dstBinding = 1;
		storageImgDescSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		storageImgDescSet.descriptorCount = 1;
		imageDescriptors[i].imageView = Renderer::getOffScreenFrameAttachment(i).imageView;
		imageDescriptors[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		storageImgDescSet.pImageInfo = &imageDescriptors[i];
		writes.push_back(storageImgDescSet);

		// Fill SceneBufferDescriptor at each offset
		VkDeviceSize allocation_size = sizeof(SceneObjRtDescBlock) * scene->getCurrentObjectCapacity();
		VkDeviceSize minAlignement = PhysicalDevice::getProperties().properties.limits.minStorageBufferOffsetAlignment;
		VkDeviceSize padding = minAlignement - (allocation_size % minAlignement);
		VkWriteDescriptorSet sceneDescWrite = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		sceneDescWrite.dstSet = bundle.frame_dependent_sets[0][i].set;
		sceneDescWrite.dstBinding = 2;
		sceneDescWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		sceneDescWrite.descriptorCount = 1;
		sceneBuffInfo[i] = { sceneBuffer.vkBuffer, i*(allocation_size + padding), allocation_size };
		sceneDescWrite.pBufferInfo = &sceneBuffInfo[i];
		writes.push_back(sceneDescWrite);
	}
	std::vector<VkDescriptorBufferInfo> bufferDescriptors(bundle.frame_dependent_sets[1].size());
	VkDeviceSize minAlignement =
		PhysicalDevice::getProperties().properties.limits.minUniformBufferOffsetAlignment;
	VkDeviceSize alignemetPadding = minAlignement - (sizeof(UniformBlock) % minAlignement);
	for (int i = 0; i < bundle.frame_dependent_sets[1].size(); i++)
	{
		VkWriteDescriptorSet uniformBufferDescSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		uniformBufferDescSet.dstSet = bundle.frame_dependent_sets[1][i].set;
		uniformBufferDescSet.dstBinding = 0;
		uniformBufferDescSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferDescSet.descriptorCount = 1;
		bufferDescriptors[i].buffer = DescriptorSetsFactory::getUniformBuffer();
		bufferDescriptors[i].offset = i * (sizeof(UniformBlock) + alignemetPadding);
		bufferDescriptors[i].range = sizeof(UniformBlock);
		uniformBufferDescSet.pBufferInfo = &bufferDescriptors[i];
		writes.push_back(uniformBufferDescSet);
	}
	vkQueueWaitIdle(Device::getGraphicQueue());
	vkUpdateDescriptorSets(Device::get(), writes.size(), writes.data(), 0, nullptr);
}

void RayTracer::initialize()
{
	LOAD_RAYTRACING_API_COMMANDS(Device::get());
}

void RayTracer::prepare(Scene3D * scene) {
	destroySceneAcceleration();
	buildBottomLevelAS();
	TLASs.resize(SwapChainMng::get()->getImageCount());
	for (int i = 0; i < TLASs.size(); i++) {
		buildTopLevelAS(scene, &TLASs[i]);
	}
	createSceneBuffer(scene);
}

void RayTracer::updateSceneData(vkengine::Scene3D* scene, unsigned imageIndex)
{
	//vkQueueWaitIdle(Device::getGraphicQueue());
	// Updating the storage buffer with objects setting taken from the scene
	std::vector<SceneObjRtDescBlock> sceneDescription;
	sceneDescription.reserve(scene->getCurrentObjectCapacity());
	for (auto objId : scene->listObjects()) {
		auto obj = scene->getObject(objId);
		sceneDescription.push_back({
			MeshManager::getMeshID(obj->getMeshName()),
			TextureManager::getSceneTextureIndex(obj->getTextureName()),
			obj->getMatrix(),
			obj->reflective});
	}
	VkDeviceSize allocation_size = sizeof(SceneObjRtDescBlock) * scene->getCurrentObjectCapacity();
	VkDeviceSize minAlignement =
		PhysicalDevice::getProperties().properties.limits.minStorageBufferOffsetAlignment;
	VkDeviceSize padding = minAlignement - (allocation_size % minAlignement);
	memcpy((char*)sceneBuffer.mappedMemory + (allocation_size + padding)*imageIndex, sceneDescription.data(), allocation_size);

	// Update matrix data for each instance and retrieve Vulkan struct
	std::vector<VkAccelerationStructureInstanceKHR> geometryInstances;
	geometryInstances.reserve(TLASs[imageIndex].instances.size());
	for (auto& instance : TLASs[imageIndex].instances) {
		instance.matrix = scene->getObject(instance.customID)->getMatrix();
		geometryInstances.push_back(instance.to_VkAcInstanceKHR());
	}
	//Memcpy data to the stage buffer, ready for transfer
	memcpy(TLASs[imageIndex].stagebuffer.mappedMemory, geometryInstances.data(), TLASs[imageIndex].bufferSize);
	//vkQueueWaitIdle(Device::getGraphicQueue());
}

void RayTracer::destroySceneAcceleration()
{	// Destroy TLAS resources
	for (auto& tlas : TLASs) {
		if (tlas.stagebuffer.mappedMemory != nullptr) {
			vkUnmapMemory(Device::get(), tlas.stagebuffer.vkMemory);
		}
		vkDestroyBuffer(Device::get(), tlas.stagebuffer.vkBuffer, nullptr);
		vkFreeMemory(Device::get(), tlas.stagebuffer.vkMemory, nullptr);
		vkDestroyBuffer(Device::get(), tlas.scratchBuffer.vkBuffer, nullptr);
		vkFreeMemory(Device::get(), tlas.scratchBuffer.vkMemory, nullptr);
		vkDestroyAccelerationStructureKHR(Device::get(), tlas.as.accelerationStructure, nullptr);
		vkFreeMemory(Device::get(), tlas.as.memory, nullptr);
		vkDestroyBuffer(Device::get(), tlas.instanceBuffer.vkBuffer, nullptr);
		vkFreeMemory(Device::get(), tlas.instanceBuffer.vkMemory, nullptr);
		tlas.instances.clear();

	}
	
	// Destroy BLAS resources
	for (auto & blas : BLASs) {
		vkDestroyAccelerationStructureKHR(Device::get(), blas.as.accelerationStructure, nullptr);
		vkFreeMemory(Device::get(), blas.as.memory, nullptr);
	}
	BLASs.clear();
	// destroy the scene descriptor uniform buffer
	if (sceneBuffer.mappedMemory != nullptr) {
		vkUnmapMemory(Device::get(), sceneBuffer.vkMemory);
	}
	vkDestroyBuffer(Device::get(),sceneBuffer.vkBuffer, nullptr);
	vkFreeMemory(Device::get(), sceneBuffer.vkMemory, nullptr);
}

void RayTracer::cleanUP()
{
	// Destroy SBT
	vkDestroyBuffer(Device::get(), shaderBindingTable.vkBuffer, nullptr);
	vkFreeMemory(Device::get(), shaderBindingTable.vkMemory, nullptr);
	// Destroy Pipeline
	vkDestroyPipeline(Device::get(), rayTracingPipeline, nullptr);
	destroySceneAcceleration();
}
