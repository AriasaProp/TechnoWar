#include "vulkan_graphics.hpp"

#define VK_USE_PLATFORM_ANDROID_KHR 1
#include "vulkan/vulkan.hpp"

// Vulkan call wrapper
#define CALL_VK(func)                                                 \
  if (VK_SUCCESS != (func)) {                                         \
    __android_log_print(ANDROID_LOG_ERROR, "Tutorial ",               \
                        "Vulkan error. File[%s], line[%d]", __FILE__, \
                        __LINE__);                                    \
    assert(false);                                                    \
  }

// Global Variables ...
VulkanDeviceInfo device;
VulkanSwapchainInfo swapchain;
VulkanBufferInfo buffers;
VulkanGfxPipelineInfo gfxPipeline;
VulkanRenderInfo render;

void vulkan_graphics::onResume() {
  // To do
}
bool vulkan_graphics::onWindowTerm(ANativeWindow *window) {
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
    CALL_VK(vkCreateInstance(&instanceCreateInfo, nullptr, &device.instance_));
    VkAndroidSurfaceCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .window = window
    };
    CALL_VK(vkCreateAndroidSurfaceKHR(device.instance_, &createInfo, nullptr, &device.surface_));
    uint32_t gpuCount = 0;
    CALL_VK(vkEnumeratePhysicalDevices(device.instance_, &gpuCount, nullptr));
    VkPhysicalDevice tmpGpus[gpuCount];
    CALL_VK(vkEnumeratePhysicalDevices(device.instance_, &gpuCount, tmpGpus));
    device.gpuDevice_ = tmpGpus[0];  // Pick up the first GPU Device
  
    // Find a GFX queue family
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device.gpuDevice_, &queueFamilyCount, nullptr);
    assert(queueFamilyCount);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device.gpuDevice_, &queueFamilyCount, queueFamilyProperties.data());
  
    uint32_t queueFamilyIndex;
    for (queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount;
         queueFamilyIndex++) {
      if (queueFamilyProperties[queueFamilyIndex].queueFlags &
          VK_QUEUE_GRAPHICS_BIT) {
        break;
      }
    }
    assert(queueFamilyIndex < queueFamilyCount);
    device.queueFamilyIndex_ = queueFamilyIndex;
  
    // Create a logical device (vulkan device)
    VkDeviceQueueCreateInfo queueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = queueFamilyIndex,
        .queueCount = 1,
        .pQueuePriorities = (float[]){1.0f}, 
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
    CALL_VK(vkCreateDevice(device.gpuDevice_, &deviceCreateInfo, nullptr, &device.device_));
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
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.gpuDevice_, device.surface_,
                                              &surfaceCapabilities);
    // Query the list of supported surface format and choose one we like
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.gpuDevice_, device.surface_,
                                         &formatCount, nullptr);
    VkSurfaceFormatKHR* formats = new VkSurfaceFormatKHR[formatCount];
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.gpuDevice_, device.surface_,
                                         &formatCount, formats);
  
    uint32_t chosenFormat;
    for (chosenFormat = 0; chosenFormat < formatCount; chosenFormat++) {
      if (formats[chosenFormat].format == VK_FORMAT_R8G8B8A8_UNORM) break;
    }
    assert(chosenFormat < formatCount);
  
    swapchain.displaySize_ = surfaceCapabilities.currentExtent;
    swapchain.displayFormat_ = formats[chosenFormat].format;
  
    VkSurfaceCapabilitiesKHR surfaceCap;
    CALL_VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.gpuDevice_, device.surface_, &surfaceCap));
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
    CALL_VK(vkCreateSwapchainKHR(device.device_, &swapchainCreateInfo, nullptr, &swapchain.swapchain_));
    // Get the length of the created swap chain
    CALL_VK(vkGetSwapchainImagesKHR(device.device_, swapchain.swapchain_, &swapchain.swapchainLength_, nullptr));
    delete[] formats;
  }

  // -----------------------------------------------------------------
  // Create render pass
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
      .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

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
  CALL_VK(vkCreateRenderPass(device.device_, &renderPassCreateInfo, nullptr, &render.renderPass_));

  // Create 2 frame buffers.
  {
    // query display attachment to swapchain
    uint32_t SwapchainImagesCount = 0;
    CALL_VK(vkGetSwapchainImagesKHR(device.device_, swapchain.swapchain_, &SwapchainImagesCount, nullptr));
    swapchain.displayImages_.resize(SwapchainImagesCount);
    CALL_VK(vkGetSwapchainImagesKHR(device.device_, swapchain.swapchain_, &SwapchainImagesCount, swapchain.displayImages_.data()));
  
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
      CALL_VK(vkCreateImageView(device.device_, &viewCreateInfo, nullptr, &swapchain.displayViews_[i]));
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
          .renderPass = render.renderPass_,
          .attachmentCount = 1,  // 2 if using depth
          .pAttachments = attachments,
          .width = static_cast<uint32_t>(swapchain.displaySize_.width),
          .height = static_cast<uint32_t>(swapchain.displaySize_.height),
         .layers = 1,
      };
      CALL_VK(vkCreateFramebuffer(device.device_, &fbCreateInfo, nullptr, &swapchain.framebuffers_[i]));
    }
  }
  // Create our vertex buffer
  {
    // -----------------------------------------------
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
  
    CALL_VK(vkCreateBuffer(device.device_, &createBufferInfo, nullptr, &buffers.vertexBuf_));
  
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
            return break;
          }
        }
        typeBits >>= 1;
      }
    }

    // Allocate memory for the buffer
    VkDeviceMemory deviceMemory;
    CALL_VK(vkAllocateMemory(device.device_, &allocInfo, nullptr, &deviceMemory));
  
    void* data;
    CALL_VK(vkMapMemory(device.device_, deviceMemory, 0, allocInfo.allocationSize,0, &data));
    memcpy(data, vertexData, sizeof(vertexData));
    vkUnmapMemory(device.device_, deviceMemory);
  
    CALL_VK(vkBindBufferMemory(device.device_, buffers.vertexBuf_, deviceMemory, 0));
    return true;
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
    CALL_VK(vkCreatePipelineLayout(device.device_, &pipelineLayoutCreateInfo, nullptr, &gfxPipeline.layout_));
    VkResult result;
    const char *vertexSrc =
      "#version 400\n"
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
    const char *fragmentSrc =
      "#version 400\n"
      "#extension GL_ARB_separate_shader_objects : enable\n"
      "#extension GL_ARB_shading_language_420pack : enable\n"
      "layout (location = 0) out vec4 uFragColor;\n"
      "void main() {\n"
      "   uFragColor = vec4(0.91, 0.26,  0.21, 1.0);\n"
      "}";
    shaderModuleCreateInfo.codeSize = strlen(fragmentSrc);
    shaderModuleCreateInfo.pCode =  (const uint32_t*)fragmentSrc;
    VkShaderModule fragmentShader;
    VkResult result = vkCreateShaderModule(device.device_, &shaderModuleCreateInfo, nullptr, &fragmentShader);
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
  
    CALL_VK(vkCreatePipelineCache(device.device_, &pipelineCacheInfo, nullptr,
                                  &gfxPipeline.cache_));
  
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
        .renderPass = render.renderPass_,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
    };
  
    VkResult pipelineResult = vkCreateGraphicsPipelines(device.device_, gfxPipeline.cache_, 1, &pipelineCreateInfo, nullptr, &gfxPipeline.pipeline_);
  
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
  CALL_VK(vkCreateCommandPool(device.device_, &cmdPoolCreateInfo, nullptr,
                              &render.cmdPool_));

  // Record a command buffer that just clear the screen
  // 1 command buffer draw in 1 framebuffer
  // In our case we need 2 command as we have 2 framebuffer
  render.cmdBufferLen_ = swapchain.swapchainLength_;
  render.cmdBuffer_ = new VkCommandBuffer[swapchain.swapchainLength_];
  VkCommandBufferAllocateInfo cmdBufferCreateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = render.cmdPool_,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = render.cmdBufferLen_,
  };
  CALL_VK(vkAllocateCommandBuffers(device.device_, &cmdBufferCreateInfo,
                                   render.cmdBuffer_));

  for (int bufferIndex = 0; bufferIndex < swapchain.swapchainLength_;
       bufferIndex++) {
    // We start by creating and declare the "beginning" our command buffer
    VkCommandBufferBeginInfo cmdBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr,
    };
    CALL_VK(vkBeginCommandBuffer(render.cmdBuffer_[bufferIndex], &cmdBufferBeginInfo));
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
      vkCmdPipelineBarrier(render.cmdBuffer_[bufferIndex], VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0, NULL, 1, &imageMemoryBarrier);
    }
    // Now we start a renderpass. Any draw command has to be recorded in a
    // renderpass
    VkClearValue clearVals{ .color { .float32 {0.0f, 0.34f, 0.90f, 1.0f}}};
    VkRenderPassBeginInfo renderPassBeginInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = render.renderPass_,
        .framebuffer = swapchain.framebuffers_[bufferIndex],
        .renderArea = {.offset { .x = 0, .y = 0,},
                       .extent = swapchain.displaySize_},
        .clearValueCount = 1,
        .pClearValues = &clearVals};
    vkCmdBeginRenderPass(render.cmdBuffer_[bufferIndex], &renderPassBeginInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
    // Bind what is necessary to the command buffer
    vkCmdBindPipeline(render.cmdBuffer_[bufferIndex],
                      VK_PIPELINE_BIND_POINT_GRAPHICS, gfxPipeline.pipeline_);
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(render.cmdBuffer_[bufferIndex], 0, 1,
                           &buffers.vertexBuf_, &offset);

    // Draw Triangle
    vkCmdDraw(render.cmdBuffer_[bufferIndex], 3, 1, 0, 0);

    vkCmdEndRenderPass(render.cmdBuffer_[bufferIndex]);

    CALL_VK(vkEndCommandBuffer(render.cmdBuffer_[bufferIndex]));
  }

  // We need to create a fence to be able, in the main loop, to wait for our
  // draw command(s) to finish before swapping the framebuffers
  VkFenceCreateInfo fenceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
  };
  CALL_VK(vkCreateFence(device.device_, &fenceCreateInfo, nullptr, &render.fence_));

  // We need to create a semaphore to be able to wait, in the main loop, for our
  // framebuffer to be available for us before drawing.
  VkSemaphoreCreateInfo semaphoreCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
  };
  CALL_VK(vkCreateSemaphore(device.device_, &semaphoreCreateInfo, nullptr,&render.semaphore_));

  device.initialized_ = true;
  return true;
}
void vulkan_graphics::needResize() {
  // To do
}
void vulkan_graphics::render() {
  if (!device.initialized_) return;
  uint32_t nextIndex;
  // Get the framebuffer index we should draw in
  CALL_VK(vkAcquireNextImageKHR(device.device_, swapchain.swapchain_, UINT64_MAX, render.semaphore_, VK_NULL_HANDLE, &nextIndex));
  CALL_VK(vkResetFences(device.device_, 1, &render.fence_));

  VkPipelineStageFlags waitStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  VkSubmitInfo submit_info = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                              .pNext = nullptr,
                              .waitSemaphoreCount = 1,
                              .pWaitSemaphores = &render.semaphore_,
                              .pWaitDstStageMask = &waitStageMask,
                              .commandBufferCount = 1,
                              .pCommandBuffers = &render.cmdBuffer_[nextIndex],
                              .signalSemaphoreCount = 0,
                              .pSignalSemaphores = nullptr};
  CALL_VK(vkQueueSubmit(device.queue_, 1, &submit_info, render.fence_));
  CALL_VK(vkWaitForFences(device.device_, 1, &render.fence_, VK_TRUE, 100000000));


  VkResult result;
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
  vkFreeCommandBuffers(device.device_, render.cmdPool_, render.cmdBufferLen_,render.cmdBuffer_);
  delete[] render.cmdBuffer_;

  vkDestroyCommandPool(device.device_, render.cmdPool_, nullptr);
  vkDestroyRenderPass(device.device_, render.renderPass_, nullptr);
  
  {
    //delete swapchain
    for (int i = 0; i < swapchain.swapchainLength_; i++) {
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

  device.initialized_ = false;
}

void vulkan_graphics::onPause() {
  // To do
}
void vulkan_graphics::onDestroy() {
  // To do
}

float vulkan_graphics::getWidth() { return 1440.0; }
float vulkan_graphics::getHeight() { return 720.0; }
void vulkan_graphics::clear(const unsigned int &) {
  // To do
}
void vulkan_graphics::clearcolor(const float &, const float &, const float &, const float &) {
  // To do
}
engine::texture_core *vulkan_graphics::gen_texture(const int &, const int &, unsigned char *){
  // To do
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
void vulkan_graphics::flat_render(engine::flat_vertex *, unsigned int) {
  // To do
}
engine::mesh_core *vulkan_graphics::gen_mesh(engine::mesh_core::data *,unsigned int,unsigned short *, unsigned int){
  // To do
}
void vulkan_graphics::mesh_render(engine::mesh_core **,const unsigned int &) {
  // To do
}
void vulkan_graphics::delete_mesh(engine::mesh_core *) {
	// To DO: 
}

vulkan_graphics::vulkan_graphics() {
  engine::graph = this;
}

vulkan_graphics::~vulkan_graphics() {
  engine::graph = nullptr;
}


