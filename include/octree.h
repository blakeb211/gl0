#pragma once
#include <forward_list>
#include <queue>
#include <vector>
#include "gamelib.h"

using v3 = glm::vec3;
using iv3 = glm::ivec3;
using byte = unsigned char;
using Queue = std::queue<gxb::Entity*>;
/* **************************************************************
 *  This file builds a uniform grid for collision testing; rendering handled by
 * render.cpp
 *  ***************************************************************/
 // @TODO: Center grid on the level so that it's unlikely objects will leave the grid
// @TODO: An alternative would be to add a new grid cell if an object enters it

namespace SpatialGrid {

  // forward declarations
  void Subdivide();
  int CalcSideLength();
  iv3 PosToGridCoords(v3 pos);
  void TestPosToGridIdFxn();

  struct BoundingBox {
    v3 min, max;
  };

  struct Cell {
    Cell() : list(20) {
      list.resize(0);
    }
    Cell(BoundingBox bb) : list(20), bb{ bb } {
      list.resize(0);
    }
    BoundingBox bb;
    std::vector<int> list;
    // list of objects
    // children
  };

  // SpatialGrid globals
  gxb::Level* level; // @TODO: add const
  std::vector<float>
    vertbufGridLines{};	 // for drawing, vertices of Octree bounding box lines
  const float targetSideL = 4.f;
  int numCells{};
  float cellL{}, worldL{};
  Cell topNode;  // whole world

  // uniform grid nodes
  std::vector<Cell> grid;
  // uniform grid id e.g. (0,0,0) to (numCells-1,numCells-1,numCells-1)
  std::vector<iv3> id;

  size_t GetVertBufGridLinesSize() {
    return vertbufGridLines.size();
  }

  iv3 GridIndexToId(size_t idx) {
    assert(idx < id.size() && idx >= 0);
    return id[idx];
  }

  void AddLinesToVertBuf(BoundingBox x) {
    auto min = x.min;
    auto max = x.max;

    auto push_floats = [&](v3 corner) {
      vertbufGridLines.push_back(corner.x);
      vertbufGridLines.push_back(corner.y);
      vertbufGridLines.push_back(corner.z);
    };

    auto push_cube = [&](v3 min, v3 max) {
      // GL_LINES draws verts 0 and 1 as first line, then 2 and 3 as next line
      push_floats(min);
      push_floats(min + v3(cellL, 0.f, 0.f));
      push_floats(min);
      push_floats(min + v3(0.f, cellL, 0.f));
      push_floats(min);
      push_floats(min + v3(0.f, 0.f, cellL));

      push_floats(max);
      push_floats(max - v3(cellL, 0.f, 0.f));
      push_floats(max);
      push_floats(max - v3(0.f, cellL, 0.f));
      push_floats(max);
      push_floats(max - v3(0.f, 0.f, cellL));

      push_floats(max - v3(0.f, cellL, 0.f));
      push_floats(max - v3(cellL, cellL, 0.f));
      push_floats(max - v3(0.f, cellL, 0.f));
      push_floats(max - v3(0.f, cellL, cellL));

      push_floats(max - v3(cellL, 0.f, 0.f));
      push_floats(max - v3(cellL, cellL, 0.f));
      push_floats(max - v3(cellL, 0.f, 0.f));
      push_floats(max - v3(cellL, 0.f, cellL));

      push_floats(max - v3(0.f, 0.f, cellL));
      push_floats(max - v3(0.f, cellL, cellL));
      push_floats(max - v3(0.f, 0.f, cellL));
      push_floats(max - v3(cellL, 0.f, cellL));
    };
    // real action happens here
    // adds 12 sets of line vertices to vertices vector so they can be drawn
    push_cube(min, max);
  }

  // get the grid id from position
  // returns (0,0,0) to (numCells-1,numCells-1,numCells-1)
  iv3 PosToGridCoords(v3 pos) {
    const auto& min = topNode.bb.min;
    auto diff = (pos - min) / cellL;
    // truncate to integer values
    return iv3{ trunc(diff.x), trunc(diff.y), trunc(diff.z) };
  }


  // this finds the index in SpatialGrid::grid and SpatialGrid:id that corresponds to 
  // a given set of grid coordinates. "Grid Id" means Grid Coordinates.
  // e.g. (0,0,0) .. (numCells - 1, numCells -1, numCells - 1)
  int GridCoordsToIndex(const iv3 id_to_match) {
    auto sz = id.size();
    for (int i = 0; i < sz; i++) {
      if (id[i] == id_to_match) {
        return i;
      }
    }
    // if we get here, the coords we put in don't exist in the grid
    LogPrintLn("Grid Coords Not Found! Returning -1 from GridCoordsToIndex. The offending id was", glm::to_string(id_to_match));
    return -1;
  }

  // find the the spatial grid cells that the object's center is in.
  // update the Cell.list to contain the object if it isn't already.
  // remove object from lists that is used to be in 
  // @TODO: separate this out so that stationary objects are only added to the 
  // grid during setup.
  // @TODO: RENAME PosToGridCoords to PosToGridCoords
  void UpdateGrid(gxb::Entity* o) {

    auto curr_grid = PosToGridCoords(o->pos);
    auto last_grid = PosToGridCoords(o->pos_last);

    int last_idx = GridCoordsToIndex(last_grid);
    int curr_idx = GridCoordsToIndex(curr_grid);

    if (curr_grid == last_grid && o->has_been_added_to_grid == true) { return; };

    // remove id from last cell's list if the cell changed
    if (curr_grid != last_grid && o->has_been_added_to_grid == true) {
      auto old_end = grid[last_idx].list.end();
      auto new_end = std::remove(grid[last_idx].list.begin(), grid[last_idx].list.end(), o->id);
      if (old_end != new_end) {
        grid[last_idx].list.erase(new_end);
      }
    }
    // add to curr cell's list if it isn't in it already
    auto iter = std::find(grid[curr_idx].list.begin(), grid[curr_idx].list.end(), o->id);
    bool not_in_curr_cell_list = (iter == grid[curr_idx].list.end()) ? true : false;

    if (not_in_curr_cell_list) {
      grid[curr_idx].list.push_back(o->id);
      o->has_been_added_to_grid = true;
      //LogPrintLn("added object id:", o->id, " to grid: ", glm::to_string(id[curr_idx]));
    }

  }

  // find min and max of x,y,z objects in level and build a uniform
  // grid enclosing it
  std::vector<float>& SetupOctree(gxb::Level* level) {
    assert(level != NULL);
    SpatialGrid::level = level;
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
    // want a cubic SpatialGrid, so take max dimension and use that for x,y, and z
    // expand max and min both outward 10%; this is very arbitrary
    min -= 0.1f * (orig_dim);
    max += 0.1f * (orig_dim);
    auto expanded_dim = max - min;
    LogPrintLn("level min, max:", glm::to_string(min), glm::to_string(max));
    // take max dimension as our cube's side length
    float cubic_dim =
      std::max({ expanded_dim.x, expanded_dim.y, expanded_dim.z });
    LogPrintLn("SpatialGrid extent:", cubic_dim);
    worldL = cubic_dim;

    max = min + v3(cubic_dim, cubic_dim, cubic_dim);

    // create SpatialGrid
    topNode = Cell{ BoundingBox{min, max} };
    // Subdivide topNode until
    // 		a bin contains fewer than a given number of points
    // 		a bin reaches a minimum size based on the length of its edges
    //AddLinesToVertBuf(topNode.bb);
    numCells = CalcSideLength();
    LogPrintLn("ideal cellL found: ", cellL, "with numcells:", numCells);

    Subdivide();
    //TestPosToGridIdFxn();
    return vertbufGridLines;
  }

  // fxn uses and modifies namespace globals to calculate
  // the side length and number of cells per dimension of the uniform grid
  int CalcSideLength() {
    assert(cellL < worldL);
    auto numCells = std::floor(worldL / targetSideL);
    while (worldL / cellL > numCells) {
      cellL += 0.00001f;
    }
    return (int)(std::roundf(worldL / cellL));
  }

  // divide world into uniform grid of numCells x numCells x numCells chunks
  // with side lengths equal to cellL >= targetSideL
  void Subdivide() {
    // start from min, add cellL to the x-coord numcells times
    const auto& min = topNode.bb.min;
    v3 old_min{};
    v3 new_min{};
    LogPrintLn("topNode.bb.min", glm::to_string(min));
    // outer loop is over Y
    for (int k = 0; k < numCells; k++) {
      old_min = min + (float)(k)*v3(0.f, cellL, 0.f);
      // middle loop is over Z
      for (int j = 0; j < numCells; j++) {
        if (j != 0) {
          old_min = old_min + v3(0.f, 0.f, cellL);
        }
        // inner loop is over X
        for (int i = 0; i < numCells; i++) {
          new_min = old_min + (float)(i)*v3(cellL, 0.f, 0.f);
          BoundingBox bb{ new_min, new_min + v3{cellL, cellL, cellL} };
          // add Node and cell id to vectors 
          grid.push_back(Cell{ bb });
          id.push_back(iv3{ i, k, j });
          AddLinesToVertBuf(bb);
          //LogPrintLn("min x,y,z:", "(", bb.min.x, ",", bb.min.y, ",", bb.min.z, ")");
          //LogPrintLn("id x,y,z:", "(", id[id.size() - 1].x, ",", id[id.size() - 1].y, ",", id[id.size() - 1].z, ")");
        }
      }
    }
  }

  void TestPosToGridIdFxn() {
    //  test PosToGridCoords()
    //	# should give (2,0,1)
    //	test_pos1 <- c(2.057,-2.900,-38.071) + 0.5*cellL
    //	# should give (6, 6, 6)
    //	test_pos2 <- c(30.171,39.271,-2.929) + 0.5*cellL
    //	# test on a boundary, should give (2,0,1) but does not
    //	test_pos3 <- c(2.057,-2.900,-38.071)
  //
  // ***** note that these values are truncated
    auto test1 = v3(0, 0, 1.649);
    LogPrintLn("TESTING::TestPosToGridIdFxn");
    LogPrintLn(glm::to_string(PosToGridCoords(test1)));
  }



}  // namespace SpatialGrid
