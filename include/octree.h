#pragma once
#include <vector>
#include <forward_list>
#include <queue>
#include "gamelib.h"
#include "render.h"

using v3 = glm::vec3;
using byte = unsigned char;
using List = std::forward_list<gxb::entity*>;
using Queue = std::queue<gxb::entity*>;
/* **************************************************************
 *  This file builds a uniform grid for collision testing; rendering handled by render.cpp
 *  ***************************************************************/

namespace octree {

  // forward declarations
  void subdivide();
  int calc_side_length();

  struct BoundingBox {
    v3 min, max;
  };

  struct Node {
    BoundingBox bb;
    std::array<gxb::entity*, 20> list;
    // list of objects 
    // children
  };

  // octree globals
  gxb::Level* level;
  std::vector<float> vertbufGridLines{}; // for drawing, vertices of Octree bounding box lines
  const float targetSideL = 7.f;
  int numCells{};
  float cellL{}, worldL{};
  Node topNode;

  void add_lines_to_vert_buf(BoundingBox x) {
    auto min = x.min;
    auto max = x.max;
    assert(glm::distance(min, max) > 3);

    auto push_floats = [&](v3 corner) {
      vertbufGridLines.push_back(corner.x);
      vertbufGridLines.push_back(corner.y);
      vertbufGridLines.push_back(corner.z);
    };

    auto push_cube = [&](v3 min, v3 max) {
      // GL_LINES draws verts 0 and 1 as first line, then 2 and 3 as next line
      auto dim = v3(max - min);
      push_floats(min);
      push_floats(min + v3(dim.x, 0.f, 0.f));
      push_floats(min);
      push_floats(min + v3(0.f, dim.y, 0.f));
      push_floats(min);
      push_floats(min + v3(0.f, 0.f, dim.z));

      push_floats(max);
      push_floats(max - v3(dim.x, 0.f, 0.f));
      push_floats(max);
      push_floats(max - v3(0.f, dim.y, 0.f));
      push_floats(max);
      push_floats(max - v3(0.f, 0.f, dim.z));

      push_floats(max - v3(0.f, dim.y, 0.f));
      push_floats(max - v3(dim.x, dim.y, 0.f));
      push_floats(max - v3(0.f, dim.y, 0.f));
      push_floats(max - v3(0.f, dim.y, dim.z));

      push_floats(max - v3(dim.x, 0.f, 0.f));
      push_floats(max - v3(dim.x, dim.y, 0.f));
      push_floats(max - v3(dim.x, 0.f, 0.f));
      push_floats(max - v3(dim.x, 0.f, dim.z));

      push_floats(max - v3(0.f, 0.f, dim.z));
      push_floats(max - v3(0.f, dim.y, dim.z));
      push_floats(max - v3(0.f, 0.f, dim.z));
      push_floats(max - v3(dim.x, 0.f, dim.z));
    };
    // real action happens here
  // adds 12 sets of line vertices to vertices vector so they can be drawn
    push_cube(min, max);
  }


  unsigned int setup_octree(gxb::Level* level) {
    assert(level != NULL);
    octree::level = level;
    glm::vec3 min{ 0 }, max{ 0 };
    for (auto& i : level->objects) {
      if (i->pos.x < min.x)
        min.x = i->pos.x;
      if (i->pos.y < min.y)
        min.y = i->pos.y;
      if (i->pos.z < min.z)
        min.z = i->pos.z;

      if (i->pos.x > max.x)
        max.x = i->pos.x;
      if (i->pos.y > max.y)
        max.y = i->pos.y;
      if (i->pos.z > max.z)
        max.z = i->pos.z;
    }

     // add a buffer around world 
    auto orig_dim = max - min;
    // want a cubic octree, so take max dimension and use that for x,y, and z
    // expand max and min both outward 10%; this is very arbitrary
    min -= 0.1f * (orig_dim);
    max += 0.1f * (orig_dim);
    auto expanded_dim = max - min;
    logPrintLn("level min, max:", glm::to_string(min), glm::to_string(max));
    // take max dimension as our cube's side length
    float cubic_dim = std::max({ expanded_dim.x,expanded_dim.y,expanded_dim.z });
    logPrintLn("octree extent:", cubic_dim);
    worldL = cubic_dim;

    max = min + v3(cubic_dim, cubic_dim, cubic_dim);

    // create octree
    topNode = Node{ BoundingBox{min,max} };
    // subdivide topNode until 
    // 		a bin contains fewer than a given number of points
    // 		a bin reaches a minimum size based on the length of its edges
    add_lines_to_vert_buf(topNode.bb);
    numCells = calc_side_length();
    logPrintLn("ideal cellL found: ", cellL, "with numcells:", numCells);

    subdivide();

    return render::buildOctreeVAO(vertbufGridLines);
  }

  // fxn uses and modifies namespace globals to calculate
  // the side length and number of cells per dimension of the uniform grid
  int calc_side_length() {
    assert(cellL < worldL);
    auto numCells = std::floor(worldL / targetSideL);
    while (worldL / cellL > numCells) {
      cellL += 0.0001f;
    }
    return (int)(std::roundf(worldL / cellL));
  }

  // divide world into uniform grid of numCells x numCells x numCells chunks 
  // with side lengths equal to cellL >= targetSideL
  void subdivide() {
    // start from min, add cellL to the x-coord numcells times
    const auto& min = topNode.bb.min;
    v3 old_min{};
    v3 new_min{};
    logPrintLn("topNode.bb.min", glm::to_string(min));
    for (int k = 0; k < numCells; k++) {
      old_min = min + (float)(k)*v3(0.f, cellL, 0.f);
      for (int j = 0; j < numCells; j++) {
        if (j != 0) { old_min = old_min + v3(0.f, 0.f, cellL); }
        for (int i = 0; i < numCells; i++) {
          new_min = old_min + (float)(i)*v3(cellL, 0.f, 0.f);
          BoundingBox bb{ new_min,new_min + v3{cellL,cellL,cellL} };
          logPrintLn("x:", bb.min.x, "y:", bb.min.y, "z:", bb.min.z);
          add_lines_to_vert_buf(bb);
        }
      }
    }
  }

  void draw_octree(unsigned int vaoOctree) {
    glBindVertexArray(vaoOctree);  // bind the octree vao
    glDrawArrays(GL_LINES, (GLint)0,
      (GLint)vertbufGridLines.size());  // uses vboOctree
  }

}  // namespace octree
