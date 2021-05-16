/* Goal: draw an octree */

#define OLC_PGE_APPLICATION
#include <cmath>
#include <vector>
#include "..\include\glm.h"
#include "olcPixelGameEngine.h"
#define PI 3.1415926535
using std::vector;

using v3 = glm::vec3;
using v2 = glm::vec2;
using m44 = glm::mat4;
using v4 = glm::vec4;

constexpr auto g_WIDTH = 450;
constexpr auto g_HEIGHT = 300;

// Override base class with your custom functionality
class Example : public olc::PixelGameEngine {
   public:
    Example() {
	// Name your application
	sAppName = "Example";
    }

   public:
    bool OnUserCreate() override {
	// Called once at the start, so create things here
	projected_points = vector<v2>(points.size());
	return true;
    }

    bool OnUserUpdate(float fElapsedTime) override {
	totElapsedTime += fElapsedTime;
	Clear(olc::BLACK);
	// create projection
	auto points_sz = points.size();
	for (int i = 0; i < points_sz; i++) {
	    auto pt = points[i];
	    model = glm::mat4(1.0f);
	    model = glm::rotate(model, totElapsedTime * 0.2f,
				glm::vec3(0.5, 0.5, 0.5));
	    auto tmp_v4 = projection * view * model * v4(pt.x, pt.y, pt.z, 1.0);
	    projected_points[i].x = tmp_v4.x;
	    projected_points[i].y = tmp_v4.y;
	    // shift and scale
	    projected_points[i].x *= 18.0f;
	    projected_points[i].y *= 18.0f;
	    projected_points[i].x += g_WIDTH / 2.0f;
	    projected_points[i].y += g_HEIGHT / 2.0f;
	}

	// Called once per frame, draws random coloured pixels
	for (int i = 0; i < points_sz - 2; i += 2) {
	    const auto& pt = projected_points[i];
	    const auto& pt2 = projected_points[i + 1];
	    const auto& pt3 = projected_points[i + 2];
	    DrawTriangle(pt.x, pt.y, pt2.x, pt2.y, pt3.x, pt3.y, olc::WHITE);
	}
	return true;
    }
    /* GAME VARS */
    float totElapsedTime;
    vector<v3> points{

	{-1, -1, 1},  {1, -1, 1},   {1, 1, 1},	 {-1, -1, 1},  {1, 1, 1},
	{-1, 1, 1},   {1, -1, 1},   {1, -1, -1}, {1, 1, -1},   {1, -1, 1},
	{1, 1, -1},   {1, 1, 1},    {1, -1, -1}, {-1, -1, -1}, {-1, 1, -1},
	{1, -1, -1},  {-1, 1, -1},  {1, 1, -1},	 {-1, -1, -1}, {-1, -1, 1},
	{-1, 1, 1},   {-1, -1, -1}, {-1, 1, 1},	 {-1, 1, -1},  {-1, 1, 1},
	{1, 1, 1},    {1, 1, -1},   {-1, 1, 1},	 {1, 1, -1},   {-1, 1, -1},
	{1, -1, 1},   {-1, -1, -1}, {1, -1, -1}, {1, -1, 1},   {-1, -1, 1},
	{-1, -1, -1},
    };

    vector<v2> projected_points;
    m44 projection;
    m44 view;
    m44 model;
};

//		  -1

// -1  x     x	  1
//
//
//
//		   1

int main() {
    Example demo;
    demo.model = glm::mat4(1.0f);
    demo.projection = glm::mat4(1.0f);
    demo.view = glm::mat4(1.0f);
    demo.projection = glm::perspective(
	(float)PI / 2.0f, (float)g_WIDTH / (float)g_HEIGHT, 0.1f, 100.0f);

    // build a grid
    for (float i = -1.f; i <= 1.f; i += 0.3333333f) 
	for (float j = -1.f; j <= 1.f; j += 0.3333333f) 
	for (float k = -1.f; k <= 1.f; k += 0.3333333f) {
	demo.points.push_back(glm::vec3{

	}

    if (demo.Construct(g_WIDTH, g_HEIGHT, 2, 2, false, true))
	demo.Start();
    return 0;
}
