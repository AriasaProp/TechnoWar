#include "vulkan_graphics.hpp"

#include <vector>
#include <unordered_set>
#include <cassert>

//{
#include <vulkan/vulkan.h>
PFN_vkCreateInstance vkCreateInstance;
PFN_vkDestroyInstance vkDestroyInstance;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
PFN_vkCreateDevice vkCreateDevice;
PFN_vkDestroyDevice vkDestroyDevice;
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties;
PFN_vkGetDeviceQueue vkGetDeviceQueue;
PFN_vkQueueSubmit vkQueueSubmit;
PFN_vkQueueWaitIdle vkQueueWaitIdle;
PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
PFN_vkAllocateMemory vkAllocateMemory;
PFN_vkFreeMemory vkFreeMemory;
PFN_vkMapMemory vkMapMemory;
PFN_vkUnmapMemory vkUnmapMemory;
PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment;
PFN_vkBindBufferMemory vkBindBufferMemory;
PFN_vkBindImageMemory vkBindImageMemory;
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
PFN_vkGetImageSparseMemoryRequirements vkGetImageSparseMemoryRequirements;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties vkGetPhysicalDeviceSparseImageFormatProperties;
PFN_vkQueueBindSparse vkQueueBindSparse;
PFN_vkCreateFence vkCreateFence;
PFN_vkDestroyFence vkDestroyFence;
PFN_vkResetFences vkResetFences;
PFN_vkGetFenceStatus vkGetFenceStatus;
PFN_vkWaitForFences vkWaitForFences;
PFN_vkCreateSemaphore vkCreateSemaphore;
PFN_vkDestroySemaphore vkDestroySemaphore;
PFN_vkCreateEvent vkCreateEvent;
PFN_vkDestroyEvent vkDestroyEvent;
PFN_vkGetEventStatus vkGetEventStatus;
PFN_vkSetEvent vkSetEvent;
PFN_vkResetEvent vkResetEvent;
PFN_vkCreateQueryPool vkCreateQueryPool;
PFN_vkDestroyQueryPool vkDestroyQueryPool;
PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
PFN_vkCreateBuffer vkCreateBuffer;
PFN_vkDestroyBuffer vkDestroyBuffer;
PFN_vkCreateBufferView vkCreateBufferView;
PFN_vkDestroyBufferView vkDestroyBufferView;
PFN_vkCreateImage vkCreateImage;
PFN_vkDestroyImage vkDestroyImage;
PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
PFN_vkCreateImageView vkCreateImageView;
PFN_vkDestroyImageView vkDestroyImageView;
PFN_vkCreateShaderModule vkCreateShaderModule;
PFN_vkDestroyShaderModule vkDestroyShaderModule;
PFN_vkCreatePipelineCache vkCreatePipelineCache;
PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
PFN_vkGetPipelineCacheData vkGetPipelineCacheData;
PFN_vkMergePipelineCaches vkMergePipelineCaches;
PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
PFN_vkCreateComputePipelines vkCreateComputePipelines;
PFN_vkDestroyPipeline vkDestroyPipeline;
PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
PFN_vkCreateSampler vkCreateSampler;
PFN_vkDestroySampler vkDestroySampler;
PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
PFN_vkResetDescriptorPool vkResetDescriptorPool;
PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
PFN_vkCreateFramebuffer vkCreateFramebuffer;
PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
PFN_vkCreateRenderPass vkCreateRenderPass;
PFN_vkDestroyRenderPass vkDestroyRenderPass;
PFN_vkGetRenderAreaGranularity vkGetRenderAreaGranularity;
PFN_vkCreateCommandPool vkCreateCommandPool;
PFN_vkDestroyCommandPool vkDestroyCommandPool;
PFN_vkResetCommandPool vkResetCommandPool;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
PFN_vkEndCommandBuffer vkEndCommandBuffer;
PFN_vkResetCommandBuffer vkResetCommandBuffer;
PFN_vkCmdBindPipeline vkCmdBindPipeline;
PFN_vkCmdSetViewport vkCmdSetViewport;
PFN_vkCmdSetScissor vkCmdSetScissor;
PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants;
PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds;
PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask;
PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask;
PFN_vkCmdSetStencilReference vkCmdSetStencilReference;
PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
PFN_vkCmdDraw vkCmdDraw;
PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
PFN_vkCmdDispatch vkCmdDispatch;
PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect;
PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
PFN_vkCmdCopyImage vkCmdCopyImage;
PFN_vkCmdBlitImage vkCmdBlitImage;
PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;
PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer;
PFN_vkCmdFillBuffer vkCmdFillBuffer;
PFN_vkCmdClearColorImage vkCmdClearColorImage;
PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage;
PFN_vkCmdClearAttachments vkCmdClearAttachments;
PFN_vkCmdResolveImage vkCmdResolveImage;
PFN_vkCmdSetEvent vkCmdSetEvent;
PFN_vkCmdResetEvent vkCmdResetEvent;
PFN_vkCmdWaitEvents vkCmdWaitEvents;
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
PFN_vkCmdBeginQuery vkCmdBeginQuery;
PFN_vkCmdEndQuery vkCmdEndQuery;
PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;
PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;
PFN_vkCmdPushConstants vkCmdPushConstants;
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
PFN_vkCmdNextSubpass vkCmdNextSubpass;
PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
// VK_KHR_surface
PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
// VK_KHR_swapchain
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
PFN_vkQueuePresentKHR vkQueuePresentKHR;
// VK_KHR_display
PFN_vkGetPhysicalDeviceDisplayPropertiesKHR vkGetPhysicalDeviceDisplayPropertiesKHR;
PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR vkGetPhysicalDeviceDisplayPlanePropertiesKHR;
PFN_vkGetDisplayPlaneSupportedDisplaysKHR vkGetDisplayPlaneSupportedDisplaysKHR;
PFN_vkGetDisplayModePropertiesKHR vkGetDisplayModePropertiesKHR;
PFN_vkCreateDisplayModeKHR vkCreateDisplayModeKHR;
PFN_vkGetDisplayPlaneCapabilitiesKHR vkGetDisplayPlaneCapabilitiesKHR;
PFN_vkCreateDisplayPlaneSurfaceKHR vkCreateDisplayPlaneSurfaceKHR;
PFN_vkCreateSharedSwapchainsKHR vkCreateSharedSwapchainsKHR;
PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR;

#ifdef USE_DEBUG_EXTENTIONS
#include <vulkan/vk_sdk_platform.h>
// VK_EXT_debug_report
PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT;
#endif

static bool Initialize_Vulkan() {
    void* libvulkan = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!libvulkan)
        return false;
    vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(dlsym(libvulkan, "vkCreateInstance"));
    vkDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(dlsym(libvulkan, "vkDestroyInstance"));
    vkEnumeratePhysicalDevices = reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(dlsym(libvulkan, "vkEnumeratePhysicalDevices"));
    vkGetPhysicalDeviceFeatures = reinterpret_cast<PFN_vkGetPhysicalDeviceFeatures>(dlsym(libvulkan, "vkGetPhysicalDeviceFeatures"));
    vkGetPhysicalDeviceFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceFormatProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceFormatProperties"));
    vkGetPhysicalDeviceImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceImageFormatProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceImageFormatProperties"));
    vkGetPhysicalDeviceProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceProperties"));
    vkGetPhysicalDeviceQueueFamilyProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceQueueFamilyProperties"));
    vkGetPhysicalDeviceMemoryProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceMemoryProperties"));
    vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(libvulkan, "vkGetInstanceProcAddr"));
    vkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(dlsym(libvulkan, "vkGetDeviceProcAddr"));
    vkCreateDevice = reinterpret_cast<PFN_vkCreateDevice>(dlsym(libvulkan, "vkCreateDevice"));
    vkDestroyDevice = reinterpret_cast<PFN_vkDestroyDevice>(dlsym(libvulkan, "vkDestroyDevice"));
    vkEnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(dlsym(libvulkan, "vkEnumerateInstanceExtensionProperties"));
    vkEnumerateDeviceExtensionProperties = reinterpret_cast<PFN_vkEnumerateDeviceExtensionProperties>(dlsym(libvulkan, "vkEnumerateDeviceExtensionProperties"));
    vkEnumerateInstanceLayerProperties = reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(dlsym(libvulkan, "vkEnumerateInstanceLayerProperties"));
    vkEnumerateDeviceLayerProperties = reinterpret_cast<PFN_vkEnumerateDeviceLayerProperties>(dlsym(libvulkan, "vkEnumerateDeviceLayerProperties"));
    vkGetDeviceQueue = reinterpret_cast<PFN_vkGetDeviceQueue>(dlsym(libvulkan, "vkGetDeviceQueue"));
    vkQueueSubmit = reinterpret_cast<PFN_vkQueueSubmit>(dlsym(libvulkan, "vkQueueSubmit"));
    vkQueueWaitIdle = reinterpret_cast<PFN_vkQueueWaitIdle>(dlsym(libvulkan, "vkQueueWaitIdle"));
    vkDeviceWaitIdle = reinterpret_cast<PFN_vkDeviceWaitIdle>(dlsym(libvulkan, "vkDeviceWaitIdle"));
    vkAllocateMemory = reinterpret_cast<PFN_vkAllocateMemory>(dlsym(libvulkan, "vkAllocateMemory"));
    vkFreeMemory = reinterpret_cast<PFN_vkFreeMemory>(dlsym(libvulkan, "vkFreeMemory"));
    vkMapMemory = reinterpret_cast<PFN_vkMapMemory>(dlsym(libvulkan, "vkMapMemory"));
    vkUnmapMemory = reinterpret_cast<PFN_vkUnmapMemory>(dlsym(libvulkan, "vkUnmapMemory"));
    vkFlushMappedMemoryRanges = reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(dlsym(libvulkan, "vkFlushMappedMemoryRanges"));
    vkInvalidateMappedMemoryRanges = reinterpret_cast<PFN_vkInvalidateMappedMemoryRanges>(dlsym(libvulkan, "vkInvalidateMappedMemoryRanges"));
    vkGetDeviceMemoryCommitment = reinterpret_cast<PFN_vkGetDeviceMemoryCommitment>(dlsym(libvulkan, "vkGetDeviceMemoryCommitment"));
    vkBindBufferMemory = reinterpret_cast<PFN_vkBindBufferMemory>(dlsym(libvulkan, "vkBindBufferMemory"));
    vkBindImageMemory = reinterpret_cast<PFN_vkBindImageMemory>(dlsym(libvulkan, "vkBindImageMemory"));
    vkGetBufferMemoryRequirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(dlsym(libvulkan, "vkGetBufferMemoryRequirements"));
    vkGetImageMemoryRequirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(dlsym(libvulkan, "vkGetImageMemoryRequirements"));
    vkGetImageSparseMemoryRequirements = reinterpret_cast<PFN_vkGetImageSparseMemoryRequirements>(dlsym(libvulkan, "vkGetImageSparseMemoryRequirements"));
    vkGetPhysicalDeviceSparseImageFormatProperties = reinterpret_cast<PFN_vkGetPhysicalDeviceSparseImageFormatProperties>(dlsym(libvulkan, "vkGetPhysicalDeviceSparseImageFormatProperties"));
    vkQueueBindSparse = reinterpret_cast<PFN_vkQueueBindSparse>(dlsym(libvulkan, "vkQueueBindSparse"));
    vkCreateFence = reinterpret_cast<PFN_vkCreateFence>(dlsym(libvulkan, "vkCreateFence"));
    vkDestroyFence = reinterpret_cast<PFN_vkDestroyFence>(dlsym(libvulkan, "vkDestroyFence"));
    vkResetFences = reinterpret_cast<PFN_vkResetFences>(dlsym(libvulkan, "vkResetFences"));
    vkGetFenceStatus = reinterpret_cast<PFN_vkGetFenceStatus>(dlsym(libvulkan, "vkGetFenceStatus"));
    vkWaitForFences = reinterpret_cast<PFN_vkWaitForFences>(dlsym(libvulkan, "vkWaitForFences"));
    vkCreateSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(dlsym(libvulkan, "vkCreateSemaphore"));
    vkDestroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(dlsym(libvulkan, "vkDestroySemaphore"));
    vkCreateEvent = reinterpret_cast<PFN_vkCreateEvent>(dlsym(libvulkan, "vkCreateEvent"));
    vkDestroyEvent = reinterpret_cast<PFN_vkDestroyEvent>(dlsym(libvulkan, "vkDestroyEvent"));
    vkGetEventStatus = reinterpret_cast<PFN_vkGetEventStatus>(dlsym(libvulkan, "vkGetEventStatus"));
    vkSetEvent = reinterpret_cast<PFN_vkSetEvent>(dlsym(libvulkan, "vkSetEvent"));
    vkResetEvent = reinterpret_cast<PFN_vkResetEvent>(dlsym(libvulkan, "vkResetEvent"));
    vkCreateQueryPool = reinterpret_cast<PFN_vkCreateQueryPool>(dlsym(libvulkan, "vkCreateQueryPool"));
    vkDestroyQueryPool = reinterpret_cast<PFN_vkDestroyQueryPool>(dlsym(libvulkan, "vkDestroyQueryPool"));
    vkGetQueryPoolResults = reinterpret_cast<PFN_vkGetQueryPoolResults>(dlsym(libvulkan, "vkGetQueryPoolResults"));
    vkCreateBuffer = reinterpret_cast<PFN_vkCreateBuffer>(dlsym(libvulkan, "vkCreateBuffer"));
    vkDestroyBuffer = reinterpret_cast<PFN_vkDestroyBuffer>(dlsym(libvulkan, "vkDestroyBuffer"));
    vkCreateBufferView = reinterpret_cast<PFN_vkCreateBufferView>(dlsym(libvulkan, "vkCreateBufferView"));
    vkDestroyBufferView = reinterpret_cast<PFN_vkDestroyBufferView>(dlsym(libvulkan, "vkDestroyBufferView"));
    vkCreateImage = reinterpret_cast<PFN_vkCreateImage>(dlsym(libvulkan, "vkCreateImage"));
    vkDestroyImage = reinterpret_cast<PFN_vkDestroyImage>(dlsym(libvulkan, "vkDestroyImage"));
    vkGetImageSubresourceLayout = reinterpret_cast<PFN_vkGetImageSubresourceLayout>(dlsym(libvulkan, "vkGetImageSubresourceLayout"));
    vkCreateImageView = reinterpret_cast<PFN_vkCreateImageView>(dlsym(libvulkan, "vkCreateImageView"));
    vkDestroyImageView = reinterpret_cast<PFN_vkDestroyImageView>(dlsym(libvulkan, "vkDestroyImageView"));
    vkCreateShaderModule = reinterpret_cast<PFN_vkCreateShaderModule>(dlsym(libvulkan, "vkCreateShaderModule"));
    vkDestroyShaderModule = reinterpret_cast<PFN_vkDestroyShaderModule>(dlsym(libvulkan, "vkDestroyShaderModule"));
    vkCreatePipelineCache = reinterpret_cast<PFN_vkCreatePipelineCache>(dlsym(libvulkan, "vkCreatePipelineCache"));
    vkDestroyPipelineCache = reinterpret_cast<PFN_vkDestroyPipelineCache>(dlsym(libvulkan, "vkDestroyPipelineCache"));
    vkGetPipelineCacheData = reinterpret_cast<PFN_vkGetPipelineCacheData>(dlsym(libvulkan, "vkGetPipelineCacheData"));
    vkMergePipelineCaches = reinterpret_cast<PFN_vkMergePipelineCaches>(dlsym(libvulkan, "vkMergePipelineCaches"));
    vkCreateGraphicsPipelines = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(dlsym(libvulkan, "vkCreateGraphicsPipelines"));
    vkCreateComputePipelines = reinterpret_cast<PFN_vkCreateComputePipelines>(dlsym(libvulkan, "vkCreateComputePipelines"));
    vkDestroyPipeline = reinterpret_cast<PFN_vkDestroyPipeline>(dlsym(libvulkan, "vkDestroyPipeline"));
    vkCreatePipelineLayout = reinterpret_cast<PFN_vkCreatePipelineLayout>(dlsym(libvulkan, "vkCreatePipelineLayout"));
    vkDestroyPipelineLayout = reinterpret_cast<PFN_vkDestroyPipelineLayout>(dlsym(libvulkan, "vkDestroyPipelineLayout"));
    vkCreateSampler = reinterpret_cast<PFN_vkCreateSampler>(dlsym(libvulkan, "vkCreateSampler"));
    vkDestroySampler = reinterpret_cast<PFN_vkDestroySampler>(dlsym(libvulkan, "vkDestroySampler"));
    vkCreateDescriptorSetLayout = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(dlsym(libvulkan, "vkCreateDescriptorSetLayout"));
    vkDestroyDescriptorSetLayout = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(dlsym(libvulkan, "vkDestroyDescriptorSetLayout"));
    vkCreateDescriptorPool = reinterpret_cast<PFN_vkCreateDescriptorPool>(dlsym(libvulkan, "vkCreateDescriptorPool"));
    vkDestroyDescriptorPool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(dlsym(libvulkan, "vkDestroyDescriptorPool"));
    vkResetDescriptorPool = reinterpret_cast<PFN_vkResetDescriptorPool>(dlsym(libvulkan, "vkResetDescriptorPool"));
    vkAllocateDescriptorSets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(dlsym(libvulkan, "vkAllocateDescriptorSets"));
    vkFreeDescriptorSets = reinterpret_cast<PFN_vkFreeDescriptorSets>(dlsym(libvulkan, "vkFreeDescriptorSets"));
    vkUpdateDescriptorSets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(dlsym(libvulkan, "vkUpdateDescriptorSets"));
    vkCreateFramebuffer = reinterpret_cast<PFN_vkCreateFramebuffer>(dlsym(libvulkan, "vkCreateFramebuffer"));
    vkDestroyFramebuffer = reinterpret_cast<PFN_vkDestroyFramebuffer>(dlsym(libvulkan, "vkDestroyFramebuffer"));
    vkCreateRenderPass = reinterpret_cast<PFN_vkCreateRenderPass>(dlsym(libvulkan, "vkCreateRenderPass"));
    vkDestroyRenderPass = reinterpret_cast<PFN_vkDestroyRenderPass>(dlsym(libvulkan, "vkDestroyRenderPass"));
    vkGetRenderAreaGranularity = reinterpret_cast<PFN_vkGetRenderAreaGranularity>(dlsym(libvulkan, "vkGetRenderAreaGranularity"));
    vkCreateCommandPool = reinterpret_cast<PFN_vkCreateCommandPool>(dlsym(libvulkan, "vkCreateCommandPool"));
    vkDestroyCommandPool = reinterpret_cast<PFN_vkDestroyCommandPool>(dlsym(libvulkan, "vkDestroyCommandPool"));
    vkResetCommandPool = reinterpret_cast<PFN_vkResetCommandPool>(dlsym(libvulkan, "vkResetCommandPool"));
    vkAllocateCommandBuffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(dlsym(libvulkan, "vkAllocateCommandBuffers"));
    vkFreeCommandBuffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(dlsym(libvulkan, "vkFreeCommandBuffers"));
    vkBeginCommandBuffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(dlsym(libvulkan, "vkBeginCommandBuffer"));
    vkEndCommandBuffer = reinterpret_cast<PFN_vkEndCommandBuffer>(dlsym(libvulkan, "vkEndCommandBuffer"));
    vkResetCommandBuffer = reinterpret_cast<PFN_vkResetCommandBuffer>(dlsym(libvulkan, "vkResetCommandBuffer"));
    vkCmdBindPipeline = reinterpret_cast<PFN_vkCmdBindPipeline>(dlsym(libvulkan, "vkCmdBindPipeline"));
    vkCmdSetViewport = reinterpret_cast<PFN_vkCmdSetViewport>(dlsym(libvulkan, "vkCmdSetViewport"));
    vkCmdSetScissor = reinterpret_cast<PFN_vkCmdSetScissor>(dlsym(libvulkan, "vkCmdSetScissor"));
    vkCmdSetLineWidth = reinterpret_cast<PFN_vkCmdSetLineWidth>(dlsym(libvulkan, "vkCmdSetLineWidth"));
    vkCmdSetDepthBias = reinterpret_cast<PFN_vkCmdSetDepthBias>(dlsym(libvulkan, "vkCmdSetDepthBias"));
    vkCmdSetBlendConstants = reinterpret_cast<PFN_vkCmdSetBlendConstants>(dlsym(libvulkan, "vkCmdSetBlendConstants"));
    vkCmdSetDepthBounds = reinterpret_cast<PFN_vkCmdSetDepthBounds>(dlsym(libvulkan, "vkCmdSetDepthBounds"));
    vkCmdSetStencilCompareMask = reinterpret_cast<PFN_vkCmdSetStencilCompareMask>(dlsym(libvulkan, "vkCmdSetStencilCompareMask"));
    vkCmdSetStencilWriteMask = reinterpret_cast<PFN_vkCmdSetStencilWriteMask>(dlsym(libvulkan, "vkCmdSetStencilWriteMask"));
    vkCmdSetStencilReference = reinterpret_cast<PFN_vkCmdSetStencilReference>(dlsym(libvulkan, "vkCmdSetStencilReference"));
    vkCmdBindDescriptorSets = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(dlsym(libvulkan, "vkCmdBindDescriptorSets"));
    vkCmdBindIndexBuffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(dlsym(libvulkan, "vkCmdBindIndexBuffer"));
    vkCmdBindVertexBuffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(dlsym(libvulkan, "vkCmdBindVertexBuffers"));
    vkCmdDraw = reinterpret_cast<PFN_vkCmdDraw>(dlsym(libvulkan, "vkCmdDraw"));
    vkCmdDrawIndexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(dlsym(libvulkan, "vkCmdDrawIndexed"));
    vkCmdDrawIndirect = reinterpret_cast<PFN_vkCmdDrawIndirect>(dlsym(libvulkan, "vkCmdDrawIndirect"));
    vkCmdDrawIndexedIndirect = reinterpret_cast<PFN_vkCmdDrawIndexedIndirect>(dlsym(libvulkan, "vkCmdDrawIndexedIndirect"));
    vkCmdDispatch = reinterpret_cast<PFN_vkCmdDispatch>(dlsym(libvulkan, "vkCmdDispatch"));
    vkCmdDispatchIndirect = reinterpret_cast<PFN_vkCmdDispatchIndirect>(dlsym(libvulkan, "vkCmdDispatchIndirect"));
    vkCmdCopyBuffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(dlsym(libvulkan, "vkCmdCopyBuffer"));
    vkCmdCopyImage = reinterpret_cast<PFN_vkCmdCopyImage>(dlsym(libvulkan, "vkCmdCopyImage"));
    vkCmdBlitImage = reinterpret_cast<PFN_vkCmdBlitImage>(dlsym(libvulkan, "vkCmdBlitImage"));
    vkCmdCopyBufferToImage = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(dlsym(libvulkan, "vkCmdCopyBufferToImage"));
    vkCmdCopyImageToBuffer = reinterpret_cast<PFN_vkCmdCopyImageToBuffer>(dlsym(libvulkan, "vkCmdCopyImageToBuffer"));
    vkCmdUpdateBuffer = reinterpret_cast<PFN_vkCmdUpdateBuffer>(dlsym(libvulkan, "vkCmdUpdateBuffer"));
    vkCmdFillBuffer = reinterpret_cast<PFN_vkCmdFillBuffer>(dlsym(libvulkan, "vkCmdFillBuffer"));
    vkCmdClearColorImage = reinterpret_cast<PFN_vkCmdClearColorImage>(dlsym(libvulkan, "vkCmdClearColorImage"));
    vkCmdClearDepthStencilImage = reinterpret_cast<PFN_vkCmdClearDepthStencilImage>(dlsym(libvulkan, "vkCmdClearDepthStencilImage"));
    vkCmdClearAttachments = reinterpret_cast<PFN_vkCmdClearAttachments>(dlsym(libvulkan, "vkCmdClearAttachments"));
    vkCmdResolveImage = reinterpret_cast<PFN_vkCmdResolveImage>(dlsym(libvulkan, "vkCmdResolveImage"));
    vkCmdSetEvent = reinterpret_cast<PFN_vkCmdSetEvent>(dlsym(libvulkan, "vkCmdSetEvent"));
    vkCmdResetEvent = reinterpret_cast<PFN_vkCmdResetEvent>(dlsym(libvulkan, "vkCmdResetEvent"));
    vkCmdWaitEvents = reinterpret_cast<PFN_vkCmdWaitEvents>(dlsym(libvulkan, "vkCmdWaitEvents"));
    vkCmdPipelineBarrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(dlsym(libvulkan, "vkCmdPipelineBarrier"));
    vkCmdBeginQuery = reinterpret_cast<PFN_vkCmdBeginQuery>(dlsym(libvulkan, "vkCmdBeginQuery"));
    vkCmdEndQuery = reinterpret_cast<PFN_vkCmdEndQuery>(dlsym(libvulkan, "vkCmdEndQuery"));
    vkCmdResetQueryPool = reinterpret_cast<PFN_vkCmdResetQueryPool>(dlsym(libvulkan, "vkCmdResetQueryPool"));
    vkCmdWriteTimestamp = reinterpret_cast<PFN_vkCmdWriteTimestamp>(dlsym(libvulkan, "vkCmdWriteTimestamp"));
    vkCmdCopyQueryPoolResults = reinterpret_cast<PFN_vkCmdCopyQueryPoolResults>(dlsym(libvulkan, "vkCmdCopyQueryPoolResults"));
    vkCmdPushConstants = reinterpret_cast<PFN_vkCmdPushConstants>(dlsym(libvulkan, "vkCmdPushConstants"));
    vkCmdBeginRenderPass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(dlsym(libvulkan, "vkCmdBeginRenderPass"));
    vkCmdNextSubpass = reinterpret_cast<PFN_vkCmdNextSubpass>(dlsym(libvulkan, "vkCmdNextSubpass"));
    vkCmdEndRenderPass = reinterpret_cast<PFN_vkCmdEndRenderPass>(dlsym(libvulkan, "vkCmdEndRenderPass"));
    vkCmdExecuteCommands = reinterpret_cast<PFN_vkCmdExecuteCommands>(dlsym(libvulkan, "vkCmdExecuteCommands"));
    vkDestroySurfaceKHR = reinterpret_cast<PFN_vkDestroySurfaceKHR>(dlsym(libvulkan, "vkDestroySurfaceKHR"));
    vkGetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceSurfaceSupportKHR"));
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
    vkGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
    vkGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
    vkCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(dlsym(libvulkan, "vkCreateSwapchainKHR"));
    vkDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(dlsym(libvulkan, "vkDestroySwapchainKHR"));
    vkGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(dlsym(libvulkan, "vkGetSwapchainImagesKHR"));
    vkAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(dlsym(libvulkan, "vkAcquireNextImageKHR"));
    vkQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(dlsym(libvulkan, "vkQueuePresentKHR"));
    vkGetPhysicalDeviceDisplayPropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPropertiesKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceDisplayPropertiesKHR"));
    vkGetPhysicalDeviceDisplayPlanePropertiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR"));
    vkGetDisplayPlaneSupportedDisplaysKHR = reinterpret_cast<PFN_vkGetDisplayPlaneSupportedDisplaysKHR>(dlsym(libvulkan, "vkGetDisplayPlaneSupportedDisplaysKHR"));
    vkGetDisplayModePropertiesKHR = reinterpret_cast<PFN_vkGetDisplayModePropertiesKHR>(dlsym(libvulkan, "vkGetDisplayModePropertiesKHR"));
    vkCreateDisplayModeKHR = reinterpret_cast<PFN_vkCreateDisplayModeKHR>(dlsym(libvulkan, "vkCreateDisplayModeKHR"));
    vkGetDisplayPlaneCapabilitiesKHR = reinterpret_cast<PFN_vkGetDisplayPlaneCapabilitiesKHR>(dlsym(libvulkan, "vkGetDisplayPlaneCapabilitiesKHR"));
    vkCreateDisplayPlaneSurfaceKHR = reinterpret_cast<PFN_vkCreateDisplayPlaneSurfaceKHR>(dlsym(libvulkan, "vkCreateDisplayPlaneSurfaceKHR"));
    vkCreateSharedSwapchainsKHR = reinterpret_cast<PFN_vkCreateSharedSwapchainsKHR>(dlsym(libvulkan, "vkCreateSharedSwapchainsKHR"));
    vkCreateAndroidSurfaceKHR = reinterpret_cast<PFN_vkCreateAndroidSurfaceKHR>(dlsym(libvulkan, "vkCreateAndroidSurfaceKHR"));
#ifdef USE_DEBUG_EXTENTIONS
    vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(dlsym(libvulkan, "vkCreateDebugReportCallbackEXT"));
    vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(dlsym(libvulkan, "vkDestroyDebugReportCallbackEXT"));
    vkDebugReportMessageEXT = reinterpret_cast<PFN_vkDebugReportMessageEXT>(dlsym(libvulkan, "vkDebugReportMessageEXT"));
#endif
    return true;
}
//}

// Global Variables ...
bool initialized_;
struct VulkanDeviceInfo {
  VkInstance instance_;
  VkPhysicalDevice gpuDevice_;
  VkDevice device_;
  uint32_t queueFamilyIndex_;
  VkSurfaceKHR surface_;
  VkQueue queue_;
};
struct VulkanSwapchainInfo {
  VkSwapchainKHR swapchain_;
  uint32_t swapchainLength_;
  VkExtent2D displaySize_;
  VkFormat displayFormat_;
  // array of frame buffers and views
  std::vector<VkImage> displayImages_;
  std::vector<VkImageView> displayViews_;
  std::vector<VkFramebuffer> framebuffers_;
};
struct VulkanBufferInfo {
  VkBuffer vertexBuf_;
};
struct VulkanGfxPipelineInfo {
  VkPipelineLayout layout_;
  VkPipelineCache cache_;
  VkPipeline pipeline_;
};
struct VulkanRenderInfo {
  VkRenderPass renderPass_;
  VkCommandPool cmdPool_;
  VkCommandBuffer* cmdBuffer_;
  uint32_t cmdBufferLen_;
  VkSemaphore semaphore_;
  VkFence fence_;
};

VulkanDeviceInfo device;
VulkanSwapchainInfo swapchain;
VulkanBufferInfo buffers;
VulkanGfxPipelineInfo gfxPipeline;
VulkanRenderInfo render_info;

VkResult result;

void vulkan_graphics::onResume() {
  // To do
}
void vulkan_graphics::onWindowInit(ANativeWindow *window) {
  // Create vulkan device
  {
    std::vector<const char*> instance_extensions;
    std::vector<const char*> device_extensions;
  
    instance_extensions.push_back("VK_KHR_surface");
    instance_extensions.push_back("VK_KHR_android_surface");
    device_extensions.push_back("VK_KHR_swapchain");
    
    //create Vulkan Application info just for vulkan create vulkan instance
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "tutorial05_triangle_window",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "tutorial",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_MAKE_VERSION(1, 1, 0),
    };
    //create instance info
    VkInstanceCreateInfo instanceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size()),
        .ppEnabledExtensionNames = instance_extensions.data(),
    };
    result = vkCreateInstance(&instanceCreateInfo, nullptr, &device.instance_);
    assert(result == VK_SUCCESS);
    VkAndroidSurfaceCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .window = window
    };
    result = vkCreateAndroidSurfaceKHR(device.instance_, &createInfo, nullptr, &device.surface_);
    assert(result == VK_SUCCESS);
    uint32_t gpuCount = 0;
    result = vkEnumeratePhysicalDevices(device.instance_, &gpuCount, nullptr);
    assert(result == VK_SUCCESS);
    VkPhysicalDevice tmpGpus[gpuCount];
    result = vkEnumeratePhysicalDevices(device.instance_, &gpuCount, tmpGpus);
    assert(result == VK_SUCCESS);
    device.gpuDevice_ = tmpGpus[0];  // Pick up the first GPU Device
  
    // Find a GFX queue family
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device.gpuDevice_, &queueFamilyCount, nullptr);
    assert(queueFamilyCount);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device.gpuDevice_, &queueFamilyCount, queueFamilyProperties.data());
  
    uint32_t queueFamilyIndex;
    for (queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; queueFamilyIndex++) {
      if (queueFamilyProperties[queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        break;
      }
    }
    assert(queueFamilyIndex < queueFamilyCount);
    device.queueFamilyIndex_ = queueFamilyIndex;
  
    // Create a logical device (vulkan device)
    float queuePriorities[1]{1.0f};
    VkDeviceQueueCreateInfo queueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = queueFamilyIndex,
        .queueCount = 1,
        .pQueuePriorities = queuePriorities, 
    };
    VkDeviceCreateInfo deviceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
        .ppEnabledExtensionNames = device_extensions.data(),
        .pEnabledFeatures = nullptr,
    };
    result = vkCreateDevice(device.gpuDevice_, &deviceCreateInfo, nullptr, &device.device_);
    assert(result == VK_SUCCESS);
    vkGetDeviceQueue(device.device_, device.queueFamilyIndex_, 0, &device.queue_);
  }
  {
    //create swapchain
    memset(&swapchain, 0, sizeof(swapchain));
  
    // **********************************************************
    // Get the surface capabilities because:
    //   - It contains the minimal and max length of the chain, we will need it
    //   - It's necessary to query the supported surface format (R8G8B8A8 for
    //   instance ...)
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.gpuDevice_, device.surface_, &surfaceCapabilities);
    // Query the list of supported surface format and choose one we like
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.gpuDevice_, device.surface_, &formatCount, nullptr);
    VkSurfaceFormatKHR* formats = new VkSurfaceFormatKHR[formatCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.gpuDevice_, device.surface_, &formatCount, formats);
  
    uint32_t chosenFormat;
    for (chosenFormat = 0; chosenFormat < formatCount; chosenFormat++) {
      if (formats[chosenFormat].format == VK_FORMAT_R8G8B8A8_UNORM) break;
    }
    assert(chosenFormat < formatCount);
  
    swapchain.displaySize_ = surfaceCapabilities.currentExtent;
    swapchain.displayFormat_ = formats[chosenFormat].format;
    VkSurfaceCapabilitiesKHR surfaceCap;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.gpuDevice_, device.surface_, &surfaceCap);
    assert(result == VK_SUCCESS);
    assert(surfaceCap.supportedCompositeAlpha | VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR);
    VkSwapchainCreateInfoKHR swapchainCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .surface = device.surface_,
        .minImageCount = surfaceCapabilities.minImageCount,
        .imageFormat = formats[chosenFormat].format,
        .imageColorSpace = formats[chosenFormat].colorSpace,
        .imageExtent = surfaceCapabilities.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &device.queueFamilyIndex_,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_FALSE,
        .oldSwapchain = VK_NULL_HANDLE,
    };
    result = (vkCreateSwapchainKHR(device.device_, &swapchainCreateInfo, nullptr, &swapchain.swapchain_));
    assert(result == VK_SUCCESS);
    // Get the length of the created swap chain
    result = (vkGetSwapchainImagesKHR(device.device_, swapchain.swapchain_, &swapchain.swapchainLength_, nullptr));
    assert(result == VK_SUCCESS);
    delete[] formats;
  }
  // Create render_info pass
  VkAttachmentDescription attachmentDescriptions{
      .format = swapchain.displayFormat_,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };
  VkAttachmentReference colourReference = {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
  };

  VkSubpassDescription subpassDescription{
      .flags = 0,
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .inputAttachmentCount = 0,
      .pInputAttachments = nullptr,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colourReference,
      .pResolveAttachments = nullptr,
      .pDepthStencilAttachment = nullptr,
      .preserveAttachmentCount = 0,
      .pPreserveAttachments = nullptr,
  };
  VkRenderPassCreateInfo renderPassCreateInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .pNext = nullptr,
      .attachmentCount = 1,
      .pAttachments = &attachmentDescriptions,
      .subpassCount = 1,
      .pSubpasses = &subpassDescription,
      .dependencyCount = 0,
      .pDependencies = nullptr,
  };
  result = vkCreateRenderPass(device.device_, &renderPassCreateInfo, nullptr, &render_info.renderPass_);
  assert(result == VK_SUCCESS);
  // Create 2 frame buffers.
  {
    // query display attachment to swapchain
    uint32_t SwapchainImagesCount = 0;
    result = (vkGetSwapchainImagesKHR(device.device_, swapchain.swapchain_, &SwapchainImagesCount, nullptr));
    assert(result == VK_SUCCESS);
    swapchain.displayImages_.resize(SwapchainImagesCount);
    result = (vkGetSwapchainImagesKHR(device.device_, swapchain.swapchain_, &SwapchainImagesCount, swapchain.displayImages_.data()));
    assert(result == VK_SUCCESS);
    // create image view for each swapchain image
    swapchain.displayViews_.resize(SwapchainImagesCount);
    for (uint32_t i = 0; i < SwapchainImagesCount; i++) {
      VkImageViewCreateInfo viewCreateInfo = {
          .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
          .pNext = nullptr,
          .flags = 0,
          .image = swapchain.displayImages_[i],
          .viewType = VK_IMAGE_VIEW_TYPE_2D,
          .format = swapchain.displayFormat_,
          .components =
              {
                  .r = VK_COMPONENT_SWIZZLE_R,
                  .g = VK_COMPONENT_SWIZZLE_G,
                  .b = VK_COMPONENT_SWIZZLE_B,
                  .a = VK_COMPONENT_SWIZZLE_A,
              },
          .subresourceRange =
              {
                  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                  .baseMipLevel = 0,
                  .levelCount = 1,
                  .baseArrayLayer = 0,
                  .layerCount = 1,
              },
      };
      result = (vkCreateImageView(device.device_, &viewCreateInfo, nullptr, &swapchain.displayViews_[i]));
      assert(result == VK_SUCCESS);
    }
    // create a framebuffer from each swapchain image
    swapchain.framebuffers_.resize(swapchain.swapchainLength_);
    for (uint32_t i = 0; i < swapchain.swapchainLength_; i++) {
      VkImageView attachments[2] = {
          swapchain.displayViews_[i], VK_NULL_HANDLE,
      };
      VkFramebufferCreateInfo fbCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .renderPass = render_info.renderPass_,
        .attachmentCount = 1,  // 2 if using depth
        .pAttachments = attachments,
        .width = static_cast<uint32_t>(swapchain.displaySize_.width),
        .height = static_cast<uint32_t>(swapchain.displaySize_.height),
        .layers = 1,
      };
      result = (vkCreateFramebuffer(device.device_, &fbCreateInfo, nullptr, &swapchain.framebuffers_[i]));
      assert(result == VK_SUCCESS);
    }
  }
  {
    // Create the triangle vertex buffer
    // Vertex positions
    const float vertexData[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };
    // Create a vertex buffer
    VkBufferCreateInfo createBufferInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = sizeof(vertexData),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &device.queueFamilyIndex_,
    };
  
    result = (vkCreateBuffer(device.device_, &createBufferInfo, nullptr, &buffers.vertexBuf_));
    assert(result == VK_SUCCESS);
  
    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device.device_, buffers.vertexBuf_, &memReq);
  
    VkMemoryAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memReq.size,
        .memoryTypeIndex = 0,  // Memory type assigned in the next step
    };
  
    // Assign the proper memory type for that buffer
    {
      uint32_t typeBits = memReq.memoryTypeBits;
      VkFlags requirements_mask = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      VkPhysicalDeviceMemoryProperties memoryProperties;
      vkGetPhysicalDeviceMemoryProperties(device.gpuDevice_, &memoryProperties);
      // Search memtypes to find first index with those properties
      for (uint32_t i = 0; i < 32; i++) {
        if (typeBits & 1) {
          // Type is available, does it match user properties?
          if ((memoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
            allocInfo.memoryTypeIndex = i;
            break;
          }
        }
        typeBits >>= 1;
      }
    }

    // Allocate memory for the buffer
    VkDeviceMemory deviceMemory;
    result = (vkAllocateMemory(device.device_, &allocInfo, nullptr, &deviceMemory));
    assert(result == VK_SUCCESS);
  
    void* data;
    result = (vkMapMemory(device.device_, deviceMemory, 0, allocInfo.allocationSize,0, &data));
    assert(result == VK_SUCCESS);
    memcpy(data, vertexData, sizeof(vertexData));
    vkUnmapMemory(device.device_, deviceMemory);
  
    result = (vkBindBufferMemory(device.device_, buffers.vertexBuf_, deviceMemory, 0));
    assert(result == VK_SUCCESS);
  }
  // Create graphics pipeline
  {
    memset(&gfxPipeline, 0, sizeof(gfxPipeline));
    // Create pipeline layout (empty)
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };
    result = (vkCreatePipelineLayout(device.device_, &pipelineLayoutCreateInfo, nullptr, &gfxPipeline.layout_));
    const char *vertexSrc = "#version 400\n"
      "#extension GL_ARB_separate_shader_objects : enable\n"
      "#extension GL_ARB_shading_language_420pack : enable\n"
      "layout (location = 0) in vec4 pos;\n"
      "void main() {\n"
      "   gl_Position = pos;\n"
      "}";
    VkShaderModuleCreateInfo shaderModuleCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .codeSize = strlen(vertexSrc),
      .pCode = (const uint32_t*)vertexSrc,
    };
    VkShaderModule vertexShader;
    result = vkCreateShaderModule(device.device_, &shaderModuleCreateInfo, nullptr, &vertexShader);
    assert(result == VK_SUCCESS);
    const char *fragmentSrc = "#version 400\n"
      "#extension GL_ARB_separate_shader_objects : enable\n"
      "#extension GL_ARB_shading_language_420pack : enable\n"
      "layout (location = 0) out vec4 uFragColor;\n"
      "void main() {\n"
      "   uFragColor = vec4(0.91, 0.26,  0.21, 1.0);\n"
      "}";
    shaderModuleCreateInfo.codeSize = strlen(fragmentSrc);
    shaderModuleCreateInfo.pCode =  (const uint32_t*)fragmentSrc;
    VkShaderModule fragmentShader;
    result = vkCreateShaderModule(device.device_, &shaderModuleCreateInfo, nullptr, &fragmentShader);
    assert(result == VK_SUCCESS);
    // Specify vertex and fragment shader stages
    VkPipelineShaderStageCreateInfo shaderStages[2]{
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShader,
            .pName = "main",
            .pSpecializationInfo = nullptr,
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShader,
            .pName = "main",
            .pSpecializationInfo = nullptr,
        }
    };
  
    VkViewport viewports{
        .x = 0,
        .y = 0,
        .width = (float)swapchain.displaySize_.width,
        .height = (float)swapchain.displaySize_.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
  
    VkRect2D scissor = {
        .offset { .x = 0, .y = 0,},
        .extent = swapchain.displaySize_,
    };
    // Specify viewport info
    VkPipelineViewportStateCreateInfo viewportInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .viewportCount = 1,
        .pViewports = &viewports,
        .scissorCount = 1,
        .pScissors = &scissor,
    };
  
    // Specify multisample info
    VkSampleMask sampleMask = ~0u;
    VkPipelineMultisampleStateCreateInfo multisampleInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 0,
        .pSampleMask = &sampleMask,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };
  
    // Specify color blend state
    VkPipelineColorBlendAttachmentState attachmentStates{
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    VkPipelineColorBlendStateCreateInfo colorBlendInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &attachmentStates,
    };
  
    // Specify rasterizer info
    VkPipelineRasterizationStateCreateInfo rasterInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1,
    };
  
    // Specify input assembler state
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };
  
    // Specify vertex input state
    VkVertexInputBindingDescription vertex_input_bindings{
        .binding = 0,
        .stride = 3 * sizeof(float),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
    VkVertexInputAttributeDescription vertex_input_attributes[1]{{
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = 0,
    }};
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertex_input_bindings,
        .vertexAttributeDescriptionCount = 1,
        .pVertexAttributeDescriptions = vertex_input_attributes,
    };
  
    // Create the pipeline cache
    VkPipelineCacheCreateInfo pipelineCacheInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,  // reserved, must be 0
        .initialDataSize = 0,
        .pInitialData = nullptr,
    };
  
    result = (vkCreatePipelineCache(device.device_, &pipelineCacheInfo, nullptr, &gfxPipeline.cache_));
    assert(result == VK_SUCCESS);
    // Create the pipeline
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssemblyInfo,
        .pTessellationState = nullptr,
        .pViewportState = &viewportInfo,
        .pRasterizationState = &rasterInfo,
        .pMultisampleState = &multisampleInfo,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &colorBlendInfo,
        .pDynamicState = nullptr,
        .layout = gfxPipeline.layout_,
        .renderPass = render_info.renderPass_,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
    };
  
    result = vkCreateGraphicsPipelines(device.device_, gfxPipeline.cache_, 1, &pipelineCreateInfo, nullptr, &gfxPipeline.pipeline_);
    assert(result == VK_SUCCESS);
    // We don't need the shaders anymore, we can release their memory
    vkDestroyShaderModule(device.device_, vertexShader, nullptr);
    vkDestroyShaderModule(device.device_, fragmentShader, nullptr);
  
    //return pipelineResult;
  }
  // -----------------------------------------------
  // Create a pool of command buffers to allocate command buffer from
  VkCommandPoolCreateInfo cmdPoolCreateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = device.queueFamilyIndex_,
  };
  result = vkCreateCommandPool(device.device_, &cmdPoolCreateInfo, nullptr, &render_info.cmdPool_);
  assert(result == VK_SUCCESS);
  // Record a command buffer that just clear the screen
  // 1 command buffer draw in 1 framebuffer
  // In our case we need 2 command as we have 2 framebuffer
  render_info.cmdBufferLen_ = swapchain.swapchainLength_;
  render_info.cmdBuffer_ = new VkCommandBuffer[swapchain.swapchainLength_];
  VkCommandBufferAllocateInfo cmdBufferCreateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = render_info.cmdPool_,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = render_info.cmdBufferLen_,
  };
  result = vkAllocateCommandBuffers(device.device_, &cmdBufferCreateInfo, render_info.cmdBuffer_);
  assert(result == VK_SUCCESS);

  for (uint32_t bufferIndex = 0; bufferIndex < swapchain.swapchainLength_; bufferIndex++) {
    // We start by creating and declare the "beginning" our command buffer
    VkCommandBufferBeginInfo cmdBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr,
    };
    result = vkBeginCommandBuffer(render_info.cmdBuffer_[bufferIndex], &cmdBufferBeginInfo);
    assert(result == VK_SUCCESS);
    {
      VkImageMemoryBarrier imageMemoryBarrier = {
          .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
          .pNext = NULL,
          .srcAccessMask = 0,
          .dstAccessMask = 0,
          .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
          .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
          .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
          .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
          .image = swapchain.displayImages_[bufferIndex],
          .subresourceRange = {
                  .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                  .baseMipLevel = 0,
                  .levelCount = 1,
                  .baseArrayLayer = 0,
                  .layerCount = 1,
              },
      };
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      vkCmdPipelineBarrier(render_info.cmdBuffer_[bufferIndex], VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
    }
    // Now we start a renderpass. Any draw command has to be recorded in a
    // renderpass
    VkClearValue clearVals{ .color { .float32 {0.0f, 0.34f, 0.90f, 1.0f}}};
    VkRenderPassBeginInfo renderPassBeginInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = render_info.renderPass_,
        .framebuffer = swapchain.framebuffers_[bufferIndex],
        .renderArea = {.offset { .x = 0, .y = 0,}, .extent = swapchain.displaySize_},
        .clearValueCount = 1,
        .pClearValues = &clearVals};
    vkCmdBeginRenderPass(render_info.cmdBuffer_[bufferIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    // Bind what is necessary to the command buffer
    vkCmdBindPipeline(render_info.cmdBuffer_[bufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gfxPipeline.pipeline_);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(render_info.cmdBuffer_[bufferIndex], 0, 1,
                           &buffers.vertexBuf_, &offset);

    // Draw Triangle
    vkCmdDraw(render_info.cmdBuffer_[bufferIndex], 3, 1, 0, 0);

    vkCmdEndRenderPass(render_info.cmdBuffer_[bufferIndex]);

    result = (vkEndCommandBuffer(render_info.cmdBuffer_[bufferIndex]));
    assert(result == VK_SUCCESS);
  }

  // We need to create a fence to be able, in the main loop, to wait for our
  // draw command(s) to finish before swapping the framebuffers
  VkFenceCreateInfo fenceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
  };
  result = (vkCreateFence(device.device_, &fenceCreateInfo, nullptr, &render_info.fence_));
  assert(result == VK_SUCCESS);

  // We need to create a semaphore to be able to wait, in the main loop, for our
  // framebuffer to be available for us before drawing.
  VkSemaphoreCreateInfo semaphoreCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
  };
  result = (vkCreateSemaphore(device.device_, &semaphoreCreateInfo, nullptr,&render_info.semaphore_));
  assert(result == VK_SUCCESS);

  initialized_ = true;
}
void vulkan_graphics::needResize() {
  // To do
}
void vulkan_graphics::render() {
  if (!initialized_) return;
  uint32_t nextIndex;
  // Get the framebuffer index we should draw in
  result = (vkAcquireNextImageKHR(device.device_, swapchain.swapchain_, UINT64_MAX, render_info.semaphore_, VK_NULL_HANDLE, &nextIndex));
  assert(result == VK_SUCCESS);
  result = (vkResetFences(device.device_, 1, &render_info.fence_));
  assert(result == VK_SUCCESS);
  VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  VkSubmitInfo submit_info {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext = nullptr,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &render_info.semaphore_,
    .pWaitDstStageMask = &waitStageMask,
    .commandBufferCount = 1,
    .pCommandBuffers = &render_info.cmdBuffer_[nextIndex],
    .signalSemaphoreCount = 0,
    .pSignalSemaphores = nullptr
  };
  result = (vkQueueSubmit(device.queue_, 1, &submit_info, render_info.fence_));
  assert(result == VK_SUCCESS);
  result = (vkWaitForFences(device.device_, 1, &render_info.fence_, VK_TRUE, 100000000));
  assert(result == VK_SUCCESS);

  VkPresentInfoKHR presentInfo{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .pNext = nullptr,
      .waitSemaphoreCount = 0,
      .pWaitSemaphores = nullptr,
      .swapchainCount = 1,
      .pSwapchains = &swapchain.swapchain_,
      .pImageIndices = &nextIndex,
      .pResults = &result,
  };
  vkQueuePresentKHR(device.queue_, &presentInfo);
}
void vulkan_graphics::onWindowTerm() {
  vkFreeCommandBuffers(device.device_, render_info.cmdPool_, render_info.cmdBufferLen_,render_info.cmdBuffer_);
  delete[] render_info.cmdBuffer_;

  vkDestroyCommandPool(device.device_, render_info.cmdPool_, nullptr);
  vkDestroyRenderPass(device.device_, render_info.renderPass_, nullptr);
  
  {
    //delete swapchain
    for (size_t i = 0; i < swapchain.swapchainLength_; i++) {
      vkDestroyFramebuffer(device.device_, swapchain.framebuffers_[i], nullptr);
      vkDestroyImageView(device.device_, swapchain.displayViews_[i], nullptr);
    }
    vkDestroySwapchainKHR(device.device_, swapchain.swapchain_, nullptr);
  }
  
  {
    //delete graphics pipeline
    if (gfxPipeline.pipeline_ == VK_NULL_HANDLE) return;
    vkDestroyPipeline(device.device_, gfxPipeline.pipeline_, nullptr);
    vkDestroyPipelineCache(device.device_, gfxPipeline.cache_, nullptr);
    vkDestroyPipelineLayout(device.device_, gfxPipeline.layout_, nullptr);
  }

  //delete buffers
  vkDestroyBuffer(device.device_, buffers.vertexBuf_, nullptr);

  vkDestroyDevice(device.device_, nullptr);
  vkDestroyInstance(device.instance_, nullptr);

  initialized_ = false;
}

void vulkan_graphics::onPause() {
  // To do
}
void vulkan_graphics::onDestroy() {
  // To do
}
float vulkan_graphics::getWidth() { return float(swapchain.displaySize_.width); }
float vulkan_graphics::getHeight() { return float(swapchain.displaySize_.height); }
void vulkan_graphics::clear(const unsigned int &) {
  // To do
}
void vulkan_graphics::clearcolor(const float &, const float &, const float &, const float &) {
  // To do
}
engine::texture_core *vulkan_graphics::gen_texture(const int &, const int &, unsigned char *) {
  // To do
  return nullptr;
}
void vulkan_graphics::bind_texture(engine::texture_core *) {
  // To do
}
void vulkan_graphics::set_texture_param(const int &, const int &) {
  // To do
}
void vulkan_graphics::delete_texture(engine::texture_core *) {
  // To do
}
void vulkan_graphics::flat_render(engine::texture_core*, engine::flat_vertex *, unsigned int) {
  // To do
}
engine::mesh_core *vulkan_graphics::gen_mesh(engine::mesh_core::data *,unsigned int,unsigned short *, unsigned int){
  // To do
  return nullptr;
}
void vulkan_graphics::mesh_render(engine::mesh_core **,const unsigned int &) {
  // To do
}
void vulkan_graphics::delete_mesh(engine::mesh_core *) {
	// To DO: 
}

vulkan_graphics::vulkan_graphics() {
  assert(Initialize_Vulkan());
  initialized_ = false;
  engine::graph = this;
}

vulkan_graphics::~vulkan_graphics() {
  engine::graph = nullptr;
}


