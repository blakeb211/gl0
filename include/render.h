#pragma once

namespace gxb {
  struct Level;
}

namespace render {

  void clearScreen();
  unsigned int buildVAO(const gxb::Level*);
  void logOpenGLInfo();
  void setGLflags();

};
