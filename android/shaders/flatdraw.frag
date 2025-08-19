#version 450
#define LOW lowp
#define MED mediump
#ifdef GL_FRAGMENT_PRECISION_HIGH
    #define HIGH highp
#else
    #define HIGH mediump
#endif

precision MED float;

layout(binding = 1) uniform sampler2D u_tex;

layout(location = 0) in vec2 v_texCoord;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = texture(u_tex, v_texCoord);
}