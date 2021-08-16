#include <glad\glad.h>
#include <gamelib.h>
#include "render.h"
#include <glm.h>
#include "octree.h"
// Forward Declares
//	namespace SpatialGrid {
//		size_t GetVertBufGridLinesSize();
//	};

void render::DrawLevel(unsigned int vao_entities, glm::mat4& model, Shader& prog_one, unsigned int vao_spatial_grid, gxb::Level* level, std::vector<gxb::PathPt>& path)
{
  glBindVertexArray(vao_entities);
  size_t curr_color_id = 0;
  const size_t num_colors = col::list.size();
  for (size_t i = 0; i < level->objects.size(); i++) {
    model = glm::mat4(1.0f);
    model = glm::translate(model, level->objects[i]->pos);
    curr_color_id =
      (curr_color_id == num_colors - 1) ? curr_color_id -= num_colors - 1 : curr_color_id += 1;

    prog_one.SetVec3("color", col::list[curr_color_id]);
    prog_one.SetMat4("model", model);

    auto mesh_ptr = level->GetMesh(level->objects[i]->mesh_id);
    assert(mesh_ptr != nullptr);

    auto num_verts_model = static_cast<unsigned>(mesh_ptr->faces.size() * 3);
    glDrawArrays(GL_TRIANGLES, static_cast<GLint>(mesh_ptr->pos_first_vert),
      num_verts_model);
  }

  if (render::DRAW_CAM_PATH) {
    // Draw CamPath
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3{ 0, 0, 0 });
    prog_one.SetMat4("model", model);
    prog_one.SetVec3("color", col::red);
    const auto tot_verts = level->raw_data.size() / 3;
    const auto cam_path_verts = path.size();
    glDrawArrays(GL_POINTS, (GLint)(tot_verts - cam_path_verts),
      (GLint)cam_path_verts);
  }

  if (render::DRAW_OCTREE) {
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3{ 0, 0, 0 });
    prog_one.SetMat4("model", model);
    glBindVertexArray(vao_spatial_grid);  // bind the SpatialGrid vao
    const auto& tot_vert_num = SpatialGrid::GetVertBufGridLinesSize() / 3;
    size_t curr_cell_idx = -1;
    // i loops over *vertices* not float. there are 3 floats per vertex
    for (int i = 0; i < tot_vert_num; i += 24)
    {
      curr_cell_idx++;
      auto id = SpatialGrid::GridIndexToId(curr_cell_idx);

      // set color of current cell before drawing
      prog_one.SetVec3("color", col::black);

      // we draw 24 verts per cell, since there are 12 lines with 2 verts each
      glDrawArrays(GL_LINES, (GLint)i,
        (GLint)24);  // uses vboOctree
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
  LogPrintLn("OpenGL Version:", glMajVers, ".", glMinVers);
}

void render::SetGlFlags() {
  glEnable(GL_DEPTH_TEST);
  //glEnable(GL_LIGHTING);
  glEnable(GL_PROGRAM_POINT_SIZE);

}

unsigned int render::BuildLevelVao(const gxb::Level* l) {
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

unsigned int render::BuildSpatialGridVao(const std::vector<float>& vertices_octree) {

  // build vao
  unsigned int vboOctree{}, vao_spatial_grid{};
  glGenVertexArrays(1, &vao_spatial_grid);
  glGenBuffers(1, &vboOctree);

  // here we set things up for the SpatialGrid vao
  // the vao will store the vbo with it and every time you bind it and call
  // glDrawArrays, it will use the vbo associated with the bound vao.
  glBindVertexArray(vao_spatial_grid);
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
  return vao_spatial_grid;
}
