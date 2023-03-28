#include "main_game.hpp"
#include "engine.hpp"

#include "math/matrix4.hpp"
#include "user_interface/user_interface.hpp"
#include <chrono>
#include <cmath>
#include <cstdlib> /* srand, rand */
#include <cstring>
#include <ctime> /* time */

engine::mesh_core *mp;
Actor ml;

Main::Main() {
  ml.x = 120;
  ml.y = 120;
  ml.width = 400;
  ml.height = 350;
  memcpy(ml.color, (unsigned char[]){0xff, 0xff, 0x00, 0xff}, 4 * sizeof(unsigned char));
  ml.img = image("test.jpeg");
  user_interface::addActor(&ml);
  engine::mesh_core::data vert[24] = {
      // front red
      {+350.0f, +350.0f, -350.0f, 0xff, 0x00, 0x00, 0xff},
      {+350.0f, -350.0f, -350.0f, 0xff, 0x00, 0x00, 0xff},
      {-350.0f, -350.0f, -350.0f, 0xff, 0x00, 0x00, 0xff},
      {-350.0f, +350.0f, -350.0f, 0xff, 0x00, 0x00, 0xff},
      // left green
      {-350.0f, +350.0f, -350.0f, 0x00, 0xff, 0x00, 0xff},
      {-350.0f, -350.0f, -350.0f, 0x00, 0xff, 0x00, 0xff},
      {-350.0f, -350.0f, +350.0f, 0x00, 0xff, 0x00, 0xff},
      {-350.0f, +350.0f, +350.0f, 0x00, 0xff, 0x00, 0xff},
      // right blue
      {+350.0f, +350.0f, +350.0f, 0x00, 0x00, 0xff, 0xff},
      {+350.0f, -350.0f, +350.0f, 0x00, 0x00, 0xff, 0xff},
      {+350.0f, -350.0f, -350.0f, 0x00, 0x00, 0xff, 0xff},
      {+350.0f, +350.0f, -350.0f, 0x00, 0x00, 0xff, 0xff},
      // bot gray
      {+350.0f, -350.0f, -350.0f, 0x33, 0x33, 0x33, 0xff},
      {+350.0f, -350.0f, +350.0f, 0x33, 0x33, 0x33, 0xff},
      {-350.0f, -350.0f, +350.0f, 0x33, 0x33, 0x33, 0xff},
      {-350.0f, -350.0f, -350.0f, 0x33, 0x33, 0x33, 0xff},
      // top purple
      {+350.0f, +350.0f, +350.0f, 0xff, 0x00, 0xff, 0xff},
      {+350.0f, +350.0f, -350.0f, 0xff, 0x00, 0xff, 0xff},
      {-350.0f, +350.0f, -350.0f, 0xff, 0x00, 0xff, 0xff},
      {-350.0f, +350.0f, +350.0f, 0xff, 0x00, 0xff, 0xff},
      // back fulle
      {-350.0f, +350.0f, +350.0f, 0x00, 0xff, 0xff, 0xff},
      {-350.0f, -350.0f, +350.0f, 0xff, 0xff, 0xff, 0xff},
      {+350.0f, -350.0f, +350.0f, 0x00, 0x00, 0xff, 0xff},
      {+350.0f, +350.0f, +350.0f, 0x00, 0xff, 0x00, 0xff}};
  unsigned short indices[36] = {
      0, 1, 3, 1, 2, 3,       // front
      4, 5, 7, 5, 6, 7,       // left
      8, 9, 11, 9, 10, 11,    // right
      12, 13, 15, 13, 14, 15, // bot
      16, 17, 19, 17, 18, 19, // top
      20, 21, 23, 21, 22, 23  // back
  };
  // engine::graph->clearcolor(1.f,0.f, 1.f,1.f);
  mp = engine::graph->gen_mesh(vert, 24, indices, 36);
}
void Main::resume() {
}
std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now(), end;
float delta = 0;
void Main::render() {
  end = std::chrono::high_resolution_clock::now();
  delta = float(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) / 1000000.f;
  start = end;
  engine::inpt->process_event();
  engine::graph->clear(7);

  matrix4::rotate(mp->trans,
                  M_PI / 12.f * (delta), // 15° /s
                  M_PI / 6.f * (delta),  // 30° /s
                  M_PI / 3.0f * (delta)  // 60° /s
  );
  engine::graph->mesh_render(&mp, 1);

  // memcpy(&ml.color, (unsigned char[]){(unsigned char)(0xff * ((float)engine::inpt->getX(0) / engine::graph->getWidth())), (unsigned char)(0xff * ((float)engine::inpt->getY(0) / engine::graph->getHeight())), 0x00}, 3 * sizeof(unsigned char));

  user_interface::draw();
}
void Main::pause() {
}
Main::~Main() {
  engine::graph->delete_mesh(mp);
  user_interface::clearActor();
}