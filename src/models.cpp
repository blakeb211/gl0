#include <slurp.h>
#include <glm/glm.hpp>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct model {
  model() = default;
  model(string mname) : name{mname} {}
  string name;
  vector<glm::vec3> vertices;
  vector<glm::vec3> normals;
  vector<glm::vec4> colors;
};

struct level {
  vector<model> models;
};

std::unique_ptr<model> load_model_from_disk(const char* name) {
  auto m = std::make_unique<model>();
  auto fileData = slurp::get_file_contents(name);

  return m;
}

void load_level(string levelName) {
  auto l = make_unique<level>();
  if (levelName == "test") {
    auto levelData = slurp::get_file_contents(R"(./levels/test.txt)");
    // split on lines
  }
}
