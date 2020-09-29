#include "raytracing.h"
#include "Device.h"
#include "MeshManager.h"
#include "PhysicalDevice.h"
#include "ApiUtils.h"
#include "vk_extensions.h"

using namespace vkengine;

std::vector<BottomLevelAS> RayTracer::BLASs;
TopLevelAS RayTracer::TLAS;

uint64_t getBufferDeviceAddress(VkBuffer buffer)
{
	VkBufferDeviceAddressInfoKHR bufferDeviceAI = { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	bufferDeviceAI.buffer = buffer;
	return vkGetBufferDeviceAddressKHR(Device::get(), &bufferDeviceAI);
}

AccelerationStructureGeometry mesh3DToASGeometryKHR(const Mesh3D* model)
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
	asGeom.flags = VkGeometryFlagBitsKHR::VK_GEOMETRY_OPAQUE_BIT_KHR;
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
	for (auto mesh : MeshManager::getMeshLibrary())
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

	// For each blas we create its AccelerationStructure and query its memory requirements
	VkDeviceSize maxScratch{ 0 }; // we want to find the worst case scratch size we could need
	for (auto& blas : BLASs) {
		/////// BLAS CREATION (vulkan object)
		VkAccelerationStructureCreateInfoKHR asCreateInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
		asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		asCreateInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		asCreateInfo.maxGeometryCount = (uint32_t)blas.gCreateinfos.size();
		asCreateInfo.pGeometryInfos = blas.gCreateinfos.data();
		// Create an acceleration structure identifier and allocate memory to
		// store the resulting structure data
		blas.as = createAcceleration(asCreateInfo);
		// SCRATCH MEMORY ESTIMATION
		// Estimate the amount of scratch memory required to build the BLAS, and
		// update the size of the scratch buffer that will be allocated to
		// sequentially build all BLASes
		VkAccelerationStructureMemoryRequirementsInfoKHR memoryReqInfo{
					  VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR };
		memoryReqInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR; // scratch
		memoryReqInfo.accelerationStructure = blas.as.accelerationStructure;
		memoryReqInfo.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
		VkMemoryRequirements2 reqMem{ VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
		vkGetAccelerationStructureMemoryRequirementsKHR(Device::get(), &memoryReqInfo, &reqMem);
		
		VkDeviceSize scratchSize = reqMem.memoryRequirements.size;
		maxScratch = std::max(maxScratch, scratchSize);

		// TODO query the true sizes of each blas [optional]
	}

	////// SCRATCH BUFFER CREATION
	VkBuffer scratchBuffer;
	VkDeviceMemory scratchMem;
	createBuffer(PhysicalDevice::get(),Device::get(), maxScratch, 
		VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, scratchBuffer, scratchMem);
	// Get its device address
	VkBufferDeviceAddressInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
	bufferInfo.buffer = scratchBuffer;
	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(Device::get(), &bufferInfo);

	// TODO // Prepare the Query for the size of compact BLAS

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
		bottomASInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		bottomASInfo.update = VK_FALSE; // we are building, not updating
		bottomASInfo.srcAccelerationStructure = VK_NULL_HANDLE;
		bottomASInfo.dstAccelerationStructure = blas.as.accelerationStructure;
		bottomASInfo.geometryArrayOfPointers = VK_FALSE;
		bottomASInfo.geometryCount = (uint32_t)blas.geometries.size();
		VkAccelerationStructureGeometryKHR* as_geometries = blas.geometries.data();
		bottomASInfo.ppGeometries = &as_geometries;
		bottomASInfo.scratchData.deviceAddress = scratchAddress;
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
	}

	submitAndWaitCommandBuffers(Device::get(),Device::getGraphicQueue(),Device::getGraphicCmdPool(), cmdBuffers);

	// We can destroy our scratch buffer
	vkDestroyBuffer(Device::get(), scratchBuffer, nullptr);
	vkFreeMemory(Device::get(), scratchMem, nullptr);
}

void RayTracer::buildTopLevelAS(Scene3D * scene)
{
	TLAS.instances.reserve((scene->get_object_num()));
	for (auto & obj_id : scene->listObjects()) {
		Object3D* obj = scene->getObject(obj_id);

		VkAccelerationStructureDeviceAddressInfoKHR addressInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR };
		addressInfo.accelerationStructure = BLASs[obj_id].as.accelerationStructure;
		VkDeviceAddress blasAddress = vkGetAccelerationStructureDeviceAddressKHR(Device::get(), &addressInfo);

		TLAS_Instance instance = {};
		instance.instance_id = obj_id;
		instance.blasAddr = blasAddress;
		instance.hitGroupId = 0;  // We will use the same hit group for all objects
		instance.matrix = obj->getMatrix();  // Position of the instance
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		TLAS.instances.push_back(instance);
	}
	// This AS does not contain geometry data so CreateGeometryTypeInfo is set to VK_GEOMETRY_TYPE_INSTANCES_KHR
	VkAccelerationStructureCreateGeometryTypeInfoKHR geometryCreate{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_GEOMETRY_TYPE_INFO_KHR };
	geometryCreate.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	geometryCreate.allowsTransforms = (VK_TRUE);

	VkAccelerationStructureCreateInfoKHR asCreateInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
	asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR; // TOP LEVEL
	asCreateInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	asCreateInfo.maxGeometryCount = 1; // Must be one to comply specification
	asCreateInfo.pGeometryInfos = &geometryCreate;

	TLAS.as = createAcceleration(asCreateInfo);

	// Compute the amount of scratch memory required by the acceleration structure builder
	VkAccelerationStructureMemoryRequirementsInfoKHR memoryRequirementsInfo{
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR };
	memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR;
	memoryRequirementsInfo.accelerationStructure = TLAS.as.accelerationStructure;
	memoryRequirementsInfo.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
	VkMemoryRequirements2 reqMem{ VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
	vkGetAccelerationStructureMemoryRequirementsKHR(Device::get(), &memoryRequirementsInfo, &reqMem);

	VkDeviceSize scratchSize = reqMem.memoryRequirements.size;

	// Allocate the scratch memory
	//SCRATCH BUFFER CREATION
	VkBuffer scratchBuffer;
	VkDeviceMemory scratchMem;
	createBuffer(PhysicalDevice::get(), Device::get(), scratchSize,
		VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, scratchBuffer, scratchMem);
	VkDeviceAddress scratchAddress = getBufferDeviceAddress(scratchBuffer);

	// For each instance, build the corresponding instance descriptor
	std::vector<VkAccelerationStructureInstanceKHR> geometryInstances;
	geometryInstances.reserve(TLAS.instances.size());
	for (const auto& inst : TLAS.instances)
	{
		geometryInstances.push_back(inst.to_VkAcInstanceKHR());
	}
	// We must load the AS instances in GPU memory
	// SIZE
	VkDeviceSize allocSize = geometryInstances.size() * sizeof(VkAccelerationStructureInstanceKHR);
	//--------------------------------------------------------------------------------------------------------------
	// STAGE buffer loading
	VkBuffer stageBuffer;
	VkDeviceMemory stageBufferMemory;
	void* mappedStageBuffer;
	createBuffer(PhysicalDevice::get(), Device::get(), allocSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		stageBuffer, stageBufferMemory);
	vkMapMemory(Device::get(),stageBufferMemory,0,allocSize,0, &mappedStageBuffer);
	memcpy(mappedStageBuffer, geometryInstances.data(), allocSize);
	vkUnmapMemory(Device::get(), stageBufferMemory);
	//Final TLAS instance buffer loading
	createBuffer(PhysicalDevice::get(), Device::get(), allocSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		TLAS.instanceBuffer, TLAS.instanceBufferMemory);
	//--------------------------------------------------------------------------------------------------------------
	// Copy data in GPU memory
	VkCommandBuffer cmdBuffer = beginSingleTimeCommandBuffer(Device::get(), Device::getGraphicCmdPool());
	VkBufferCopy copyRegion = {};
	copyRegion.size = allocSize;
	vkCmdCopyBuffer(cmdBuffer, stageBuffer, TLAS.instanceBuffer, 1, &copyRegion);
	//We put a barrier to make all as_structure build operations to wait for previous commands to end their write operations
	VkMemoryBarrier barrier = {VK_STRUCTURE_TYPE_MEMORY_BARRIER};
	barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
	// The barrier defines the type of the operations. But GPU work is in pipeline fascion so we must set the stages affected by the barrier
	vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 
		0, 1, &barrier, 0, nullptr, 0, nullptr);
	//--------------------------------------------------------------------------------------------------------------
	//FINALLY we have the data in deice local memory safely written ready for the TLAS build
	VkAccelerationStructureGeometryDataKHR gData = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR };
	gData.instances.arrayOfPointers = VK_FALSE;
	gData.instances.data.deviceAddress = getBufferDeviceAddress(TLAS.instanceBuffer);
	VkAccelerationStructureGeometryKHR topASGeometry = { VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
	topASGeometry.geometryType = VkGeometryTypeKHR::VK_GEOMETRY_TYPE_INSTANCES_KHR;
	topASGeometry.geometry = gData;

	VkAccelerationStructureBuildGeometryInfoKHR topASInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
	topASInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	topASInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	topASInfo.update = VK_FALSE;
	topASInfo.srcAccelerationStructure = VK_NULL_HANDLE;
	topASInfo.dstAccelerationStructure = TLAS.as.accelerationStructure;
	topASInfo.geometryArrayOfPointers = VK_FALSE;
	topASInfo.geometryCount = 1;
	VkAccelerationStructureGeometryKHR * pTopGeom = &topASGeometry;
	topASInfo.ppGeometries = &pTopGeom;
	topASInfo.scratchData.deviceAddress = scratchAddress;

	VkAccelerationStructureBuildOffsetInfoKHR offsets = {static_cast<uint32_t>(TLAS.instances.size()),0,0,0};
	const VkAccelerationStructureBuildOffsetInfoKHR* pOffsets = &offsets;

	vkCmdBuildAccelerationStructureKHR(cmdBuffer, 1, &topASInfo, &pOffsets);

	std::vector<VkCommandBuffer> buffers = { cmdBuffer };
	submitAndWaitCommandBuffers(Device::get(), Device::getGraphicQueue(), Device::getGraphicCmdPool(), buffers);
	//--------------------------------------------------------------------------------------------------------------

	//Stage CleanUp
	vkDestroyBuffer(Device::get(), stageBuffer, nullptr);
	vkFreeMemory(Device::get(), stageBufferMemory, nullptr);
	// Scratch buffer CleanUp
	vkDestroyBuffer(Device::get(), scratchBuffer, nullptr);
	vkFreeMemory(Device::get(), scratchMem, nullptr);
}

void RayTracer::initialize()
{
	LOAD_RAYTRACING_API_COMMANDS(Device::get());
}

void RayTracer::prepare(Scene3D * scene) {
	cleanUP();
	buildBottomLevelAS();
	buildTopLevelAS(scene);
}

void RayTracer::cleanUP()
{
	// Destroy TLAS resources
	vkDestroyAccelerationStructureKHR(Device::get(), TLAS.as.accelerationStructure, nullptr);
	vkFreeMemory(Device::get(), TLAS.as.memory, nullptr);
	vkDestroyBuffer(Device::get(), TLAS.instanceBuffer, nullptr);
	vkFreeMemory(Device::get(), TLAS.instanceBufferMemory, nullptr);
	TLAS.instances.clear();

	// Destroy BLAS resources
	for (auto & blas : BLASs) {
		vkDestroyAccelerationStructureKHR(Device::get(), blas.as.accelerationStructure, nullptr);
		vkFreeMemory(Device::get(), blas.as.memory, nullptr);
	}
	BLASs.clear();
}
