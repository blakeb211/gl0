#pragma once
#include <map>
#include <string>
//@DESIGN: prefer passing a variable around to creating a global. watch out for
// thread issues.
//@TODO: Consider a settings json or other file instead of a source code file
namespace global {
inline std::string appRoot = std::string{R"(c:\cprojects\gl0\)"};
inline std::string texturePath = appRoot + R"(textures\)";
inline std::string modelPath = appRoot + R"(models\)";
inline std::string levelPath = appRoot + R"(levels\)";
inline const unsigned int SCR_WIDTH = 800;
inline const unsigned int SCR_HEIGHT = 600;

enum class ENTITY_TYPE { unknown, hero, box, ground };

inline std::map<std::string, ENTITY_TYPE> str_to_type{
    {"hero", ENTITY_TYPE::hero},
    {"box", ENTITY_TYPE::box},
    {"ground", ENTITY_TYPE::ground}};

inline std::map<ENTITY_TYPE, std::string> type_to_str{};

}  // namespace global