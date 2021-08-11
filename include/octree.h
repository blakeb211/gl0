#pragma once
#include <gamelib.h>
#include <vector>
using v3 = glm::vec3;

namespace octree {

gxb::Level* level;
unsigned int vboOctree{}, vaoOctree{};
std::vector<float> vertices_octree{};

void build_lines_vert_buf(v3 min, v3 max) {
    assert(glm::distance(min, max) > 3);

    auto push_floats = [&](v3 corner) {
	vertices_octree.push_back(corner.x);
	vertices_octree.push_back(corner.y);
	vertices_octree.push_back(corner.z);
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
	push_cube(min,max);
	push_cube(min,min + (max-min)/2.f);
}

void setup(gxb::Level* level) {
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
	auto orig_dim = max-min; 
    min -= 0.1f*(orig_dim);
    max += 0.1f*(orig_dim);

    build_lines_vert_buf(min, max);
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
    glBufferData(GL_ARRAY_BUFFER, vertices_octree.size() * sizeof(float),
		 vertices_octree.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
			  (void*)0);
    glEnableVertexAttribArray(0);

    // unbind the VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void draw_octree() {
    glBindVertexArray(vaoOctree);  // bind the octree vao
    glDrawArrays(GL_LINES, (GLint)0,
		 (GLint)vertices_octree.size());  // uses vboOctree
}

void draw() {
    draw_octree();
}

}  // namespace octree
