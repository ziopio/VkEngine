#pragma once
#include "Scene3D.h"
#include "Mesh.h"
#include "Device.h"
#include "commons.h"


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

// A TLAS Instance points to a geometry inside one BLAS
// Like a scene object would point to its mesh
struct TLAS_Instance {
	VkDeviceAddress				blasAddr;
	uint32_t					instance_id{ 0 };  // Instance Index (gl_InstanceID)
	uint32_t					hitGroupId{ 0 };  // Hit group index in the SBT
	uint32_t					mask{ 0xFF };     // Visibility mask, will be AND-ed with ray mask
	VkGeometryInstanceFlagsKHR  flags{ VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR };
	glm::mat4					matrix{ glm::mat4(1) };  // Identity
	// Helper to handle conversion of the transfor matrix from column major 4x4 to row major 3x4
	VkAccelerationStructureInstanceKHR to_VkAcInstanceKHR() const {
		glm::mat4 t = glm::transpose(matrix);

		VkAccelerationStructureInstanceKHR acInstance = {};
		acInstance.instanceCustomIndex = instance_id,
		acInstance.mask = mask;
		acInstance.instanceShaderBindingTableRecordOffset = hitGroupId;
		acInstance.flags = flags;
		acInstance.accelerationStructureReference = blasAddr;

		memcpy(&acInstance.transform, &t, sizeof(VkTransformMatrixKHR));
		return acInstance;
	};
};

struct TopLevelAS {
	AccelerationStructure as;
	Buffer instanceBuffer;
	std::vector<TLAS_Instance> instances;
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
	static void prepare(vkengine::Scene3D * scene);
	static void cleanUP();
private:
	static void buildBottomLevelAS();
	static void buildTopLevelAS(vkengine::Scene3D * scene);
	static void createRtDescriptorSets();
private:
	// Accelleration structures
	static TopLevelAS TLAS;
	static std::vector<BottomLevelAS> BLASs;
};