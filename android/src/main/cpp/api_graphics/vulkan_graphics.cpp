#include "vulkan_graphics.hpp"

#include <vector>
#include <unordered_set>
#include <cassert>
#include "vulkan_wrapper.h"
// Global Variables ...
bool initialized_;
VkInstance instance_;
VkPhysicalDevice gpuDevice_;
VkDevice device_;
VkSurfaceKHR surface_;
VkResult result;

void vulkan_graphics::onResume() {
  // To do
}
void vulkan_graphics::onWindowInit(ANativeWindow *window)  {
  VkApplicationInfo appInfo = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pNext = nullptr,
      .pApplicationName = "tutorial01_load_vulkan",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "tutorial",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_MAKE_VERSION(1, 1, 0),
  };

  // prepare necessary extensions: Vulkan on Android need these to function
  std::vector<const char *> instanceExt, deviceExt;
  instanceExt.push_back("VK_KHR_surface");
  instanceExt.push_back("VK_KHR_android_surface");
  deviceExt.push_back("VK_KHR_swapchain");

  // Create the Vulkan instance
  VkInstanceCreateInfo instanceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = static_cast<uint32_t>(instanceExt.size()),
      .ppEnabledExtensionNames = instanceExt.data(),
  };
  result = (vkCreateInstance(&instanceCreateInfo, nullptr, &instance_));
  assert(VK_SUCCESS == result);
  // if we create a surface, we need the surface extension
  VkAndroidSurfaceCreateInfoKHR createInfo{
      .sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
      .pNext = nullptr,
      .flags = 0,
      .window = window};
  result = (vkCreateAndroidSurfaceKHR(instance_, &createInfo, nullptr, &surface_));
  assert(VK_SUCCESS == result);

  // Find one GPU to use:
  // On Android, every GPU device is equal -- supporting
  // graphics/compute/present
  // for this sample, we use the very first GPU device found on the system
  uint32_t gpuCount = 0;
  result = (vkEnumeratePhysicalDevices(instance_, &gpuCount, nullptr));
  assert(VK_SUCCESS == result);
  VkPhysicalDevice tmpGpus[gpuCount];
  result = (vkEnumeratePhysicalDevices(instance_, &gpuCount, tmpGpus));
  assert(VK_SUCCESS == result);
  gpuDevice_ = tmpGpus[0];  // Pick up the first GPU Device

  // check for vulkan info on this GPU device
  VkPhysicalDeviceProperties gpuProperties;
  vkGetPhysicalDeviceProperties(gpuDevice_, &gpuProperties);

  VkSurfaceCapabilitiesKHR surfaceCapabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpuDevice_, surface_, &surfaceCapabilities);

  // Find a GFX queue family
  uint32_t queueFamilyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(gpuDevice_, &queueFamilyCount, nullptr);
  assert(queueFamilyCount);
  std::vector<VkQueueFamilyProperties>  queueFamilyProperties(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(gpuDevice_, &queueFamilyCount, queueFamilyProperties.data());

  uint32_t queueFamilyIndex = 0;
  while (queueFamilyIndex < queueFamilyCount) {
    if (queueFamilyProperties[queueFamilyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) break;
    queueFamilyIndex++;
  }
  assert(queueFamilyIndex < queueFamilyCount);
  // Create a logical device from GPU we picked
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
      .enabledExtensionCount = static_cast<uint32_t>(deviceExt.size()),
      .ppEnabledExtensionNames = deviceExt.data(),
      .pEnabledFeatures = nullptr,
  };

  result = vkCreateDevice(gpuDevice_, &deviceCreateInfo, nullptr, &device_);
  assert(VK_SUCCESS == result);
  initialized_ = true;
  return 0;
}
void vulkan_graphics::needResize() {
  // To do
}
void vulkan_graphics::render() {
  
}
void vulkan_graphics::onWindowTerm() {
  vkDestroySurfaceKHR(instance_, surface_, nullptr);
  vkDestroyDevice(device_, nullptr);
  vkDestroyInstance(instance_, nullptr);
  
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
  if(!InitVulkan()) throw "error load libvulkan.so";
  initialized_ = false;
  engine::graph = this;
}

vulkan_graphics::~vulkan_graphics() {
  TermVulkan();
  engine::graph = nullptr;
}


