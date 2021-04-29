#define OLC_PGE_APPLICATION
#include "..\include\headers.h"
#include "olcPixelGameEngine.h"
#include "..\include\gamelib.h"
#include "..\include\glm.h" 

using namespace std;

using vec3 = glm::vec3;

struct camPath {
    camPath() = delete;
    camPath(vector<vec3> control_points) { cps = control_points; }
    void createPathFromCps() {
	// 0 1 2 3 4 5

	for (int cpIdx = 0; cpIdx <= cps.size() - 3; cpIdx += 2) {
	    const vec3& p0 = cps[cpIdx];
	    const vec3& p1 = cps[cpIdx + 1];
	    const vec3& p2 = cps[cpIdx + 2];
	    float x, y, z, t;
	    x = y = z = t = 0.0f;
	    while (t < 1.0) {
		x = (1 - t) * (1 - t) * p0.x + 2 * (1 - t) * t * p1.x +
		    t * t * p2.x;
		y = (1 - t) * (1 - t) * p0.y + 2 * (1 - t) * t * p1.y +
		    t * t * p2.y;
		z = (1 - t) * (1 - t) * p0.z + 2 * (1 - t) * t * p1.z +
		    t * t * p2.z;
		pts.push_back(vec3{x, y, z});
		t += 0.10f;
	    }
	}
    }
    vector<vec3> cam_dir;
    vector<vec3> pts;
    vector<vec3> cps;
};

class Example : public olc::PixelGameEngine {
   public:
    Example() { sAppName = "Example"; }

   public:
    bool OnUserCreate() override {
	// Called once at the start, so create things here
	vector<vec3> cps = {vec3{3.0f, 100.0f, 0.f},
		vec3{50.0f, 120.f, 0.f},
			    vec3{75.0f, 100.f, 0.f},
			    vec3{100.0f, 85.f, 0.f},
			    vec3{125.0f, 100.f, 0.f},
			    vec3{175.0f, 110.f, 0.f},
			    vec3{220.0f, 100.f, 0.f},
			    vec3{240.0f, 85.f, 0.f},
			    vec3{256.0f, 95.f, 0.f}};
	path = make_unique<camPath>(camPath(cps));
	cout << "number of control points: " << path->cps.size() << endl;
	path->createPathFromCps();
	return true;
    }

    bool OnUserUpdate(float fElapsedTime) override {

	this->Clear(olc::Pixel(olc::DARK_GREY));

    // draw path
	for (const auto & pt : path->pts) {
		Draw(pt.x, pt.y, olc::Pixel(255, 0, 10));
	}
	
	// draw control points	
	for (const auto & pt : path->cps) {
		Draw(pt.x, pt.y, olc::Pixel(0, 255, 10));
	}
	
	return true;
    }
	unique_ptr<camPath> path;
};

int main() {
	//auto level = gxb::load_level("test"); Example demo;
	// @TODO: try LogPrintLn here 
	string s = "test string";
	//logPrintLn("testing logPrintLn", s, "string printed before this"); // errors
 	//logErr(s, 86, s);
	Example demo;
    if (demo.Construct(256, 256, 3, 3, false, true, false))
	demo.Start();

    return 0;
}
