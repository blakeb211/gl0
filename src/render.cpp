#include <glad\glad.h>
#include <gamelib.h>
#include <render.h>

void render::clearScreen() {
  glClearColor(0.2f, 0.3f, 0.7f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void render::logOpenGLInfo() {
  // Query GL
  int glMajVers, glMinVers;
  glGetIntegerv(GL_MAJOR_VERSION, &glMajVers);
  glGetIntegerv(GL_MINOR_VERSION, &glMinVers);
  logPrintLn("OpenGL Version:", glMajVers, ".", glMinVers);
}

void render::setGLflags() {
  glEnable(GL_DEPTH_TEST);
  //glEnable(GL_LIGHTING);
  glEnable(GL_PROGRAM_POINT_SIZE);
}

unsigned int render::buildVAO(const gxb::Level* l) {
  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  assert(l != nullptr);
  unsigned int VBO{};
  unsigned int VAO{};

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  // bind the Vertex Array Object first, then bind and set vertex
  // buffer(s), and then configure vertex attributes(s)
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, l->raw_data.size() * sizeof(float),
    l->raw_data.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // an unbind the VBO
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glBindVertexArray(0);
  return VAO;
}







