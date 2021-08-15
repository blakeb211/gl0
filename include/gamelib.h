#pragma once
#include "camera.h"
#include "glm.h"
#include "headers.h"
#include "log.h"
#include <magic_enum.h>
//@DESIGN: prefer passing a variable around to creating a global. watch out for
// thread issues.


namespace gxb {
  inline std::string appRoot = std::string{ R"(c:\cprojects\gl0\)" };
  inline std::string texturePath = appRoot + R"(textures\)";
  inline std::string rootModelPath = appRoot + R"(models\)";
  inline std::string rootLevelPath = appRoot + R"(levels\)";
  inline std::string rootShaderPath = appRoot + R"(shaders\)";

  inline const unsigned int SCR_WIDTH = 1024;
  inline const unsigned int SCR_HEIGHT = 768;

  inline std::hash<std::string> strHasher;

  enum class ENTITY_TYPE {
    unknown, hero, box, ground,
    moving_ground_x, moving_ground_z, moving_ground_y, fruit, baddie
  };

  inline std::map<std::string, ENTITY_TYPE> str_to_type{
      {"hero", ENTITY_TYPE::hero},     {"box", ENTITY_TYPE::box},
      {"ground", ENTITY_TYPE::ground}, {"fruit", ENTITY_TYPE::fruit},
      {"baddie", ENTITY_TYPE::baddie}, {"moving_ground_x", ENTITY_TYPE::moving_ground_x},
    {"moving_ground_z", ENTITY_TYPE::moving_ground_z},
    {"moving_ground_y", ENTITY_TYPE::moving_ground_y},
  };

  inline std::map<ENTITY_TYPE, std::string> type_to_str{};

  inline void initTypeToStrMap() {
    auto create_entry = [](decltype(*str_to_type.begin()) thing) { type_to_str[thing.second] = thing.first; };
    std::for_each(str_to_type.begin(), str_to_type.end(), create_entry);
  }

  //********************************************************
  //					IdFactory
  //********************************************************
  struct IdFactory {
    static unsigned getNewId() {
      int retVal = 0;
      retVal = count_;
      count_++;
      return retVal;
    }

  private:
    IdFactory() {}
    inline static unsigned count_{ 0 };
  };

  //********************************************************

  struct PathPt {
    glm::vec3 pos;
    float dist;
  };


  // A simple binary state machine for moving platforms
  struct FSM_bin {
    enum class States { pos = 1, neg = -1 };
    States current{ States::pos };
    void check_transition(float dist, glm::vec3 facing, float target, const glm::vec3 pos_dir) {
      if (magic_enum::enum_name(current) == "pos" && glm::dot(facing, pos_dir) > 0 && dist >= target) {
        current = States::neg;
      }
      if (magic_enum::enum_name(current) == "neg" && glm::dot(facing, -1.f * pos_dir) > 0 && dist >= target) {
        current = States::pos;
      }
    }
  };

  struct entity {
    entity()
      : pos{ 0, 0, 0 }, pos_start{ 0,0,0 }, pos_last{ 0,0,0 }, rot{ 0, 0, 0 }, mesh_id{ 0 }, id{ IdFactory::getNewId() }, state_machine{ FSM_bin::States::pos }, has_been_added_to_grid{ false } {}
    const unsigned id;
    std::size_t mesh_id;
    ENTITY_TYPE type;
    glm::vec3 pos;
    glm::vec3 rot;
    glm::vec3 pos_start; // could be in derived type instead
    glm::vec3 pos_last;
    bool has_been_added_to_grid;
    FSM_bin state_machine; // could be in derived type instead
  };


  struct mesh {
    mesh() : hash_code{ 0 }, pos_first_vert{ 0 } {}
    std::string name;
    std::size_t hash_code;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> faces;
    std::vector<glm::vec3> normals;
    size_t pos_first_vert;
  };

  struct CamPath {
    CamPath() = delete;
    CamPath(std::vector<glm::vec3> control_points) { cps = control_points; }
    void createPathFromCps() {
      using vec3 = glm::vec3;
      using std::vector;
      // cps.size() == 3 + 2
      for (size_t cpIdx = 0; cpIdx <= cps.size() - 3; cpIdx += 2) {
        const vec3& p0 = cps[cpIdx];
        const vec3& p1 = cps[cpIdx + 1];
        const vec3& p2 = cps[cpIdx + 2];
        float x, y, z, t;
        x = y = z = t = 0.0f;
        while (t <= 1.0) {
          x = (1 - t) * (1 - t) * p0.x + 2 * (1 - t) * t * p1.x + t * t * p2.x;
          y = (1 - t) * (1 - t) * p0.y + 2 * (1 - t) * t * p1.y + t * t * p2.y;
          z = (1 - t) * (1 - t) * p0.z + 2 * (1 - t) * t * p1.z + t * t * p2.z;
          pts.push_back(vec3{ x, y, z });
          t += 0.05f;
        }
      }
    }
    std::vector<glm::vec3> cam_dir;
    std::vector<glm::vec3> pts;
    std::vector<glm::vec3> cps;
  };

  struct Level {
    std::vector<std::unique_ptr<mesh>> meshes;
    std::vector<int> mesh_first_vertex;
    std::vector<std::unique_ptr<entity>> objects;
    std::vector<unsigned int> vaos;
    std::vector<float> raw_data;

    mesh* getMesh(size_t hashCode) {
      for (size_t i = 0; i < meshes.size(); i++) {
        if (meshes[i]->hash_code == hashCode) {
          return meshes[i].get();
        }
      }
      return nullptr;
    }
  };

  inline std::unique_ptr<std::string> levelPath(std::string name,
    const char* ext) {
    auto path = std::make_unique<std::string>(rootLevelPath + name + "." + ext);
    return path;
  }

  inline std::unique_ptr<std::string> modelPath(std::string name) {
    auto path = std::make_unique<std::string>(rootModelPath + name + ".obj");
    return path;
  }

  inline std::unique_ptr<std::string> shaderPath(std::string name) {
    auto path = std::make_unique<std::string>(rootShaderPath + name);
    return path;
  }

  inline std::optional<std::pair<int, int>> extract_pair_of_ints(
    std::string& token, std::string& delim) {
    size_t pos = 0;
    pos = token.find(delim);
    if (pos == std::string::npos) {
      return std::nullopt;  // return on error
    }
    std::string firstNum;
    int faceId, normalId;
    faceId = normalId = -1;
    firstNum = token.substr(0, pos);
    faceId = (unsigned int)atoi(firstNum.c_str());
    token = token.erase(0, pos + delim.length());
    normalId = (unsigned int)atoi(token.c_str());
    return std::optional<std::pair<int, int>>{std::make_pair(faceId, normalId)};
  }

  // v  float float float
  // vn float float float
  // f  1// 1 22//22 9//9
  //
  inline std::unique_ptr<mesh> load_mesh_from_disk(const char* name) {
    using std::string;
    using std::stringstream;
    stringstream fileData{};
    fileData = slurp::get_file_contents(modelPath(name)->c_str());

    // add error checking and return null
    auto m = std::make_unique<mesh>();
    m->name = name;

    unsigned int lineNum = 0;
    string line{}, firstTok{};

    while (1) {
      lineNum++;
      getline(fileData, line, '\n');

      if (fileData.fail()) break;

      stringstream lineStream{ line };
      lineStream >> firstTok;

      if (firstTok == "v") {
        // read vertex
        glm::vec3 pos = { 0.f, 0.f, 0.f };
        lineStream >> pos.x >> pos.y >> pos.z;
        if (lineStream.fail()) {
          logErr(__FILE__, __LINE__, "trouble reading position data");
        }
        m->vertices.push_back(pos);
      }

      if (firstTok == "vn") {
        // read normal
        glm::vec3 normal;
        lineStream >> normal.x >> normal.y >> normal.z;
        if (lineStream.fail()) {
          logPrintLn("error reading normal coords");
        }
        m->normals.push_back(normal);
      }

      if (firstTok == "f") {
        string token;
        glm::u32vec3 faces;

        for (int i = 0; i < 3; i++) {
          lineStream >> token;  // example 1//1
          std::string delimiter = "//";
          auto resultPair = extract_pair_of_ints(token, delimiter);
          if (!resultPair) {
            logErr("ERROR:: < file line # , model name > ::", lineNum, name);
          }
          auto [faceId, normalId] = *resultPair;
          faces[i] = faceId;
        }
        m->faces.push_back(faces);
      }
    }

    return m;
  }

  inline std::vector<PathPt> load_campath(std::string levelName) {
    using std::vector, std::ifstream, glm::vec3, std::make_unique;
    bool fileExist = slurp::checkFileExist(rootLevelPath, levelName, "cmp");
    if (!fileExist) {
      logPrintLn("CamPath file for level", levelName, " was not found.\n");
      return vector<PathPt>{};
    }

    // load camPath
    vector<PathPt> pts;
    auto pathData =
      slurp::get_file_contents(levelPath(levelName, "cmp")->c_str());

    while (pathData.good()) {
      float x{}, y{}, z{};
      pathData >> x >> y >> z;
      if (pathData.good())
        pts.push_back(PathPt{ vec3{x, y, -z}, 0.0f });
    }
    logPrintLn(pts.size(), "points loaded from", levelName, ".cmp");

    logPrintLn("CamPath file for ", levelName, " loaded");

    return std::move(pts);
  }
  // format of level
  // 							 pos   rot
  // entity_type  model_name  x y z x y z
  inline std::unique_ptr<Level> load_level(std::string levelName) {
    auto l = std::make_unique<Level>();
    std::string line, entityName = "";

    bool levelExist = slurp::checkFileExist(rootLevelPath, levelName, "txt");

    if (levelExist) {
      auto levelData =
        slurp::get_file_contents(levelPath(levelName, "txt")->c_str());
      logPrintLn("SUCCESS:: level", levelName, "slurped from disk");

      int lineNum = 0;

      logPrintLn("mesh         	  v        n        f        hash");

      while (levelData.good()) {
        lineNum++;
        getline(levelData, line,
          '\n');  // getline sets stream bits on error

        std::stringstream lineStream{ line };
        lineStream >> entityName;
        auto type = str_to_type.count(entityName) ? str_to_type[entityName]
          : ENTITY_TYPE::unknown;
        if (type == ENTITY_TYPE::unknown) {
          logErr(levelName, lineNum, "entity type unknown");
          continue;
        }

        std::string meshName;
        glm::vec3 Pos{};
        glm::vec3 Rot{};
        lineStream >> meshName;
        lineStream >> Pos.x >> Pos.y >> Pos.z;
        lineStream >> Rot.x >> Rot.y >> Rot.z;

        const bool lineStreamBad = lineStream.fail();
        if (lineStreamBad && levelData.eof()) {
          break;
        }

        if (lineStreamBad) {
          logPrintLn("ERROR: wrong values on line <", lineNum, ">",
            "level:", levelName);
          continue;
        }

        // load model file into level struct
        auto meshPtr = std::make_unique<mesh>();
        auto entityPtr = std::make_unique<entity>();

        // set meshPtr properties
        size_t meshHashCode = strHasher(meshName);

        entityPtr->type = str_to_type[entityName];
        entityPtr->pos = Pos;
        entityPtr->pos_last = Pos;
        entityPtr->pos_start = Pos;
        entityPtr->rot = Rot;
        entityPtr->mesh_id = meshHashCode;

        bool modelExist = slurp::checkFileExist(rootModelPath, meshName, "obj");
        bool meshAlreadyLoaded =
          (l->getMesh(meshHashCode) == nullptr) ? false : true;

        if (!modelExist) {
          // can only reach this line if model file was not found
          logPrintLn("ERROR::missing mesh file:", meshName,
            "while loading level:", levelName);
          return nullptr;
        }
        if (meshAlreadyLoaded) {
          //
          mesh* mesh_ = l->getMesh(meshHashCode);
          std::copy(mesh_->vertices.begin(), mesh_->vertices.end(),
            std::back_inserter(meshPtr->vertices));
          std::copy(mesh_->faces.begin(), mesh_->faces.end(),
            std::back_inserter(meshPtr->faces));
          std::copy(mesh_->normals.begin(), mesh_->normals.end(),
            std::back_inserter(meshPtr->normals));
          //
        }
        else {
          // read in vertices, normals, and faces from disk
          meshPtr = load_mesh_from_disk(meshName.c_str());
          //
        }

        // set meshPtr properties
        meshPtr->name = meshName;
        meshPtr->hash_code = meshHashCode;
        // create one giant raw_data array on the level to hold all model
        // triangles
        // @TODO: save the starting vertex in the raw_data array to the
        // entity struct
        // @Note: vertices are in raw data in the order that model is in the
        // meshes vector
        if (!meshAlreadyLoaded) {
          int facesAddedToRaw = 0;
          auto& v = meshPtr->vertices;
          meshPtr->pos_first_vert = l->raw_data.size() / 3;
          for (const auto& face : meshPtr->faces) {
            // push a float onto vertexarray
            // @NOTE: faces integers in object file start at 1 instead
            // of 0
            l->raw_data.push_back(v[(size_t)face.x - 1].x);
            l->raw_data.push_back(v[(size_t)face.x - 1].y);
            l->raw_data.push_back(v[(size_t)face.x - 1].z);
            l->raw_data.push_back(v[(size_t)face.y - 1].x);
            l->raw_data.push_back(v[(size_t)face.y - 1].y);
            l->raw_data.push_back(v[(size_t)face.y - 1].z);
            l->raw_data.push_back(v[(size_t)face.z - 1].x);
            l->raw_data.push_back(v[(size_t)face.z - 1].y);
            l->raw_data.push_back(v[(size_t)face.z - 1].z);
            facesAddedToRaw++;
          }
          const std::string spacer(15 - meshName.length(), ' ');
          logPrintLn(meshName, spacer, meshPtr->vertices.size(),
            meshPtr->normals.size(), meshPtr->faces.size(),
            meshPtr->hash_code);

          assert(meshPtr->hash_code != 0);
          l->meshes.push_back(std::move(meshPtr));
        }

        l->objects.push_back(std::move(entityPtr));

      }  // end while
      logPrintLn("objects created:", l->objects.size());
      logPrintLn("meshes loaded from disk:", l->meshes.size());
      return std::move(l);
    }
    return nullptr;
  }

}  // namespace gxb


