#include <glad\glad.h>
//
#include <GLFW\glfw3.h>
#include <stb\stb_image.h>
//
#include "gamelib.h"
#include "glm.h"
#include "headers.h"

void framebuf_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, gxb::Camera& cam, float deltaTime);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void init_textures();
unsigned int init_vertices();
void logOpenGLInfo();
GLFWwindow* initGLFW(unsigned int w,
    unsigned int h,
    const char* title,
    GLFWframebuffersizefun);

// GLOBALS
// -------------------------------------------
gxb::Camera camera {};
float lastX = gxb::SCR_WIDTH / 2, lastY = gxb::SCR_HEIGHT / 2;
bool firstMouse = true;

int main()
{
    gxb::initTypeToStrMap(); // creates str_to_type

    FrameRater fr {};

    setLogFile("log.txt");

    // glfw: initialize and setup callbacks
    const auto& w = gxb::SCR_WIDTH;
    const auto& h = gxb::SCR_HEIGHT;
    GLFWwindow* window = initGLFW(w, h, "Learn OpenGL ", framebuf_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glEnable(GL_DEPTH_TEST);
    glfwSwapInterval(1); // vsync
    logOpenGLInfo();

    auto futureLevelPtr = async(std::launch::async, gxb::load_level, "test");
    auto level = futureLevelPtr.get();

    auto progOne = Shader(*gxb::shaderPath("3pos3color.vs"),
        *gxb::shaderPath("colorFromVertex.fs"));
    //int VAO = init_vertices();
    int VAO = level->buildVAO();

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    // Game loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        fr.UpdateTimes();
        float deltaTime = fr.lastTimeInMs();

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        processInput(window, camera, deltaTime);

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
        glClearColor(0.2f, 0.3f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);

        for (unsigned int i = 0; i < level->models.size(); i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, level->models[i]->pos);
            float angle = 20.0f * i;
            if ((i + 1) % 3 == 0) {
                model = glm::rotate(
                    model, glm::radians((float)glfwGetTime() * (angle + 1)),
                    glm::vec3(1.0f, 0.3f, 0.5f));
            } else {
                model = glm::rotate(model, glm::radians(angle + 1),
                    glm::vec3(1.0f, 0.3f, 0.5f));
            }
            progOne.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, level->models[i]->raw_data.size());
        }

        glBindVertexArray(0); // no need to unbind it every time

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse
        // moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    closeLog();
    return 0;
}

void logOpenGLInfo()
{
    // Query GL
    int glMajVers, glMinVers;
    glGetIntegerv(GL_MAJOR_VERSION, &glMajVers);
    glGetIntegerv(GL_MINOR_VERSION, &glMinVers);
    logPrintLn({ "OpenGL Version:", glMajVers, ".", glMinVers });
}

// returns the VAO id
unsigned int init_vertices()
{
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s),
    // and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
        (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
        (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // note that this is allowed, the call to glVertexAttribPointer registered
    // VBO as the vertex attribute's bound vertex buffer object so afterwards we
    // can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally
    // modify this VAO, but this rarely happens. Modifying other VAOs requires a
    // call to glBindVertexArray anyways so we generally don't unbind VAOs (nor
    // VBOs) when it's not directly necessary.
    glBindVertexArray(0);

    return VAO;
}

void init_textures()
{
    unsigned int tex1;
    glGenTextures(1, &tex1);
    glBindTexture(GL_TEXTURE_2D, tex1);
    int width, height, nrChannels;
    std::string filePath { gxb::texturePath + std::string("wooden_container.jpg") };
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
            GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
        logPrintLn({ "texture loaded ( w:", width, "h:", height,
            "nchan:", nrChannels, ")" });
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
        logPrintLn({ "texture loaded ( w:", width, "h:", height,
            "nchan:", nrChannels, ")" });
    } else {
        logErr(__FILE__, __LINE__, "failure to load texture data");
    }
}

// process all input: query GLFW whether relevant keys are pressed/released
// this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, gxb::Camera& cam, float deltaTime)
{
    const float cameraSpeed = 0.05f; // adjust accordingly
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

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuf_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse) {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos; // reversed since y-coordinates go from bottom to top

    lastX = (float)xpos;
    lastY = (float)ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

GLFWwindow* initGLFW(unsigned int w,
    unsigned int h,
    const char* title,
    GLFWframebuffersizefun fun)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(w, h, title, NULL, NULL);
    if (window == NULL) {
        logPrintLn({ "Failed to create GLFW window" });
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, fun);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        logPrintLn({ "Failed to initialize GLAD" });
        return nullptr;
    }
    return window;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
}
