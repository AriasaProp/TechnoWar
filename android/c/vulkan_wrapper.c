#define VK_NO_PROTOTYPES 1
#include "log.h"
#include "manager.h"
#include "common.h"

#include <dlfcn.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#if defined(__ARM_ARCH) && __ARM_ARCH < 7
#error "Vulkan is not supported for the 'armeabi' NDK ABI"
#elif defined(__ARM_ARCH) && __ARM_ARCH >= 7 && defined(__ARM_32BIT_STATE)
// On Android 32-bit ARM targets, Vulkan functions use the "hardfloat"
// calling convention, i.e. float parameters are passed in registers. This
// is true even if the rest of the application passes floats on the stack,
// as it does by default when compiling for the armeabi-v7a NDK ABI.
#define VKAPI_ATTR __attribute__((pcs("aapcs-vfp")))
#define VKAPI_CALL
#define VKAPI_PTR VKAPI_ATTR
#else
// On other platforms, use the default calling convention
#define VKAPI_ATTR
#define VKAPI_CALL
#define VKAPI_PTR
#endif

// TODO: implement vulkan_core here directly
#include "vulkan_core.h"

// VK_KHR_android_surface is a preprocessor guard. Do not pass it to API calls.
#define VK_KHR_android_surface 1
struct ANativeWindow;
#define VK_KHR_ANDROID_SURFACE_SPEC_VERSION   6
#define VK_KHR_ANDROID_SURFACE_EXTENSION_NAME "VK_KHR_android_surface"
typedef VkFlags VkAndroidSurfaceCreateFlagsKHR;
typedef struct VkAndroidSurfaceCreateInfoKHR {
  VkStructureType sType;
  const void *pNext;
  VkAndroidSurfaceCreateFlagsKHR flags;
  struct ANativeWindow *window;
} VkAndroidSurfaceCreateInfoKHR;

typedef VkResult(VKAPI_PTR *PFN_vkCreateAndroidSurfaceKHR)(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface);

// VK_ANDROID_external_memory_android_hardware_buffer is a preprocessor guard. Do not pass it to API calls.
#define VK_ANDROID_external_memory_android_hardware_buffer 1
struct AHardwareBuffer;
#define VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_SPEC_VERSION   5
#define VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME "VK_ANDROID_external_memory_android_hardware_buffer"
typedef struct VkAndroidHardwareBufferUsageANDROID {
  VkStructureType sType;
  void *pNext;
  uint64_t androidHardwareBufferUsage;
} VkAndroidHardwareBufferUsageANDROID;

typedef struct VkAndroidHardwareBufferPropertiesANDROID {
  VkStructureType sType;
  void *pNext;
  VkDeviceSize allocationSize;
  uint32_t memoryTypeBits;
} VkAndroidHardwareBufferPropertiesANDROID;

typedef struct VkAndroidHardwareBufferFormatPropertiesANDROID {
  VkStructureType sType;
  void *pNext;
  VkFormat format;
  uint64_t externalFormat;
  VkFormatFeatureFlags formatFeatures;
  VkComponentMapping samplerYcbcrConversionComponents;
  VkSamplerYcbcrModelConversion suggestedYcbcrModel;
  VkSamplerYcbcrRange suggestedYcbcrRange;
  VkChromaLocation suggestedXChromaOffset;
  VkChromaLocation suggestedYChromaOffset;
} VkAndroidHardwareBufferFormatPropertiesANDROID;

typedef struct VkImportAndroidHardwareBufferInfoANDROID {
  VkStructureType sType;
  const void *pNext;
  struct AHardwareBuffer *buffer;
} VkImportAndroidHardwareBufferInfoANDROID;

typedef struct VkMemoryGetAndroidHardwareBufferInfoANDROID {
  VkStructureType sType;
  const void *pNext;
  VkDeviceMemory memory;
} VkMemoryGetAndroidHardwareBufferInfoANDROID;

typedef struct VkExternalFormatANDROID {
  VkStructureType sType;
  void *pNext;
  uint64_t externalFormat;
} VkExternalFormatANDROID;

typedef struct VkAndroidHardwareBufferFormatProperties2ANDROID {
  VkStructureType sType;
  void *pNext;
  VkFormat format;
  uint64_t externalFormat;
  VkFormatFeatureFlags2 formatFeatures;
  VkComponentMapping samplerYcbcrConversionComponents;
  VkSamplerYcbcrModelConversion suggestedYcbcrModel;
  VkSamplerYcbcrRange suggestedYcbcrRange;
  VkChromaLocation suggestedXChromaOffset;
  VkChromaLocation suggestedYChromaOffset;
} VkAndroidHardwareBufferFormatProperties2ANDROID;

typedef VkResult(VKAPI_PTR *PFN_vkGetAndroidHardwareBufferPropertiesANDROID)(VkDevice device, const struct AHardwareBuffer *buffer, VkAndroidHardwareBufferPropertiesANDROID *pProperties);
typedef VkResult(VKAPI_PTR *PFN_vkGetMemoryAndroidHardwareBufferANDROID)(VkDevice device, const VkMemoryGetAndroidHardwareBufferInfoANDROID *pInfo, struct AHardwareBuffer **pBuffer);

// VK_ANDROID_external_format_resolve is a preprocessor guard. Do not pass it to API calls.
#define VK_ANDROID_external_format_resolve                1
#define VK_ANDROID_EXTERNAL_FORMAT_RESOLVE_SPEC_VERSION   1
#define VK_ANDROID_EXTERNAL_FORMAT_RESOLVE_EXTENSION_NAME "VK_ANDROID_external_format_resolve"
typedef struct VkPhysicalDeviceExternalFormatResolveFeaturesANDROID {
  VkStructureType sType;
  void *pNext;
  VkBool32 externalFormatResolve;
} VkPhysicalDeviceExternalFormatResolveFeaturesANDROID;

typedef struct VkPhysicalDeviceExternalFormatResolvePropertiesANDROID {
  VkStructureType sType;
  void *pNext;
  VkBool32 nullColorAttachmentWithExternalFormatResolve;
  VkChromaLocation externalFormatResolveChromaOffsetX;
  VkChromaLocation externalFormatResolveChromaOffsetY;
} VkPhysicalDeviceExternalFormatResolvePropertiesANDROID;

typedef struct VkAndroidHardwareBufferFormatResolvePropertiesANDROID {
  VkStructureType sType;
  void *pNext;
  VkFormat colorAttachmentFormat;
} VkAndroidHardwareBufferFormatResolvePropertiesANDROID;

// Vulkan function pointers
static PFN_vkCreateInstance vkCreateInstance;
static PFN_vkDestroyInstance vkDestroyInstance;
static PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
static PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
static PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
static PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
static PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
static PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
static PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
static PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
static PFN_vkCreateDevice vkCreateDevice;
static PFN_vkDestroyDevice vkDestroyDevice;
static PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
static PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
static PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
static PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties;
static PFN_vkGetDeviceQueue vkGetDeviceQueue;
static PFN_vkQueueSubmit vkQueueSubmit;
static PFN_vkQueueWaitIdle vkQueueWaitIdle;
static PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
static PFN_vkAllocateMemory vkAllocateMemory;
static PFN_vkFreeMemory vkFreeMemory;
static PFN_vkMapMemory vkMapMemory;
static PFN_vkUnmapMemory vkUnmapMemory;
static PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
static PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
static PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment;
static PFN_vkBindBufferMemory vkBindBufferMemory;
static PFN_vkBindImageMemory vkBindImageMemory;
static PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
static PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
static PFN_vkGetImageSparseMemoryRequirements vkGetImageSparseMemoryRequirements;
static PFN_vkGetPhysicalDeviceSparseImageFormatProperties vkGetPhysicalDeviceSparseImageFormatProperties;
static PFN_vkQueueBindSparse vkQueueBindSparse;
static PFN_vkCreateFence vkCreateFence;
static PFN_vkDestroyFence vkDestroyFence;
static PFN_vkResetFences vkResetFences;
static PFN_vkGetFenceStatus vkGetFenceStatus;
static PFN_vkWaitForFences vkWaitForFences;
static PFN_vkCreateSemaphore vkCreateSemaphore;
static PFN_vkDestroySemaphore vkDestroySemaphore;
static PFN_vkCreateEvent vkCreateEvent;
static PFN_vkDestroyEvent vkDestroyEvent;
static PFN_vkGetEventStatus vkGetEventStatus;
static PFN_vkSetEvent vkSetEvent;
static PFN_vkResetEvent vkResetEvent;
static PFN_vkCreateQueryPool vkCreateQueryPool;
static PFN_vkDestroyQueryPool vkDestroyQueryPool;
static PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
static PFN_vkCreateBuffer vkCreateBuffer;
static PFN_vkDestroyBuffer vkDestroyBuffer;
static PFN_vkCreateBufferView vkCreateBufferView;
static PFN_vkDestroyBufferView vkDestroyBufferView;
static PFN_vkCreateImage vkCreateImage;
static PFN_vkDestroyImage vkDestroyImage;
static PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
static PFN_vkCreateImageView vkCreateImageView;
static PFN_vkDestroyImageView vkDestroyImageView;
static PFN_vkCreateShaderModule vkCreateShaderModule;
static PFN_vkDestroyShaderModule vkDestroyShaderModule;
static PFN_vkCreatePipelineCache vkCreatePipelineCache;
static PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
static PFN_vkGetPipelineCacheData vkGetPipelineCacheData;
static PFN_vkMergePipelineCaches vkMergePipelineCaches;
static PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
static PFN_vkCreateComputePipelines vkCreateComputePipelines;
static PFN_vkDestroyPipeline vkDestroyPipeline;
static PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
static PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
static PFN_vkCreateSampler vkCreateSampler;
static PFN_vkDestroySampler vkDestroySampler;
static PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
static PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
static PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
static PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
static PFN_vkResetDescriptorPool vkResetDescriptorPool;
static PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
static PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
static PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
static PFN_vkCreateFramebuffer vkCreateFramebuffer;
static PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
static PFN_vkCreateRenderPass vkCreateRenderPass;
static PFN_vkDestroyRenderPass vkDestroyRenderPass;
static PFN_vkGetRenderAreaGranularity vkGetRenderAreaGranularity;
static PFN_vkCreateCommandPool vkCreateCommandPool;
static PFN_vkDestroyCommandPool vkDestroyCommandPool;
static PFN_vkResetCommandPool vkResetCommandPool;
static PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
static PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
static PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
static PFN_vkEndCommandBuffer vkEndCommandBuffer;
static PFN_vkResetCommandBuffer vkResetCommandBuffer;
static PFN_vkCmdBindPipeline vkCmdBindPipeline;
static PFN_vkCmdSetViewport vkCmdSetViewport;
static PFN_vkCmdSetScissor vkCmdSetScissor;
static PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
static PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
static PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants;
static PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds;
static PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask;
static PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask;
static PFN_vkCmdSetStencilReference vkCmdSetStencilReference;
static PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
static PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
static PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
static PFN_vkCmdDraw vkCmdDraw;
static PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
static PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
static PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
static PFN_vkCmdDispatch vkCmdDispatch;
static PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect;
static PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
static PFN_vkCmdCopyImage vkCmdCopyImage;
static PFN_vkCmdBlitImage vkCmdBlitImage;
static PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
static PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;
static PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer;
static PFN_vkCmdFillBuffer vkCmdFillBuffer;
static PFN_vkCmdClearColorImage vkCmdClearColorImage;
static PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage;
static PFN_vkCmdClearAttachments vkCmdClearAttachments;
static PFN_vkCmdResolveImage vkCmdResolveImage;
static PFN_vkCmdSetEvent vkCmdSetEvent;
static PFN_vkCmdResetEvent vkCmdResetEvent;
static PFN_vkCmdWaitEvents vkCmdWaitEvents;
static PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
static PFN_vkCmdBeginQuery vkCmdBeginQuery;
static PFN_vkCmdEndQuery vkCmdEndQuery;
static PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
static PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;
static PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;
static PFN_vkCmdPushConstants vkCmdPushConstants;
static PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
static PFN_vkCmdNextSubpass vkCmdNextSubpass;
static PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
static PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
static PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
static PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
static PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
static PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
static PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
static PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
static PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
static PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
static PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
static PFN_vkQueuePresentKHR vkQueuePresentKHR;
static PFN_vkGetPhysicalDeviceDisplayPropertiesKHR vkGetPhysicalDeviceDisplayPropertiesKHR;
static PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR vkGetPhysicalDeviceDisplayPlanePropertiesKHR;
static PFN_vkGetDisplayPlaneSupportedDisplaysKHR vkGetDisplayPlaneSupportedDisplaysKHR;
static PFN_vkGetDisplayModePropertiesKHR vkGetDisplayModePropertiesKHR;
static PFN_vkCreateDisplayModeKHR vkCreateDisplayModeKHR;
static PFN_vkGetDisplayPlaneCapabilitiesKHR vkGetDisplayPlaneCapabilitiesKHR;
static PFN_vkCreateDisplayPlaneSurfaceKHR vkCreateDisplayPlaneSurfaceKHR;
static PFN_vkCreateSharedSwapchainsKHR vkCreateSharedSwapchainsKHR;
static PFN_vkCreateAndroidSurfaceKHR vkCreateAndroidSurfaceKHR;

static void *vulkan_load(void) {
  void *v = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
  if (!v)
    return NULL;

#define GET_VULKAN_FUNCTION(name) name = (PFN_##name)dlsym(v, #name);

  GET_VULKAN_FUNCTION(vkCreateInstance);
  GET_VULKAN_FUNCTION(vkDestroyInstance);
  GET_VULKAN_FUNCTION(vkEnumeratePhysicalDevices);
  GET_VULKAN_FUNCTION(vkGetPhysicalDeviceFeatures);
  GET_VULKAN_FUNCTION(vkGetPhysicalDeviceFormatProperties);
  GET_VULKAN_FUNCTION(vkGetPhysicalDeviceImageFormatProperties);
  GET_VULKAN_FUNCTION(vkGetPhysicalDeviceProperties);
  GET_VULKAN_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
  GET_VULKAN_FUNCTION(vkGetPhysicalDeviceMemoryProperties);
  GET_VULKAN_FUNCTION(vkGetInstanceProcAddr);
  GET_VULKAN_FUNCTION(vkGetDeviceProcAddr);
  GET_VULKAN_FUNCTION(vkCreateDevice);
  GET_VULKAN_FUNCTION(vkDestroyDevice);
  GET_VULKAN_FUNCTION(vkEnumerateInstanceExtensionProperties);
  GET_VULKAN_FUNCTION(vkEnumerateDeviceExtensionProperties);
  GET_VULKAN_FUNCTION(vkEnumerateInstanceLayerProperties);
  GET_VULKAN_FUNCTION(vkEnumerateDeviceLayerProperties);
  GET_VULKAN_FUNCTION(vkGetDeviceQueue);
  GET_VULKAN_FUNCTION(vkQueueSubmit);
  GET_VULKAN_FUNCTION(vkQueueWaitIdle);
  GET_VULKAN_FUNCTION(vkDeviceWaitIdle);
  GET_VULKAN_FUNCTION(vkAllocateMemory);
  GET_VULKAN_FUNCTION(vkFreeMemory);
  GET_VULKAN_FUNCTION(vkMapMemory);
  GET_VULKAN_FUNCTION(vkUnmapMemory);
  GET_VULKAN_FUNCTION(vkFlushMappedMemoryRanges);
  GET_VULKAN_FUNCTION(vkInvalidateMappedMemoryRanges);
  GET_VULKAN_FUNCTION(vkGetDeviceMemoryCommitment);
  GET_VULKAN_FUNCTION(vkBindBufferMemory);
  GET_VULKAN_FUNCTION(vkBindImageMemory);
  GET_VULKAN_FUNCTION(vkGetBufferMemoryRequirements);
  GET_VULKAN_FUNCTION(vkGetImageMemoryRequirements);
  GET_VULKAN_FUNCTION(vkGetImageSparseMemoryRequirements);
  GET_VULKAN_FUNCTION(vkGetPhysicalDeviceSparseImageFormatProperties);
  GET_VULKAN_FUNCTION(vkQueueBindSparse);
  GET_VULKAN_FUNCTION(vkCreateFence);
  GET_VULKAN_FUNCTION(vkDestroyFence);
  GET_VULKAN_FUNCTION(vkResetFences);
  GET_VULKAN_FUNCTION(vkGetFenceStatus);
  GET_VULKAN_FUNCTION(vkWaitForFences);
  GET_VULKAN_FUNCTION(vkCreateSemaphore);
  GET_VULKAN_FUNCTION(vkDestroySemaphore);
  GET_VULKAN_FUNCTION(vkCreateEvent);
  GET_VULKAN_FUNCTION(vkDestroyEvent);
  GET_VULKAN_FUNCTION(vkGetEventStatus);
  GET_VULKAN_FUNCTION(vkSetEvent);
  GET_VULKAN_FUNCTION(vkResetEvent);
  GET_VULKAN_FUNCTION(vkCreateQueryPool);
  GET_VULKAN_FUNCTION(vkDestroyQueryPool);
  GET_VULKAN_FUNCTION(vkGetQueryPoolResults);
  GET_VULKAN_FUNCTION(vkCreateBuffer);
  GET_VULKAN_FUNCTION(vkDestroyBuffer);
  GET_VULKAN_FUNCTION(vkCreateBufferView);
  GET_VULKAN_FUNCTION(vkDestroyBufferView);
  GET_VULKAN_FUNCTION(vkCreateImage);
  GET_VULKAN_FUNCTION(vkDestroyImage);
  GET_VULKAN_FUNCTION(vkGetImageSubresourceLayout);
  GET_VULKAN_FUNCTION(vkCreateImageView);
  GET_VULKAN_FUNCTION(vkDestroyImageView);
  GET_VULKAN_FUNCTION(vkCreateShaderModule);
  GET_VULKAN_FUNCTION(vkDestroyShaderModule);
  GET_VULKAN_FUNCTION(vkCreatePipelineCache);
  GET_VULKAN_FUNCTION(vkDestroyPipelineCache);
  GET_VULKAN_FUNCTION(vkGetPipelineCacheData);
  GET_VULKAN_FUNCTION(vkMergePipelineCaches);
  GET_VULKAN_FUNCTION(vkCreateGraphicsPipelines);
  GET_VULKAN_FUNCTION(vkCreateComputePipelines);
  GET_VULKAN_FUNCTION(vkDestroyPipeline);
  GET_VULKAN_FUNCTION(vkCreatePipelineLayout);
  GET_VULKAN_FUNCTION(vkDestroyPipelineLayout);
  GET_VULKAN_FUNCTION(vkCreateSampler);
  GET_VULKAN_FUNCTION(vkDestroySampler);
  GET_VULKAN_FUNCTION(vkCreateDescriptorSetLayout);
  GET_VULKAN_FUNCTION(vkDestroyDescriptorSetLayout);
  GET_VULKAN_FUNCTION(vkCreateDescriptorPool);
  GET_VULKAN_FUNCTION(vkDestroyDescriptorPool);
  GET_VULKAN_FUNCTION(vkResetDescriptorPool);
  GET_VULKAN_FUNCTION(vkAllocateDescriptorSets);
  GET_VULKAN_FUNCTION(vkFreeDescriptorSets);
  GET_VULKAN_FUNCTION(vkUpdateDescriptorSets);
  GET_VULKAN_FUNCTION(vkCreateFramebuffer);
  GET_VULKAN_FUNCTION(vkDestroyFramebuffer);
  GET_VULKAN_FUNCTION(vkCreateRenderPass);
  GET_VULKAN_FUNCTION(vkDestroyRenderPass);
  GET_VULKAN_FUNCTION(vkGetRenderAreaGranularity);
  GET_VULKAN_FUNCTION(vkCreateCommandPool);
  GET_VULKAN_FUNCTION(vkDestroyCommandPool);
  GET_VULKAN_FUNCTION(vkResetCommandPool);
  GET_VULKAN_FUNCTION(vkAllocateCommandBuffers);
  GET_VULKAN_FUNCTION(vkFreeCommandBuffers);
  GET_VULKAN_FUNCTION(vkBeginCommandBuffer);
  GET_VULKAN_FUNCTION(vkEndCommandBuffer);
  GET_VULKAN_FUNCTION(vkResetCommandBuffer);
  GET_VULKAN_FUNCTION(vkCmdBindPipeline);
  GET_VULKAN_FUNCTION(vkCmdSetViewport);
  GET_VULKAN_FUNCTION(vkCmdSetScissor);
  GET_VULKAN_FUNCTION(vkCmdSetLineWidth);
  GET_VULKAN_FUNCTION(vkCmdSetDepthBias);
  GET_VULKAN_FUNCTION(vkCmdSetBlendConstants);
  GET_VULKAN_FUNCTION(vkCmdSetDepthBounds);
  GET_VULKAN_FUNCTION(vkCmdSetStencilCompareMask);
  GET_VULKAN_FUNCTION(vkCmdSetStencilWriteMask);
  GET_VULKAN_FUNCTION(vkCmdSetStencilReference);
  GET_VULKAN_FUNCTION(vkCmdBindDescriptorSets);
  GET_VULKAN_FUNCTION(vkCmdBindIndexBuffer);
  GET_VULKAN_FUNCTION(vkCmdBindVertexBuffers);
  GET_VULKAN_FUNCTION(vkCmdDraw);
  GET_VULKAN_FUNCTION(vkCmdDrawIndexed);
  GET_VULKAN_FUNCTION(vkCmdDrawIndirect);
  GET_VULKAN_FUNCTION(vkCmdDrawIndexedIndirect);
  GET_VULKAN_FUNCTION(vkCmdDispatch);
  GET_VULKAN_FUNCTION(vkCmdDispatchIndirect);
  GET_VULKAN_FUNCTION(vkCmdCopyBuffer);
  GET_VULKAN_FUNCTION(vkCmdCopyImage);
  GET_VULKAN_FUNCTION(vkCmdBlitImage);
  GET_VULKAN_FUNCTION(vkCmdCopyBufferToImage);
  GET_VULKAN_FUNCTION(vkCmdCopyImageToBuffer);
  GET_VULKAN_FUNCTION(vkCmdUpdateBuffer);
  GET_VULKAN_FUNCTION(vkCmdFillBuffer);
  GET_VULKAN_FUNCTION(vkCmdClearColorImage);
  GET_VULKAN_FUNCTION(vkCmdClearDepthStencilImage);
  GET_VULKAN_FUNCTION(vkCmdClearAttachments);
  GET_VULKAN_FUNCTION(vkCmdResolveImage);
  GET_VULKAN_FUNCTION(vkCmdSetEvent);
  GET_VULKAN_FUNCTION(vkCmdResetEvent);
  GET_VULKAN_FUNCTION(vkCmdWaitEvents);
  GET_VULKAN_FUNCTION(vkCmdPipelineBarrier);
  GET_VULKAN_FUNCTION(vkCmdBeginQuery);
  GET_VULKAN_FUNCTION(vkCmdEndQuery);
  GET_VULKAN_FUNCTION(vkCmdResetQueryPool);
  GET_VULKAN_FUNCTION(vkCmdWriteTimestamp);
  GET_VULKAN_FUNCTION(vkCmdCopyQueryPoolResults);
  GET_VULKAN_FUNCTION(vkCmdPushConstants);
  GET_VULKAN_FUNCTION(vkCmdBeginRenderPass);
  GET_VULKAN_FUNCTION(vkCmdNextSubpass);
  GET_VULKAN_FUNCTION(vkCmdEndRenderPass);
  GET_VULKAN_FUNCTION(vkCmdExecuteCommands);
  GET_VULKAN_FUNCTION(vkDestroySurfaceKHR);
  GET_VULKAN_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR);
  GET_VULKAN_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
  GET_VULKAN_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR);
  GET_VULKAN_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR);
  GET_VULKAN_FUNCTION(vkCreateSwapchainKHR);
  GET_VULKAN_FUNCTION(vkDestroySwapchainKHR);
  GET_VULKAN_FUNCTION(vkGetSwapchainImagesKHR);
  GET_VULKAN_FUNCTION(vkAcquireNextImageKHR);
  GET_VULKAN_FUNCTION(vkQueuePresentKHR);
  GET_VULKAN_FUNCTION(vkGetPhysicalDeviceDisplayPropertiesKHR);
  GET_VULKAN_FUNCTION(vkGetPhysicalDeviceDisplayPlanePropertiesKHR);
  GET_VULKAN_FUNCTION(vkGetDisplayPlaneSupportedDisplaysKHR);
  GET_VULKAN_FUNCTION(vkGetDisplayModePropertiesKHR);
  GET_VULKAN_FUNCTION(vkCreateDisplayModeKHR);
  GET_VULKAN_FUNCTION(vkGetDisplayPlaneCapabilitiesKHR);
  GET_VULKAN_FUNCTION(vkCreateDisplayPlaneSurfaceKHR);
  GET_VULKAN_FUNCTION(vkCreateSharedSwapchainsKHR);
  GET_VULKAN_FUNCTION(vkCreateAndroidSurfaceKHR);

#undef GET_VULKAN_FUNCTION
  return v;
}

static struct vulkan_src {
  struct ANativeWindow *window;
  int isInitialized;
  int isResized;
  void *libvulkan;

  VkInstance instance_;
  VkPhysicalDevice gpuDevice_;
  VkDevice device_;
  uint32_t queueFamilyIndex_;
  VkSurfaceKHR surface_;
  VkQueue queue_;

  VkSwapchainKHR swapchain_;
  uint32_t swapchainLength_;
  VkExtent2D displaySize_;
  VkFormat displayFormat_;
  VkImage *displayImages_;
  VkImageView *displayViews_;
  VkFramebuffer *framebuffers_;

  VkPipelineLayout layout_;
  VkPipelineCache cache_;
  VkPipeline pipeline_;

  VkRenderPass renderPass_;

  VkBuffer vertexBuf_;
  VkDeviceMemory vertexBufMemory_;

  VkCommandPool cmdPool_;
  VkCommandBuffer *cmdBuffer_;
  uint32_t cmdBufferLen_;
  VkSemaphore imageAvailableSemaphore_;
  VkSemaphore renderFinishedSemaphore_;
  VkFence *inFlightFences_;
  size_t currentFrame;
} *src = NULL;
#define CALLVK(X)                                                      \
  do {                                                                 \
    VkResult result = X;                                               \
    if (result != VK_SUCCESS) {                                        \
      LOGE("Vulkan failure: %x at %s:%d", result, __FILE__, __LINE__); \
      return 0;                                                        \
    }                                                                  \
  } while (0)

static int loadShaderFromFile(const char *filePath, VkShaderModule *shaderModule, VkDevice device) {
  FILE *file = fopen(filePath, "rb");
  if (!file) {
    LOGE("Failed to open shader file: %s", filePath);
    return 0;
  }

  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *shaderCode = (char *)malloc(fileSize);
  if (!shaderCode) {
    LOGE("Out of memory trying to allocate for shader");
    fclose(file);
    return 0;
  }
  fread(shaderCode, 1, fileSize, file);
  fclose(file);

  VkShaderModuleCreateInfo shaderModuleCreateInfo = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pNext = NULL,
    .codeSize = fileSize,
    .pCode = (const uint32_t *)shaderCode,
    .flags = 0,
  };

  if (vkCreateShaderModule(device, &shaderModuleCreateInfo, NULL, shaderModule) != VK_SUCCESS) {
    LOGE("Failed to create shader module for %s", filePath);
    free(shaderCode);
    return 0;
  }

  free(shaderCode);
  return 1;
}

static void cleanupSwapChain() {
  if (!src->device_)
    return;
  vkDeviceWaitIdle(src->device_);

  if (src->framebuffers_) {
    for (uint32_t i = 0; i < src->swapchainLength_; i++) {
      vkDestroyFramebuffer(src->device_, src->framebuffers_[i], NULL);
    }
    free(src->framebuffers_);
    src->framebuffers_ = 0;
  }
  if (src->displayViews_) {
    for (uint32_t i = 0; i < src->swapchainLength_; i++) {
      vkDestroyImageView(src->device_, src->displayViews_[i], NULL);
    }
    free(src->displayViews_);
    src->displayViews_ = 0;
  }
  if (src->pipeline_) {
    vkDestroyPipeline(src->device_, src->pipeline_, NULL);
    src->pipeline_ = 0;
  }
  if (src->layout_) {
    vkDestroyPipelineLayout(src->device_, src->layout_, NULL);
    src->layout_ = 0;
  }
  if (src->renderPass_) {
    vkDestroyRenderPass(src->device_, src->renderPass_, NULL);
    src->renderPass_ = 0;
  }
  if (src->swapchain_) {
    vkDestroySwapchainKHR(src->device_, src->swapchain_, NULL);
    src->swapchain_ = 0;
  }
  if (src->displayImages_) {
    free(src->displayImages_);
    src->displayImages_ = 0;
  }
}

static void vulkan_onWindowCreate(void *w) {
  src->window = (struct ANativeWindow *)w;
}

static void vulkan_onWindowDestroy(void) {
  cleanupSwapChain();
  if (src->instance_ && src->surface_) {
    vkDestroySurfaceKHR(src->instance_, src->surface_, NULL);
    src->surface_ = 0;
  }
  src->window = NULL;
  src->isInitialized = 0;
}

static void vulkan_onWindowResizeDisplay(void) { src->isResized = 1; }
static void vulkan_onWindowResize(void) { src->isResized = 1; }
static void vulkan_resizeInsets(float x, float y, float z, float w) {}

static int vulkan_preRender(void) {
  if (!src->window) {
    return 0;
  }

  if (src->isResized) {
    cleanupSwapChain();
    src->isResized = 0;
  }
  if (!src->instance_) {
    VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "TechnoWar",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "TechnoWar",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_1,
    };
    const char *extensions[] = {"VK_KHR_surface", "VK_KHR_android_surface"};
    VkInstanceCreateInfo instanceCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appInfo,
      .enabledExtensionCount = 2,
      .ppEnabledExtensionNames = extensions,
    };
    CALLVK(vkCreateInstance(&instanceCreateInfo, NULL, &src->instance_));
  }

  if (!src->surface_) {
    VkAndroidSurfaceCreateInfoKHR createInfo = {
      .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
      .window = src->window};
    CALLVK(vkCreateAndroidSurfaceKHR(src->instance_, &createInfo, NULL, &src->surface_));
  }

  if (!src->gpuDevice_) {
    uint32_t gpuCount = 0;
    CALLVK(vkEnumeratePhysicalDevices(src->instance_, &gpuCount, NULL));
    VkPhysicalDevice *devices = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * gpuCount);
    CALLVK(vkEnumeratePhysicalDevices(src->instance_, &gpuCount, devices));
    src->gpuDevice_ = devices[0];
    free(devices);

    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(src->gpuDevice_, &queueFamilyCount, NULL);
    VkQueueFamilyProperties *queueFamilyProperties = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(src->gpuDevice_, &queueFamilyCount, queueFamilyProperties);
    uint32_t i;
    for (i = 0; i < queueFamilyCount; ++i) {
      VkBool32 supportsPresent;
      vkGetPhysicalDeviceSurfaceSupportKHR(src->gpuDevice_, i, src->surface_, &supportsPresent);
      if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supportsPresent) {
        src->queueFamilyIndex_ = i;
        break;
      }
    }
    free(queueFamilyProperties);
  }

  if (!src->device_) {
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = src->queueFamilyIndex_,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority,
    };
    const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo deviceCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueCreateInfo,
      .enabledExtensionCount = 1,
      .ppEnabledExtensionNames = deviceExtensions,
    };
    CALLVK(vkCreateDevice(src->gpuDevice_, &deviceCreateInfo, NULL, &src->device_));
    vkGetDeviceQueue(src->device_, src->queueFamilyIndex_, 0, &src->queue_);
  }

  if (!src->swapchain_) {
    VkSurfaceCapabilitiesKHR surfaceCap;
    CALLVK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(src->gpuDevice_, src->surface_, &surfaceCap));
    src->displaySize_ = surfaceCap.currentExtent;

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(src->gpuDevice_, src->surface_, &formatCount, NULL);
    VkSurfaceFormatKHR *formats = (VkSurfaceFormatKHR *)malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(src->gpuDevice_, src->surface_, &formatCount, formats);
    uint32_t chosenFormat;
    for (chosenFormat = 0; chosenFormat < formatCount; chosenFormat++) {
      if (formats[chosenFormat].format == VK_FORMAT_R8G8B8A8_UNORM)
        break;
    }
    if (chosenFormat >= formatCount)
      chosenFormat = 0;
    src->displayFormat_ = formats[chosenFormat].format;

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = src->surface_,
      .minImageCount = surfaceCap.minImageCount,
      .imageFormat = src->displayFormat_,
      .imageColorSpace = formats[chosenFormat].colorSpace,
      .imageExtent = surfaceCap.currentExtent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .preTransform = surfaceCap.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
      .presentMode = VK_PRESENT_MODE_FIFO_KHR,
      .clipped = VK_TRUE,
    };
    CALLVK(vkCreateSwapchainKHR(src->device_, &swapchainCreateInfo, NULL, &src->swapchain_));
    free(formats);

    CALLVK(vkGetSwapchainImagesKHR(src->device_, src->swapchain_, &src->swapchainLength_, NULL));
    src->displayImages_ = (VkImage *)malloc(sizeof(VkImage) * src->swapchainLength_);
    CALLVK(vkGetSwapchainImagesKHR(src->device_, src->swapchain_, &src->swapchainLength_, src->displayImages_));

    src->displayViews_ = (VkImageView *)malloc(sizeof(VkImageView) * src->swapchainLength_);
    for (uint32_t i = 0; i < src->swapchainLength_; i++) {
      VkImageViewCreateInfo viewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = src->displayImages_[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = src->displayFormat_,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1},
      };
      CALLVK(vkCreateImageView(src->device_, &viewCreateInfo, NULL, &src->displayViews_[i]));
    }
  }

  if (!src->renderPass_) {
    VkAttachmentDescription attachment = {
      .format = src->displayFormat_,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    VkAttachmentReference color_ref = {.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpass = {
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_ref,
    };
    VkRenderPassCreateInfo rp_info = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &attachment,
      .subpassCount = 1,
      .pSubpasses = &subpass,
    };
    CALLVK(vkCreateRenderPass(src->device_, &rp_info, NULL, &src->renderPass_));
  }

  if (!src->pipeline_) {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    CALLVK(vkCreatePipelineLayout(src->device_, &pipelineLayoutCreateInfo, NULL, &src->layout_));

    VkShaderModule vertexShader, fragmentShader;
    if (!loadShaderFromFile("../shaders/tri.vert.spv", &vertexShader, src->device_) ||
        !loadShaderFromFile("../shaders/tri.frag.spv", &fragmentShader, src->device_)) {
      LOGE("Failed to load shaders");
      return 0;
    }

    VkPipelineShaderStageCreateInfo shaderStages[2] = {
      {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = vertexShader, .pName = "main"},
      {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = fragmentShader, .pName = "main"}};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
    VkViewport viewport = {.width = (float)src->displaySize_.width, .height = (float)src->displaySize_.height, .maxDepth = 1.0f};
    VkRect2D scissor = {.extent = src->displaySize_};
    VkPipelineViewportStateCreateInfo viewportInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .pViewports = &viewport, .scissorCount = 1, .pScissors = &scissor};
    VkPipelineRasterizationStateCreateInfo rasterInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, .polygonMode = VK_POLYGON_MODE_FILL, .cullMode = VK_CULL_MODE_NONE, .frontFace = VK_FRONT_FACE_CLOCKWISE, .lineWidth = 1.0f};
    VkPipelineMultisampleStateCreateInfo multisampleInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};
    VkPipelineColorBlendAttachmentState attachmentState = {.colorWriteMask = 0xf};
    VkPipelineColorBlendStateCreateInfo colorBlendInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, .attachmentCount = 1, .pAttachments = &attachmentState};

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = 2,
      .pStages = shaderStages,
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssemblyInfo,
      .pViewportState = &viewportInfo,
      .pRasterizationState = &rasterInfo,
      .pMultisampleState = &multisampleInfo,
      .pColorBlendState = &colorBlendInfo,
      .layout = src->layout_,
      .renderPass = src->renderPass_,
    };
    CALLVK(vkCreateGraphicsPipelines(src->device_, NULL, 1, &pipelineCreateInfo, NULL, &src->pipeline_));

    vkDestroyShaderModule(src->device_, vertexShader, NULL);
    vkDestroyShaderModule(src->device_, fragmentShader, NULL);
  }

  if (!src->framebuffers_) {
    src->framebuffers_ = (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * src->swapchainLength_);
    for (uint32_t i = 0; i < src->swapchainLength_; i++) {
      VkFramebufferCreateInfo fb_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = src->renderPass_,
        .attachmentCount = 1,
        .pAttachments = &src->displayViews_[i],
        .width = src->displaySize_.width,
        .height = src->displaySize_.height,
        .layers = 1,
      };
      CALLVK(vkCreateFramebuffer(src->device_, &fb_info, NULL, &src->framebuffers_[i]));
    }
  }

  if (!src->cmdPool_) {
    VkCommandPoolCreateInfo cmdPoolCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = src->queueFamilyIndex_,
    };
    CALLVK(vkCreateCommandPool(src->device_, &cmdPoolCreateInfo, NULL, &src->cmdPool_));
  }

  if (!src->cmdBuffer_) {
    src->cmdBufferLen_ = src->swapchainLength_;
    src->cmdBuffer_ = (VkCommandBuffer *)malloc(sizeof(VkCommandBuffer) * src->cmdBufferLen_);
    VkCommandBufferAllocateInfo cmdBufferAllocateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = src->cmdPool_,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = src->cmdBufferLen_,
    };
    CALLVK(vkAllocateCommandBuffers(src->device_, &cmdBufferAllocateInfo, src->cmdBuffer_));
  }

  if (!src->imageAvailableSemaphore_) {
    VkSemaphoreCreateInfo semaphoreInfo = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    CALLVK(vkCreateSemaphore(src->device_, &semaphoreInfo, NULL, &src->imageAvailableSemaphore_));
    CALLVK(vkCreateSemaphore(src->device_, &semaphoreInfo, NULL, &src->renderFinishedSemaphore_));
  }

  if (!src->inFlightFences_) {
    src->inFlightFences_ = (VkFence *)malloc(sizeof(VkFence) * src->swapchainLength_);
    VkFenceCreateInfo fenceInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    for (uint32_t i = 0; i < src->swapchainLength_; i++) {
      CALLVK(vkCreateFence(src->device_, &fenceInfo, NULL, &src->inFlightFences_[i]));
    }
  }

  for (uint32_t i = 0; i < src->swapchainLength_; i++) {
    VkCommandBufferBeginInfo beginInfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    CALLVK(vkBeginCommandBuffer(src->cmdBuffer_[i], &beginInfo));
    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    VkRenderPassBeginInfo renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = src->renderPass_,
      .framebuffer = src->framebuffers_[i],
      .renderArea.offset = {0, 0},
      .renderArea.extent = src->displaySize_,
      .clearValueCount = 1,
      .pClearValues = &clearColor,
    };
    vkCmdBeginRenderPass(src->cmdBuffer_[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(src->cmdBuffer_[i], VK_PIPELINE_BIND_POINT_GRAPHICS, src->pipeline_);
    vkCmdDraw(src->cmdBuffer_[i], 3, 1, 0, 0);
    vkCmdEndRenderPass(src->cmdBuffer_[i]);
    CALLVK(vkEndCommandBuffer(src->cmdBuffer_[i]));
  }

  src->isInitialized = 1;
  return 1;
}

static void vulkan_postRender(void) {
  if (!src->isInitialized)
    return;

  vkWaitForFences(src->device_, 1, &src->inFlightFences_[src->currentFrame], VK_TRUE, UINT64_MAX);
  vkResetFences(src->device_, 1, &src->inFlightFences_[src->currentFrame]);

  uint32_t imageIndex;
  vkAcquireNextImageKHR(src->device_, src->swapchain_, UINT64_MAX, src->imageAvailableSemaphore_, NULL, &imageIndex);

  VkSubmitInfo submitInfo = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO};
  VkSemaphore waitSemaphores[] = {src->imageAvailableSemaphore_};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &src->cmdBuffer_[imageIndex];

  VkSemaphore signalSemaphores[] = {src->renderFinishedSemaphore_};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(src->queue_, 1, &submitInfo, src->inFlightFences_[src->currentFrame]) != VK_SUCCESS) {
    LOGE("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR presentInfo = {.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {src->swapchain_};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;

  vkQueuePresentKHR(src->queue_, &presentInfo);

  src->currentFrame = (src->currentFrame + 1) % src->swapchainLength_;
}

static void vulkan_term(void) {
  if (src->device_) {
    vkDeviceWaitIdle(src->device_);
    cleanupSwapChain();

    if (src->cmdPool_) {
      vkDestroyCommandPool(src->device_, src->cmdPool_, NULL);
    }
    if (src->vertexBuf_) {
      vkDestroyBuffer(src->device_, src->vertexBuf_, NULL);
    }
    if (src->vertexBufMemory_) {
      vkFreeMemory(src->device_, src->vertexBufMemory_, NULL);
    }
    if (src->imageAvailableSemaphore_) {
      vkDestroySemaphore(src->device_, src->imageAvailableSemaphore_, NULL);
    }
    if (src->renderFinishedSemaphore_) {
      vkDestroySemaphore(src->device_, src->renderFinishedSemaphore_, NULL);
    }
    if (src->inFlightFences_) {
      for (uint32_t i = 0; i < src->swapchainLength_; i++) {
        vkDestroyFence(src->device_, src->inFlightFences_[i], NULL);
      }
      free(src->inFlightFences_);
    }
    vkDestroyDevice(src->device_, NULL);
  }
  if (src->instance_) {
    if (src->surface_) {
      vkDestroySurfaceKHR(src->instance_, src->surface_, NULL);
    }
    vkDestroyInstance(src->instance_, NULL);
  }
  dlclose(src->libvulkan);
  free(src);
  src = NULL;
}

int vulkan_init(void) {
  src = (struct vulkan_src *)calloc(1, sizeof(struct vulkan_src));
  if (!src) {
    LOGE("Failed to allocate memory for vulkan_src");
    return 0;
  }

  if (!(src->libvulkan = vulkan_load())) {
    LOGE("Vulkan library error");
    free(src);
    src = NULL;
    return 0;
  }

  androidGraphics_onWindowCreate = vulkan_onWindowCreate;
  androidGraphics_onWindowDestroy = vulkan_onWindowDestroy;
  androidGraphics_onWindowResizeDisplay = vulkan_onWindowResizeDisplay;
  androidGraphics_onWindowResize = vulkan_onWindowResize;
  androidGraphics_resizeInsets = vulkan_resizeInsets;
  androidGraphics_preRender = vulkan_preRender;
  androidGraphics_postRender = vulkan_postRender;
  androidGraphics_term = vulkan_term;

  return 1;
}
