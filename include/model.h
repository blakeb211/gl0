#pragma once
#include "gamelib.h"
//
#include <glm/glm.hpp>

struct model {
  model() {
    for (int i = 0; i < 3; i++) {
      this->pos[i] = 0;
      this->rot[i] = 0;
    }
  }
  std::string name;
  std::vector<glm::vec3> vertices;
  std::vector<glm::u32vec3> faces;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec4> colors;
  glm::vec3 pos;
  glm::vec3 rot;
};

struct level {
  std::vector<std::unique_ptr<model>> models;
};

std::unique_ptr<level> load_level(std::string);

std::pair<int, int> extract_pair_of_ints(std::string& token,
                                         std::string& delimiter,
                                         const char*& name);
