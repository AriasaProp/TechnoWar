#include "vulkan_graphics.hpp"

#include <vector>
#include <unordered_set>
#include <cassert>
#include "vulkan_wrapper.hpp"

//{
VkResult result_;
struct VulkanDeviceInfo {
  bool initialized_;
  VkInstance instance_;
  VkPhysicalDevice gpuDevice_;
  VkDevice device_;
  uint32_t queueFamilyIndex_;
  VkSurfaceKHR surface_;
  VkQueue queue_;
} device;
struct VulkanSwapchainInfo {
  VkSwapchainKHR swapchain_;
  uint32_t swapchainLength_;
  VkExtent2D displaySize_;
  VkFormat displayFormat_;
  std::vector<VkImage> displayImages_;
  std::vector<VkImageView> displayViews_;
  std::vector<VkFramebuffer> framebuffers_;
} swapchain;
struct VulkanBufferInfo {
  VkBuffer vertexBuf_;
} buffers;
struct VulkanGfxPipelineInfo {
  VkPipelineLayout layout_;
  VkPipelineCache cache_;
  VkPipeline pipeline_;
} gfxPipeline;
struct VulkanRenderInfo {
  VkRenderPass renderPass_;
  VkCommandPool cmdPool_;
  VkCommandBuffer* cmdBuffer_;
  uint32_t cmdBufferLen_;
  VkSemaphore semaphore_;
  VkFence fence_;
} render_;
void setImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStages, VkPipelineStageFlags destStages) {
  VkImageMemoryBarrier imageMemoryBarrier {
      .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .pNext = NULL,
      .srcAccessMask = 0,
      .dstAccessMask = 0,
      .oldLayout = oldImageLayout,
      .newLayout = newImageLayout,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = image,
      .subresourceRange =
          {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .baseMipLevel = 0,
              .levelCount = 1,
              .baseArrayLayer = 0,
              .layerCount = 1,
          },
  };

  switch (oldImageLayout) {
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
      imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      break;
    default:
      break;
  }
  switch (newImageLayout) {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      break;
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
      imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
      break;
    default:
      break;
  }
  vkCmdPipelineBarrier(cmdBuffer, srcStages, destStages, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
}
void CreateVulkanDevice(ANativeWindow* platformWindow, VkApplicationInfo* appInfo) {
  std::vector<const char*> instance_extensions;
  std::vector<const char*> device_extensions;
  instance_extensions.push_back("VK_KHR_surface");
  instance_extensions.push_back("VK_KHR_android_surface");
  device_extensions.push_back("VK_KHR_swapchain");
  VkInstanceCreateInfo instanceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .pApplicationInfo = appInfo,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size()),
      .ppEnabledExtensionNames = instance_extensions.data(),
  };
  result_ = vkCreateInstance(&instanceCreateInfo, nullptr, &device.instance_);
  assert(result_ == VK_SUCCESS);
  VkAndroidSurfaceCreateInfoKHR createInfo{
      .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .window = platformWindow};
  result_ = vkCreateAndroidSurfaceKHR(device.instance_, &createInfo, nullptr, &device.surface_);
  assert(result_ == VK_SUCCESS);
  uint32_t gpuCount = 0;
  result_ = vkEnumeratePhysicalDevices(device.instance_, &gpuCount, nullptr);
  assert(result_ == VK_SUCCESS);
  VkPhysicalDevice tmpGpus[gpuCount];
  result_ = vkEnumeratePhysicalDevices(device.instance_, &gpuCount, tmpGpus);
  assert(result_ == VK_SUCCESS);
  device.gpuDevice_ = tmpGpus[0];  // Pick up the first GPU Device

  // Find a GFX queue family
  uint32_t queueFamilyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(device.gpuDevice_, &queueFamilyCount, nullptr);
  assert(queueFamilyCount);
  std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device.gpuDevice_, &queueFamilyCount, queueFamilyProperties.data());

  uint32_t queueFamilyIndex = 0;
  while (queueFamilyIndex < queueFamilyCount) {
    if (queueFamilyProperties[queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) break;
    queueFamilyIndex++;
  }
  assert(queueFamilyIndex < queueFamilyCount);
  device.queueFamilyIndex_ = queueFamilyIndex;

  // Create a logical device (vulkan device)
  float priorities[] = {
      1.0f,
  };
  VkDeviceQueueCreateInfo queueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueFamilyIndex = queueFamilyIndex,
      .queueCount = 1,
      .pQueuePriorities = priorities,
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

  result_ = vkCreateDevice(device.gpuDevice_, &deviceCreateInfo, nullptr, &device.device_);
  assert(result_ == VK_SUCCESS);
  vkGetDeviceQueue(device.device_, device.queueFamilyIndex_, 0, &device.queue_);
}
void CreateSwapChain() {
  memset(&swapchain, 0, sizeof(swapchain));
  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.gpuDevice_, device.surface_, &surfaceCapabilities);
  // Query the list of supported surface format and choose one we like
  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device.gpuDevice_, device.surface_, &formatCount, nullptr);
  VkSurfaceFormatKHR* formats = new VkSurfaceFormatKHR[formatCount];
  vkGetPhysicalDeviceSurfaceFormatsKHR(device.gpuDevice_, device.surface_, &formatCount, formats);
  uint32_t chosenFormat = 0;
  while (chosenFormat < formatCount) {
    if (formats[chosenFormat].format == VK_FORMAT_R8G8B8A8_UNORM) break;
    chosenFormat++;
  }
  assert(chosenFormat < formatCount);
  swapchain.displaySize_ = surfaceCapabilities.currentExtent;
  swapchain.displayFormat_ = formats[chosenFormat].format;
  VkSurfaceCapabilitiesKHR surfaceCap;
  result_ = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.gpuDevice_, device.surface_, &surfaceCap);
  assert(result_ == VK_SUCCESS);
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
  result_ = vkCreateSwapchainKHR(device.device_, &swapchainCreateInfo, nullptr, &swapchain.swapchain_);
  assert(result_ == VK_SUCCESS);
  result_ = vkGetSwapchainImagesKHR(device.device_, swapchain.swapchain_, &swapchain.swapchainLength_, nullptr);
  assert(result_ == VK_SUCCESS);
  delete[] formats;
}
void DeleteSwapChain() {
  for (uint32_t i = 0; i < swapchain.swapchainLength_; i++) {
    vkDestroyFramebuffer(device.device_, swapchain.framebuffers_[i], nullptr);
    vkDestroyImageView(device.device_, swapchain.displayViews_[i], nullptr);
  }
  vkDestroySwapchainKHR(device.device_, swapchain.swapchain_, nullptr);
}
void CreateFrameBuffers(VkRenderPass& renderPass, VkImageView depthView = VK_NULL_HANDLE) {
  // query display attachment to swapchain
  uint32_t SwapchainImagesCount = 0;
  result_ = (vkGetSwapchainImagesKHR(device.device_, swapchain.swapchain_,
                                  &SwapchainImagesCount, nullptr));
  assert(result_ == VK_SUCCESS);
  swapchain.displayImages_.resize(SwapchainImagesCount);
  result_ = (vkGetSwapchainImagesKHR(device.device_, swapchain.swapchain_,
                                  &SwapchainImagesCount,
                                  swapchain.displayImages_.data()));

  assert(result_ == VK_SUCCESS);
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
    result_ = (vkCreateImageView(device.device_, &viewCreateInfo, nullptr,
                              &swapchain.displayViews_[i]));
  
    assert(result_ == VK_SUCCESS);
  }
  // create a framebuffer from each swapchain image
  swapchain.framebuffers_.resize(swapchain.swapchainLength_);
  for (uint32_t i = 0; i < swapchain.swapchainLength_; i++) {
    VkImageView attachments[2] = {
        swapchain.displayViews_[i], depthView,
    };
    VkFramebufferCreateInfo fbCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .renderPass = renderPass,
        .attachmentCount = 1,  // 2 if using depth
        .pAttachments = attachments,
        .width = static_cast<uint32_t>(swapchain.displaySize_.width),
        .height = static_cast<uint32_t>(swapchain.displaySize_.height),
       .layers = 1,
    };
    fbCreateInfo.attachmentCount = (depthView == VK_NULL_HANDLE ? 1 : 2);

    result_ = (vkCreateFramebuffer(device.device_, &fbCreateInfo, nullptr,
                                &swapchain.framebuffers_[i]));
  
    assert(result_ == VK_SUCCESS);
  }
}
bool MapMemoryTypeToIndex(uint32_t typeBits, VkFlags requirements_mask, uint32_t* typeIndex) {
  VkPhysicalDeviceMemoryProperties memoryProperties;
  vkGetPhysicalDeviceMemoryProperties(device.gpuDevice_, &memoryProperties);
  // Search memtypes to find first index with those properties
  for (uint32_t i = 0; i < 32; i++) {
    if ((typeBits & 1) == 1) {
      // Type is available, does it match user properties?
      if ((memoryProperties.memoryTypes[i].propertyFlags & requirements_mask) ==
          requirements_mask) {
        *typeIndex = i;
        return true;
      }
    }
    typeBits >>= 1;
  }
  return false;
}
void CreateBuffers() {
  const float vertexData[] = {
      -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
  };
  
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

  result_ = vkCreateBuffer(device.device_, &createBufferInfo, nullptr, &buffers.vertexBuf_);
  assert(result_ == VK_SUCCESS);
  VkMemoryRequirements memReq;
  vkGetBufferMemoryRequirements(device.device_, buffers.vertexBuf_, &memReq);
  VkMemoryAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .pNext = nullptr,
      .allocationSize = memReq.size,
      .memoryTypeIndex = 0,  // Memory type assigned in the next step
  };

  // Assign the proper memory type for that buffer
  MapMemoryTypeToIndex(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex);

  // Allocate memory for the buffer
  VkDeviceMemory deviceMemory;
  result_ = vkAllocateMemory(device.device_, &allocInfo, nullptr, &deviceMemory);
  assert(result_ == VK_SUCCESS);
  void* data;
  result_ = vkMapMemory(device.device_, deviceMemory, 0, allocInfo.allocationSize, 0, &data);
  assert(result_ == VK_SUCCESS);
  memcpy(data, vertexData, sizeof(vertexData));
  vkUnmapMemory(device.device_, deviceMemory);
  result_ = vkBindBufferMemory(device.device_, buffers.vertexBuf_, deviceMemory, 0);
  assert(result_ == VK_SUCCESS);
}
void loadShader(const char* src, VkShaderModule* shaderOut) {
  VkShaderModuleCreateInfo shaderModuleCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .codeSize = strlen(src),
      .pCode = (const uint32_t*)src,
  };
  result_ = vkCreateShaderModule(device.device_, &shaderModuleCreateInfo, nullptr, shaderOut);
  assert(result_ == VK_SUCCESS);
}
void CreateGraphicsPipeline() {
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
  result_ = (vkCreatePipelineLayout(device.device_, &pipelineLayoutCreateInfo, nullptr, &gfxPipeline.layout_));
  assert(result_ == VK_SUCCESS);
  VkShaderModule vertexShader;
  VkShaderModule fragmentShader;
  {
    const char *vertexSrc = "#version 400"
      "\n#extension GL_ARB_separate_shader_objects : enable"
      "\n#extension GL_ARB_shading_language_420pack : enable"
      "\nlayout (location = 0) in vec4 pos;"
      "\nvoid main() {"
      "\n   gl_Position = pos;"
      "\n}";
    loadShader(vertexSrc, &vertexShader);
  }
  {
    const char *fragmentSrc = "#version 400"
      "\n#extension GL_ARB_separate_shader_objects : enable"
      "\n#extension GL_ARB_shading_language_420pack : enable"
      "\nlayout (location = 0) out vec4 uFragColor;"
      "\nvoid main() {"
      "\n   uFragColor = vec4(0.91, 0.26,  0.21, 1.0);"
      "\n}";
    loadShader(fragmentSrc, &fragmentShader);
  }

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
      }};

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
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
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

  result_ = (vkCreatePipelineCache(device.device_, &pipelineCacheInfo, nullptr, &gfxPipeline.cache_));
  assert(result_ == VK_SUCCESS);
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
      .renderPass = render_.renderPass_,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = 0,
  };

  result_ = vkCreateGraphicsPipelines(device.device_, gfxPipeline.cache_, 1, &pipelineCreateInfo, nullptr, &gfxPipeline.pipeline_);
  vkDestroyShaderModule(device.device_, vertexShader, nullptr);
  vkDestroyShaderModule(device.device_, fragmentShader, nullptr);
  assert(result_ == VK_SUCCESS);
}
void DeleteGraphicsPipeline() {
  if (gfxPipeline.pipeline_ == VK_NULL_HANDLE) return;
  vkDestroyPipeline(device.device_, gfxPipeline.pipeline_, nullptr);
  vkDestroyPipelineCache(device.device_, gfxPipeline.cache_, nullptr);
  vkDestroyPipelineLayout(device.device_, gfxPipeline.layout_, nullptr);
}
//}

void vulkan_graphics::onResume() {
  // To do
  resume = true;
}
void vulkan_graphics::onWindowInit(ANativeWindow *window) {
  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = "tutorial05_triangle_window",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "tutorial",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_MAKE_VERSION(1, 1, 0),
  };
  CreateVulkanDevice(window, &appInfo);
  CreateSwapChain();
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

  VkAttachmentReference colourReference {.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

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
  result_ = vkCreateRenderPass(device.device_, &renderPassCreateInfo, nullptr, &render_.renderPass_);
  assert(result_ == VK_SUCCESS);
  CreateFrameBuffers(render_.renderPass_);
  CreateBuffers();  // create vertex buffers
  // Create graphics pipeline
  CreateGraphicsPipeline();
  VkCommandPoolCreateInfo cmdPoolCreateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = device.queueFamilyIndex_,
  };
  result_ = vkCreateCommandPool(device.device_, &cmdPoolCreateInfo, nullptr, &render_.cmdPool_);
  assert(result_ == VK_SUCCESS);
  render_.cmdBufferLen_ = swapchain.swapchainLength_;
  render_.cmdBuffer_ = new VkCommandBuffer[swapchain.swapchainLength_];
  VkCommandBufferAllocateInfo cmdBufferCreateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = render_.cmdPool_,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = render_.cmdBufferLen_,
  };
  result_ = (vkAllocateCommandBuffers(device.device_, &cmdBufferCreateInfo,
                                   render_.cmdBuffer_));

  assert(result_ == VK_SUCCESS);
  for (uint32_t bufferIndex = 0; bufferIndex < swapchain.swapchainLength_; bufferIndex++) {
    // We start by creating and declare the "beginning" our command buffer
    VkCommandBufferBeginInfo cmdBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr,
    };
    result_ = (vkBeginCommandBuffer(render_.cmdBuffer_[bufferIndex], &cmdBufferBeginInfo));
    assert(result_ == VK_SUCCESS);
    // transition the display image to color attachment layout
    setImageLayout(render_.cmdBuffer_[bufferIndex],
                   swapchain.displayImages_[bufferIndex],
                   VK_IMAGE_LAYOUT_UNDEFINED,
                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                   VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                   VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    // Now we start a renderpass. Any draw command has to be recorded in a
    // renderpass
    VkClearValue clearVals{ .color { .float32 {0.0f, 0.34f, 0.90f, 1.0f}}};
    VkRenderPassBeginInfo renderPassBeginInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = render_.renderPass_,
        .framebuffer = swapchain.framebuffers_[bufferIndex],
        .renderArea = {.offset { .x = 0, .y = 0,},
                       .extent = swapchain.displaySize_},
        .clearValueCount = 1,
        .pClearValues = &clearVals};
    vkCmdBeginRenderPass(render_.cmdBuffer_[bufferIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    // Bind what is necessary to the command buffer
    vkCmdBindPipeline(render_.cmdBuffer_[bufferIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, gfxPipeline.pipeline_);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(render_.cmdBuffer_[bufferIndex], 0, 1, &buffers.vertexBuf_, &offset);

    // Draw Triangle
    vkCmdDraw(render_.cmdBuffer_[bufferIndex], 3, 1, 0, 0);
    vkCmdEndRenderPass(render_.cmdBuffer_[bufferIndex]);
    result_ = (vkEndCommandBuffer(render_.cmdBuffer_[bufferIndex]));
    assert(result_ == VK_SUCCESS);
  }

  // We need to create a fence to be able, in the main loop, to wait for our
  // draw command(s) to finish before swapping the framebuffers
  VkFenceCreateInfo fenceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
  };
  result_ = (vkCreateFence(device.device_, &fenceCreateInfo, nullptr, &render_.fence_));
  assert(result_ == VK_SUCCESS);
  VkSemaphoreCreateInfo semaphoreCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
  };
  result_ = (vkCreateSemaphore(device.device_, &semaphoreCreateInfo, nullptr, &render_.semaphore_));
  assert(result_ == VK_SUCCESS);
  device.initialized_ = true;
}
void vulkan_graphics::needResize() {
  // To do
  resize = true;
}
void vulkan_graphics::render() {
  if (resize) { resize = false; }
  if (resume) { resume = false; running = true; }
  if (running && device.initialized_) {
    uint32_t nextIndex;
    result_ = vkAcquireNextImageKHR(device.device_, swapchain.swapchain_, UINT64_MAX, render_.semaphore_, VK_NULL_HANDLE, &nextIndex);
    assert(result_ == VK_SUCCESS);
    result_ = vkResetFences(device.device_, 1, &render_.fence_);
    assert(result_ == VK_SUCCESS);
    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .pNext = nullptr,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &render_.semaphore_,
      .pWaitDstStageMask = &waitStageMask,
      .commandBufferCount = 1,
      .pCommandBuffers = &render_.cmdBuffer_[nextIndex],
      .signalSemaphoreCount = 0,
      .pSignalSemaphores = nullptr,
    };
    result_ = vkQueueSubmit(device.queue_, 1, &submit_info, render_.fence_);
    assert(result_ == VK_SUCCESS);
    result_ = vkWaitForFences(device.device_, 1, &render_.fence_, VK_TRUE, 100000000);
    assert(result_ == VK_SUCCESS);
  
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .swapchainCount = 1,
        .pSwapchains = &swapchain.swapchain_,
        .pImageIndices = &nextIndex,
        .pResults = &result_,
    };
    vkQueuePresentKHR(device.queue_, &presentInfo);
    assert(result_ == VK_SUCCESS);
  }
  if (pause) {
    pause = false;
    running = false;
  }
  if (destroyed) {
    // To do 
  }
}
void vulkan_graphics::onWindowTerm() {
  vkFreeCommandBuffers(device.device_, render_.cmdPool_, render_.cmdBufferLen_, render_.cmdBuffer_);
  delete[] render_.cmdBuffer_;
  vkDestroyCommandPool(device.device_, render_.cmdPool_, nullptr);
  vkDestroyRenderPass(device.device_, render_.renderPass_, nullptr);
  // Delete Buffers
  vkDestroyBuffer(device.device_, buffers.vertexBuf_, nullptr);
  DeleteSwapChain();
  DeleteGraphicsPipeline();
  vkDestroyDevice(device.device_, nullptr);
  vkDestroyInstance(device.instance_, nullptr);
  device.initialized_ = false;
}
void vulkan_graphics::onPause() {
  // To do
  pause = true;
  render();
}
void vulkan_graphics::onDestroy() {
  // To do
  destroyed = true;
  render();
}
float vulkan_graphics::getWidth() { return 0;/* float(swapchain.displaySize_.width);*/ }
float vulkan_graphics::getHeight() { return 0;/*float(swapchain.displaySize_.height);*/ }
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
  //assert(InitVulkan());
  engine::graph = this;
}

vulkan_graphics::~vulkan_graphics() {
  //TermVulkan();
  engine::graph = nullptr;
}


