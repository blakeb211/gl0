#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "global.h"
#include "log.h"
#include "slurp.h"
using namespace std;

// will need to be able to deep copy models

struct model {
  model() = default;
  string name;
  vector<glm::vec3> vertices;
  vector<glm::u32vec3> faces;
  vector<glm::vec3> normals;
  vector<glm::vec4> colors;
};

struct level {
  vector<model> models;
};

unique_ptr<string> levelPath(string name) {
  auto path = make_unique<string>(global::levelPath + name + ".txt");
  return path;
}

unique_ptr<string> modelPath(string name) {
  auto path = make_unique<string>(global::modelPath + name + ".obj");
  return path;
}

std::unique_ptr<model> load_model_from_disk(const char* name) {
  auto m = std::make_unique<model>();
  auto fileData = slurp::get_file_contents(modelPath(name)->c_str());

  return m;
}

// format of level
// entity_type  model_name  x y z
void load_level(string levelName) {
  enum class ENTITY_TYPE { unknown, hero, box, ground };
  map<string, ENTITY_TYPE> str_to_type{{"hero", ENTITY_TYPE::hero},
                                       {"box", ENTITY_TYPE::box},
                                       {"ground", ENTITY_TYPE::ground}};

  auto l = make_unique<level>();
  string line, first_tok = "";

  if (slurp::fileExists(levelName)) {
    auto levelData =
        slurp::get_file_contents(levelPath(levelName.c_str())->c_str());
    logPrintLn({"level 'test' slurped from disk successfully"});
    while (levelData.good()) {
      ENTITY_TYPE currType{};
      getline(levelData, line, '\n');  // getline sets stream bits on error
      stringstream lineStream{line};
      lineStream >> first_tok;
      auto type = str_to_type.count(first_tok) ? str_to_type[first_tok]
                                               : ENTITY_TYPE::unknown;

      // add a reverse map of str_to_type
      string modelName;
      glm::vec3 modelPos;

      unique_ptr<model> modelPtr{};

      switch (type) {
        case ENTITY_TYPE::hero:
          lineStream >> modelName >> modelPos.x >> modelPos.y >> modelPos.z;
          modelPtr = load_model_from_disk(modelName.c_str());

          cout << "hero: " << modelName << " " << glm::to_string(modelPos)
               << endl;
          continue;
        case ENTITY_TYPE::box:
          lineStream >> modelName >> modelPos.x >> modelPos.y >> modelPos.z;
          cout << "box: " << modelName << " " << glm::to_string(modelPos)
               << endl;
          continue;
        case ENTITY_TYPE::ground:
          lineStream >> modelName >> modelPos.x >> modelPos.y >> modelPos.z;
          cout << "ground: " << modelName << " " << glm::to_string(modelPos)
               << endl;
          continue;
        default:
          logPrintLn({"unknown line found in level file"});
          continue;
      }
    }
  }
}
