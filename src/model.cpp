#include <glm/glm.hpp>
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

// load_level parses a level file, loads models, and creates a level struct from
// it. should global startup produce a list of files in the level dir?

void load_level(string levelName) {
  enum class LINE_TYPE { unknown, cube, sphere, plane };
  map<string, LINE_TYPE> str_to_type{{"cube", LINE_TYPE::cube},
                                     {"sphere", LINE_TYPE::sphere},
                                     {"plane", LINE_TYPE::plane}};

  auto l = make_unique<level>();
  string line, first_tok = "";

  if (levelName == "test") {
    auto levelData = slurp::get_file_contents(levelPath("test")->c_str());
    logPrintLn({"level 'test' slurped from disk successfully"});
    while (levelData.good()) {
      LINE_TYPE currType{};
      levelData >> first_tok;
      getline(levelData, line, '\n');  // getline sets stream bits on error
      auto type = str_to_type.count(first_tok) ? str_to_type[first_tok]
                                               : LINE_TYPE::unknown;
      switch (type) {
        case LINE_TYPE::cube:
          logPrintLn({"cube line found"});
          break;
        case LINE_TYPE::sphere:
          logPrintLn({"sphere line found"});
          break;
        case LINE_TYPE::plane:
          logPrintLn({"plane line found"});
          break;
        default:
          logPrintLn({"unknown line found in level file"});
          break;
      }
    }
  }
}
