#pragma once
#include "camera.h"
#include "glm.h"
#include "headers.h"

//@DESIGN: prefer passing a variable around to creating a global. watch out for
// thread issues.

namespace gxb {
inline std::string appRoot = std::string{R"(c:\cprojects\gl0\)"};
inline std::string texturePath = appRoot + R"(textures\)";
inline std::string rootModelPath = appRoot + R"(models\)";
inline std::string rootLevelPath = appRoot + R"(levels\)";
inline std::string rootShaderPath = appRoot + R"(shaders\)";

inline const unsigned int SCR_WIDTH = 800;
inline const unsigned int SCR_HEIGHT = 600;

inline std::hash<std::string> strHasher;

enum class ENTITY_TYPE { unknown, hero, box, ground, fruit };

inline std::map<std::string, ENTITY_TYPE> str_to_type{
    {"hero", ENTITY_TYPE::hero},
    {"box", ENTITY_TYPE::box},
    {"ground", ENTITY_TYPE::ground},
    {"fruit", ENTITY_TYPE::fruit},
};

inline std::map<ENTITY_TYPE, std::string> type_to_str{};

void initTypeToStrMap() {
    for (const auto& mapItem : str_to_type) {
	type_to_str[mapItem.second] = mapItem.first;
    }
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
    static unsigned count_;
};
//********************************************************

unsigned IdFactory::count_ = 0;

struct entity {
    entity()
	: pos{0, 0, 0}, rot{0, 0, 0}, mesh_id{0}, id{IdFactory::getNewId()} {}
    const unsigned id;
    std::size_t mesh_id;
    ENTITY_TYPE type;
    glm::vec3 pos;
    glm::vec3 rot;
};

struct mesh {
    mesh() : hash_code{0}, pos_first_vert{0} {}
    std::string name;
    std::size_t hash_code;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> faces;
    std::vector<glm::vec3> normals;
    size_t pos_first_vert;
};

struct level {
    std::vector<std::unique_ptr<mesh>> meshes;
    std::vector<int> mesh_first_vertex;
    std::vector<std::unique_ptr<entity>> objects;
    std::vector<unsigned int> vaos;
    std::vector<float> raw_data;
    std::vector<unsigned int> buildVAO() {
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------

	std::vector<unsigned int> VBO(meshes.size(), 0);
	std::vector<unsigned int> VAO(meshes.size(), 0);

	glGenVertexArrays(1, &VAO[0]);
	glGenBuffers(1, &VBO[0]);
	// bind the Vertex Array Object first, then bind and set vertex
	// buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, this->raw_data.size() * sizeof(float),
		     this->raw_data.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
			      (void*)0);
	glEnableVertexAttribArray(0);

	// glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
	//		      (void*)(3 * sizeof(float)));
	// glEnableVertexAttribArray(1);
	// note that this is allowed, the call to glVertexAttribPointer
	// registered VBO as the vertex attribute's bound vertex buffer object
	// so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't
	// accidentally modify this VAO, but this rarely happens. Modifying
	// other VAOs requires a call to glBindVertexArray anyways so we
	// generally don't unbind VAOs (nor VBOs) when it's not directly
	// necessary.
	glBindVertexArray(0);
	return VAO;
    }

    mesh* getMesh(size_t hashCode) {
	for (int i = 0; i < meshes.size(); i++) {
	    if (meshes[i]->hash_code == hashCode) {
		return meshes[i].get();
	    }
	}
	return nullptr;
    }
};

inline std::unique_ptr<std::string> levelPath(std::string name) {
    auto path = std::make_unique<std::string>(rootLevelPath + name + ".txt");
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
    std::string& token,
    std::string& delim) {
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

	if (fileData.fail())
	    break;

	stringstream lineStream{line};
	lineStream >> firstTok;

	if (firstTok == "v") {
	    // read vertex
	    glm::vec3 pos{0.f, 0.f, 0.f};
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
		    logErr("ERROR:: < file line # , model name > ::", lineNum,
			   name);
		}
		auto [faceId, normalId] = *resultPair;
		faces[i] = faceId;
	    }
	    m->faces.push_back(faces);
	}
    }

    return m;
}

// format of level
// entity_type  model_name  x y z
inline std::unique_ptr<level> load_level(std::string levelName) {
    auto l = std::make_unique<level>();
    std::string line, entityName = "";

    bool levelExist = slurp::checkFileExist(rootLevelPath, levelName, "txt");

    if (levelExist) {
	auto levelData =
	    slurp::get_file_contents(levelPath(levelName)->c_str());
	logPrintLn("SUCCESS:: level", levelName, "slurped from disk");

	int lineNum = 0;

	logPrintLn("mesh         v         n         f        hash");

	while (levelData.good()) {
	    lineNum++;
	    getline(levelData, line,
		    '\n');  // getline sets stream bits on error

	    std::stringstream lineStream{line};
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

	    if (lineStream.fail()) {
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
	    entityPtr->rot = Rot;
	    entityPtr->mesh_id = meshHashCode;

	    bool modelExist =
		slurp::checkFileExist(rootModelPath, meshName, "obj");
	    bool meshAlreadyLoaded =
		(l->getMesh(meshHashCode) == nullptr) ? false : true;

	    if (!modelExist) {
		// can only reach this line if model file was not found
		logErr(__FILE__, __LINE__, meshName.c_str());
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
	    } else {
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

	    logPrintLn("pushing back obj => type:",
		       type_to_str[entityPtr->type], "id:", entityPtr->id);
	    l->objects.push_back(std::move(entityPtr));

	}  // end while
	logPrintLn("objects created:", l->objects.size());
	logPrintLn("meshes loaded from disk:", l->meshes.size());
	return std::move(l);
    }
	return nullptr;
}

}  // namespace gxb
