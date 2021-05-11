#include <glad\glad.h>
//
#include <GLFW\glfw3.h>
#include <stb\stb_image.h>
//
#include "FrameRater.h"
#include "Shader.h"
#include "gamelib.h"
#include "glm.h"
#include "headers.h"
#define FREE_MOVE 0

// -------------------------------------------
// TYPEDEFS
// -------------------------------------------
using VecPP = std::vector<gxb::PathPt>;

// -------------------------------------------
// FORWARD DECLARATIONS
// -------------------------------------------
void framebuf_size_callback(GLFWwindow* window, int width, int height);
void processInput_camOnly(GLFWwindow* window, gxb::Camera& cam,
                          float deltaTime);
void processInput_playerOnly(GLFWwindow* window, float deltaTime);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_callback_null(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
glm::vec3 selectNextCamPoint(const gxb::level* const l, gxb::Camera& cam,
                             const VecPP& newCamPos);

void addCamPathToRawData(const VecPP& path, gxb::level* l);

void clearScreen();
unsigned int buildVAO(const gxb::level*);
void logOpenGLInfo();
GLFWwindow* initGLFW(unsigned int w, unsigned int h, const char* title,
                     GLFWframebuffersizefun);

// -------------------------------------------
// GLOBALS
// -------------------------------------------
gxb::Camera camera{};
float lastX = gxb::SCR_WIDTH / 2, lastY = gxb::SCR_HEIGHT / 2;
bool firstMouse = true;
std::unique_ptr<gxb::level> level = nullptr;
std::vector<gxb::PathPt> path;
glm::vec3 newCamGoalPos{};
constexpr auto CAM_MOVE_SPEED = 0.001f;

int main() {
  gxb::initTypeToStrMap();  // creates str_to_type
  FrameRater fr{};
  setLogFile("log.txt");

  const auto& w = gxb::SCR_WIDTH;
  const auto& h = gxb::SCR_HEIGHT;
  GLFWwindow* window = initGLFW(w, h, "Learn OpenGL ", framebuf_size_callback);

  //@TODO: combine into one load later

  auto futureLevelPtr = async(std::launch::async, gxb::load_level, "test");
  level = futureLevelPtr.get();
  
  auto futureCamPts = async(std::launch::async, gxb::load_campath, "test");
  path = futureCamPts.get();

  auto progOne = Shader(*gxb::shaderPath("3pos3color.vs"),
                        *gxb::shaderPath("colorFromVertex.fs"));

  // add camPath points to level raw_data before building VAO
  for (auto &i : path) {
	std::cout << i.pos.x << " " << i.pos.y << " " << i.pos.z << " " << std::endl;
  }
  addCamPathToRawData(path, level.get());
  const auto tot_floats = level->raw_data.size();
  const auto cam_path_floats = path.size() * 3;
  auto pt1x = level->raw_data[tot_floats - cam_path_floats];
  auto pt1y = level->raw_data[tot_floats - cam_path_floats + 1];
  auto pt1z = level->raw_data[tot_floats - cam_path_floats + 2];

  logPrintLn("cam path point 0:", pt1x, pt1y, pt1z);

  auto VAO = buildVAO(level.get());

  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection = glm::mat4(1.0f);

  // Game loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
    fr.UpdateTimes();
    float deltaTime = fr.lastTimeInMs();

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

#if FREE_MOVE
    processInput_camOnly(window, camera, deltaTime);
#else
    processInput_playerOnly(window, deltaTime);
    newCamGoalPos = selectNextCamPoint(level.get(), camera, path);
    // smoothly move cam towards goal pos
    if (camera.Position != newCamGoalPos) {
      const auto camDp = newCamGoalPos - camera.Position;
      camera.moveTo(camera.Position + deltaTime * CAM_MOVE_SPEED * camDp);
      camera.Front = level->objects[0]->pos - camera.Position;  // look at hero
    }
#endif

    progOne.use();

    // set transformations
    model = glm::mat4(1.0f);

    view = camera.GetViewMatrix();
    progOne.setMat4("view", view);

    projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f,
                                  0.1f, 100.0f);
    progOne.setMat4("projection", projection);

    // render
    // ------
    clearScreen();
    glBindVertexArray(VAO);
    size_t colorId = 0;
    const size_t numColor = col::list.size();
    for (size_t i = 0; i < level->objects.size(); i++) {
      model = glm::mat4(1.0f);
      model = glm::translate(model, level->objects[i]->pos);

      colorId =
          (colorId == numColor - 1) ? colorId -= numColor - 1 : colorId += 1;
      progOne.setVec3("color", col::list[colorId]);
      progOne.setMat4("model", model);

      auto meshPtr = level->getMesh(level->objects[i]->mesh_id);
      assert(meshPtr != nullptr);

      unsigned numVertsCurrModel = (unsigned)(meshPtr->faces.size() * 3);
      glDrawArrays(GL_TRIANGLES, (GLint)meshPtr->pos_first_vert,
                   numVertsCurrModel);
    }
    // Draw CamPath
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3{0, 0, 0});
    progOne.setMat4("model", model);
    progOne.setVec3("color", col::red);
    const auto tot_verts = level->raw_data.size() / 3;
    const auto cam_path_verts = path.size();
    glDrawArrays(GL_POINTS, (GLint)(tot_verts - cam_path_verts),
                 (GLint)cam_path_verts);

    glBindVertexArray(0);
    glfwSwapBuffers(window);
    glfwPollEvents();
    fr.printFrameRateIfFreqHasBeenReached();
  }  // end game loop

  glfwTerminate();
  closeLog();
  return 0;
}

void logOpenGLInfo() {
  // Query GL
  int glMajVers, glMinVers;
  glGetIntegerv(GL_MAJOR_VERSION, &glMajVers);
  glGetIntegerv(GL_MINOR_VERSION, &glMinVers);
  logPrintLn("OpenGL Version:", glMajVers, ".", glMinVers);
}

void clearScreen() {
  glClearColor(0.2f, 0.3f, 0.7f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

// process all input: move player only
// ---------------------------------------------------------------------------------------------------------
void processInput_playerOnly(GLFWwindow* window, float deltaTime) {
  constexpr auto playerSpeed = 0.01f;
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  auto& pos = level->objects[0]->pos;
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    pos.x += playerSpeed * deltaTime;
  }

  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    pos.x -= playerSpeed * deltaTime;
  }

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    pos.z -= playerSpeed * deltaTime;
  }

  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    pos.z += playerSpeed * deltaTime;
  }

  if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) __noop;

  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) __noop;
}

// process all input: move camera only
// ---------------------------------------------------------------------------------------------------------
void processInput_camOnly(GLFWwindow* window, gxb::Camera& cam,
                          float deltaTime) {
  const float cameraSpeed = 0.05f;  // adjust accordingly
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    cam.ProcessKeyboard(gxb::Camera_Movement::RIGHT, deltaTime);

  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    cam.ProcessKeyboard(gxb::Camera_Movement::LEFT, deltaTime);

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    cam.ProcessKeyboard(gxb::Camera_Movement::FORWARD, deltaTime);

  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    cam.ProcessKeyboard(gxb::Camera_Movement::BACKWARD, deltaTime);

  if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) __noop;

  if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) __noop;
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuf_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
  if (firstMouse) {
    lastX = (float)xpos;
    lastY = (float)ypos;
    firstMouse = false;
  }

  float xoffset = (float)xpos - lastX;
  float yoffset =
      lastY -
      (float)ypos;  // reversed since y-coordinates go from bottom to top

  lastX = (float)xpos;
  lastY = (float)ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

void mouse_callback_null(GLFWwindow* window, double xpos, double ypos) {
  __noop;
  return;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  camera.ProcessMouseScroll((float)yoffset);
}

GLFWwindow* initGLFW(unsigned int w, unsigned int h, const char* title,
                     GLFWframebuffersizefun fun) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // glfw window creation
  // --------------------
  GLFWwindow* window = glfwCreateWindow(w, h, title, NULL, NULL);
  if (window == NULL) {
    logPrintLn("Failed to create GLFW window");
    glfwTerminate();
    return nullptr;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, fun);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    logPrintLn("Failed to initialize GLAD");
    return nullptr;
  }

#if FREE_MOVE
  glfwSetCursorPosCallback(window, mouse_callback);
#else
  glfwSetCursorPosCallback(window, mouse_callback_null);
#endif

  glfwSetScrollCallback(window, scroll_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glfwSwapInterval(1);  // vsync
  logOpenGLInfo();

  clearScreen();

  return window;
}

glm::vec3 selectNextCamPoint(const gxb::level* const l, gxb::Camera& cam,
                             const VecPP& path) {
  const auto heroPos = l->objects[0]->pos;
  const auto negZvec = glm::vec3{0.f, 0.f, -1.f};

  size_t path_sz = path.size();
  std::vector<std::pair<float,float>> dist_ang(path_sz);

  glm::vec3 pathPos{}, cam2hero{};
  float dist{}, angle{};
  
  for (int i = 0; i < path_sz; i++ ) {
	pathPos = path[i].pos;
    cam2hero = glm::vec3{glm::normalize(heroPos - pathPos)};
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

  auto check = [distCutoff, angCutoff](const pair & dap) { 
	  return (dap.first < distCutoff && dap.second < angCutoff); };

  for (int i = 0; i < path_sz; i++) {
	if (check(dist_ang[i])) {
		return path[i].pos;	
	}
  }
  return glm::vec3{}; // if no valid campath found, cam goes to 0,0,0
}

void addCamPathToRawData(const VecPP& path, gxb::level* l) {
  auto func = [l](const gxb::PathPt pp) {
    for (int i = 0; i < 3; i++) {
      l->raw_data.push_back(pp.pos[i]);
    }
  };
  std::for_each(path.begin(), path.end(), func);
}

unsigned int buildVAO(const gxb::level* l) {
  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  assert(level != nullptr);
  unsigned int VBO{};
  unsigned int VAO{};

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  // bind the Vertex Array Object first, then bind and set vertex
  // buffer(s), and then configure vertex attributes(s)
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, l->raw_data.size() * sizeof(float),
               l->raw_data.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
  //		      (void*)(3 * sizeof(float)));
  // glEnableVertexAttribArray(1);
  // note that this is allowed, the call to glVertexAttribPointer
  // registered VBO as the vertex attribute's bound vertex buffer object
  // so afterwards we can safely unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // You can unbind the VAO afterwards so other VAO calls won't
  // accidentally modify this VAO, but this rarely happens. Modifying
  // other VAOs requires a call to glBindVertexArray anyways so we
  // generally don't unbind VAOs (nor VBOs) when it's not directly
  // necessary.
  glBindVertexArray(0);
  return VAO;
}
