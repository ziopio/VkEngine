# VkEngine
Experimental Real-Time 3D rendering engine based on the modern Vulkan API.

Supports Vulkan-Raytracing !!!
(if your GPU drivers support the needed vulkan extensions)

## Disclaimer

Old university project, a lot of bad code, lots of warnings and Italian comments xD

This is a project developed with Visual Studio and targeting Windows SDKs.
The code itself is cross-platform by design but the build system is Windows specific.

Ideally, this codebase should use Cmake and rely on it to generate build configurations for different IDEs and Operating Systems.

## Building on Windows (least paifull way)

This project does not use CMAKE so have fun doing some tedious manual work...

- Install Visual Studio, latest is better.
- Install the latest Vulkan SDK, this project was ok with 1.2 but should work also with 1.3.
- Clone the repo.
- Download and extract the [Data](https://drive.google.com/file/d/15FUx1QjGZAkPCjJmOF_exL-c1_LVn3X0/view?usp=sharing) folder in the main directory.
- Go inside VkEngine/Shaders and fix all .bat SPIRV scripts, checking that they match your Vulkan SDK installation.
- Run all scripts and compile all the shaders.
- Open the project with VS, update the solution to latest Windows SDK.
- Check all include and lib dirs in each project configuration inside VS.
- Is important that the vulkan-1.lib, vulkan.h and glfw3.lib are reachable.
- Check that Editor is set as startup project.
- Press Play and if it gives errors have fun debugging :D

With newer versions of Vulkan there might be new Validation layer errors shown if runned in debug mode.

## Documentation

  // TODO here
