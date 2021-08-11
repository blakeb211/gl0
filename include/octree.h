#pragma once
#include <gamelib.h>
#include <vector>
using v3 = glm::vec3;

namespace octree {

  gxb::Level* level;
  unsigned int vboOctree{}, vaoOctree{};
  std::vector<float> vertices_octree{};

  void build_Octree(v3 min, v3 max) {
	  assert(glm::distance(min,max) > 3);
	  vertices_octree.push_back(min.x);
	  vertices_octree.push_back(min.y);
	  vertices_octree.push_back(min.z);
	  vertices_octree.push_back(max.x);
	  vertices_octree.push_back(max.y);
	  vertices_octree.push_back(max.z);
  }

  void setup(gxb::Level* level) {
    assert(level != NULL);
    octree::level = level;
    glm::vec3 min{ 0 }, max{ 0 };
    for (auto& i : level->objects) {
      if (i->pos.x < min.x) min.x = i->pos.x;
      if (i->pos.y < min.y) min.y = i->pos.y;
      if (i->pos.z < min.z) min.z = i->pos.z;

      if (i->pos.x > max.x) max.x = i->pos.x;
      if (i->pos.y > max.y) max.y = i->pos.y;
      if (i->pos.z > max.z) max.z = i->pos.z;
    }
    logPrintLn("level min,max:", glm::to_string(min), glm::to_string(max));
    logPrintLn("octree extent:", glm::to_string(max - min));
    // add a buffer around octree
    min *= 1.1;
    max *= 1.1;
	build_Octree(min, max);
	logPrintLn("vertices_Octree.size = ", vertices_octree.size());
    // build vao
    glGenVertexArrays(1, &vaoOctree);
    glGenBuffers(1, &vboOctree);

    // here we set things up for the octree vao
    // the vao will store the vbo with it and every time you bind it and call 
    // glDrawArrays, it will use the vbo associated with the bound vao.
    glBindVertexArray(vaoOctree);
    glBindBuffer(GL_ARRAY_BUFFER, vboOctree);
    // vboOctree buffer data is from vertices_octree
    glBufferData(GL_ARRAY_BUFFER, vertices_octree.size() * sizeof(float), vertices_octree.data(), GL_STATIC_DRAW);

    // this will need modified
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // an unbind the VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  void draw_octree() {
    glBindVertexArray(vaoOctree); //bind the octree vao
    glDrawArrays(GL_LINES,(GLint)0,(GLint)vertices_octree.size()); //uses vboOctree

  }

  void draw() {
    draw_octree();
  }


}
