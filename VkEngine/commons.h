#pragma once
#define VK_ENABLE_BETA_EXTENSIONS // Needed for Nvidia RTX
#include <vulkan/vulkan.h>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>


#include <chrono>
#include <array>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>
#include <queue>
#include <set>
#include <algorithm>
#include <fstream>
#include <map>
#include <unordered_map>
#include <thread>
#include <numeric>
#include <assert.h>