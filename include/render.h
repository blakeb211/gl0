#pragma once
#include <vector>

namespace gxb {
  struct Level;
}

namespace render {

  void clearScreen();
  unsigned int buildVAO(const gxb::Level*);
  void logOpenGLInfo();
  void setGLflags();
  unsigned int buildOctreeVAO(const std::vector<float>&);
};
