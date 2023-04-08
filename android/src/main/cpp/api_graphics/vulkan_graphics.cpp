#include "vulkan_graphics.hpp"

#include <vector>
#include <unordered_set>
#include <cassert>
#include "vulkan_wrapper.hpp"

void vulkan_graphics::onResume() {
  // To do
}
void vulkan_graphics::onWindowInit(ANativeWindow *window) {
  
}
void vulkan_graphics::needResize() {
  // To do
}
void vulkan_graphics::render() {
  
}
void vulkan_graphics::onWindowTerm() {
  
}
void vulkan_graphics::onPause() {
  // To do
}
void vulkan_graphics::onDestroy() {
  // To do
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
  if(!InitVulkan()) throw "error load libvulkan.so";
  initialized_ = false;
  engine::graph = this;
}

vulkan_graphics::~vulkan_graphics() {
  TermVulkan();
  engine::graph = nullptr;
}


