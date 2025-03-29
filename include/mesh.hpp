#pragma once

#include "glad.h"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>

constexpr GLuint POSITION_LOCATION = 0;
constexpr GLuint NORMAL_LOCATION = 1;
constexpr GLuint TEX_COORD_LOCATION = 2;

class Mesh {
public:
    Mesh();
    ~Mesh();
    bool load_mesh(const char* filename);
    void render();

private:
    GLuint EBO;
    std::vector<GLushort> indices;

    GLuint VAO, VBO_pos, VBO_norm, VBO_tc;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;

    void unload_mesh();
};