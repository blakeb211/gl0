#pragma once
#include "gamelib.h"
//@DESIGN: prefer passing a variable around to creating a global. watch out for
// thread issues.
//@TODO: Consider a settings json or other file instead of a source code file
namespace global {
inline std::string appRoot = std::string { R"(c:\cprojects\gl0\)" };
inline std::string texturePath = appRoot + R"(textures\)";
inline std::string rootModelPath = appRoot + R"(models\)";
inline std::string rootLevelPath = appRoot + R"(levels\)";
inline std::string rootShaderPath = appRoot + R"(shaders\)";

inline const unsigned int SCR_WIDTH = 800;
inline const unsigned int SCR_HEIGHT = 600;

enum class ENTITY_TYPE { unknown,
    hero,
    box,
    ground };

inline std::map<std::string, ENTITY_TYPE> str_to_type {
    { "hero", ENTITY_TYPE::hero },
    { "box", ENTITY_TYPE::box },
    { "ground", ENTITY_TYPE::ground }
};

inline std::map<ENTITY_TYPE, std::string> type_to_str {};

inline std::unique_ptr<std::string> levelPath(std::string name)
{
    auto path = std::make_unique<std::string>(global::rootLevelPath + name + ".txt");
    return path;
}

inline std::unique_ptr<std::string> modelPath(std::string name)
{
    auto path = std::make_unique<std::string>(global::rootModelPath + name + ".obj");
    return path;
}

inline std::unique_ptr<std::string> shaderPath(std::string name)
{
    auto path = std::make_unique<std::string>(global::rootShaderPath + name);
    return path;
}

} // namespace global