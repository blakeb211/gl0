#pragma once
#include <string>

//@DESIGN: prefer passing a variable around to creating a global. watch out for
// thread issues.
namespace global {
inline std::string appRoot = std::string{R"(c:\cprojects\gl0\)"};
inline std::string texturePath = appRoot + R"(textures\)";
inline std::string modelPath = appRoot + R"(models\)";
inline std::string levelPath = appRoot + R"(levels\)";
inline const unsigned int SCR_WIDTH = 800;
inline const unsigned int SCR_HEIGHT = 600;
}  // namespace global