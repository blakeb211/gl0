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
 *  This file builds an Octree but the rendering is all handed by render.cpp
 *  ***************************************************************/

namespace octree {

gxb::Level* level;
std::vector<float> vertices_linegrid{};

struct BoundingBox {
    v3 min, max;
};

void build_lines_vert_buf(v3 min, v3 max) {
    assert(glm::distance(min, max) > 3);

    auto push_floats = [&](v3 corner) {
	vertices_linegrid.push_back(corner.x);
	vertices_linegrid.push_back(corner.y);
	vertices_linegrid.push_back(corner.z);
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
    push_cube(min, max);
    push_cube(min, min + (max - min) / 2.f);
}

unsigned int setup(gxb::Level* level) {
    assert(level != NULL);
    octree::level = level;
    glm::vec3 min{0}, max{0};
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

    logPrintLn("level min,max:", glm::to_string(min), glm::to_string(max));
    logPrintLn("octree extent:", glm::to_string(max - min));

    // add a buffer around octree
    auto orig_dim = max - min;
    min -= 0.1f * (orig_dim);
    max += 0.1f * (orig_dim);

    build_lines_vert_buf(min, max);
    logPrintLn("vertices_Octree.size = ", vertices_linegrid.size());

    return render::buildOctreeVAO(vertices_linegrid);
}

void draw_octree(unsigned int vaoOctree) {
    glBindVertexArray(vaoOctree);  // bind the octree vao
    glDrawArrays(GL_LINES, (GLint)0,
		 (GLint)vertices_linegrid.size());  // uses vboOctree
}

void draw(unsigned int vaoOctree) {
    draw_octree(vaoOctree);
}

}  // namespace octree
