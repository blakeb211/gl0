#define OLC_PGE_APPLICATION
#include <vector>
#include "olcPixelGameEngine.h"

using namespace std;

struct vec3 {
    float x, y, z;
};

struct camPath {
    camPath() = delete;
    camPath(vector<vec3> control_points) { cps = control_points; }
    void createPathFromCps() {
	for (int cpIdx = 0; cpIdx <= cps.size() - 3; cpIdx += 3) {
		vec3 pt0, pt1, p2;
	    auto [x, y, z, t] = {0.f, 0.f, 0.f, 0.f};
	    x = (1 - t) * (1 - t) * pt0.x + 2 * (1 - t) * t * pt1.x + t * t * pt2.x;
	    y = (1 - t) * (1 - t) * pt0.y + 2 * (1 - t) * t * pt1.y + t * t * pt2.y;
	    z = (1 - t) * (1 - t) * pt0.z + 2 * (1 - t) * t * pt1.z + t * t * pt2.z;
	}
    }
    vector<vec3> cam_dir;
    vector<vec3> pts;
   private:
    vector<vec3> cps;
};

class Example : public olc::PixelGameEngine {
   public:
    Example() {
	camPath cp{};
	sAppName = "Example";
    }

   public:
    bool OnUserCreate() override {
	// Called once at the start, so create things here
	return true;
    }

    bool OnUserUpdate(float fElapsedTime) override {
	// called once per frame
	for (int x = 0; x < ScreenWidth(); x++)
	    for (int y = 0; y < ScreenHeight(); y++)
		Draw(x, y, olc::Pixel(0, 0, rand() % 255));
	return true;
    }
};

int main() {
    Example demo;
    if (demo.Construct(256, 256, 4, 4))
	demo.Start();

    return 0;
}
