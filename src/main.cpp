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

void framebuf_size_callback(GLFWwindow* window, int width, int height);
void processInput_camOnly(GLFWwindow* window,
			  gxb::Camera& cam,
			  float deltaTime);
void processInput(GLFWwindow* window, gxb::Camera& cam, float deltaTime);
void processInput_playerOnly(GLFWwindow* window, float deltaTime);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void init_textures();
void clearScreen();
std::vector<unsigned int> buildVAO(const gxb::level*); 
void logOpenGLInfo();
GLFWwindow* initGLFW(unsigned int w,
		     unsigned int h,
		     const char* title,
		     GLFWframebuffersizefun);

// GLOBALS
// -------------------------------------------
gxb::Camera camera{};
float lastX = gxb::SCR_WIDTH / 2, lastY = gxb::SCR_HEIGHT / 2;
bool firstMouse = true;
std::unique_ptr<gxb::level> level = nullptr;
std::vector<glm::vec3> path{};

int main() {
    gxb::initTypeToStrMap();  // creates str_to_type
    FrameRater fr{};

    setLogFile("log.txt");

    // glfw: initialize and setup callbacks
    const auto& w = gxb::SCR_WIDTH;
    const auto& h = gxb::SCR_HEIGHT;
    GLFWwindow* window =
	initGLFW(w, h, "Learn OpenGL ", framebuf_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);
    glfwSwapInterval(1);  // vsync
    logOpenGLInfo();

    // clear screen
    clearScreen();

	//@TODO: combine into one load later 
	//@TODO: remove camera free look later    
	
	auto futureLevelPtr = async(std::launch::async, gxb::load_level, "test");
    level = futureLevelPtr.get();

	auto futureCamPts = async(std::launch::async, gxb::load_campath, "test");
	path = futureCamPts.get();

    auto progOne = Shader(*gxb::shaderPath("3pos3color.vs"),
			  *gxb::shaderPath("colorFromVertex.fs"));

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

	// processInput_camOnly(window, camera, deltaTime);
	//processInput(window, camera, deltaTime);
	processInput_playerOnly(window, deltaTime);

	// make camera follow camera path
	glm::vec3 heroFacingDir{};
	glm::vec3 lookPos{};
	glm::vec3 newCamPos{};
	const auto heroPos = level->objects[0]->pos;

	camera.moveTo(heroPos + glm::vec3{0.f,+5.0f, +15.0f});
	camera.lookAt(lookPos);
	progOne.use();



	// set transformations
	model = glm::mat4(1.0f);

	view = camera.GetViewMatrix();
	progOne.setMat4("view", view);

	projection = glm::perspective(glm::radians(camera.Zoom),
				      800.0f / 600.0f, 0.1f, 100.0f);
	progOne.setMat4("projection", projection);

	// render
	// ------
	clearScreen();
	glBindVertexArray(VAO[0]);
	size_t colorId = 0;
	const size_t numColor = col::list.size();
	for (size_t i = 0; i < level->objects.size(); i++) {
	    model = glm::mat4(1.0f);
	    model = glm::translate(model, level->objects[i]->pos);

	    colorId = (colorId == numColor - 1) ? colorId -= numColor - 1
						: colorId += 1;
	    progOne.setVec3("color", col::list[colorId]);
	    progOne.setMat4("model", model);

	    auto meshPtr = level->getMesh(level->objects[i]->mesh_id);
	    assert(meshPtr != nullptr);

	    unsigned numVertsCurrModel = (unsigned)(meshPtr->faces.size() * 3);
	    glDrawArrays(GL_TRIANGLES, (GLint)meshPtr->pos_first_vert,
			 numVertsCurrModel);
	}

	glBindVertexArray(0);
	glfwSwapBuffers(window);
	glfwPollEvents();
	fr.printFrameRateIfFreqHasBeenReached();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
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
void init_textures() {
    unsigned int tex1;
    glGenTextures(1, &tex1);
    glBindTexture(GL_TEXTURE_2D, tex1);
    int width, height, nrChannels;
    std::string filePath{gxb::texturePath +
			 std::string("wooden_container.jpg")};
    unsigned char* data =
	stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
		     GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
	logPrintLn("texture loaded ( w:", width, "h:", height,
		   "nchan:", nrChannels, ")");
    } else {
	logErr(__FILE__, __LINE__, "failure to load texture data");
    }

    unsigned int tex2;
    glGenTextures(1, &tex2);
    glBindTexture(GL_TEXTURE_2D, tex2);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex2);
    filePath = gxb::texturePath + std::string("awesomeface.png");
    data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
		     GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
	logPrintLn("texture loaded ( w:", width, "h:", height,
		   "nchan:", nrChannels, ")");
    } else {
	logErr(__FILE__, __LINE__, "failure to load texture data");
    }
}

// process all input: move player only
// ---------------------------------------------------------------------------------------------------------
void processInput_playerOnly(GLFWwindow* window,float deltaTime) {
	constexpr auto playerSpeed = 0.01f;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
	level->objects[0]->pos.x += playerSpeed * deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
	level->objects[0]->pos.x -= playerSpeed * deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
	level->objects[0]->pos.z -= playerSpeed * deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
	level->objects[0]->pos.z += playerSpeed * deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	__noop;

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
	__noop;
}

// process all input: move camera only
// ---------------------------------------------------------------------------------------------------------
void processInput_camOnly(GLFWwindow* window,
			  gxb::Camera& cam,
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

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	__noop;

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
	__noop;
}

// process all input: move player and camera
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, gxb::Camera& cam, float deltaTime) {
    const float cameraSpeed = 0.01f;  // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
	level->objects[0]->pos.x += cameraSpeed * deltaTime;
	cam.ProcessKeyboard(gxb::Camera_Movement::RIGHT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
	level->objects[0]->pos.x -= cameraSpeed * deltaTime;
	cam.ProcessKeyboard(gxb::Camera_Movement::LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
	level->objects[0]->pos.z -= cameraSpeed * deltaTime;
	cam.ProcessKeyboard(gxb::Camera_Movement::FORWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
	level->objects[0]->pos.z += cameraSpeed * deltaTime;
	cam.ProcessKeyboard(gxb::Camera_Movement::BACKWARD, deltaTime);
    }

    cam.Position.x = level->objects[0]->pos.x;
    cam.Position.y = level->objects[0]->pos.y + 5;
    cam.Position.z = level->objects[0]->pos.z + 16;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	__noop;

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
	__noop;
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

GLFWwindow* initGLFW(unsigned int w,
		     unsigned int h,
		     const char* title,
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
    return window;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll((float)yoffset);
}

std::vector<unsigned int> buildVAO(const gxb::level* l) {
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
	assert(level != nullptr);
    std::vector<unsigned int> VBO(l->meshes.size(), 0);
    std::vector<unsigned int> VAO(l->meshes.size(), 0);

    glGenVertexArrays(1, &VAO[0]);
    glGenBuffers(1, &VBO[0]);
    // bind the Vertex Array Object first, then bind and set vertex
    // buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, l->raw_data.size() * sizeof(float),
		 l->raw_data.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
			  (void*)0);
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
