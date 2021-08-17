#pragma once
#include <vector>
#include "shader.h"

namespace gxb {
  struct Level;
}

namespace render {

  void DrawLevel(unsigned int vaoEntities, glm::mat4& model, Shader& progOne, unsigned int vaoOctree, gxb::Level*, std::vector<gxb::PathPt>&);
  void clearScreen();
  unsigned int BuildLevelVao(const gxb::Level*);
  void logOpenGLInfo();
  void SetGlFlags();
  unsigned int BuildSpatialGridVao(const std::vector<float>&);

  // make these toggleable with a hotkey
  inline const auto DRAW_CAM_PATH = 0;
  inline const auto DRAW_OCTREE = 1;
  inline const auto DRAW_OBJECT_POS = 1;

};
