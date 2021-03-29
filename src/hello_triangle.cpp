#include <glad/glad.h>
//
#include <GLFW/glfw3.h>
#include <stb\stb_image.h>
#include <glm/gtx/string_cast.hpp>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include "log.h"
//
#include <iostream>

#include "FrameRater.h"
#include "Shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
GLFWwindow* initGLFW(unsigned int w,
                     unsigned int h,
                     const char* title,
                     GLFWframebuffersizefun);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float offset;
long unsigned int fcount = 0;

int main() {
  // init frame rate
  FrameRater<2000> fr;
  // init log
  setLogFile("log.txt");
  // glfw: initialize and configure
  // ------------------------------
  GLFWwindow* window = initGLFW(SCR_WIDTH, SCR_HEIGHT, "Learn OpenGL",
                                framebuffer_size_callback);
  // Query GL
  int glMajVers, glMinVers;
  glGetIntegerv(GL_MAJOR_VERSION, &glMajVers);
  glGetIntegerv(GL_MINOR_VERSION, &glMinVers);
  logPrintLn({"OpenGL Version:", glMajVers, ".", glMinVers});

  // init texture
  // ------------------------------

  unsigned int tex1;
  // create, bind, generate texture
  glGenTextures(1, &tex1);
  glBindTexture(GL_TEXTURE_2D, tex1);

  int width, height, nrChannels;
  unsigned char* data = stbi_load(R"(.\textures\wooden_container.jpg)", &width,
                                  &height, &nrChannels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    logPrintLn(
        {"texture loaded (w:", width, "h:", height, "nchan:", nrChannels});
  } else {
    logPrintLn({"Failed to load texture data"});
  }

  unsigned int tex2;
  // create, bind, generate texture
  glGenTextures(1, &tex2);
  glBindTexture(GL_TEXTURE_2D, tex2);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tex2);

  data = stbi_load(R"(.\textures\awesomeface.png)", &width, &height,
                   &nrChannels, 0);
  if (data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    logPrintLn(
        {"texture loaded (w:", width, "h:", height, "nchan:", nrChannels});
  } else {
    logPrintLn({"Failed to load texture data"});
  }

  // create shader program
  Shader progOne =
      Shader(R"(.\shaders\3pos3color.vs)", R"(.\shaders\colorFromVertex.fs)");

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

  // uncomment this call to draw in wireframe polygons.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  // Test GLM

  // game loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
    // input
    // -----
    processInput(window);
    progOne.setFloat("offset", offset);
    // render
    // ------
    glClearColor(0.2f, 0.3f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw triangle #1
    /* float timeValue = glfwGetTime();
     float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
     int vertexColorLocation = glGetUniformLocation(shaderProgram,
     "ourColor"); if (vertexColorLocation == -1) { logPrintLn({"uniform
     ourColor was not found in the shader program"});
     }*/
    // create transformations
    glm::mat4 transform = glm::mat4(
        1.0f);  // make sure to initialize matrix to identity matrix first
    transform = glm::translate(transform, glm::vec3(0.5f, -0.5f, 0.0f));
    transform = glm::rotate(transform, (float)glfwGetTime(),
                            glm::vec3(0.0f, 0.0f, 1.0f));

    // get matrix's uniform location and set matrix
    progOne.use();
    unsigned int transformLoc = glGetUniformLocation(progOne.ID, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

    // glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
    // seeing as we only have a single VAO there's
    // no need to bind it every time, but we'll do
    // so to keep things a bit more organized
    glBindTexture(GL_TEXTURE_2D, tex1);
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
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  // glfw: terminate, clearing all previously allocated GLFW resources.
  // ------------------------------------------------------------------
  glfwTerminate();
  closeLog();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window) {
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
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina
  // displays.
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
