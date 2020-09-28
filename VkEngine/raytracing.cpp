#include "raytracing.h"
#include "Device.h"
#include "MeshManager.h"
#include "PhysicalDevice.h"
#include "ApiUtils.h"
#include "vk_extensions.h"

std::vector<BottomLevelAS> RayTracer::BLASs;

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
	gCreate.geometryType = VkGeometryTypeKHR::VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	gCreate.indexType = VkIndexType::VK_INDEX_TYPE_UINT32;
	gCreate.vertexFormat = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
	gCreate.maxPrimitiveCount = model->getIdxCount() / 3; // how many triangles
	gCreate.maxVertexCount = model->getVertexCount();
	gCreate.allowsTransforms = VK_FALSE; // Not adding transformation matrices

	VkDeviceAddress vertexAddr = getBufferDeviceAddress(model->getVkVertexBuffer());
	VkDeviceAddress indexAddr = getBufferDeviceAddress(model->getVkIndexBuffer());

	// We use triangles but other "geometries" are supported like AABBs or INSTANCES data
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

void RayTracer::buildTopLevelAS(vkengine::Scene3D * scene)
{
	for (auto & obj : scene->listObjects()) {

	}
}

void RayTracer::initialize()
{
	LOAD_RAYTRACING_API_COMMANDS(Device::get());
}

void RayTracer::prepare(vkengine::Scene3D * scene) {
	cleanUP();
	buildBottomLevelAS();
	buildTopLevelAS(scene);
}

void RayTracer::cleanUP()
{
	for (auto blas : BLASs) {
		vkDestroyAccelerationStructureKHR(Device::get(), blas.as.accelerationStructure, nullptr);
		vkFreeMemory(Device::get(), blas.as.memory, nullptr);
	}
	BLASs.clear();
}
