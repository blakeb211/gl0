#pragma once
#include <vector>
#include "shader.h"

namespace gxb {
  struct Level;
  struct PathPt;
};

namespace render {

  void DrawLevel(unsigned int vaoEntities, glm::mat4& model, Shader& progOne, unsigned int vaoOctree, gxb::Level*, std::vector<gxb::PathPt>&);
  void clearScreen();
  unsigned int BuildLevelVao(const gxb::Level*);
  void logOpenGLInfo();
  void SetGlFlags();
  unsigned int BuildSpatialGridVao(const std::vector<float>&);

};
