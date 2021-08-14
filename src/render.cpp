#include <glad\glad.h>
#include <gamelib.h>
#include "render.h"
#include <glm.h>
#include "octree.h"
// Forward Declares
//	namespace octree {
//		size_t getVertBufGridLinesSize();
//	};

void render::draw_level(unsigned int vaoEntities, glm::mat4& model, Shader& progOne, unsigned int vaoOctree, gxb::Level* level, std::vector<gxb::PathPt>& path)
{
  glBindVertexArray(vaoEntities);
  size_t colorId = 0;
  const size_t numColor = col::list.size();
  for (size_t i = 0; i < level->objects.size(); i++) {
    model = glm::mat4(1.0f);
    model = glm::translate(model, level->objects[i]->pos);
    colorId =
      (colorId == numColor - 1) ? colorId -= numColor - 1 : colorId += 1;

    progOne.setVec3("color", col::list[colorId]);
    progOne.setMat4("model", model);

    auto meshPtr = level->getMesh(level->objects[i]->mesh_id);
    assert(meshPtr != nullptr);

    unsigned numVertsCurrModel = (unsigned)(meshPtr->faces.size() * 3);
    glDrawArrays(GL_TRIANGLES, (GLint)meshPtr->pos_first_vert,
      numVertsCurrModel);
  }

  if (render::DRAW_CAM_PATH) {
    // Draw CamPath
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3{ 0, 0, 0 });
    progOne.setMat4("model", model);
    progOne.setVec3("color", col::red);
    const auto tot_verts = level->raw_data.size() / 3;
    const auto cam_path_verts = path.size();
    glDrawArrays(GL_POINTS, (GLint)(tot_verts - cam_path_verts),
      (GLint)cam_path_verts);
  }

  if (render::DRAW_OCTREE) {
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3{ 0, 0, 0 });
    progOne.setMat4("model", model);
    glBindVertexArray(vaoOctree);  // bind the octree vao
    const auto& vert_buf_sz = octree::getVertBufGridLinesSize();
    size_t curr_cell_idx = -1;
    // six floats per grid line x 12 lines per cell, 
    // so 72 floats passed to glDrawArrays per uniform grid cell
    for (int i = 0; i < vert_buf_sz - 72; i += 72)
    {
      curr_cell_idx++;
      auto id = octree::grid_idx_to_id(curr_cell_idx);
      if (id[0] == 0) {
        progOne.setVec3("color", col::green);
      }
      else {
        progOne.setVec3("color", col::darkred);
      }
      glDrawArrays(GL_LINES, (GLint)i,
        (GLint)72);  // uses vboOctree
    }
    glBindVertexArray(0);
  }
}

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

unsigned int render::buildOctreeVAO(const std::vector<float>& vertices_octree) {

  // build vao
  unsigned int vboOctree{}, vaoOctree{};
  glGenVertexArrays(1, &vaoOctree);
  glGenBuffers(1, &vboOctree);

  // here we set things up for the octree vao
  // the vao will store the vbo with it and every time you bind it and call
  // glDrawArrays, it will use the vbo associated with the bound vao.
  glBindVertexArray(vaoOctree);
  glBindBuffer(GL_ARRAY_BUFFER, vboOctree);
  // vboOctree buffer data is from vertices_octree
  glBufferData(GL_ARRAY_BUFFER, vertices_octree.size() * sizeof(float),
    vertices_octree.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
    (void*)0);
  glEnableVertexAttribArray(0);

  // unbind the VBO and VAO
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  return vaoOctree;
}
