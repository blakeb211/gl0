#include "model.h"
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "global.h"
#include "log.h"
#include "slurp.h"
using namespace std;

// will need to be able to deep copy models

struct model {
  model() {
    for (int i = 0; i < 3; i++) {
      this->pos[i] = 0;
      this->rot[i] = 0;
    }
  }
  string name;
  vector<glm::vec3> vertices;
  vector<glm::u32vec3> faces;
  vector<glm::vec3> normals;
  vector<glm::vec4> colors;
  glm::vec3 pos;
  glm::vec3 rot;
};

struct level {
  vector<unique_ptr<model>> models;
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
  auto fileData = slurp::get_file_contents(modelPath(name)->c_str());

  // add error checking and return null
  auto m = std::make_unique<model>();
  m->name = name;
  logPrintLn({"model <", name, "> slurped from disk successfully"});

  unsigned int lineNum = 0;
  string line{}, firstTok{};

  while (1) {
    lineNum++;

    getline(fileData, line, '\n');
    if (fileData.fail()) {
      break;
    }

    stringstream lineStream{line};
    lineStream >> firstTok;

    if (firstTok == "v") {
      // read vertex
      glm::vec3 pos;
      lineStream >> pos.x >> pos.y >> pos.z;
      if (lineStream.fail()) {
        logErr(__FILE__, __LINE__, "trouble reading position data");
      }
      // logPrintLn({"vertex pos:", glm::to_string(pos)});
      m->vertices.push_back(pos);
    }

    if (firstTok == "vn") {
      // read normal
      glm::vec3 normal;
      lineStream >> normal.x >> normal.y >> normal.z;
      if (lineStream.fail()) {
        logPrintLn({"error reading normal coords"});
      }
      // logPrintLn({"normal:", glm::to_string(normal)});
      m->normals.push_back(normal);
    }

    if (firstTok == "f") {
      string token;
      glm::u32vec3 faces;

      for (int i = 0; i < 3; i++) {
        lineStream >> token;  // example 1//1
        std::string delimiter = "//";
        auto numPair = extract_pair_of_ints(token, delimiter, name);
        auto faceId = numPair.first;
        auto normalId = numPair.second;
        faces[i] = faceId;
        // logPrintLn({"face id: ", faceId, "normal id:", normalId});
      }
      m->faces.push_back(faces);
    }
  }

  return m;
}

pair<int, int> extract_pair_of_ints(string& token,
                                    string& delim,
                                    const char*& name) {
  size_t pos = 0;
  pos = token.find(delim);
  if (pos == std::string::npos) {
    logPrintLn({"model:", name, "missing face delimeter"});
  }
  string firstNum;
  int faceId, normalId;
  faceId = normalId = -1;
  firstNum = token.substr(0, pos);
  faceId = (unsigned int)atoi(firstNum.c_str());
  token = token.erase(0, pos + delim.length());
  normalId = (unsigned int)atoi(token.c_str());
  return make_pair(faceId, normalId);
}

// format of level
// entity_type  model_name  x y z
void load_level(string levelName) {
  auto l = make_unique<level>();
  string line, entityName = "";

  bool levelExist = slurp::checkFileExist(global::levelPath, levelName, "txt");

  if (levelExist) {
    auto levelData = slurp::get_file_contents(levelPath(levelName)->c_str());
    logPrintLn({"level", levelName, "slurped from disk successfully"});

    using namespace global;  // used to pull in ENTITY_TYPE and str_to_type
    int lineNum = 0;

    while (levelData.good()) {
      lineNum++;
      ENTITY_TYPE currType{};

      getline(levelData, line, '\n');  // getline sets stream bits on error

      stringstream lineStream{line};
      lineStream >> entityName;
      auto type = str_to_type.count(entityName) ? str_to_type[entityName]
                                                : ENTITY_TYPE::unknown;
      if (type == ENTITY_TYPE::unknown) {
        logErr(__FILE__, __LINE__, "entity type unknown");
        continue;
      }

      // read name of model, model initial position, model initial rotation
      string modelName;
      glm::vec3 Pos{};
      glm::vec3 Rot{};
      lineStream >> modelName;
      lineStream >> Pos.x >> Pos.y >> Pos.z;
      lineStream >> Rot.x >> Rot.y >> Rot.z;

      if (lineStream.fail()) {
        logPrintLn({"ERROR: wrong values on line <", lineNum, ">",
                    "level:", levelName});
        continue;
      }

      // load model file into level struct
      unique_ptr<model> modelPtr;
      bool modelExist =
          slurp::checkFileExist(global::modelPath, modelName, "obj");

      if (modelExist) {
        modelPtr = load_model_from_disk(modelName.c_str());
        logPrintLn({type_to_str[type], modelName, glm::to_string(Pos),
                    glm::to_string(Rot)});
        logPrintLn({"model stats |", "verts:", modelPtr->vertices.size(),
                    modelPtr->normals.size(), modelPtr->faces.size()});
        l->models.push_back(std::move(modelPtr));
        continue;
      }

      // can only reach this line if there was an error
      logErr(__FILE__, __LINE__, modelName.c_str());
    }
  }
}
