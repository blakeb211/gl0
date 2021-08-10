#pragma once
#include <gamelib.h>

namespace octree {

  gxb::Level* level;

  void setup(gxb::Level* level) {
    octree::level = level;
    glm::vec3 mins{ 0 }, maxes{ 0 };
    for (auto& i : level->objects) {
      if (i->pos.x < mins.x) mins.x = i->pos.x;
      if (i->pos.y < mins.y) mins.y = i->pos.y;
      if (i->pos.z < mins.z) mins.z = i->pos.z;

      if (i->pos.x > maxes.x) maxes.x = i->pos.x;
      if (i->pos.y > maxes.y) maxes.y = i->pos.y;
      if (i->pos.z > maxes.z) maxes.z = i->pos.z;
    }
    logPrintLn("octree extent:", glm::to_string(mins), glm::to_string(maxes));
  };

  void draw() {};







}