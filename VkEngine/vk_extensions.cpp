#include "vk_extensions.h"
#include <assert.h>


// Vulkan raytracing functions: these are in beta and must be loaded manualy :(
static PFN_vkGetBufferDeviceAddressKHR pfn_vkGetBufferDeviceAddressKHR = 0;
static PFN_vkCreateAccelerationStructureKHR  pfn_vkCreateAccelerationStructureKHR = 0;
static PFN_vkDestroyAccelerationStructureKHR  pfn_vkDestroyAccelerationStructureKHR = 0;
static PFN_vkCmdBuildAccelerationStructuresKHR  pfn_vkCmdBuildAccelerationStructuresKHR = 0;
static PFN_vkBuildAccelerationStructuresKHR  pfn_vkBuildAccelerationStructureKHR = 0;
static PFN_vkGetAccelerationStructureDeviceAddressKHR  pfn_vkGetAccelerationStructureDeviceAddressKHR = 0;
static PFN_vkCmdTraceRaysKHR  pfn_vkCmdTraceRaysKHR = 0;
static PFN_vkGetRayTracingShaderGroupHandlesKHR  pfn_vkGetRayTracingShaderGroupHandlesKHR = 0;
static PFN_vkCreateRayTracingPipelinesKHR  pfn_vkCreateRayTracingPipelinesKHR = 0;
static PFN_vkCmdWriteAccelerationStructuresPropertiesKHR  pfn_vkCmdWriteAccelerationStructuresPropertiesKHR = 0;
static PFN_vkCmdCopyAccelerationStructureKHR  pfn_vkCmdCopyAccelerationStructureKHR = 0;
static PFN_vkGetAccelerationStructureBuildSizesKHR pfn_vkGetAccelerationStructureBuildSizesKHR = 0;

void LOAD_RAYTRACING_API_COMMANDS(VkDevice device) {
	pfn_vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR"));
	pfn_vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));
	pfn_vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
	pfn_vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));
	pfn_vkBuildAccelerationStructureKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkBuildAccelerationStructureKHR"));
	pfn_vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR"));
	pfn_vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
	pfn_vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
	pfn_vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));
	pfn_vkCmdWriteAccelerationStructuresPropertiesKHR = reinterpret_cast<PFN_vkCmdWriteAccelerationStructuresPropertiesKHR>(vkGetDeviceProcAddr(device, "vkCmdWriteAccelerationStructuresPropertiesKHR"));
	pfn_vkCmdCopyAccelerationStructureKHR = reinterpret_cast<PFN_vkCmdCopyAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkCmdCopyAccelerationStructureKHR"));
	pfn_vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));
}

VKAPI_ATTR void VKAPI_CALL vkGetAccelerationStructureBuildSizesKHR(
	VkDevice                                    device,
	VkAccelerationStructureBuildTypeKHR         buildType,
	const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
	const uint32_t* pMaxPrimitiveCounts,
	VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo)
{
	assert(pfn_vkGetAccelerationStructureBuildSizesKHR);
	return pfn_vkGetAccelerationStructureBuildSizesKHR( device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyAccelerationStructureKHR(VkCommandBuffer commandBuffer,	const VkCopyAccelerationStructureInfoKHR* pInfo)
{
	assert(pfn_vkCmdCopyAccelerationStructureKHR);
	return pfn_vkCmdCopyAccelerationStructureKHR( commandBuffer, pInfo);
}

VKAPI_ATTR void VKAPI_CALL vkCmdWriteAccelerationStructuresPropertiesKHR(VkCommandBuffer commandBuffer, uint32_t accelerationStructureCount, 
	const VkAccelerationStructureKHR* pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery)
{
	assert(pfn_vkCmdWriteAccelerationStructuresPropertiesKHR);
	return pfn_vkCmdWriteAccelerationStructuresPropertiesKHR( commandBuffer, accelerationStructureCount,
		 pAccelerationStructures, queryType, queryPool, firstQuery);
}

VKAPI_ATTR VkDeviceAddress VKAPI_CALL vkGetBufferDeviceAddressKHR(
	VkDevice device,
	const VkBufferDeviceAddressInfoKHR* pInfo)
{
	assert(pfn_vkGetBufferDeviceAddressKHR);
	return pfn_vkGetBufferDeviceAddressKHR(device, pInfo);
}
VKAPI_ATTR VkResult VKAPI_CALL vkCreateAccelerationStructureKHR(
	VkDevice device,
	const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkAccelerationStructureKHR* pAccelerationStructure)
{
	assert(pfn_vkCreateAccelerationStructureKHR);
	return pfn_vkCreateAccelerationStructureKHR(device, pCreateInfo, pAllocator, pAccelerationStructure);
}
VKAPI_ATTR void VKAPI_CALL vkDestroyAccelerationStructureKHR(
	VkDevice device,
	VkAccelerationStructureKHR accelerationStructure,
	const VkAllocationCallbacks* pAllocator)
{
	assert(pfn_vkDestroyAccelerationStructureKHR);
	pfn_vkDestroyAccelerationStructureKHR(device, accelerationStructure, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkCmdBuildAccelerationStructuresKHR(
	VkCommandBuffer buffer,
	uint32_t infoCount,
	const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
	const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos)
{
	assert(pfn_vkCmdBuildAccelerationStructuresKHR);
	pfn_vkCmdBuildAccelerationStructuresKHR(buffer, infoCount, pInfos, ppBuildRangeInfos);
}

VKAPI_ATTR void VKAPI_CALL vkCmdTraceRaysKHR(
	VkCommandBuffer commandBuffer,
	const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
	const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
	const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
	const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
	uint32_t width,
	uint32_t height,
	uint32_t depth)
{
	assert(pfn_vkCmdTraceRaysKHR);
	pfn_vkCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}
VKAPI_ATTR VkResult VKAPI_CALL vkCreateRayTracingPipelinesKHR(
	VkDevice device, 
	VkDeferredOperationKHR deferredOperation,
	VkPipelineCache pipelineCache,
	uint32_t createInfoCount,
	const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
	const VkAllocationCallbacks* pAllocator,
	VkPipeline* pPipelines)
{
	assert(pfn_vkCreateRayTracingPipelinesKHR);
	return pfn_vkCreateRayTracingPipelinesKHR(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
VKAPI_ATTR VkResult VKAPI_CALL vkGetRayTracingShaderGroupHandlesKHR(
	VkDevice device,
	VkPipeline pipeline,
	uint32_t firstGroup,
	uint32_t groupCount,
	size_t dataSize,
	void* pData)
{
	assert(pfn_vkGetRayTracingShaderGroupHandlesKHR);
	return pfn_vkGetRayTracingShaderGroupHandlesKHR(device, pipeline, firstGroup, groupCount, dataSize, pData);
}
VKAPI_ATTR VkDeviceAddress VKAPI_CALL vkGetAccelerationStructureDeviceAddressKHR(
	VkDevice device,
	const VkAccelerationStructureDeviceAddressInfoKHR* pInfo)
{
	assert(pfn_vkGetAccelerationStructureDeviceAddressKHR);
	return pfn_vkGetAccelerationStructureDeviceAddressKHR(device, pInfo);
}

