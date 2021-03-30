#include <glm/glm.hpp>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "global.h"
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
// it.
void load_level(string levelName) {
  auto l = make_unique<level>();
  if (levelName == "test") {
    auto levelData = slurp::get_file_contents(levelPath("test")->c_str());
    // split on lines
    // read in first string
  }
}
