#ifndef __glad_h_
#define __glad_h_

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__SCITECH_SNAP__)
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif

#include "khrplatform.h"

extern void gladLoadGLES(void*(*)(const char*));

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef khronos_int8_t GLbyte;
typedef khronos_uint8_t GLubyte;
typedef khronos_int16_t GLshort;
typedef khronos_uint16_t GLushort;
typedef int GLint;
typedef unsigned int GLuint;
typedef khronos_int32_t GLclampx;
typedef int GLsizei;
typedef khronos_float_t GLfloat;
typedef khronos_float_t GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void *GLeglClientBufferEXT;
typedef void *GLeglImageOES;
typedef char GLchar;
typedef char GLcharARB;
typedef unsigned int GLhandleARB;
typedef khronos_uint16_t GLhalf;
typedef khronos_uint16_t GLhalfARB;
typedef khronos_int32_t GLfixed;
#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1060)
#if defined(__khrplatform_h_)
typedef khronos_intptr_t GLintptr;
#else
typedef long GLintptr;
#endif
#else
#if defined(__khrplatform_h_)
typedef khronos_intptr_t GLintptr;
#else
typedef ptrdiff_t GLintptr;
#endif
#endif
#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1060)
#if defined(__khrplatform_h_)
typedef khronos_intptr_t GLintptrARB;
#else
typedef long GLintptrARB;
#endif
#else
#if defined(__khrplatform_h_)
typedef khronos_intptr_t GLintptrARB;
#else
typedef ptrdiff_t GLintptrARB;
#endif
#endif
#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1060)
#if defined(__khrplatform_h_)
typedef khronos_ssize_t GLsizeiptr;
#else
typedef long GLsizeiptr;
#endif
#else
#if defined(__khrplatform_h_)
typedef khronos_ssize_t GLsizeiptr;
#else
typedef ptrdiff_t GLsizeiptr;
#endif
#endif
#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ > 1060)
#if defined(__khrplatform_h_)
typedef khronos_ssize_t GLsizeiptrARB;
#else
typedef long GLsizeiptrARB;
#endif
#else
#if defined(__khrplatform_h_)
typedef khronos_ssize_t GLsizeiptrARB;
#else
typedef ptrdiff_t GLsizeiptrARB;
#endif
#endif
typedef khronos_int64_t GLint64;
typedef khronos_int64_t GLint64EXT;
typedef khronos_uint64_t GLuint64;
typedef khronos_uint64_t GLuint64EXT;
typedef struct __GLsync *GLsync;;

typedef void (APIENTRY *GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
typedef void (APIENTRY *GLDEBUGPROCARB)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
typedef void (APIENTRY *GLDEBUGPROCAMD)(GLuint id,GLenum category,GLenum severity,GLsizei length,const GLchar *message,void *userParam);

#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_FALSE 0
#define GL_TRUE 1
#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006
#define GL_ZERO 0
#define GL_ONE 1
#define GL_SRC_COLOR 0x0300
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_ALPHA 0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_DST_COLOR 0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_SRC_ALPHA_SATURATE 0x0308
#define GL_FUNC_ADD 0x8006
#define GL_BLEND_EQUATION 0x8009
#define GL_BLEND_EQUATION_RGB 0x8009
#define GL_BLEND_EQUATION_ALPHA 0x883D
#define GL_FUNC_SUBTRACT 0x800A
#define GL_FUNC_REVERSE_SUBTRACT 0x800B
#define GL_BLEND_DST_RGB 0x80C8
#define GL_BLEND_SRC_RGB 0x80C9
#define GL_BLEND_DST_ALPHA 0x80CA
#define GL_BLEND_SRC_ALPHA 0x80CB
#define GL_CONSTANT_COLOR 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR 0x8002
#define GL_CONSTANT_ALPHA 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004
#define GL_BLEND_COLOR 0x8005
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_STREAM_DRAW 0x88E0
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_BUFFER_SIZE 0x8764
#define GL_BUFFER_USAGE 0x8765
#define GL_CURRENT_VERTEX_ATTRIB 0x8626
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_FRONT_AND_BACK 0x0408
#define GL_TEXTURE_2D 0x0DE1
#define GL_CULL_FACE 0x0B44
#define GL_BLEND 0x0BE2
#define GL_DITHER 0x0BD0
#define GL_STENCIL_TEST 0x0B90
#define GL_DEPTH_TEST 0x0B71
#define GL_SCISSOR_TEST 0x0C11
#define GL_POLYGON_OFFSET_FILL 0x8037
#define GL_SAMPLE_ALPHA_TO_COVERAGE 0x809E
#define GL_SAMPLE_COVERAGE 0x80A0
#define GL_NO_ERROR 0
#define GL_INVALID_ENUM 0x0500
#define GL_INVALID_VALUE 0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_OUT_OF_MEMORY 0x0505
#define GL_CW 0x0900
#define GL_CCW 0x0901
#define GL_LINE_WIDTH 0x0B21
#define GL_ALIASED_POINT_SIZE_RANGE 0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE 0x846E
#define GL_CULL_FACE_MODE 0x0B45
#define GL_FRONT_FACE 0x0B46
#define GL_DEPTH_RANGE 0x0B70
#define GL_DEPTH_WRITEMASK 0x0B72
#define GL_DEPTH_CLEAR_VALUE 0x0B73
#define GL_DEPTH_FUNC 0x0B74
#define GL_STENCIL_CLEAR_VALUE 0x0B91
#define GL_STENCIL_FUNC 0x0B92
#define GL_STENCIL_FAIL 0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL 0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS 0x0B96
#define GL_STENCIL_REF 0x0B97
#define GL_STENCIL_VALUE_MASK 0x0B93
#define GL_STENCIL_WRITEMASK 0x0B98
#define GL_STENCIL_BACK_FUNC 0x8800
#define GL_STENCIL_BACK_FAIL 0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL 0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS 0x8803
#define GL_STENCIL_BACK_REF 0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK 0x8CA4
#define GL_STENCIL_BACK_WRITEMASK 0x8CA5
#define GL_VIEWPORT 0x0BA2
#define GL_SCISSOR_BOX 0x0C10
#define GL_COLOR_CLEAR_VALUE 0x0C22
#define GL_COLOR_WRITEMASK 0x0C23
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_MAX_TEXTURE_SIZE 0x0D33
#define GL_MAX_VIEWPORT_DIMS 0x0D3A
#define GL_SUBPIXEL_BITS 0x0D50
#define GL_RED_BITS 0x0D52
#define GL_GREEN_BITS 0x0D53
#define GL_BLUE_BITS 0x0D54
#define GL_ALPHA_BITS 0x0D55
#define GL_DEPTH_BITS 0x0D56
#define GL_STENCIL_BITS 0x0D57
#define GL_POLYGON_OFFSET_UNITS 0x2A00
#define GL_POLYGON_OFFSET_FACTOR 0x8038
#define GL_TEXTURE_BINDING_2D 0x8069
#define GL_SAMPLE_BUFFERS 0x80A8
#define GL_SAMPLES 0x80A9
#define GL_SAMPLE_COVERAGE_VALUE 0x80AA
#define GL_SAMPLE_COVERAGE_INVERT 0x80AB
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS 0x86A3
#define GL_DONT_CARE 0x1100
#define GL_FASTEST 0x1101
#define GL_NICEST 0x1102
#define GL_GENERATE_MIPMAP_HINT 0x8192
#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_FIXED 0x140C
#define GL_DEPTH_COMPONENT 0x1902
#define GL_ALPHA 0x1906
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_LUMINANCE 0x1909
#define GL_LUMINANCE_ALPHA 0x190A
#define GL_UNSIGNED_SHORT_4_4_4_4 0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1 0x8034
#define GL_UNSIGNED_SHORT_5_6_5 0x8363
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_MAX_VERTEX_ATTRIBS 0x8869
#define GL_MAX_VERTEX_UNIFORM_VECTORS 0x8DFB
#define GL_MAX_VARYING_VECTORS 0x8DFC
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_TEXTURE_IMAGE_UNITS 0x8872
#define GL_MAX_FRAGMENT_UNIFORM_VECTORS 0x8DFD
#define GL_SHADER_TYPE 0x8B4F
#define GL_DELETE_STATUS 0x8B80
#define GL_LINK_STATUS 0x8B82
#define GL_VALIDATE_STATUS 0x8B83
#define GL_ATTACHED_SHADERS 0x8B85
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH 0x8B87
#define GL_ACTIVE_ATTRIBUTES 0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH 0x8B8A
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_CURRENT_PROGRAM 0x8B8D
#define GL_NEVER 0x0200
#define GL_LESS 0x0201
#define GL_EQUAL 0x0202
#define GL_LEQUAL 0x0203
#define GL_GREATER 0x0204
#define GL_NOTEQUAL 0x0205
#define GL_GEQUAL 0x0206
#define GL_ALWAYS 0x0207
#define GL_KEEP 0x1E00
#define GL_REPLACE 0x1E01
#define GL_INCR 0x1E02
#define GL_DECR 0x1E03
#define GL_INVERT 0x150A
#define GL_INCR_WRAP 0x8507
#define GL_DECR_WRAP 0x8508
#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
#define GL_EXTENSIONS 0x1F03
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_TEXTURE 0x1702
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP 0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE 0x851C
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_TEXTURE5 0x84C5
#define GL_TEXTURE6 0x84C6
#define GL_TEXTURE7 0x84C7
#define GL_TEXTURE8 0x84C8
#define GL_TEXTURE9 0x84C9
#define GL_TEXTURE10 0x84CA
#define GL_TEXTURE11 0x84CB
#define GL_TEXTURE12 0x84CC
#define GL_TEXTURE13 0x84CD
#define GL_TEXTURE14 0x84CE
#define GL_TEXTURE15 0x84CF
#define GL_TEXTURE16 0x84D0
#define GL_TEXTURE17 0x84D1
#define GL_TEXTURE18 0x84D2
#define GL_TEXTURE19 0x84D3
#define GL_TEXTURE20 0x84D4
#define GL_TEXTURE21 0x84D5
#define GL_TEXTURE22 0x84D6
#define GL_TEXTURE23 0x84D7
#define GL_TEXTURE24 0x84D8
#define GL_TEXTURE25 0x84D9
#define GL_TEXTURE26 0x84DA
#define GL_TEXTURE27 0x84DB
#define GL_TEXTURE28 0x84DC
#define GL_TEXTURE29 0x84DD
#define GL_TEXTURE30 0x84DE
#define GL_TEXTURE31 0x84DF
#define GL_ACTIVE_TEXTURE 0x84E0
#define GL_REPEAT 0x2901
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_MIRRORED_REPEAT 0x8370
#define GL_FLOAT_VEC2 0x8B50
#define GL_FLOAT_VEC3 0x8B51
#define GL_FLOAT_VEC4 0x8B52
#define GL_INT_VEC2 0x8B53
#define GL_INT_VEC3 0x8B54
#define GL_INT_VEC4 0x8B55
#define GL_BOOL 0x8B56
#define GL_BOOL_VEC2 0x8B57
#define GL_BOOL_VEC3 0x8B58
#define GL_BOOL_VEC4 0x8B59
#define GL_FLOAT_MAT2 0x8B5A
#define GL_FLOAT_MAT3 0x8B5B
#define GL_FLOAT_MAT4 0x8B5C
#define GL_SAMPLER_2D 0x8B5E
#define GL_SAMPLER_CUBE 0x8B60
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE 0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE 0x8625
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_VERTEX_ATTRIB_ARRAY_POINTER 0x8645
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_IMPLEMENTATION_COLOR_READ_TYPE 0x8B9A
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT 0x8B9B
#define GL_COMPILE_STATUS 0x8B81
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_SHADER_SOURCE_LENGTH 0x8B88
#define GL_SHADER_COMPILER 0x8DFA
#define GL_SHADER_BINARY_FORMATS 0x8DF8
#define GL_NUM_SHADER_BINARY_FORMATS 0x8DF9
#define GL_LOW_FLOAT 0x8DF0
#define GL_MEDIUM_FLOAT 0x8DF1
#define GL_HIGH_FLOAT 0x8DF2
#define GL_LOW_INT 0x8DF3
#define GL_MEDIUM_INT 0x8DF4
#define GL_HIGH_INT 0x8DF5
#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_RGBA4 0x8056
#define GL_RGB5_A1 0x8057
#define GL_RGB565 0x8D62
#define GL_DEPTH_COMPONENT16 0x81A5
#define GL_STENCIL_INDEX8 0x8D48
#define GL_RENDERBUFFER_WIDTH 0x8D42
#define GL_RENDERBUFFER_HEIGHT 0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT 0x8D44
#define GL_RENDERBUFFER_RED_SIZE 0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE 0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE 0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE 0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE 0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE 0x8D55
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL 0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_STENCIL_ATTACHMENT 0x8D20
#define GL_NONE 0
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS 0x8CD9
#define GL_FRAMEBUFFER_UNSUPPORTED 0x8CDD
#define GL_FRAMEBUFFER_BINDING 0x8CA6
#define GL_RENDERBUFFER_BINDING 0x8CA7
#define GL_MAX_RENDERBUFFER_SIZE 0x84E8
#define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506
#define GL_READ_BUFFER 0x0C02
#define GL_UNPACK_ROW_LENGTH 0x0CF2
#define GL_UNPACK_SKIP_ROWS 0x0CF3
#define GL_UNPACK_SKIP_PIXELS 0x0CF4
#define GL_PACK_ROW_LENGTH 0x0D02
#define GL_PACK_SKIP_ROWS 0x0D03
#define GL_PACK_SKIP_PIXELS 0x0D04
#define GL_COLOR 0x1800
#define GL_DEPTH 0x1801
#define GL_STENCIL 0x1802
#define GL_RED 0x1903
#define GL_RGB8 0x8051
#define GL_RGBA8 0x8058
#define GL_RGB10_A2 0x8059
#define GL_TEXTURE_BINDING_3D 0x806A
#define GL_UNPACK_SKIP_IMAGES 0x806D
#define GL_UNPACK_IMAGE_HEIGHT 0x806E
#define GL_TEXTURE_3D 0x806F
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_MAX_3D_TEXTURE_SIZE 0x8073
#define GL_UNSIGNED_INT_2_10_10_10_REV 0x8368
#define GL_MAX_ELEMENTS_VERTICES 0x80E8
#define GL_MAX_ELEMENTS_INDICES 0x80E9
#define GL_TEXTURE_MIN_LOD 0x813A
#define GL_TEXTURE_MAX_LOD 0x813B
#define GL_TEXTURE_BASE_LEVEL 0x813C
#define GL_TEXTURE_MAX_LEVEL 0x813D
#define GL_MIN 0x8007
#define GL_MAX 0x8008
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_MAX_TEXTURE_LOD_BIAS 0x84FD
#define GL_TEXTURE_COMPARE_MODE 0x884C
#define GL_TEXTURE_COMPARE_FUNC 0x884D
#define GL_CURRENT_QUERY 0x8865
#define GL_QUERY_RESULT 0x8866
#define GL_QUERY_RESULT_AVAILABLE 0x8867
#define GL_BUFFER_MAPPED 0x88BC
#define GL_BUFFER_MAP_POINTER 0x88BD
#define GL_STREAM_READ 0x88E1
#define GL_STREAM_COPY 0x88E2
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA
#define GL_MAX_DRAW_BUFFERS 0x8824
#define GL_DRAW_BUFFER0 0x8825
#define GL_DRAW_BUFFER1 0x8826
#define GL_DRAW_BUFFER2 0x8827
#define GL_DRAW_BUFFER3 0x8828
#define GL_DRAW_BUFFER4 0x8829
#define GL_DRAW_BUFFER5 0x882A
#define GL_DRAW_BUFFER6 0x882B
#define GL_DRAW_BUFFER7 0x882C
#define GL_DRAW_BUFFER8 0x882D
#define GL_DRAW_BUFFER9 0x882E
#define GL_DRAW_BUFFER10 0x882F
#define GL_DRAW_BUFFER11 0x8830
#define GL_DRAW_BUFFER12 0x8831
#define GL_DRAW_BUFFER13 0x8832
#define GL_DRAW_BUFFER14 0x8833
#define GL_DRAW_BUFFER15 0x8834
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS 0x8B4A
#define GL_SAMPLER_3D 0x8B5F
#define GL_SAMPLER_2D_SHADOW 0x8B62
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_UNPACK_BUFFER 0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING 0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING 0x88EF
#define GL_FLOAT_MAT2x3 0x8B65
#define GL_FLOAT_MAT2x4 0x8B66
#define GL_FLOAT_MAT3x2 0x8B67
#define GL_FLOAT_MAT3x4 0x8B68
#define GL_FLOAT_MAT4x2 0x8B69
#define GL_FLOAT_MAT4x3 0x8B6A
#define GL_SRGB 0x8C40
#define GL_SRGB8 0x8C41
#define GL_SRGB8_ALPHA8 0x8C43
#define GL_COMPARE_REF_TO_TEXTURE 0x884E
#define GL_MAJOR_VERSION 0x821B
#define GL_MINOR_VERSION 0x821C
#define GL_NUM_EXTENSIONS 0x821D
#define GL_RGBA32F 0x8814
#define GL_RGB32F 0x8815
#define GL_RGBA16F 0x881A
#define GL_RGB16F 0x881B
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER 0x88FD
#define GL_MAX_ARRAY_TEXTURE_LAYERS 0x88FF
#define GL_MIN_PROGRAM_TEXEL_OFFSET 0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET 0x8905
#define GL_MAX_VARYING_COMPONENTS 0x8B4B
#define GL_TEXTURE_2D_ARRAY 0x8C1A
#define GL_TEXTURE_BINDING_2D_ARRAY 0x8C1D
#define GL_R11F_G11F_B10F 0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV 0x8C3B
#define GL_RGB9_E5 0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV 0x8C3E
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH 0x8C76
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE 0x8C7F
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS 0x8C80
#define GL_TRANSFORM_FEEDBACK_VARYINGS 0x8C83
#define GL_TRANSFORM_FEEDBACK_BUFFER_START 0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE 0x8C85
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN 0x8C88
#define GL_RASTERIZER_DISCARD 0x8C89
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS 0x8C8B
#define GL_INTERLEAVED_ATTRIBS 0x8C8C
#define GL_SEPARATE_ATTRIBS 0x8C8D
#define GL_TRANSFORM_FEEDBACK_BUFFER 0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING 0x8C8F
#define GL_RGBA32UI 0x8D70
#define GL_RGB32UI 0x8D71
#define GL_RGBA16UI 0x8D76
#define GL_RGB16UI 0x8D77
#define GL_RGBA8UI 0x8D7C
#define GL_RGB8UI 0x8D7D
#define GL_RGBA32I 0x8D82
#define GL_RGB32I 0x8D83
#define GL_RGBA16I 0x8D88
#define GL_RGB16I 0x8D89
#define GL_RGBA8I 0x8D8E
#define GL_RGB8I 0x8D8F
#define GL_RED_INTEGER 0x8D94
#define GL_RGB_INTEGER 0x8D98
#define GL_RGBA_INTEGER 0x8D99
#define GL_SAMPLER_2D_ARRAY 0x8DC1
#define GL_SAMPLER_2D_ARRAY_SHADOW 0x8DC4
#define GL_SAMPLER_CUBE_SHADOW 0x8DC5
#define GL_UNSIGNED_INT_VEC2 0x8DC6
#define GL_UNSIGNED_INT_VEC3 0x8DC7
#define GL_UNSIGNED_INT_VEC4 0x8DC8
#define GL_INT_SAMPLER_2D 0x8DCA
#define GL_INT_SAMPLER_3D 0x8DCB
#define GL_INT_SAMPLER_CUBE 0x8DCC
#define GL_INT_SAMPLER_2D_ARRAY 0x8DCF
#define GL_UNSIGNED_INT_SAMPLER_2D 0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_3D 0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_CUBE 0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY 0x8DD7
#define GL_BUFFER_ACCESS_FLAGS 0x911F
#define GL_BUFFER_MAP_LENGTH 0x9120
#define GL_BUFFER_MAP_OFFSET 0x9121
#define GL_DEPTH_COMPONENT32F 0x8CAC
#define GL_DEPTH32F_STENCIL8 0x8CAD
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV 0x8DAD
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE 0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE 0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE 0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE 0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE 0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE 0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE 0x8217
#define GL_FRAMEBUFFER_DEFAULT 0x8218
#define GL_FRAMEBUFFER_UNDEFINED 0x8219
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#define GL_DEPTH_STENCIL 0x84F9
#define GL_UNSIGNED_INT_24_8 0x84FA
#define GL_DEPTH24_STENCIL8 0x88F0
#define GL_UNSIGNED_NORMALIZED 0x8C17
#define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING 0x8CAA
#define GL_RENDERBUFFER_SAMPLES 0x8CAB
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER 0x8CD4
#define GL_MAX_COLOR_ATTACHMENTS 0x8CDF
#define GL_COLOR_ATTACHMENT1 0x8CE1
#define GL_COLOR_ATTACHMENT2 0x8CE2
#define GL_COLOR_ATTACHMENT3 0x8CE3
#define GL_COLOR_ATTACHMENT4 0x8CE4
#define GL_COLOR_ATTACHMENT5 0x8CE5
#define GL_COLOR_ATTACHMENT6 0x8CE6
#define GL_COLOR_ATTACHMENT7 0x8CE7
#define GL_COLOR_ATTACHMENT8 0x8CE8
#define GL_COLOR_ATTACHMENT9 0x8CE9
#define GL_COLOR_ATTACHMENT10 0x8CEA
#define GL_COLOR_ATTACHMENT11 0x8CEB
#define GL_COLOR_ATTACHMENT12 0x8CEC
#define GL_COLOR_ATTACHMENT13 0x8CED
#define GL_COLOR_ATTACHMENT14 0x8CEE
#define GL_COLOR_ATTACHMENT15 0x8CEF
#define GL_COLOR_ATTACHMENT16 0x8CF0
#define GL_COLOR_ATTACHMENT17 0x8CF1
#define GL_COLOR_ATTACHMENT18 0x8CF2
#define GL_COLOR_ATTACHMENT19 0x8CF3
#define GL_COLOR_ATTACHMENT20 0x8CF4
#define GL_COLOR_ATTACHMENT21 0x8CF5
#define GL_COLOR_ATTACHMENT22 0x8CF6
#define GL_COLOR_ATTACHMENT23 0x8CF7
#define GL_COLOR_ATTACHMENT24 0x8CF8
#define GL_COLOR_ATTACHMENT25 0x8CF9
#define GL_COLOR_ATTACHMENT26 0x8CFA
#define GL_COLOR_ATTACHMENT27 0x8CFB
#define GL_COLOR_ATTACHMENT28 0x8CFC
#define GL_COLOR_ATTACHMENT29 0x8CFD
#define GL_COLOR_ATTACHMENT30 0x8CFE
#define GL_COLOR_ATTACHMENT31 0x8CFF
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
#define GL_MAX_SAMPLES 0x8D57
#define GL_HALF_FLOAT 0x140B
#define GL_MAP_READ_BIT 0x0001
#define GL_MAP_WRITE_BIT 0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT 0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT 0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT 0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT 0x0020
#define GL_RG 0x8227
#define GL_RG_INTEGER 0x8228
#define GL_R8 0x8229
#define GL_RG8 0x822B
#define GL_R16F 0x822D
#define GL_R32F 0x822E
#define GL_RG16F 0x822F
#define GL_RG32F 0x8230
#define GL_R8I 0x8231
#define GL_R8UI 0x8232
#define GL_R16I 0x8233
#define GL_R16UI 0x8234
#define GL_R32I 0x8235
#define GL_R32UI 0x8236
#define GL_RG8I 0x8237
#define GL_RG8UI 0x8238
#define GL_RG16I 0x8239
#define GL_RG16UI 0x823A
#define GL_RG32I 0x823B
#define GL_RG32UI 0x823C
#define GL_VERTEX_ARRAY_BINDING 0x85B5
#define GL_R8_SNORM 0x8F94
#define GL_RG8_SNORM 0x8F95
#define GL_RGB8_SNORM 0x8F96
#define GL_RGBA8_SNORM 0x8F97
#define GL_SIGNED_NORMALIZED 0x8F9C
#define GL_PRIMITIVE_RESTART_FIXED_INDEX 0x8D69
#define GL_COPY_READ_BUFFER 0x8F36
#define GL_COPY_WRITE_BUFFER 0x8F37
#define GL_COPY_READ_BUFFER_BINDING 0x8F36
#define GL_COPY_WRITE_BUFFER_BINDING 0x8F37
#define GL_UNIFORM_BUFFER 0x8A11
#define GL_UNIFORM_BUFFER_BINDING 0x8A28
#define GL_UNIFORM_BUFFER_START 0x8A29
#define GL_UNIFORM_BUFFER_SIZE 0x8A2A
#define GL_MAX_VERTEX_UNIFORM_BLOCKS 0x8A2B
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS 0x8A2D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS 0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS 0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE 0x8A30
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS 0x8A31
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS 0x8A33
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT 0x8A34
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH 0x8A35
#define GL_ACTIVE_UNIFORM_BLOCKS 0x8A36
#define GL_UNIFORM_TYPE 0x8A37
#define GL_UNIFORM_SIZE 0x8A38
#define GL_UNIFORM_NAME_LENGTH 0x8A39
#define GL_UNIFORM_BLOCK_INDEX 0x8A3A
#define GL_UNIFORM_OFFSET 0x8A3B
#define GL_UNIFORM_ARRAY_STRIDE 0x8A3C
#define GL_UNIFORM_MATRIX_STRIDE 0x8A3D
#define GL_UNIFORM_IS_ROW_MAJOR 0x8A3E
#define GL_UNIFORM_BLOCK_BINDING 0x8A3F
#define GL_UNIFORM_BLOCK_DATA_SIZE 0x8A40
#define GL_UNIFORM_BLOCK_NAME_LENGTH 0x8A41
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS 0x8A42
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES 0x8A43
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER 0x8A44
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER 0x8A46
#define GL_INVALID_INDEX 0xFFFFFFFF
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS 0x9122
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS 0x9125
#define GL_MAX_SERVER_WAIT_TIMEOUT 0x9111
#define GL_OBJECT_TYPE 0x9112
#define GL_SYNC_CONDITION 0x9113
#define GL_SYNC_STATUS 0x9114
#define GL_SYNC_FLAGS 0x9115
#define GL_SYNC_FENCE 0x9116
#define GL_SYNC_GPU_COMMANDS_COMPLETE 0x9117
#define GL_UNSIGNALED 0x9118
#define GL_SIGNALED 0x9119
#define GL_ALREADY_SIGNALED 0x911A
#define GL_TIMEOUT_EXPIRED 0x911B
#define GL_CONDITION_SATISFIED 0x911C
#define GL_WAIT_FAILED 0x911D
#define GL_SYNC_FLUSH_COMMANDS_BIT 0x00000001
#define GL_TIMEOUT_IGNORED 0xFFFFFFFFFFFFFFFF
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR 0x88FE
#define GL_ANY_SAMPLES_PASSED 0x8C2F
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE 0x8D6A
#define GL_SAMPLER_BINDING 0x8919
#define GL_RGB10_A2UI 0x906F
#define GL_TEXTURE_SWIZZLE_R 0x8E42
#define GL_TEXTURE_SWIZZLE_G 0x8E43
#define GL_TEXTURE_SWIZZLE_B 0x8E44
#define GL_TEXTURE_SWIZZLE_A 0x8E45
#define GL_GREEN 0x1904
#define GL_BLUE 0x1905
#define GL_INT_2_10_10_10_REV 0x8D9F
#define GL_TRANSFORM_FEEDBACK 0x8E22
#define GL_TRANSFORM_FEEDBACK_PAUSED 0x8E23
#define GL_TRANSFORM_FEEDBACK_ACTIVE 0x8E24
#define GL_TRANSFORM_FEEDBACK_BINDING 0x8E25
#define GL_PROGRAM_BINARY_RETRIEVABLE_HINT 0x8257
#define GL_PROGRAM_BINARY_LENGTH 0x8741
#define GL_NUM_PROGRAM_BINARY_FORMATS 0x87FE
#define GL_PROGRAM_BINARY_FORMATS 0x87FF
#define GL_COMPRESSED_R11_EAC 0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC 0x9271
#define GL_COMPRESSED_RG11_EAC 0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC 0x9273
#define GL_COMPRESSED_RGB8_ETC2 0x9274
#define GL_COMPRESSED_SRGB8_ETC2 0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC 0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC 0x9279
#define GL_TEXTURE_IMMUTABLE_FORMAT 0x912F
#define GL_MAX_ELEMENT_INDEX 0x8D6B
#define GL_NUM_SAMPLE_COUNTS 0x9380
#define GL_TEXTURE_IMMUTABLE_LEVELS 0x82DF
#define GL_COMPUTE_SHADER 0x91B9
#define GL_MAX_COMPUTE_UNIFORM_BLOCKS 0x91BB
#define GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS 0x91BC
#define GL_MAX_COMPUTE_IMAGE_UNIFORMS 0x91BD
#define GL_MAX_COMPUTE_SHARED_MEMORY_SIZE 0x8262
#define GL_MAX_COMPUTE_UNIFORM_COMPONENTS 0x8263
#define GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS 0x8264
#define GL_MAX_COMPUTE_ATOMIC_COUNTERS 0x8265
#define GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS 0x8266
#define GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS 0x90EB
#define GL_MAX_COMPUTE_WORK_GROUP_COUNT 0x91BE
#define GL_MAX_COMPUTE_WORK_GROUP_SIZE 0x91BF
#define GL_COMPUTE_WORK_GROUP_SIZE 0x8267
#define GL_DISPATCH_INDIRECT_BUFFER 0x90EE
#define GL_DISPATCH_INDIRECT_BUFFER_BINDING 0x90EF
#define GL_COMPUTE_SHADER_BIT 0x00000020
#define GL_DRAW_INDIRECT_BUFFER 0x8F3F
#define GL_DRAW_INDIRECT_BUFFER_BINDING 0x8F43
#define GL_MAX_UNIFORM_LOCATIONS 0x826E
#define GL_FRAMEBUFFER_DEFAULT_WIDTH 0x9310
#define GL_FRAMEBUFFER_DEFAULT_HEIGHT 0x9311
#define GL_FRAMEBUFFER_DEFAULT_SAMPLES 0x9313
#define GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS 0x9314
#define GL_MAX_FRAMEBUFFER_WIDTH 0x9315
#define GL_MAX_FRAMEBUFFER_HEIGHT 0x9316
#define GL_MAX_FRAMEBUFFER_SAMPLES 0x9318
#define GL_UNIFORM 0x92E1
#define GL_UNIFORM_BLOCK 0x92E2
#define GL_PROGRAM_INPUT 0x92E3
#define GL_PROGRAM_OUTPUT 0x92E4
#define GL_BUFFER_VARIABLE 0x92E5
#define GL_SHADER_STORAGE_BLOCK 0x92E6
#define GL_ATOMIC_COUNTER_BUFFER 0x92C0
#define GL_TRANSFORM_FEEDBACK_VARYING 0x92F4
#define GL_ACTIVE_RESOURCES 0x92F5
#define GL_MAX_NAME_LENGTH 0x92F6
#define GL_MAX_NUM_ACTIVE_VARIABLES 0x92F7
#define GL_NAME_LENGTH 0x92F9
#define GL_TYPE 0x92FA
#define GL_ARRAY_SIZE 0x92FB
#define GL_OFFSET 0x92FC
#define GL_BLOCK_INDEX 0x92FD
#define GL_ARRAY_STRIDE 0x92FE
#define GL_MATRIX_STRIDE 0x92FF
#define GL_IS_ROW_MAJOR 0x9300
#define GL_ATOMIC_COUNTER_BUFFER_INDEX 0x9301
#define GL_BUFFER_BINDING 0x9302
#define GL_BUFFER_DATA_SIZE 0x9303
#define GL_NUM_ACTIVE_VARIABLES 0x9304
#define GL_ACTIVE_VARIABLES 0x9305
#define GL_REFERENCED_BY_VERTEX_SHADER 0x9306
#define GL_REFERENCED_BY_FRAGMENT_SHADER 0x930A
#define GL_REFERENCED_BY_COMPUTE_SHADER 0x930B
#define GL_TOP_LEVEL_ARRAY_SIZE 0x930C
#define GL_TOP_LEVEL_ARRAY_STRIDE 0x930D
#define GL_LOCATION 0x930E
#define GL_VERTEX_SHADER_BIT 0x00000001
#define GL_FRAGMENT_SHADER_BIT 0x00000002
#define GL_ALL_SHADER_BITS 0xFFFFFFFF
#define GL_PROGRAM_SEPARABLE 0x8258
#define GL_ACTIVE_PROGRAM 0x8259
#define GL_PROGRAM_PIPELINE_BINDING 0x825A
#define GL_ATOMIC_COUNTER_BUFFER_BINDING 0x92C1
#define GL_ATOMIC_COUNTER_BUFFER_START 0x92C2
#define GL_ATOMIC_COUNTER_BUFFER_SIZE 0x92C3
#define GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS 0x92CC
#define GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS 0x92D0
#define GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS 0x92D1
#define GL_MAX_VERTEX_ATOMIC_COUNTERS 0x92D2
#define GL_MAX_FRAGMENT_ATOMIC_COUNTERS 0x92D6
#define GL_MAX_COMBINED_ATOMIC_COUNTERS 0x92D7
#define GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE 0x92D8
#define GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS 0x92DC
#define GL_ACTIVE_ATOMIC_COUNTER_BUFFERS 0x92D9
#define GL_UNSIGNED_INT_ATOMIC_COUNTER 0x92DB
#define GL_MAX_IMAGE_UNITS 0x8F38
#define GL_MAX_VERTEX_IMAGE_UNIFORMS 0x90CA
#define GL_MAX_FRAGMENT_IMAGE_UNIFORMS 0x90CE
#define GL_MAX_COMBINED_IMAGE_UNIFORMS 0x90CF
#define GL_IMAGE_BINDING_NAME 0x8F3A
#define GL_IMAGE_BINDING_LEVEL 0x8F3B
#define GL_IMAGE_BINDING_LAYERED 0x8F3C
#define GL_IMAGE_BINDING_LAYER 0x8F3D
#define GL_IMAGE_BINDING_ACCESS 0x8F3E
#define GL_IMAGE_BINDING_FORMAT 0x906E
#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 0x00000001
#define GL_ELEMENT_ARRAY_BARRIER_BIT 0x00000002
#define GL_UNIFORM_BARRIER_BIT 0x00000004
#define GL_TEXTURE_FETCH_BARRIER_BIT 0x00000008
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#define GL_COMMAND_BARRIER_BIT 0x00000040
#define GL_PIXEL_BUFFER_BARRIER_BIT 0x00000080
#define GL_TEXTURE_UPDATE_BARRIER_BIT 0x00000100
#define GL_BUFFER_UPDATE_BARRIER_BIT 0x00000200
#define GL_FRAMEBUFFER_BARRIER_BIT 0x00000400
#define GL_TRANSFORM_FEEDBACK_BARRIER_BIT 0x00000800
#define GL_ATOMIC_COUNTER_BARRIER_BIT 0x00001000
#define GL_ALL_BARRIER_BITS 0xFFFFFFFF
#define GL_IMAGE_2D 0x904D
#define GL_IMAGE_3D 0x904E
#define GL_IMAGE_CUBE 0x9050
#define GL_IMAGE_2D_ARRAY 0x9053
#define GL_INT_IMAGE_2D 0x9058
#define GL_INT_IMAGE_3D 0x9059
#define GL_INT_IMAGE_CUBE 0x905B
#define GL_INT_IMAGE_2D_ARRAY 0x905E
#define GL_UNSIGNED_INT_IMAGE_2D 0x9063
#define GL_UNSIGNED_INT_IMAGE_3D 0x9064
#define GL_UNSIGNED_INT_IMAGE_CUBE 0x9066
#define GL_UNSIGNED_INT_IMAGE_2D_ARRAY 0x9069
#define GL_IMAGE_FORMAT_COMPATIBILITY_TYPE 0x90C7
#define GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE 0x90C8
#define GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS 0x90C9
#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#define GL_SHADER_STORAGE_BUFFER_BINDING 0x90D3
#define GL_SHADER_STORAGE_BUFFER_START 0x90D4
#define GL_SHADER_STORAGE_BUFFER_SIZE 0x90D5
#define GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS 0x90D6
#define GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS 0x90DA
#define GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS 0x90DB
#define GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS 0x90DC
#define GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS 0x90DD
#define GL_MAX_SHADER_STORAGE_BLOCK_SIZE 0x90DE
#define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT 0x90DF
#define GL_SHADER_STORAGE_BARRIER_BIT 0x00002000
#define GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES 0x8F39
#define GL_DEPTH_STENCIL_TEXTURE_MODE 0x90EA
#define GL_STENCIL_INDEX 0x1901
#define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5E
#define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5F
#define GL_SAMPLE_POSITION 0x8E50
#define GL_SAMPLE_MASK 0x8E51
#define GL_SAMPLE_MASK_VALUE 0x8E52
#define GL_TEXTURE_2D_MULTISAMPLE 0x9100
#define GL_MAX_SAMPLE_MASK_WORDS 0x8E59
#define GL_MAX_COLOR_TEXTURE_SAMPLES 0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES 0x910F
#define GL_MAX_INTEGER_SAMPLES 0x9110
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE 0x9104
#define GL_TEXTURE_SAMPLES 0x9106
#define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS 0x9107
#define GL_TEXTURE_WIDTH 0x1000
#define GL_TEXTURE_HEIGHT 0x1001
#define GL_TEXTURE_DEPTH 0x8071
#define GL_TEXTURE_INTERNAL_FORMAT 0x1003
#define GL_TEXTURE_RED_SIZE 0x805C
#define GL_TEXTURE_GREEN_SIZE 0x805D
#define GL_TEXTURE_BLUE_SIZE 0x805E
#define GL_TEXTURE_ALPHA_SIZE 0x805F
#define GL_TEXTURE_DEPTH_SIZE 0x884A
#define GL_TEXTURE_STENCIL_SIZE 0x88F1
#define GL_TEXTURE_SHARED_SIZE 0x8C3F
#define GL_TEXTURE_RED_TYPE 0x8C10
#define GL_TEXTURE_GREEN_TYPE 0x8C11
#define GL_TEXTURE_BLUE_TYPE 0x8C12
#define GL_TEXTURE_ALPHA_TYPE 0x8C13
#define GL_TEXTURE_DEPTH_TYPE 0x8C16
#define GL_TEXTURE_COMPRESSED 0x86A1
#define GL_SAMPLER_2D_MULTISAMPLE 0x9108
#define GL_INT_SAMPLER_2D_MULTISAMPLE 0x9109
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE 0x910A
#define GL_VERTEX_ATTRIB_BINDING 0x82D4
#define GL_VERTEX_ATTRIB_RELATIVE_OFFSET 0x82D5
#define GL_VERTEX_BINDING_DIVISOR 0x82D6
#define GL_VERTEX_BINDING_OFFSET 0x82D7
#define GL_VERTEX_BINDING_STRIDE 0x82D8
#define GL_VERTEX_BINDING_BUFFER 0x8F4F
#define GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET 0x82D9
#define GL_MAX_VERTEX_ATTRIB_BINDINGS 0x82DA
#define GL_MAX_VERTEX_ATTRIB_STRIDE 0x82E5

// --- GL ES 2.0 Core Functions ---
extern void (*glActiveTexture)(GLenum texture);
extern void (*glAttachShader)(GLuint program, GLuint shader);
extern void (*glBindAttribLocation)(GLuint program, GLuint index, const GLchar *name);
extern void (*glBindBuffer)(GLenum target, GLuint buffer);
extern void (*glBindFramebuffer)(GLenum target, GLuint framebuffer);
extern void (*glBindRenderbuffer)(GLenum target, GLuint renderbuffer);
extern void (*glBindTexture)(GLenum target, GLuint texture);
extern void (*glBlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void (*glBlendEquation)(GLenum mode);
extern void (*glBlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha);
extern void (*glBlendFunc)(GLenum sfactor, GLenum dfactor);
extern void (*glBlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
extern void (*glClear)(GLbitfield mask);
extern void (*glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void (*glClearDepthf)(GLfloat d);
extern void (*glClearStencil)(GLint s);
extern void (*glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
extern void (*glCompileShader)(GLuint shader);
extern void (*glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
extern void (*glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
extern void (*glCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern void (*glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern GLuint (*glCreateProgram)(void);
extern GLuint (*glCreateShader)(GLenum type);
extern void (*glCullFace)(GLenum mode);
extern void (*glDeleteBuffers)(GLsizei n, const GLuint *buffers);
extern void (*glDeleteFramebuffers)(GLsizei n, const GLuint *framebuffers);
extern void (*glDeleteProgram)(GLuint program);
extern void (*glDeleteRenderbuffers)(GLsizei n, const GLuint *renderbuffers);
extern void (*glDeleteShader)(GLuint shader);
extern void (*glDeleteTextures)(GLsizei n, const GLuint *textures);
extern void (*glDepthFunc)(GLenum func);
extern void (*glDepthMask)(GLboolean flag);
extern void (*glDepthRangef)(GLfloat n, GLfloat f);
extern void (*glDetachShader)(GLuint program, GLuint shader);
extern void (*glDisable)(GLenum cap);
extern void (*glDisableVertexAttribArray)(GLuint index);
extern void (*glDrawArrays)(GLenum mode, GLint first, GLsizei count);
extern void (*glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void *indices);
extern void (*glEnable)(GLenum cap);
extern void (*glEnableVertexAttribArray)(GLuint index);
extern void (*glFinish)(void);
extern void (*glFlush)(void);
extern void (*glFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern void (*glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void (*glFrontFace)(GLenum mode);
extern void (*glGenBuffers)(GLsizei n, GLuint *buffers);
extern void (*glGenerateMipmap)(GLenum target);
extern void (*glGenFramebuffers)(GLsizei n, GLuint *framebuffers);
extern void (*glGenRenderbuffers)(GLsizei n, GLuint *renderbuffers);
extern void (*glGenTextures)(GLsizei n, GLuint *textures);
extern void (*glGetActiveAttrib)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
extern void (*glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
extern void (*glGetAttachedShaders)(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
extern GLint (*glGetAttribLocation)(GLuint program, const GLchar *name);
extern void (*glGetBooleanv)(GLenum pname, GLboolean *data);
extern void (*glGetBufferParameteriv)(GLenum target, GLenum pname, GLint *params);
extern GLenum (*glGetError)(void);
extern void (*glGetFloatv)(GLenum pname, GLfloat *data);
extern void (*glGetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint *params);
extern void (*glGetIntegerv)(GLenum pname, GLint *data);
extern void (*glGetProgramiv)(GLuint program, GLenum pname, GLint *params);
extern void (*glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void (*glGetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint *params);
extern void (*glGetShaderiv)(GLuint shader, GLenum pname, GLint *params);
extern void (*glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void (*glGetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
extern void (*glGetShaderSource)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
extern const GLubyte *(*glGetString)(GLenum name);
extern void (*glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat *params);
extern void (*glGetTexParameteriv)(GLenum target, GLenum pname, GLint *params);
extern GLint (*glGetUniformLocation)(GLuint program, const GLchar *name);
extern void (*glGetUniformfv)(GLuint program, GLint location, GLfloat *params);
extern void (*glGetUniformiv)(GLuint program, GLint location, GLint *params);
extern void (*glGetVertexAttribfv)(GLuint index, GLenum pname, GLfloat *params);
extern void (*glGetVertexAttribiv)(GLuint index, GLenum pname, GLint *params);
extern void (*glGetVertexAttribPointerv)(GLuint index, GLenum pname, void **pointer);
extern void (*glHint)(GLenum target, GLenum mode);
extern GLboolean (*glIsBuffer)(GLuint buffer);
extern GLboolean (*glIsEnabled)(GLenum cap);
extern GLboolean (*glIsFramebuffer)(GLuint framebuffer);
extern GLboolean (*glIsProgram)(GLuint program);
extern GLboolean (*glIsRenderbuffer)(GLuint renderbuffer);
extern GLboolean (*glIsShader)(GLuint shader);
extern GLboolean (*glIsTexture)(GLuint texture);
extern void (*glLineWidth)(GLfloat width);
extern void (*glLinkProgram)(GLuint program);
extern void (*glPixelStorei)(GLenum pname, GLint param);
extern void (*glPolygonOffset)(GLfloat factor, GLfloat units);
extern void (*glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
extern void (*glReleaseShaderCompiler)(void);
extern void (*glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
extern void (*glSampleCoverage)(GLfloat value, GLboolean invert);
extern void (*glScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
extern void (*glShaderBinary)(GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
extern void (*glShaderSource)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
extern void (*glStencilFunc)(GLenum func, GLint ref, GLuint mask);
extern void (*glStencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask);
extern void (*glStencilMask)(GLuint mask);
API void (*glStencilMaskSeparate)(GLenum face, GLuint mask);
extern void (*glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
extern void (*glStencilOpSeparate)(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
extern void (*glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
extern void (*glTexParameterf)(GLenum target, GLenum pname, GLfloat param);
extern void (*glTexParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
extern void (*glTexParameteri)(GLenum target, GLenum pname, GLint param);
extern void (*glTexParameteriv)(GLenum target, GLenum pname, const GLint *params);
extern void (*glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
extern void (*glUniform1f)(GLint location, GLfloat v0);
extern void (*glUniform1fv)(GLint location, GLsizei count, const GLfloat *value);
extern void (*glUniform1i)(GLint location, GLint v0);
extern void (*glUniform1iv)(GLint location, GLsizei count, const GLint *value);
extern void (*glUniform2f)(GLint location, GLfloat v0, GLfloat v1);
extern void (*glUniform2fv)(GLint location, GLsizei count, const GLfloat *value);
extern void (*glUniform2i)(GLint location, GLint v0, GLint v1);
extern void (*glUniform2iv)(GLint location, GLsizei count, const GLint *value);
extern void (*glUniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void (*glUniform3fv)(GLint location, GLsizei count, const GLfloat *value);
extern void (*glUniform3i)(GLint location, GLint v0, GLint v1, GLint v2);
extern void (*glUniform3iv)(GLint location, GLsizei count, const GLint *value);
extern void (*glUniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void (*glUniform4fv)(GLint location, GLsizei count, const GLfloat *value);
extern void (*glUniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
extern void (*glUniform4iv)(GLint location, GLsizei count, const GLint *value);
extern void (*glUniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (*glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (*glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (*glUseProgram)(GLuint program);
extern void (*glValidateProgram)(GLuint program);
extern void (*glVertexAttrib1f)(GLuint index, GLfloat x);
extern void (*glVertexAttrib1fv)(GLuint index, const GLfloat *v);
extern void (*glVertexAttrib2f)(GLuint index, GLfloat x, GLfloat y);
extern void (*glVertexAttrib2fv)(GLuint index, const GLfloat *v);
extern void (*glVertexAttrib3f)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
extern void (*glVertexAttrib3fv)(GLuint index, const GLfloat *v);
extern void (*glVertexAttrib4f)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void (*glVertexAttrib4fv)(GLuint index, const GLfloat *v);
extern void (*glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
extern void (*glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);

// --- GL ES 3.0 Core Functions ---
extern void (*glReadBuffer)(GLenum src);
extern void (*glDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
extern void (*glTexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
extern void (*glTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
extern void (*glCopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void (*glCompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
extern void (*glCompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
extern void (*glGenQueries)(GLsizei n, GLuint *ids);
extern void (*glDeleteQueries)(GLsizei n, const GLuint *ids);
extern GLboolean (*glIsQuery)(GLuint id);
extern void (*glBeginQuery)(GLenum target, GLuint id);
extern void (*glEndQuery)(GLenum target);
extern void (*glGetQueryiv)(GLenum target, GLenum pname, GLint *params);
extern void (*glGetQueryObjectuiv)(GLuint id, GLenum pname, GLuint *params);
extern void (*glUnmapBuffer)(GLenum target);
extern const void *(*glMapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
extern void (*glFlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length);
extern void (*glGenSamplers)(GLsizei count, GLuint *samplers);
extern void (*glDeleteSamplers)(GLsizei count, const GLuint *samplers);
extern GLboolean (*glIsSampler)(GLuint sampler);
extern void (*glBindSampler)(GLuint unit, GLuint sampler);
extern void (*glSamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param);
extern void (*glSamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat *params);
extern void (*glSamplerParameteri)(GLuint sampler, GLenum pname, GLint param);
extern void (*glSamplerParameteriv)(GLuint sampler, GLenum pname, const GLint *params);
extern void (*glGetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat *params);
extern void (*glGetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint *params);
extern void (*glVertexAttribDivisor)(GLuint index, GLuint divisor);
extern void (*glBindTransformFeedback)(GLenum target, GLuint id);
extern void (*glDeleteTransformFeedbacks)(GLsizei n, const GLuint *ids);
extern GLboolean (*glIsTransformFeedback)(GLuint id);
extern void (*glGenTransformFeedbacks)(GLsizei n, GLuint *ids);
extern void (*glPauseTransformFeedback)(void);
extern void (*glResumeTransformFeedback)(void);
extern void (*glBeginTransformFeedback)(GLenum primitiveMode);
extern void (*glEndTransformFeedback)(void);
extern void (*glTransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode);
extern void (*glGetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
extern void (*glBindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
extern void (*glBindBufferBase)(GLenum target, GLuint index, GLuint buffer);
API void (*glGetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
extern void (*glGetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
extern void (*glGetUniformBlockIndex)(GLuint program, const GLchar *uniformBlockName);
extern void (*glUniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
extern void (*glGetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
extern void (*glGetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
extern GLsync (*glFenceSync)(GLenum condition, GLbitfield flags);
extern GLboolean (*glIsSync)(GLsync sync);
extern void (*glDeleteSync)(GLsync sync);
extern GLenum (*glClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
extern void (*glWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
extern void (*glGetInteger64v)(GLenum pname, GLint64 *data);
extern void (*glGetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
extern void (*glCopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
extern void (*glGetBufferParameteri64v)(GLenum target, GLenum pname, GLint64 *params);
extern void (*glAttachShader)(GLuint program, GLuint shader); // Re-declaration from GLES 2.0, but still used
extern void (*glBlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
extern GLenum (*glCheckFramebufferStatus)(GLenum target);
extern void (*glDrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
extern void (*glDrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
extern void (*glFramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
extern void (*glGetActiveAttrib)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name); // Re-declaration
extern void (*glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name); // Re-declaration
extern void (*glGetAttachedShaders)(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders); // Re-declaration
extern GLint (*glGetAttribLocation)(GLuint program, const GLchar *name); // Re-declaration
extern void (*glGetBufferParameteriv)(GLenum target, GLenum pname, GLint *params); // Re-declaration
extern void (*glGetIntegeri_v)(GLenum target, GLuint index, GLint *data);
extern void (*glGetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
extern void (*glGetProgramiv)(GLuint program, GLenum pname, GLint *params); // Re-declaration
extern void (*glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog); // Re-declaration
extern void (*glGetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint *params); // Re-declaration
extern void (*glGetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision); // Re-declaration
extern void (*glGetShaderSource)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source); // Re-declaration
extern const GLubyte *(*glGetStringi)(GLenum name, GLuint index);
extern void (*glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat *params); // Re-declaration
extern void (*glGetTexParameteriv)(GLenum target, GLenum pname, GLint *params); // Re-declaration
extern GLint (*glGetUniformLocation)(GLuint program, const GLchar *name); // Re-declaration
extern void (*glGetUniformfv)(GLuint program, GLint location, GLfloat *params); // Re-declaration
extern void (*glGetUniformiv)(GLuint program, GLint location, GLint *params); // Re-declaration
extern void (*glGetVertexAttribfv)(GLuint index, GLenum pname, GLfloat *params); // Re-declaration
extern void (*glGetVertexAttribiv)(GLuint index, GLenum pname, GLint *params); // Re-declaration
extern void (*glGetVertexAttribPointerv)(GLuint index, GLenum pname, void **pointer); // Re-declaration
extern void (*glProgramBinary)(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
extern void (*glProgramParameteri)(GLuint program, GLenum pname, GLint value);
extern void (*glSamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param); // Re-declaration
extern void (*glSamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat *params); // Re-declaration
extern void (*glSamplerParameteri)(GLuint sampler, GLenum pname, GLint param); // Re-declaration
extern void (*glSamplerParameteriv)(GLuint sampler, GLenum pname, const GLint *params); // Re-declaration
extern void (*glTexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
extern void (*glTexStorage3D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
extern void (*glUniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (*glUniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (*glUniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (*glUniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (*glUniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (*glUniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (*glVertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w);
extern void (*glVertexAttribI4iv)(GLuint index, const GLint *v);
extern void (*glVertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
extern void (*glVertexAttribI4uiv)(GLuint index, const GLuint *v);
extern void (*glVertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
extern void (*glGetVertexAttribIiv)(GLuint index, GLenum pname, GLint *params);
extern void (*glGetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint *params);
extern void (*glGetFragDataLocation)(GLuint program, const GLchar *name);
extern GLint (*glGetFragDataLocation)(GLuint program, const GLchar *name);
extern void (*glSamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param); // Re-declaration
extern void (*glSamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat *params); // Re-declaration
extern void (*glSamplerParameteri)(GLuint sampler, GLenum pname, GLint param); // Re-declaration
extern void (*glSamplerParameteriv)(GLuint sampler, GLenum pname, const GLint *params); // Re-declaration

// --- GL ES 3.1 Core Functions ---
extern void (*glDispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
extern void (*glDispatchComputeIndirect)(GLintptr indirect);
extern void (*glDrawArraysIndirect)(GLenum mode, const void *indirect);
extern void (*glDrawElementsIndirect)(GLenum mode, GLenum type, const void *indirect);
extern void (*glFramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level);
extern void (*glGetProgramResourceIndex)(GLuint program, GLenum programInterface, const GLchar *name);
extern void (*glGetProgramResourceName)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
extern void (*glGetProgramResourceiv)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
extern GLint (*glGetProgramResourceLocation)(GLuint program, GLenum programInterface, const GLchar *name);
extern void (*glShaderStorageBlockBinding)(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding);
extern void (*glTexBuffer)(GLenum target, GLenum internalformat, GLuint buffer);
extern void (*glTexBufferRange)(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
extern void (*glTexImage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);

// --- GL ES 3.2 Core Functions ---
extern void (*glBlendBarrier)(void);
extern void (*glDebugMessageControl)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
extern void (*glDebugMessageInsert)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
extern void (*glDebugMessageCallback)(GLDEBUGPROC callback, const void *userParam);
extern GLuint (*glGetDebugMessageLog)(GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
extern void (*glPushDebugGroup)(GLenum source, GLuint id, GLsizei length, const GLchar *buf);
extern void (*glPopDebugGroup)(void);
extern void (*glObjectLabel)(GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
extern void (*glGetObjectLabel)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
extern void (*glObjectPtrLabel)(const void *ptr, GLsizei length, const GLchar *label);
extern void (*glGetObjectPtrLabel)(const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
extern void (*glGetPointerv)(GLenum pname, void **params);
extern void (*glEnable)(GLenum cap); // Re-declaration (already in ES 2.0)
extern void (*glDisable)(GLenum cap); // Re-declaration (already in ES 2.0)
extern void (*glPrimitiveBoundingBoxEXT)(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW);
extern void (*glPatchParameteriEXT)(GLenum pname, GLint value);
extern void (*glDrawElementsBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);
extern void (*glDrawRangeElementsBaseVertex)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
extern void (*glDrawElementsInstancedBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
extern void (*glDrawElementsInstancedBaseInstanceEXT)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance);
extern void (*glDrawElementsBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex); // Re-declaration
extern void (*glDrawRangeElementsBaseVertex)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex); // Re-declaration
extern void (*glDrawElementsInstancedBaseVertex)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex); // Re-declaration
extern void (*glDrawArraysIndirect)(GLenum mode, const void *indirect); // Re-declaration
extern void (*glDrawElementsIndirect)(GLenum mode, GLenum type, const void *indirect); // Re-declaration
extern void (*glGetBufferParameteri64v)(GLenum target, GLenum pname, GLint64 *params); // Re-declaration
extern void (*glFramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level); // Re-declaration
extern void (*glPrimitiveBoundingBox)(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW); // Core in GLES 3.2
extern void (*glPatchParameteri)(GLenum pname, GLint value); // Core in GLES 3.2

#endif
