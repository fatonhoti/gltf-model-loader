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

    struct MeshEntry {
        GLuint VAO, VBO_pos, VBO_norm, VBO_tc, EBO;
        uint32_t offset = 0;
        uint32_t count = 0;
    };
    std::vector<MeshEntry> meshes{};

    void unload_mesh();
};