#include <glad\glad.h>
//
#include <GLFW\glfw3.h>
#include <stb\stb_image.h>
//
#include "framerater.h"
#include "gamelib.h"
#include "glm.h"
#include "headers.h"
#include "render.h"
#include "shader.h"
#include <magic_enum.h>

// -------------------------------------------
// TYPEDEFS
// -------------------------------------------
using VecPP = std::vector<gxb::PathPt>;
using v3 = glm::vec3;
using iv3 = glm::ivec3;

// -------------------------------------------
// FORWARD DECLARATIONS
// -------------------------------------------
namespace SpatialGrid
{ // if SpatialGrid wasn't header only I could remove this
std::vector<float> &SetupOctree(gxb::Level *);
void UpdateGrid(gxb::Entity *const);
void ClearGrid();
iv3 PosToGridCoords(const v3 &pos);
}; // namespace SpatialGrid

void TestNaiveCollision();
void FrameBufSizeCallback(GLFWwindow *window, int width, int height);
void ProcessInputCamOnly(GLFWwindow *window, gxb::Camera &cam, float delta_time);
void ProcessInputPlayerOnly(GLFWwindow *window, float delta_time);
void MouseCallback(GLFWwindow *window, double x_pos, double y_pos);
void MouseCallbackNull(GLFWwindow *window, double x_pos, double y_pos);
void ScrollCallback(GLFWwindow *window, double x_offset, double y_offset);
v3 SelectNextCamPoint(const gxb::Level *const l, gxb::Camera &cam, const VecPP &newCamPos);
void AddCamPathToRawData(const VecPP &path, gxb::Level *l);

GLFWwindow *InitGlfw(unsigned int w, unsigned int h, const char *title, GLFWframebuffersizefun);
void CamGoalSeek(float delta_time);
// -------------------------------------------
// GLOBALS
// -------------------------------------------
gxb::Camera camera{};
float last_x = gxb::SCR_WIDTH / 2, lastY = gxb::SCR_HEIGHT / 2;
bool first_mouse = true;
std::unique_ptr<gxb::Level> level = nullptr;
std::vector<gxb::PathPt> path;
constexpr auto CAM_MOVE_SPEED = 0.001f;

int main()
{
	gxb::initTypeToStrMap(); // creates str_to_type
	FrameRater fr{};
	SetLogFile("log.txt");

	const auto &w = gxb::SCR_WIDTH;
	const auto &h = gxb::SCR_HEIGHT;
	GLFWwindow *window = InitGlfw(w, h, "Learn OpenGL ", FrameBufSizeCallback);

	//@TODO: add ability to switch levels while game running
	// Load Level
	auto level_name = "test";

	//@TODO: Not sure if there is a point in doing this in another thread unless I'm
	// going to save it to a future_ptr, do some other work and call .get() on it later
	// Possibly it is preventing the screen from freezing?
	level = async(std::launch::async, gxb::LoadLevel, level_name).get();
	path = async(std::launch::async, gxb::LoadCamPath, level_name).get();

	auto vertBufGridLinesRef = SpatialGrid::SetupOctree(level.get());
	auto vao_spatial_grid = render::BuildSpatialGridVao(vertBufGridLinesRef);
	TestNaiveCollision();

	auto prog_one = Shader(*gxb::ShaderPath("3pos3color.vs"), *gxb::ShaderPath("colorFromVertex.fs"));

	// add camPath points to level raw_data before building VAO
	// so I can draw them if I need to debug.
	AddCamPathToRawData(path, level.get());

	render::SetGlFlags();
	auto vao = render::BuildLevelVao(level.get());

	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	// lambda for moving platform behavior
	auto update_platform_pos = [&](std::unique_ptr<gxb::Entity> &o, const v3 &pos_dir, const float &frame_time,
								   float speedup = 1.0) {
		v3 facing = o->pos - o->pos_last;
		facing = glm::normalize(facing);
		o->pos_last = o->pos;
		if (magic_enum::enum_name(o->state_machine.current) == "pos")
		{
			o->pos += pos_dir * 0.001f * frame_time * speedup;
		}
		else
		{
			o->pos += -1.0f * pos_dir * 0.001f * frame_time * speedup;
		}
		o->state_machine.check_transition(glm::distance(o->pos, o->pos_start), facing, 4, pos_dir);
	};

	// Game loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		fr.UpdateTimes();
		float delta_time = fr.lastTimeInMs();

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		if (Flags::FREE_MOVE)
			ProcessInputCamOnly(window, camera, delta_time);
		else
		{
			ProcessInputPlayerOnly(window, delta_time);
			CamGoalSeek(delta_time);
		}

		// update window title with player position
		const glm::vec3 &pos = level->objects[0]->pos;
		const glm::ivec3 grid_coords{SpatialGrid::PosToGridCoords((v3 &)pos)};
		auto str = std::string(glm::to_string(pos) + " " + glm::to_string(grid_coords));
		glfwSetWindowTitle(window, str.c_str());

		prog_one.Use();

		// update objects
		// movement is moderated by the elapsed time in case we change the framerate later
		for (auto &o : level->objects)
		{
			auto const elapsed = fr.lastTimeInMs();
			switch (o->type)
			{
				v3 pos_dir;
			case gxb::EntityType::moving_ground_x:
				pos_dir = v3(1.f, 0.f, 0.f);
				update_platform_pos(o, pos_dir, elapsed, 1.3f);
				break;
			case gxb::EntityType::moving_ground_y:
				pos_dir = v3(0.f, 1.f, 0.f);
				update_platform_pos(o, pos_dir, elapsed, 2.0f);
				break;
			case gxb::EntityType::moving_ground_z:
				pos_dir = v3(0.f, 0.f, 1.f);
				update_platform_pos(o, pos_dir, elapsed, 5.0f);
				break;
			}
		}
		// update spatial grid
		if (fr.frame_count % 2 == 0)
		{
			SpatialGrid::ClearGrid();
			for (auto &o : level->objects)
				SpatialGrid::UpdateGrid(o.get());
		}

		// set transformations
		view = camera.GetViewMatrix();
		prog_one.SetMat4("view", view);

		projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
		prog_one.SetMat4("projection", projection);

		// render
		// ------
		render::clearScreen();
		render::DrawLevel(vao, prog_one, vao_spatial_grid, level.get(), path);
		glfwSwapBuffers(window);
		glfwPollEvents();
		fr.printFrameRateIfFreqHasBeenReached();
	} // end game loop

	glfwTerminate();
	closeLog();
	return 0;
}

// process all input: move player only
// ---------------------------------------------------------------------------------------------------------
void ProcessInputPlayerOnly(GLFWwindow *window, float delta_time)
{
	const auto playerSpeed = 0.017f;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	auto &pos = level->objects[0]->pos;
	// save last position before changing the position
	level->objects[0]->pos_last = pos;

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		pos.x += playerSpeed * delta_time;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		pos.x -= playerSpeed * delta_time;
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		pos.z -= playerSpeed * delta_time;
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		pos.z += playerSpeed * delta_time;
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		pos.y += playerSpeed * delta_time;
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS)
	{
		pos.y -= playerSpeed * delta_time;
	}

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		__noop;

	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		__noop;
}

// process all input: move camera only
// ---------------------------------------------------------------------------------------------------------
void ProcessInputCamOnly(GLFWwindow *window, gxb::Camera &cam, float delta_time)
{
	const float cameraSpeed = 0.16f; // adjust accordingly
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		cam.ProcessKeyboard(gxb::Camera_Movement::RIGHT, delta_time, cameraSpeed);

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		cam.ProcessKeyboard(gxb::Camera_Movement::LEFT, delta_time, cameraSpeed);

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		cam.ProcessKeyboard(gxb::Camera_Movement::FORWARD, delta_time, cameraSpeed);

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		cam.ProcessKeyboard(gxb::Camera_Movement::BACKWARD, delta_time, cameraSpeed);

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		__noop;

	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		__noop;
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void FrameBufSizeCallback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void MouseCallback(GLFWwindow *window, double x_pos, double y_pos)
{
	if (first_mouse)
	{
		last_x = (float)x_pos;
		lastY = (float)y_pos;
		first_mouse = false;
	}

	float xoffset = (float)x_pos - last_x;
	float yoffset = lastY - (float)y_pos; // reversed since y-coordinates go from bottom to top

	last_x = (float)x_pos;
	lastY = (float)y_pos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void MouseCallbackNull(GLFWwindow *window, double xpos, double ypos)
{
	__noop;
	return;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll((float)yoffset);
}

GLFWwindow *InitGlfw(unsigned int w, unsigned int h, const char *title, GLFWframebuffersizefun fun)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow *window = glfwCreateWindow(w, h, title, NULL, NULL);
	if (window == NULL)
	{
		LogPrintLn("Failed to create GLFW window");
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, fun);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		LogPrintLn("Failed to initialize GLAD");
		return nullptr;
	}

	if (Flags::FREE_MOVE)
		glfwSetCursorPosCallback(window, MouseCallback);
	else
		glfwSetCursorPosCallback(window, MouseCallbackNull);

	glfwSetScrollCallback(window, ScrollCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (Flags::VSYNC)
		glfwSwapInterval(1); // vsync on
	else
		glfwSwapInterval(0); // vsync off

	render::logOpenGLInfo();

	render::clearScreen();

	return window;
}

v3 SelectNextCamPoint(const gxb::Level *const l, gxb::Camera &cam, const VecPP &path)
{
	const auto heroPos = l->objects[0]->pos;
	const auto negZvec = v3{0.f, 0.f, -1.f};

	size_t path_sz = path.size();
	std::vector<std::pair<float, float>> dist_ang(path_sz);

	v3 pathPos{}, cam2hero{};
	float dist{}, angle{};

	for (int i = 0; i < path_sz; i++)
	{
		pathPos = path[i].pos;
		cam2hero = v3{glm::normalize(heroPos - pathPos)};
		dist = glm::distance(heroPos, path[i].pos);
		angle = glm::acos(glm::dot(negZvec, cam2hero));
		dist_ang[i].first = dist;
		dist_ang[i].second = angle;
	}

	// find first pathPt that has distance < 20 && angle < 40
	using pair = std::pair<float, float>;
	// set camera to new position
	constexpr auto distCutoff = 30;
	constexpr auto angCutoff = 20;

	auto check = [distCutoff, angCutoff](const pair &dap) {
		return (dap.first < distCutoff && dap.second < angCutoff);
	};

	for (int i = 0; i < path_sz; i++)
	{
		if (check(dist_ang[i]))
		{
			return path[i].pos;
		}
	}
	return v3{}; // if no valid campath found, cam goes to 0,0,0
}

void AddCamPathToRawData(const VecPP &path, gxb::Level *l)
{
	auto func = [l](const gxb::PathPt pp) {
		for (int i = 0; i < 3; i++)
		{
			l->raw_data.push_back(pp.pos[i]);
		}
	};
	std::for_each(path.begin(), path.end(), func);
}

void CamGoalSeek(float delta_time)
{
	auto new_cam_goal_pos = SelectNextCamPoint(level.get(), camera, path);
	// smoothly move cam towards goal pos
	if (camera.Position != new_cam_goal_pos)
	{
		const auto camDp = new_cam_goal_pos - camera.Position;
		camera.moveTo(camera.Position + delta_time * CAM_MOVE_SPEED * camDp);
		camera.Front = level->objects[0]->pos - camera.Position; // look at hero
	}
}

void TestNaiveCollision()
{
	const auto ocnt = level->objects.size();
	int num_checks{0};
	for (int i = 0; i < ocnt - 1; i++)
	{
		for (int j = i + 1; j < ocnt; j++)
		{
			num_checks++;
		}
	}
	LogPrintLn("NAIVE: num of collision checks:", num_checks);
}
