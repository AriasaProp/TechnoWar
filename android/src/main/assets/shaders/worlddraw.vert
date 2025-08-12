#version 300 es
#define LOW lowp
#define MED mediump
#ifdef GL_FRAGMENT_PRECISION_HIGH
    #define HIGH highp
#else
    #define HIGH mediump
#endif

uniform mat4 worldview_proj;
uniform mat4 trans_proj;

in vec4 a_position;
in vec4 a_color;
out vec4 v_color;

void main() {
    v_color = a_color;
    gl_Position = ubo.worldview_proj * ubo.trans_proj * a_position;
}