// USAGE: This program reads in a level and campath file, lets users modify the
// campath with the mouse, and then saves the campath. Initial campath is
// created from a ppm file using a different program.
//
#define OLC_PGE_APPLICATION
#include "..\include\gamelib.h"
#include "..\include\glm.h"
#include "..\include\headers.h"
#include "olcPixelGameEngine.h"
#include "ppm_reader.h"
// @NOTE: campath editor operates in world coordinators

using std::make_pair, std::make_unique, std::string;
using std::unique_ptr, std::tie, std::to_string, std::max_element;
using std::vector, std::pair, std::cout, glm::vec3, std::endl;
namespace fs = std::filesystem;

/************************************************
 *					GLOBALS
 *
 ************************************************/
size_t frameCnt = 0;
constexpr auto WORLD_Y_MAX = 30.0f;
constexpr auto WORLD_X_MAX = 50.0f;
float z_data_max{};
float y_data_max{};
float x_data_max{};
vector<vec3> g_cps{};

constexpr auto comp_zmax = [](const glm::vec3& a, const glm::vec3& b) -> bool {
    return a.z < b.z;
};
constexpr auto comp_ymax = [](const glm::vec3& a, const glm::vec3& b) -> bool {
    return a.y < b.y;
};
constexpr auto comp_xmax = [](const glm::vec3& a, const glm::vec3& b) -> bool {
    return a.x < b.x;
};

/************************************************/
//@TODO:
// pop up pt coords
// print out camPath points in a table
// ability to drag control points to change the path
// ability to save camPath

struct camPath {
    camPath() = delete;
    camPath(vector<vec3> control_points) { cps = control_points; }
    void createPathFromCps() {
	for (size_t cpIdx = 0; cpIdx <= cps.size() - 3; cpIdx += 2) {
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

enum class View {
    ZY = 0,
    ZX = 1,
};

pair<float, float> world_to_screen(const olc::PixelGameEngine* gm,
				   const View vw,
				   const glm::vec3 in) {
    float y_offset{}, x_offset{};
    float y_scale{}, x_scale{};
    float x_coord{}, y_coord{};
    switch (vw) {
	case View::ZY:
	    // Y is still Y but shifted
	    // Z is shown on the X-Axis
	    x_offset = 10;
	    x_scale = gm->ScreenWidth() / (z_data_max + x_offset);
	    x_coord = x_offset + in.z * x_scale;

	    y_offset = gm->ScreenHeight() / 2.f;
	    y_scale = -1.f * gm->ScreenHeight() / (2.f * WORLD_Y_MAX);
	    y_coord = y_offset + in.y * y_scale;

	    break;
	case View::ZX:
	    // X data is shown on the Y-Axis
	    // Z is shown on the X-Axis
	    x_offset = 10;
	    x_scale = gm->ScreenWidth() / (z_data_max + x_offset);
	    x_coord = x_offset + in.z * x_scale;

	    y_offset = gm->ScreenHeight() / 2.f;
	    y_scale = -1.f * gm->ScreenHeight() / (2.f * WORLD_X_MAX);
	    y_coord = y_offset + in.x * y_scale;
	    break;
    };
    return make_pair(x_coord, y_coord);
}

class Example : public olc::PixelGameEngine {
   public:
    View currView{View::ZX};
    unique_ptr<camPath> path;

   public:
    Example() { sAppName = "Example"; }
    bool OnUserCreate() override {
	// Called once at the start, so create things here
	this->path = make_unique<camPath>(camPath(g_cps));
	return true;
    }

    bool OnUserUpdate(float fElapsedTime) override {
	frameCnt++;
	this->Clear(olc::Pixel(olc::DARK_GREY));

	auto keyState = GetKey(olc::Key::V);
	if (keyState.bReleased) {
	    if (currView == View::ZY) {
		currView = View::ZX;
	    } else {
		currView = View::ZY;
	    }
	}

	// draw axes
	// ZY
	// Z = 0 					X = ScreenHeight / 2
	// Z = WorldZMax 			X = ScreenWidth
	// Y = 0 					ScreenHeight / 2
	// Y = WorldYMax 			0
	//

	if (frameCnt % 15 == 0) {
	    const auto z_data_max_it =
		max_element(path->pts.begin(), path->pts.end(), comp_zmax);
	    const auto y_data_max_it =
		max_element(path->pts.begin(), path->pts.end(), comp_ymax);
	    const auto x_data_max_it =
		max_element(path->pts.begin(), path->pts.end(), comp_xmax);

	    z_data_max = (*z_data_max_it).z;
	    y_data_max = (*y_data_max_it).y;
	    x_data_max = (*x_data_max_it).x;
	}

	// draw axes
	int x0{}, y0{}, xmax{}, ymax{};
	switch (currView) {
	    case View::ZY:
		// y = y + ScreenHeight/2
		tie(x0, y0) =
		    world_to_screen(this, View::ZY, glm::vec3{0.f, 0.f, 0.f});
		tie(xmax, ymax) = world_to_screen(
		    this, View::ZY, glm::vec3{0.f, WORLD_Y_MAX, z_data_max});
		DrawLine(x0, y0, xmax, y0, olc::WHITE);
		DrawLine(xmax / 2, ymax, xmax / 2, y0, olc::WHITE);
		break;
	    case View::ZX:
		tie(x0, y0) =
		    world_to_screen(this, View::ZY, glm::vec3{0.f, 0.f, 0.f});
		tie(xmax, ymax) = world_to_screen(
		    this, View::ZY, glm::vec3{0.f, WORLD_X_MAX, z_data_max});
		DrawLine(x0, y0, xmax, y0, olc::WHITE);
		DrawLine(xmax / 2, ymax, xmax / 2, y0, olc::WHITE);
		break;
	};

	// draw path
	// draw control points
	switch (currView) {
	    case View::ZY:

		for (const auto& pt : path->pts) {
		    auto [x, y] = world_to_screen(this, View::ZY, pt);
		    Draw(x, y, olc::RED);
		    DrawCircle(x, y, 3, olc::RED);
		}

		for (const auto& pt : path->cps) {
		    // draw circles around control pts
		    auto [x, y] = world_to_screen(this, View::ZY, pt);
		    Draw(x, y, olc::GREEN);
		    DrawCircle(x, y, 3, olc::GREEN);
		}

		break;
	    case View::ZX:

		for (const auto& pt : path->pts) {
		    auto [x, y] = world_to_screen(this, View::ZX, pt);
		    Draw(x, y, olc::RED);
		    DrawCircle(x, y, 3, olc::RED);
		}

		for (const auto& pt : path->cps) {
		    // draw circles around control pts
		    auto [x, y] = world_to_screen(this, View::ZX, pt);
		    Draw(x, y, olc::GREEN);
		    DrawCircle(x, y, 3, olc::GREEN);
		}

		break;
	};

	// draw axis labels
	switch (currView) {
	    {
		case View::ZY:
		    const string z_label_left = "Z(0)";
		    const string y_label_top =
			"Y(" + to_string((int)WORLD_Y_MAX) + ")";
		    float x = 2.0f, y = ScreenHeight() / 2.f + 5.f;
		    DrawString(x, y, z_label_left, olc::WHITE, 1);
		    tie(x, y) = world_to_screen(
			this, View::ZY,
			glm::vec3{0.f, WORLD_Y_MAX, (z_data_max + 2) / 2.f});
		    DrawString(x, y, y_label_top, olc::WHITE, 1);
		    break;
	    }
	    case View::ZX: {
		const string z_label_left = "Z(0)";
		const string y_label_top =
		    "X(" + to_string((int)WORLD_X_MAX) + ")";
		float x = 2.0f, y = ScreenHeight() / 2.f + 5.f;
		DrawString(x, y, z_label_left, olc::WHITE, 1);
		tie(x, y) = world_to_screen(
		    this, View::ZX,
		    glm::vec3{WORLD_X_MAX, 0.f, (z_data_max + 2) / 2.f});
		DrawString(x, y, y_label_top, olc::WHITE, 1);
		break;
	    }
	};
	return true;
    }
};

int main(int argc, char** argv) {
    if (argc != 2) {
	cout << argv[0];
	cout << ":must give an argument to run. Either\n";
	cout << "A) a ppm file to load a campath from a ppm OR\n";
	cout << "B) a level name to load a level and its campath\n";
	return -1;
    }
    setLogFile("log.txt");

    // read level name
    string fName{argv[1]};

    vector<vec3> cps{};
    // if this is a ppm file, create a campath from the ppm
    if (is_ppm_file) {
	g_cps = get_cps_from_ppm(fName);
    }
    if (is_level_file) {
	// if this is a level file (no extension), load level and campath
	g_cps = get_cps_from_campath(fName);
    }

	// construct an engine object using the g_cps as the source of the control points
    // this matches a screen dimension of roughly 1000x700
    Example demo;
    //if (demo.Construct(500, 350, 2, 2, false, true, false)) {
	//demo.Start();
    //}
    //demo.path->createPathFromCps();

    cout << "file name" << fName << endl;

    //auto level = gxb::load_level("test");


    return 0;
}
