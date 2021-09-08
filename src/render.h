#pragma once
#include <vector>
#include "shader.h"

namespace gxb {
  struct Level;
  struct PathPt;
};

namespace render {

  void DrawLevel(const unsigned int vaoEntities, const Shader& progOne, const unsigned int vaoOctree, const gxb::Level* const, const std::vector<gxb::PathPt>&);
  void clearScreen();
  unsigned int BuildLevelVao(const gxb::Level*);
  void logOpenGLInfo();
  void SetGlFlags();
  unsigned int BuildSpatialGridVao(const std::vector<float>&);

  inline std::vector<unsigned> highlighted_entities{};
};
