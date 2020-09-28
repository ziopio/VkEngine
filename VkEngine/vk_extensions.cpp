#include "vk_extensions.h"
#include <assert.h>



// Vulkan raytracing functions: these are in beta and must be loaded manualy :(
static PFN_vkGetBufferDeviceAddressKHR pfn_vkGetBufferDeviceAddressKHR = 0;
static PFN_vkBindAccelerationStructureMemoryKHR  pfn_vkBindAccelerationStructureMemoryKHR = 0;
static PFN_vkCreateAccelerationStructureKHR  pfn_vkCreateAccelerationStructureKHR = 0;
static PFN_vkDestroyAccelerationStructureKHR  pfn_vkDestroyAccelerationStructureKHR = 0;
static PFN_vkGetAccelerationStructureMemoryRequirementsKHR  pfn_vkGetAccelerationStructureMemoryRequirementsKHR = 0;
static PFN_vkCmdBuildAccelerationStructureKHR  pfn_vkCmdBuildAccelerationStructureKHR = 0;
static PFN_vkBuildAccelerationStructureKHR  pfn_vkBuildAccelerationStructureKHR = 0;
static PFN_vkGetAccelerationStructureDeviceAddressKHR  pfn_vkGetAccelerationStructureDeviceAddressKHR = 0;
static PFN_vkCmdTraceRaysKHR  pfn_vkCmdTraceRaysKHR = 0;
static PFN_vkGetRayTracingShaderGroupHandlesKHR  pfn_vkGetRayTracingShaderGroupHandlesKHR = 0;
static PFN_vkCreateRayTracingPipelinesKHR  pfn_vkCreateRayTracingPipelinesKHR = 0;

void LOAD_RAYTRACING_API_COMMANDS(VkDevice device) {
	pfn_vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR"));
	pfn_vkBindAccelerationStructureMemoryKHR = reinterpret_cast<PFN_vkBindAccelerationStructureMemoryKHR>(vkGetDeviceProcAddr(device, "vkBindAccelerationStructureMemoryKHR"));
	pfn_vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));
	pfn_vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
	pfn_vkGetAccelerationStructureMemoryRequirementsKHR = reinterpret_cast<PFN_vkGetAccelerationStructureMemoryRequirementsKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureMemoryRequirementsKHR"));
	pfn_vkCmdBuildAccelerationStructureKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructureKHR"));
	pfn_vkBuildAccelerationStructureKHR = reinterpret_cast<PFN_vkBuildAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkBuildAccelerationStructureKHR"));
	pfn_vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR"));
	pfn_vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
	pfn_vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
	pfn_vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));
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
VKAPI_ATTR void VKAPI_CALL vkGetAccelerationStructureMemoryRequirementsKHR(
	VkDevice device,
	const VkAccelerationStructureMemoryRequirementsInfoKHR* pInfo,
	VkMemoryRequirements2* pMemoryRequirements)
{
	assert(pfn_vkGetAccelerationStructureMemoryRequirementsKHR);
	pfn_vkGetAccelerationStructureMemoryRequirementsKHR(device, pInfo, pMemoryRequirements);
}
VKAPI_ATTR VkResult VKAPI_CALL vkBindAccelerationStructureMemoryKHR(
	VkDevice device,
	uint32_t bindInfoCount,
	const VkBindAccelerationStructureMemoryInfoKHR* pBindInfos)
{
	assert(pfn_vkBindAccelerationStructureMemoryKHR);
	return pfn_vkBindAccelerationStructureMemoryKHR(device, bindInfoCount, pBindInfos);
}
VKAPI_ATTR void VKAPI_CALL vkCmdBuildAccelerationStructureKHR(
	VkCommandBuffer commandBuffer,
	uint32_t infoCount,
	const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
	const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos)
{
	assert(pfn_vkCmdBuildAccelerationStructureKHR);
	pfn_vkCmdBuildAccelerationStructureKHR(commandBuffer, infoCount, pInfos, ppOffsetInfos);
}

VKAPI_ATTR VkResult VKAPI_CALL vkBuildAccelerationStructureKHR(
	VkDevice device,
	uint32_t infoCount,
	const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
	const VkAccelerationStructureBuildOffsetInfoKHR* const* ppOffsetInfos)
{
	assert(pfn_vkBuildAccelerationStructureKHR);
	return pfn_vkBuildAccelerationStructureKHR(device, infoCount, pInfos, ppOffsetInfos);
}
VKAPI_ATTR void VKAPI_CALL vkCmdTraceRaysKHR(
	VkCommandBuffer commandBuffer,
	const VkStridedBufferRegionKHR* pRaygenShaderBindingTable,
	const VkStridedBufferRegionKHR* pMissShaderBindingTable,
	const VkStridedBufferRegionKHR* pHitShaderBindingTable,
	const VkStridedBufferRegionKHR* pCallableShaderBindingTable,
	uint32_t width,
	uint32_t height,
	uint32_t depth)
{
	assert(pfn_vkCmdTraceRaysKHR);
	pfn_vkCmdTraceRaysKHR(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}
VKAPI_ATTR VkResult VKAPI_CALL vkCreateRayTracingPipelinesKHR(
	VkDevice device,
	VkPipelineCache pipelineCache,
	uint32_t createInfoCount,
	const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
	const VkAllocationCallbacks* pAllocator,
	VkPipeline* pPipelines)
{
	assert(pfn_vkCreateRayTracingPipelinesKHR);
	return pfn_vkCreateRayTracingPipelinesKHR(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
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

