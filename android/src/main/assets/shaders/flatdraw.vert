#version 450
#define LOW lowp
#define MED mediump
#ifdef GL_FRAGMENT_PRECISION_HIGH
    #define HIGH highp
#else
    #define HIGH mediump
#endif

layout(binding = 0) uniform UniformBufferObject {
    mat4 proj;
} ubo;

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec2 a_texCoord;

layout(location = 0) out vec2 v_texCoord;

void main() {
    v_texCoord = a_texCoord;
    gl_Position = ubo.proj * a_position;
}