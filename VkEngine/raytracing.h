#pragma once
#include "commons.h"
#include "Mesh.h"


// Holds data for a ray tracing scratch buffer that is used as a temporary storage
struct RayTracingScratchBuffer
{
	uint64_t deviceAddress = 0;
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
};

// Describes a Mesh inside a Bottom Level AS
struct AccelerationStructureGeometry {
	VkAccelerationStructureCreateGeometryTypeInfoKHR info; // that defines how the AS will be constructed.
	VkAccelerationStructureGeometryKHR geometry; // the geometry for build the AS, in this case, from triangles.
	VkAccelerationStructureBuildOffsetInfoKHR offset; // the offset, which correspond to the actual wanted geometry when building.
};

// Ray tracing acceleration structure
struct AccelerationStructure {
	VkAccelerationStructureKHR accelerationStructure;
	uint64_t handle;
	VkDeviceMemory memory = VK_NULL_HANDLE;
};

// One BLAS can hold multiple geometries
struct BottomLevelAS {
	AccelerationStructure as;
	std::vector<VkAccelerationStructureCreateGeometryTypeInfoKHR> gCreateinfos; // that defines how the AS will be constructed.
	std::vector<VkAccelerationStructureGeometryKHR> geometries; // the geometry for build the AS, in this case, from triangles.
	std::vector<VkAccelerationStructureBuildOffsetInfoKHR> offsets; // the offset, which correspond to the actual wanted geometry when building.
};

/*
	Gets the device address from a buffer that's required for some of the buffers used for ray tracing
*/
uint64_t getBufferDeviceAddress(VkBuffer buffer);

/*
	Fills an AccelerationStructureGeometry structure from A Mesh3D object
*/
AccelerationStructureGeometry mesh3DToASGeometryKHR(const Mesh3D* model);

/*
	Creates an Acceleration Structure Vulkan object from give create infos, 
	then takes care to alloc and bind memory for it.
*/
AccelerationStructure createAcceleration(VkAccelerationStructureCreateInfoKHR& accel_);

class RayTracer {
public:
	static void initialize();
	static void prepare();
	static void cleanUP();
private:
	static void buildBottomLevelAS();
	//static void buildTopLevelAS();
	static std::vector<BottomLevelAS> BLASs;
};