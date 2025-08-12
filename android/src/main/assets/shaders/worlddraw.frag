#version 300 es
#define LOW lowp
#define MED mediump
#ifdef GL_FRAGMENT_PRECISION_HIGH
    #define HIGH highp
#else
    #define HIGH mediump
#endif
precision MED float;
in vec4 v_color;
layout(location = 0) out vec4 fragColor;
void main() {
    fragColor = v_color;
}