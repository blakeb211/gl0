#pragma once
#include "gamelib.h"
#include "glm.h"
#include "headers.h"

//@DESIGN: prefer passing a variable around to creating a global. watch out for
// thread issues.
//@TODO: Consider wrapping in a namespace

namespace gxb {
inline std::string appRoot = std::string { R"(c:\cprojects\gl0\)" };
inline std::string texturePath = appRoot + R"(textures\)";
inline std::string rootModelPath = appRoot + R"(models\)";
inline std::string rootLevelPath = appRoot + R"(levels\)";
inline std::string rootShaderPath = appRoot + R"(shaders\)";

inline const unsigned int SCR_WIDTH = 800;
inline const unsigned int SCR_HEIGHT = 600;

inline std::hash<std::string> strHasher;

enum class ENTITY_TYPE { unknown,
    hero,
    box,
    ground };

inline std::map<std::string, ENTITY_TYPE> str_to_type {
    { "hero", ENTITY_TYPE::hero },
    { "box", ENTITY_TYPE::box },
    { "ground", ENTITY_TYPE::ground },
};

inline std::map<ENTITY_TYPE, std::string> type_to_str {};

void initTypeToStrMap()
{
    for (const auto& mapItem : str_to_type) {
        type_to_str[mapItem.second] = mapItem.first;
    }
}

struct mesh {
    std::string name;
    std::size_t hash_code;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> faces;
    std::vector<glm::vec3> normals;
};

// @TODO: rename to object
struct object {
    object()
    {
        for (int i = 0; i < 3; i++) {
            this->pos[i] = 0;
            this->rot[i] = 0;
        }
    }
    std::string name;
    //@TODO: switch to mesh
    std::vector<glm::vec4> colors;
    glm::vec3 pos;
    glm::vec3 rot;
    std::size_t mesh_id;
};

struct level {
    std::vector<std::unique_ptr<object>> models;
    std::vector<std::unique_ptr<mesh>> meshes;
    std::vector<unsigned int> vaos;
    std::vector<float> raw_data;

    std::vector<unsigned int> buildVAO()
    {
        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------

        std::vector<unsigned int> VBO(models.size(), 0);
        std::vector<unsigned int> VAO(models.size(), 0);

        glGenVertexArrays(1, &VAO[0]);
        glGenBuffers(1, &VBO[0]);
        // bind the Vertex Array Object first, then bind and set vertex
        // buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO[0]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        // @TODO: need to load the vertices into model in drawing order
        glBufferData(GL_ARRAY_BUFFER, this->raw_data.size() * sizeof(float),
            this->raw_data.data(), GL_STATIC_DRAW);

        // @TODO: need to load the vertices into model in drawing order
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
            (void*)0);
        glEnableVertexAttribArray(0);

        //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
        //		      (void*)(3 * sizeof(float)));
        //glEnableVertexAttribArray(1);
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

        for (int i = 0; i < VAO.size(); i++) {

            printf("VAO / VBO [%d] = %d / %d\n", i, VAO[i], VBO[i]);
        }
        return VAO;
    }
    // return true if a mesh with that mesh_id is already loaded
    bool meshExists(size_t mesh_id)
    {
        for (const auto& currMesh : this->meshes) {
            if (currMesh->hash_code == mesh_id) {
                return true;
            }
        }
        return false;
    }
};

inline std::unique_ptr<level> load_level(std::string);

inline std::pair<int, int> extract_pair_of_ints(std::string& token,
    std::string& delimiter);

inline std::unique_ptr<std::string> levelPath(std::string name)
{
    auto path = std::make_unique<std::string>(rootLevelPath + name + ".txt");
    return path;
}

inline std::unique_ptr<std::string> modelPath(std::string name)
{
    auto path = std::make_unique<std::string>(rootModelPath + name + ".obj");
    return path;
}

inline std::unique_ptr<std::string> shaderPath(std::string name)
{
    auto path = std::make_unique<std::string>(rootShaderPath + name);
    return path;
}

// namespace // v  float float float
// vn float float float
// f  1// 1 22//22 9//9

//@TODO mesh should be added to level meshes vector, so this fxn should take a level parameter
inline std::unique_ptr<mesh> load_mesh_from_disk(const char* name)
{
    using std::string;
    using std::stringstream;
    stringstream fileData {};
    fileData = slurp::get_file_contents(modelPath(name)->c_str());

    // add error checking and return null
    auto m = std::make_unique<mesh>();
    m->name = name;
    logPrintLn({ "SUCCESS:: model <", name, "> slurped from disk" });

    unsigned int lineNum = 0;
    string line {}, firstTok {};

    while (1) {
        lineNum++;

        getline(fileData, line, '\n');
        if (fileData.fail()) {
            break;
        }

        stringstream lineStream { line };
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
                logPrintLn({ "error reading normal coords" });
            }
            // logPrintLn({"normal:", glm::to_string(normal)});
            m->normals.push_back(normal);
        }

        if (firstTok == "f") {
            string token;
            glm::u32vec3 faces;

            for (int i = 0; i < 3; i++) {
                lineStream >> token; // example 1//1
                std::string delimiter = "//";
                auto [faceId, normalId] = extract_pair_of_ints(token, delimiter);
                if (faceId == -999 && normalId == -999) {
                    logErr("ERROR:: < file line # , model name > ::", lineNum,
                        name);
                }
                faces[i] = faceId;
                // logPrintLn({"face id: ", faceId, "normal id:", normalId});
            }
            m->faces.push_back(faces);
        }
    }

    return m;
}

// format of level
// entity_type  model_name  x y z
std::unique_ptr<level> load_level(std::string levelName)
{
    auto l = std::make_unique<level>();
    std::string line, entityName = "";

    bool levelExist = slurp::checkFileExist(rootLevelPath, levelName, "txt");

    if (levelExist) {
        auto levelData = slurp::get_file_contents(levelPath(levelName)->c_str());
        logPrintLn({ "SUCCESS:: level", levelName, "slurped from disk" });

        int lineNum = 0;

        while (levelData.good()) {
            lineNum++;
            ENTITY_TYPE currType {};

            getline(levelData, line,
                '\n'); // getline sets stream bits on error

            std::stringstream lineStream { line };
            lineStream >> entityName;
            auto type = str_to_type.count(entityName) ? str_to_type[entityName]
                                                      : ENTITY_TYPE::unknown;
            if (type == ENTITY_TYPE::unknown) {
                logErr(__FILE__, __LINE__, "entity type unknown");
                continue;
            }

            // read name of model, model initial position, model initial
            // rotation
            std::string meshName;
            glm::vec3 Pos {};
            glm::vec3 Rot {};
            lineStream >> meshName;
            lineStream >> Pos.x >> Pos.y >> Pos.z;
            lineStream >> Rot.x >> Rot.y >> Rot.z;

            if (lineStream.fail()) {
                logPrintLn({ "ERROR: wrong values on line <", lineNum, ">",
                    "level:", levelName });
                continue;
            }

            // @TODO:
            // create static member function for whether mesh exists
            // check if mesh has already been loaded, if not, load it
            size_t mesh_id = strHasher(meshName);
            if (l->meshExists(mesh_id)) {
                logPrintLn({ "Mesh ", mesh_id, "already loaded" });
                continue;
            }

            // load model file into level struct
            std::unique_ptr<object> objectPtr;
            std::unique_ptr<mesh> meshPtr;
            bool modelFileExists = slurp::checkFileExist(rootModelPath, meshName, "obj");

            if (modelFileExists) {
                meshPtr = load_mesh_from_disk(meshName.c_str());
                logPrintLn({ type_to_str[type], meshName, glm::to_string(Pos),
                    glm::to_string(Rot) });

                logPrintLn({ "model stats |",
                    "verts, normals, faces:", meshPtr->vertices.size(),
                    meshPtr->normals.size(), meshPtr->faces.size() });

                // create one giant raw_data array on the level to hold all model triangles
                // @Note: vertices are in raw data in the order that model is in the models vector
                int facesAddedToRaw = 0;
                auto& v = meshPtr->vertices;
                for (const auto& face : meshPtr->faces) {
                    // faces integers in object file start at 1 instead of 0
                    l->raw_data.push_back(v[face.x - 1].x);
                    l->raw_data.push_back(v[face.x - 1].y);
                    l->raw_data.push_back(v[face.x - 1].z);
                    l->raw_data.push_back(v[face.y - 1].x);
                    l->raw_data.push_back(v[face.y - 1].y);
                    l->raw_data.push_back(v[face.y - 1].z);
                    l->raw_data.push_back(v[face.z - 1].x);
                    l->raw_data.push_back(v[face.z - 1].y);
                    l->raw_data.push_back(v[face.z - 1].z);
                    facesAddedToRaw++;
                }
                logPrintLn({ "faces added to raw_data:", facesAddedToRaw });

                objectPtr->pos = Pos;
                objectPtr->rot = Rot;
                const size_t meshHashId = strHasher(meshName);
                objectPtr->mesh_id = meshHashId;
                meshPtr->hash_code = meshHashId;
                meshPtr->name = meshName;
                objectPtr->name = entityName;
                l->models.push_back(std::move(objectPtr));
                l->meshes.push_back(std::move(meshPtr));
                continue;
            }

            // can only reach this line if there was an error
            logErr(__FILE__, __LINE__, meshName.c_str());
        }
    }
    return std::move(l);
}

// @TODO: return an optional or values and an error
std::pair<int, int> extract_pair_of_ints(std::string& token,
    std::string& delim)
{
    size_t pos = 0;
    pos = token.find(delim);
    if (pos == std::string::npos) {
        return std::make_pair(-999, -999); // return on error
    }
    std::string firstNum;
    int faceId, normalId;
    faceId = normalId = -1;
    firstNum = token.substr(0, pos);
    faceId = (unsigned int)atoi(firstNum.c_str());
    token = token.erase(0, pos + delim.length());
    normalId = (unsigned int)atoi(token.c_str());
    return std::make_pair(faceId, normalId);
}

// Defines several possible options for camera movement. Used as abstraction to
// stay away from window-system specific input methods
enum Camera_Movement { FORWARD,
    BACKWARD,
    LEFT,
    RIGHT };

// Default camera values used internally by Camera class
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 0.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 25.0f;

// An abstract camera class that processes input and calculates the
// corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera {
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = YAW,
        float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f))
        , MovementSpeed(SPEED)
        , MouseSensitivity(SENSITIVITY)
        , Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX,
        float posY,
        float posZ,
        float upX,
        float upY,
        float upZ,
        float yaw,
        float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f))
        , MovementSpeed(SPEED)
        , MouseSensitivity(SENSITIVITY)
        , Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt
    // Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // processes input received from any keyboard-like input system. Accepts
    // input parameter in the form of camera defined ENUM (to abstract it from
    // windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime * SENSITIVITY;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    // processes input received from a mouse input system. Expects the offset
    // value in both the x and y direction.
    void ProcessMouseMovement(float xoffset,
        float yoffset,
        GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get
        // flipped
        if (constrainPitch) {
            if (Pitch > 85.0f)
                Pitch = 85.0f;
            if (Pitch < -85.0f)
                Pitch = -85.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires
    // input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset * SENSITIVITY;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 25.0f)
            Zoom = 25.0f;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(
            Front, WorldUp)); // normalize the vectors, because their length
        // gets closer to 0 the more you look up or down
        // which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

} // namespace gxb
