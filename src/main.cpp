#include <glad\glad.h>
//
#include "global.h"
#include "log.h"
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtx\string_cast.hpp>
#include <stb\stb_image.h>
//
#include "FrameRater.h"
#include "Shader.h"
#include "gamelib.h"
#include "model.h"

void framebuf_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, glm::vec3& camOffset);
void init_textures();
unsigned int init_vertices();
void initReverseTypeMap();
void logOpenGLInfo();

GLFWwindow* initGLFW(unsigned int w,
    unsigned int h,
    const char* title,
    GLFWframebuffersizefun);

int main()
{
    initReverseTypeMap();
    // init frame rate counter
    FrameRater<1000> fr;

    // init log
    setLogFile("log.txt");

    // glfw: initialize and configure
    auto& w = global::SCR_WIDTH;
    auto& h = global::SCR_HEIGHT;
    GLFWwindow* window = initGLFW(w, h, "Learn OpenGL ", framebuf_size_callback);

    logOpenGLInfo();
    // init_textures();
    auto level = load_level("test");
    Shader progOne = Shader(R"(.\shaders\3pos3color.vs)", R"(.\shaders\colorFromVertex.fs)");
    int VAO = init_vertices();

    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(60.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    glEnable(GL_DEPTH_TEST);

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)
    };

    glBindVertexArray(VAO);
    glm::vec3 camOffset { 0.f, 0.f, 0.f };
    // Game loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // input
        // -----
        processInput(window, camOffset);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        progOne.use();

        // create transformations
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);

        model = glm::rotate(model, glm::radians((float)glfwGetTime() * 22.0f),
            glm::vec3(1.0f, 1.0f, 0.0f));
        view = glm::translate(view, glm::vec3(0.0f + camOffset.x, 0.0f + camOffset.y, -3.0f + camOffset.z));

        progOne.setMat4("model", model);
        progOne.setMat4("view", view);
        progOne.setMat4("projection", projection);
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);

        for (unsigned int i = 0; i < 10; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            if ((i + 1) % 3 == 0) {
                model = glm::rotate(model, glm::radians((float)glfwGetTime() * (angle + 1)), glm::vec3(1.0f, 0.3f, 0.5f));
            } else {
                model = glm::rotate(model, glm::radians(angle + 1), glm::vec3(1.0f, 0.3f, 0.5f));
            }
            progOne.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glBindVertexArray(0); // no need to unbind it every time

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse
        // moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
        fr.sleep();
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
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

void initReverseTypeMap()
{
    for (const auto& mapItem : global::str_to_type) {
        global::type_to_str[mapItem.second] = mapItem.first;
    }
}

void init_textures()
{
    unsigned int tex1;
    glGenTextures(1, &tex1);
    glBindTexture(GL_TEXTURE_2D, tex1);
    int width, height, nrChannels;
    std::string filePath { global::texturePath + std::string("wooden_container.jpg") };
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
    filePath = global::texturePath + std::string("awesomeface.png");
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
void processInput(GLFWwindow* window, glm::vec3& camOff)
{
    const auto camSpeed = 0.01f;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camOff.x -= camSpeed;

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camOff.x += camSpeed;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camOff.y -= camSpeed;

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camOff.y += camSpeed;

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        camOff.z += camSpeed;

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        camOff.z -= camSpeed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuf_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
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
