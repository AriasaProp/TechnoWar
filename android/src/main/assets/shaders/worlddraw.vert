#version 450
#define LOW lowp
#define MED mediump
#ifdef GL_FRAGMENT_PRECISION_HIGH
    #define HIGH highp
#else
    #define HIGH mediump
#endif

layout(binding = 0) uniform UniformBufferObject {
    mat4 worldview_proj;
    mat4 trans_proj;
} ubo;

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_color;

layout(location = 0) out vec4 v_color;

void main() {
    v_color = a_color;
    gl_Position = ubo.worldview_proj * ubo.trans_proj * a_position;
}