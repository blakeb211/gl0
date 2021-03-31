#include <cstdlib>
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

// v  float float float
// vn float float float
// f  1// 1 22//22 9//9
std::unique_ptr<model> load_model_from_disk(const char* name) {
  auto m = std::make_unique<model>();
  auto fileData = slurp::get_file_contents(modelPath(name)->c_str());
  logPrintLn({"model <", name, "> slurped from disk successfully"});

  unsigned int lineNum = 0;
  string line{}, firstTok{};

  while (1) {
    lineNum++;
    getline(fileData, line, '\n');
    if (fileData.fail())
      break;
    stringstream lineStream{line};
    lineStream >> firstTok;

    if (firstTok == "v") {
      // read vertex
    }
    if (firstTok == "vn") {
      // read normal
    }
    if (firstTok == "f") {
      string token, firstNum;
      for (int i = 0; i < 3; i++) {
        lineStream >> token;  // example 1//1

        std::string delimiter = "//";
        size_t pos = 0;
        int faceId, normalId;
        faceId = normalId = -1;

        pos = token.find(delimiter);
        if (pos == std::string::npos) {
          logPrintLn(
              {"model:", name, "line:", lineNum, "no face delimeter found"});
        }

        firstNum = token.substr(0, pos);
        faceId = (unsigned int)atoi(firstNum.c_str());
        token = token.erase(0, pos + delimiter.length());
        normalId = (unsigned int)atoi(token.c_str());
        logPrintLn({"face id: ", faceId, "normal id:", normalId});
      }
    }
  }
  return m;
}

// format of level
// entity_type  model_name  x y z
void load_level(string levelName) {
  auto l = make_unique<level>();
  string line, entityName = "";

  if (slurp::checkFileExist(global::levelPath, levelName, "txt")) {
    auto levelData =
        slurp::get_file_contents(levelPath(levelName.c_str())->c_str());

    logPrintLn({"level", levelName, "slurped from disk successfully"});

    using namespace global;
    int lineNum = 0;

    while (levelData.good()) {
      lineNum++;
      ENTITY_TYPE currType{};
      getline(levelData, line, '\n');  // getline sets stream bits on error
      stringstream lineStream{line};
      lineStream >> entityName;
      auto type = str_to_type.count(entityName) ? str_to_type[entityName]
                                                : ENTITY_TYPE::unknown;
      if (type == global::ENTITY_TYPE::unknown) {
        logPrintLn({"ERROR: unknown entity <", entityName, "> found in level",
                    levelName});
        continue;
      }

      // add a reverse map of str_to_type
      string modelName;
      glm::vec3 Pos{};
      glm::vec3 Rot{};
      lineStream >> modelName >> Pos.x >> Pos.y >> Pos.z;
      lineStream >> Rot.x >> Rot.y >> Rot.z;
      if (lineStream.fail()) {
        logPrintLn({"ERROR: wrong values on line <", lineNum, ">",
                    "level:", levelName});
        continue;
      }
      if (slurp::checkFileExist(global::modelPath, modelName, "obj")) {
        auto modelPtr = load_model_from_disk(modelName.c_str());
        // need reverse map
        logPrintLn({type_to_str[type], modelName, glm::to_string(Pos),
                    glm::to_string(Rot)});
      } else {
        logPrintLn({"ERROR: model <", modelName, "> file does not exist"});
      }
    }
  }
}
