#include <glad\glad.h>
//
#include <GLFW\glfw3.h>
#include <stb\stb_image.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtx\string_cast.hpp>
#include "global.h"
#include "log.h"
//
#include <array>
#include "FrameRater.h"
#include "Shader.h"
#include "model.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void init_textures();
unsigned int init_vertices();
void initReverseTypeMap();
void logOpenGLInfo();

GLFWwindow* initGLFW(unsigned int w,
                     unsigned int h,
                     const char* title,
                     GLFWframebuffersizefun);

int main() {
  initReverseTypeMap();
  // init frame rate
  FrameRater<2000> fr;
  // init log
  setLogFile("log.txt");
  float offset = 0.0f;
  // glfw: initialize and configure
  // ------------------------------
  GLFWwindow* window = initGLFW(global::SCR_WIDTH, global::SCR_HEIGHT,
                                "Learn OpenGL", framebuffer_size_callback);
  logOpenGLInfo();
  init_textures();
  load_level("test");
  // create shader program
  Shader progOne =
      Shader(R"(.\shaders\3pos3color.vs)", R"(.\shaders\colorFromVertex.fs)");
  int VAO = init_vertices();

  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // Game loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
    // input
    // -----
    processInput(window);
    // create transformations
    glm::mat4 transform = glm::mat4(1.0f);  // init to identity first
    transform = glm::rotate(transform, (float)glfwGetTime(),
                            glm::vec3(0.0f, 0.4f, 1.0f));
    progOne.setMat4("transform", transform);
    // render
    // ------
    glClearColor(0.2f, 0.3f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    progOne.use();

    // glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
    // seeing as we only have a single VAO there's
    // no need to bind it every time, but we'll do
    // so to keep things a bit more organized
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);  // no need to unbind it every time

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

void logOpenGLInfo() {
  // Query GL
  int glMajVers, glMinVers;
  glGetIntegerv(GL_MAJOR_VERSION, &glMajVers);
  glGetIntegerv(GL_MINOR_VERSION, &glMinVers);
  logPrintLn({"OpenGL Version:", glMajVers, ".", glMinVers});
}

// returns the VAO id
unsigned int init_vertices() {
  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  float vertices[] = {
      // positions          // colors           // texture coords
      0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top right
      0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom left
      -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left
  };

  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s),
  // and then configure vertex attributes(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);
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

void initReverseTypeMap() {
  for (const auto& mapItem : global::str_to_type) {
    global::type_to_str[mapItem.second] = mapItem.first;
  }
}

void init_textures() {
  unsigned int tex1;
  glGenTextures(1, &tex1);
  glBindTexture(GL_TEXTURE_2D, tex1);
  int width, height, nrChannels;
  std::string filePath{global::texturePath +
                       std::string("wooden_container.jpg")};
  unsigned char* data =
      stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    logPrintLn({"texture loaded ( w:", width, "h:", height,
                "nchan:", nrChannels, ")"});
  } else {
    logPrintLn({"Failed to load texture data"});
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
    logPrintLn({"texture loaded ( w:", width, "h:", height,
                "nchan:", nrChannels, ")"});
  } else {
    logPrintLn({"Failed to load texture data"});
  }
}

// process all input: query GLFW whether relevant keys are pressed/released
// this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window) {
  float offset = 0.0f;
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && offset < 0.5f)
    offset += 0.01f;

  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && offset > -0.5f)
    offset -= 0.01f;
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
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
    logPrintLn({"Failed to create GLFW window"});
    glfwTerminate();
    return nullptr;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, fun);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    logPrintLn({"Failed to initialize GLAD"});
    return nullptr;
  }
  return window;
}
