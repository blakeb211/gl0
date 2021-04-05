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

void initReverseTypeMap()
{
    for (const auto& mapItem : str_to_type) {
        type_to_str[mapItem.second] = mapItem.first;
    }
}

struct model {
    model()
    {
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
inline std::unique_ptr<model> load_model_from_disk(const char* name)
{
    using std::string;
    using std::stringstream;
    stringstream fileData {};
    fileData = slurp::get_file_contents(modelPath(name)->c_str());

    // add error checking and return null
    auto m = std::make_unique<model>();
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
            std::string modelName;
            glm::vec3 Pos {};
            glm::vec3 Rot {};
            lineStream >> modelName;
            lineStream >> Pos.x >> Pos.y >> Pos.z;
            lineStream >> Rot.x >> Rot.y >> Rot.z;

            if (lineStream.fail()) {
                logPrintLn({ "ERROR: wrong values on line <", lineNum, ">",
                    "level:", levelName });
                continue;
            }

            // load model file into level struct
            std::unique_ptr<model> modelPtr;
            bool modelExist = slurp::checkFileExist(rootModelPath, modelName, "obj");

            if (modelExist) {
                modelPtr = load_model_from_disk(modelName.c_str());
                logPrintLn({ type_to_str[type], modelName, glm::to_string(Pos),
                    glm::to_string(Rot) });

                logPrintLn({ "model stats |",
                    "verts, normals, faces:", modelPtr->vertices.size(),
                    modelPtr->normals.size(), modelPtr->faces.size() });

                modelPtr->pos = Pos;
                modelPtr->rot = Rot;
                modelPtr->name = modelName;
                l->models.push_back(std::move(modelPtr));
                continue;
            }

            // can only reach this line if there was an error
            logErr(__FILE__, __LINE__, modelName.c_str());
        }
    }
    return std::move(l);
}

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

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values used internally by Camera class
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 25.0f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
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
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH)
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
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
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

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch) {
            if (Pitch > 85.0f)
                Pitch = 85.0f;
            if (Pitch < -85.0f)
                Pitch = -85.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
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
        Right = glm::normalize(glm::cross(Front, WorldUp)); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

} // namespace gxb
