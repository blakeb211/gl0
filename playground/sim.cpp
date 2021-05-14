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

constexpr auto g_WIDTH = 160;
constexpr auto g_HEIGHT = 100;

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
	// create projection
	auto points_sz = points.size();
	for (int i = 0; i < points_sz; i++) {
		auto pt = points[i];
		pt.z += 10.0f;
		model = glm::mat4(1.0f);
		model = glm::rotate(model, fElapsedTime * 0.4f, glm::vec3(1.0,1.0,1.0));
	    auto tmp_v4 = projection * view * model * v4(pt.x, pt.y, pt.z, 1.0);
		projected_points[i].x = tmp_v4.x;
		projected_points[i].y = tmp_v4.y;
		// shift and scale
		projected_points[i].x *= 25.0f;
		projected_points[i].y *= 25.0f;
		projected_points[i].x += g_WIDTH / 3.0f;
		projected_points[i].y += g_HEIGHT / 3.0f;
	}

	// Called once per frame, draws random coloured pixels
	for (int i = 0; i < points_sz; i++) {
	const auto & pt = projected_points[i];
	Draw(pt.x, pt.y,
		     olc::BLUE);
	}
	return true;
    }
    /* GAME VARS */
    vector<v3> points{

{-1,-1,1}, {1,-1,1}, {1,1,1},
 {-1,-1,1}, {1,1,1}, {-1,1,1},
 {1,-1,1}, {1,-1,-1}, {1,1,-1},
 {1,-1,1}, {1,1,-1}, {1,1,1},
 {1,-1,-1}, {-1,-1,-1}, {-1,1,-1},
 {1,-1,-1}, {-1,1,-1}, {1,1,-1},
 {-1,-1,-1}, {-1,-1,1}, {-1,1,1},
 {-1,-1,-1}, {-1,1,1}, {-1,1,-1},
 {-1,1,1}, {1,1,1}, {1,1,-1},
 {-1,1,1}, {1,1,-1}, {-1,1,-1},
 {1,-1,1}, {-1,-1,-1}, {1,-1,-1},
 {1,-1,1}, {-1,-1, 1}, {-1,-1,-1},
 };

    vector<v2> projected_points;
    m44 projection;
    m44 view;
    m44 model;
};

int main() {
    Example demo;
    demo.model = glm::mat4(1.0f);
    demo.projection = glm::mat4(1.0f);
    demo.view = glm::mat4(1.0f);
    demo.projection = glm::perspective(
	(float)PI / 2.0f, (float)g_WIDTH / (float)g_HEIGHT, 0.1f, 100.0f);

    if (demo.Construct(g_WIDTH, g_HEIGHT, 4, 4))
	demo.Start();
    return 0;
}
