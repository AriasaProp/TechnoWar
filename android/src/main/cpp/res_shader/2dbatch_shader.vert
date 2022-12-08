R"(#version 300 es
#define LOW lowp
#define MED mediump
#ifdef GL_FRAGMENT_PRECISION_HIGH
	#define HIGH highp
#else
	#define HIGH mediump
#endif
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_texCoord;
uniform mat4 u_projTrans;
out vec4 v_color;
out vec2 v_texCoord;
void main() {
  v_color = a_color;
  v_texCoord = a_texCoord;
  gl_Position =  u_projTrans * a_position;
})"