#pragma once
#include <gamelib.h>
#include <vector>

namespace octree {

  gxb::Level* level;
  unsigned int vboOctree{}, vaoOctree{};

  void setup(gxb::Level* level) {
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

    // build vao
    glGenVertexArrays(1, &vaoOctree);
    glGenBuffers(1, &vboOctree);

    // here we set things up for the octree vao
    // the vao will store the vbo with it and every time you bind it and call 
    // glDrawArrays, it will use the vbo associated with the bound vao.
    glBindVertexArray(vaoOctree);
    glBindBuffer(GL_ARRAY_BUFFER, vboOctree);
    //using the vertices_octree vertex array
    const unsigned int num_lines = 1;
    std::vector<float> vertices_octree{};
    glBufferData(GL_ARRAY_BUFFER, vertices_octree.size() * sizeof(float), vertices_octree.data(), GL_STATIC_DRAW);

    // this will need modified
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // an unbind the VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  };

  void draw_octree() {

    glBindVertexArray(vaoOctree); //bind the octree vao
    //glDrawArrays(...); //will use vboOctree

  };

  void draw() {
    draw_octree();
  };







}
