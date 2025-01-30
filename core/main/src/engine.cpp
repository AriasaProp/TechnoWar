#include "engine.h"

namespace engine {
// Pointer definitions for graphics namespace
namespace graphics {
  float (*getWidth) ();
  float (*getHeight) ();
  void (*clear) (const unsigned int &);
  void (*clearcolor) (const float &, const float &, const float &, const float &);
  texture_core *(*gen_texture) (const int &, const int &, unsigned char *);
  void (*bind_texture) (texture_core *);
  void (*set_texture_param) (const int &, const int &);
  void (*delete_texture) (texture_core *);
  void (*flat_render) (texture_core *, flat_vertex *, const unsigned int);
  mesh_core *(*gen_mesh) (mesh_core::data *, unsigned int, unsigned short *, unsigned int);
  void (*mesh_render) (mesh_core **, const unsigned int &);
  void (*delete_mesh) (mesh_core *);
  void (*to_flat_coordinate) (float &, float &);
} // namespace graphics

// Pointer definitions for input namespace
namespace input {
  sensor_value (*getSensorValue) (const char *);
  void (*getPointerPos) (float *, unsigned int);
  void (*getPointerDelta) (float *, unsigned int);
  bool (*justTouched) ();
  bool (*onTouched) ();
  bool (*isTouched) (unsigned int);
  float (*getPressure) (unsigned int);
  bool (*isButtonPressed) (int button);
  bool (*isButtonJustPressed) (int button);
  bool (*isKeyPressed) (int key);
  bool (*isKeyJustPressed) (int key);
  void (*process_event) ();
} // namespace input

// Pointer definitions for assets namespace
namespace assets {
  asset *(*open_asset) (const char *);
  void *(*asset_buffer) (const char *, unsigned int *);
} // namespace assets

// Pointer definitions for info namespace
namespace info {
  const char *(*get_platform_info) ();
  long (*memory) ();
} // namespace info

} // namespace engine