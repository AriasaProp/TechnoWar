R"(#version 300 es
#define LOW lowp
#define MED mediump
#ifdef GL_FRAGMENT_PRECISION_HIGH
	#define HIGH highp
#else
	#define HIGH mediump
#endif
layout(location = 0) out vec4 gl_FragColor;
uniform sampler2D u_texture;
in vec4 v_color;
in vec2 v_texCoord;
void main(){
  gl_FragColor = v_color * texture(u_texture, v_texCoord);
})"