#pragma once
#include "flags.h"
#include "gamelib.h"
#include <climits>
#include <vector>

using v3 = glm::vec3;
using iv3 = glm::ivec3;
/* **************************************************************
 *  This file builds a uniform grid for collision testing; rendering handled by
 * render.cpp
 *  ***************************************************************/

namespace SpatialGrid
{

// forward declarations
void Subdivide();
int CalcSideLength();
iv3 PosToGridCoords(const v3 &pos);
void TestingStuffForOctree();

struct BoundingBox
{
	v3 min, max;
};

constexpr size_t default_cell_list_length = 20;

// Individual cell of the uniform grid
struct Cell
{
	Cell() : list(default_cell_list_length)
	{
		list.resize(0);
	}
	Cell(BoundingBox bb) : list(default_cell_list_length), bb{bb}
	{
		list.resize(0);
	}
	// extent of this cell
	BoundingBox bb;
	// list of object ids that are partly or wholly in this cell
	std::vector<int> list;
};

// Globals
gxb::Level *level;					   // @TODO: add const
std::vector<float> vertbufGridLines{}; // for drawing, vertices of Octree bounding box lines
constexpr float targetSideL = 1.74f;
constexpr int MAX_CELL_OCCUPATION_PER_ENTITY = 16;
constexpr int MAX_ENTITIES = 500;
int numCells{};
float cellL{}, worldL{};
Cell topNode; // whole world

std::vector<std::vector<unsigned>> entity_id_to_list_of_cell_ids{MAX_ENTITIES};

// uniform grid cells; size is numCells^3
std::vector<Cell> grid;
// grid coordinates e.g. 0,1,2 that correspond to each cell in grid
std::vector<iv3> id;

size_t GetVertBufGridLinesSize()
{
	return vertbufGridLines.size();
}

iv3 GridIndexToId(const size_t idx)
{
	if constexpr (Flags::USE_ASSERTIONS)
		assert(idx < id.size() && idx >= 0);

	return id[idx];
}

void AddLinesToVertBuf(const BoundingBox x)
{
	const auto min = x.min;
	const auto max = x.max;

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
iv3 PosToGridCoords(const v3 &pos)
{
	const auto &min = topNode.bb.min;
	const auto diff = (pos - min) / cellL;
	// truncate to integer values
	return iv3{trunc(diff.x), trunc(diff.y), trunc(diff.z)};
}

size_t GridCoordsToIndex(const iv3 coords) {
	if (coords.x < 0 || coords.y < 0 || coords.z < 0 || coords.x > numCells -1 || coords.y > numCells - 1 || coords.z > numCells - 1)
		return UINT_MAX;
	return coords.x + numCells * (coords.z + numCells*coords.y); 
}

// this is a scratch pad vector used in the UpdateGridMethod
std::vector<iv3> grid_cells_entity_intersects;

void ClearGrid()
{
	const auto sz = grid.size();
	for (int i = 0; i < sz; i++)
		grid[i].list.resize(0);
}

void UpdateGrid(gxb::Entity *const o)
{
	if constexpr (Flags::USE_ASSERTIONS)
		assert(o != nullptr);

	const auto &o_mesh = level->GetMesh(o->mesh_hash);
	const auto &o_radius = o_mesh->spherical_diameter / 2.0f;

	grid_cells_entity_intersects.resize(0);

	grid_cells_entity_intersects.push_back(PosToGridCoords(o->pos)); // center of obj

	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(0.f, 0.f, 1.f))); // +z boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(0.f, 0.f, -1.f))); // -z boundary of obj

	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(1.f, 0.f, 0.f))); // +x boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(-1.f, 0.f, 0.f))); // -x boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(0.f, 1.f, 0.f))); // +y boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(0.f, -1.f, 0.f))); // -y boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(1.f, 1.f, 0.f))); // +x+y boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(1.f, -1.f, 0.f))); // +x-y boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(-1.f, 1.f, 0.f))); // -x+y boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(-1.f, -1.f, 0.f))); // -x-y boundary of obj

	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(1.f, 0.f, 1.f))); // +x+z boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(-1.f, 0.f, 1.f))); // -x+z boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(0.f, 1.f, 1.f))); // +y+z boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(0.f, -1.f, 1.f))); // -y+z boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(1.f, 1.f, 1.f))); // +x+y+z boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(1.f, -1.f, 1.f))); // +x-y+z boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(-1.f, 1.f, 1.f))); // -x+y+z boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(-1.f, -1.f, 1.f))); // -x-y+z boundary of obj

	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(1.f, 0.f, -1.f))); // +x-z boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(-1.f, 0.f, -1.f))); // -x-z boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(0.f, 1.f, -1.f))); // +y-z boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(0.f, -1.f, -1.f))); // -y-z boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(1.f, 1.f, -1.f))); // +x+y-z boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(1.f, -1.f, -1.f))); // +x-y-z boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(-1.f, 1.f, -1.f))); // -x+y-z boundary of obj
	grid_cells_entity_intersects.push_back(
		PosToGridCoords(o->pos + o_radius * v3(-1.f, -1.f, -1.f))); // -x-y-z boundary of obj

	decltype(grid_cells_entity_intersects)::iterator end_it;
	end_it = std::unique(grid_cells_entity_intersects.begin(), grid_cells_entity_intersects.end());

	for (auto curr_coord_it = grid_cells_entity_intersects.begin(); curr_coord_it != end_it; curr_coord_it++)
	{
		size_t curr_idx = GridCoordsToIndex(*curr_coord_it);
		if (curr_idx != UINT_MAX)
			grid[curr_idx].list.push_back(o->id);
	}
}

// find min and max of x,y,z objects in level and build a uniform
// grid enclosing it
std::vector<float> &SetupOctree(gxb::Level *level)
{
	if constexpr (Flags::USE_ASSERTIONS)
		assert(level != NULL);

	grid_cells_entity_intersects.resize(30);

	const auto sz = entity_id_to_list_of_cell_ids.size();
	for (int i = 0; i < sz; i++)
	{
		entity_id_to_list_of_cell_ids[i] = std::vector<unsigned>(MAX_CELL_OCCUPATION_PER_ENTITY);
		entity_id_to_list_of_cell_ids[i].resize(0);
	}

	// match object id to cells that it occupies
	SpatialGrid::level = level;
	glm::vec3 min{0}, max{0};

	for (auto &i : level->objects)
	{
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
	Log::PrintLn("level min, max:", glm::to_string(min), glm::to_string(max));
	// take max dimension as our cube's side length
	float cubic_dim = std::max({expanded_dim.x, expanded_dim.y, expanded_dim.z});
	Log::PrintLn("SpatialGrid extent:", cubic_dim);
	worldL = cubic_dim;

	max = min + v3(cubic_dim, cubic_dim, cubic_dim);

	// create SpatialGrid
	topNode = Cell{BoundingBox{min, max}};
	// Subdivide topNode until
	// 		a bin contains fewer than a given number of points
	// 		a bin reaches a minimum size based on the length of its edges
	// AddLinesToVertBuf(topNode.bb);
	numCells = CalcSideLength();
	Log::PrintLn("ideal cellL found: ", cellL, "with numcells:", numCells);

	Subdivide();
	//TestingStuffForOctree();
	// returns reference to the spatial grid's outlines so they can be drawn
	return vertbufGridLines;
}

// fxn uses and modifies namespace globals to calculate
// the side length and number of cells per dimension of the uniform grid
int CalcSideLength()
{
	if constexpr (Flags::USE_ASSERTIONS)
		assert(cellL < worldL);

	auto numCells = std::floor(worldL / targetSideL);
	while (worldL / cellL > numCells)
	{
		cellL += 0.00001f;
	}
	return (int)(std::roundf(worldL / cellL));
}

// divide world into uniform grid of numCells x numCells x numCells chunks
// with side lengths equal to cellL >= targetSideL
void Subdivide()
{
	// start from min, add cellL to the x-coord numcells times
	const auto &min = topNode.bb.min;
	v3 old_min{};
	v3 new_min{};
	Log::PrintLn("topNode.bb.min", glm::to_string(min));
	// outer loop is over Y
	for (int k = 0; k < numCells; k++)
	{
		old_min = min + (float)(k)*v3(0.f, cellL, 0.f);
		// middle loop is over Z
		for (int j = 0; j < numCells; j++)
		{
			if (j != 0)
			{
				old_min = old_min + v3(0.f, 0.f, cellL);
			}
			// inner loop is over X
			for (int i = 0; i < numCells; i++)
			{
				new_min = old_min + (float)(i)*v3(cellL, 0.f, 0.f);
				BoundingBox bb{new_min, new_min + v3{cellL, cellL, cellL}};
				// add Node and cell id to vectors
				grid.push_back(Cell{bb});
				id.push_back(iv3{i, k, j});
				AddLinesToVertBuf(bb);
			}
		}
	}
}

//@TODO: ADD PROPER TESTS USING CTEST OR GOOGLE TEST
void TestingStuffForOctree()
{
	Log::PrintLn("TESTING::TestingStuffForOctree");
	using std::cout, std::endl;
	cout << "id.size():" << id.size() << endl;
	for (int i = 0; i < id.size(); i++)
	Log::PrintLn(id[i].x,id[i].y,id[i].z, i);
}

} // namespace SpatialGrid
