#pragma once
 
 #include "definitions.h"
 
 #include <vulkan/vulkan.h>
 
 
 typedef struct vulkan_context {
     VkInstance instance;
     VkAllocationCallbacks* allocator;
 } vulkan_context;
