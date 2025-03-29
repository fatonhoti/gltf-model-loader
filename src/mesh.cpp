#include "mesh.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include <filesystem>
#include <assert.h>

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
    unload_mesh();
}

bool Mesh::load_mesh(const char *filename)
{
    if (VAO != 0)
        unload_mesh();

    auto path_model = std::filesystem::current_path() / "assets" / "models" / filename;

    printf("Loading '%s'\n", path_model.c_str());

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    //bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path_model);
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path_model);

    if (!warn.empty())
        printf("Warn: %s\n", warn.c_str());

    if (!err.empty())
        printf("Err: %s\n", err.c_str());

    if (!ret) {
        printf("Failed to parse glTF.\n");
        return -1;
    }

    const auto to_glm_vec3 = [](const float* data, const size_t size, std::vector<glm::vec3>& out) -> void {
        out.reserve(size);
        for (size_t i = 0; i < size * 3; i += 3)
            out.emplace_back(data[i + 0], data[i + 1], data[i + 2]);
    };

    const auto to_glm_vec2 = [](const float* data, const size_t size, std::vector<glm::vec2>& out) -> void {
        out.reserve(size);
        for (size_t i = 0; i < size * 3; i += 2)
            out.emplace_back(data[i + 0], data[i + 1]);
    };

    printf("Number of meshes: %zu\n", model.meshes.size());
    for (const auto& mesh : model.meshes) {
        printf("Mesh name: %s\n", mesh.name.c_str());
        printf("\tNumber of primitives: %zu\n", mesh.primitives.size());
        for (const auto& primitive : mesh.primitives) {
            assert(primitive.mode == TINYGLTF_MODE_TRIANGLES && "Only supporting standard triangle meshes (not STRIP nor FAN).\n");

            if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                const auto& acc_pos = model.accessors[primitive.attributes.at("POSITION")];
                const auto& bv_pos = model.bufferViews[acc_pos.bufferView];
                const auto& buffer = model.buffers[bv_pos.buffer];
                const float* data_pos = reinterpret_cast<const float*>(&buffer.data[bv_pos.byteOffset + acc_pos.byteOffset]);
                to_glm_vec3(data_pos, acc_pos.count, positions);
            }

            if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                const auto& acc_norm = model.accessors[primitive.attributes.at("NORMAL")];
                const auto& bv_norm = model.bufferViews[acc_norm.bufferView];
                const auto& buffer = model.buffers[bv_norm.buffer];
                const float* data_norm = reinterpret_cast<const float*>(&buffer.data[bv_norm.byteOffset + acc_norm.byteOffset]);
                to_glm_vec3(data_norm, acc_norm.count, normals);
            }
            
            if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                const auto& acc_tc = model.accessors[primitive.attributes.at("TEXCOORD_0")];
                const auto& bv_tc = model.bufferViews[acc_tc.bufferView];
                const auto& buffer = model.buffers[bv_tc.buffer];
                const float* data_tc = reinterpret_cast<const float*>(&buffer.data[bv_tc.byteOffset + acc_tc.byteOffset]);
                to_glm_vec2(data_tc, acc_tc.count, texCoords);
            }
            
            assert(primitive.indices != -1 && "This mesh primitive is missing indices.");
            
            const auto& acc_ind = model.accessors[primitive.indices];
            const auto& bv_ind = model.bufferViews[acc_ind.bufferView];
            const auto& buffer = model.buffers[bv_ind.buffer];
            indices.resize(acc_ind.count);
            
            switch(acc_ind.componentType) {
                /*
                const GLubyte* data_ind;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                    data_ind = reinterpret_cast<const GLubyte*>(&buffer.data[bv_ind.byteOffset + acc_ind.byteOffset]);
                    std::memcpy(indices.data(), data_ind, acc_ind.count * sizeof(GLubyte));
                    break;
                */
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                    const GLushort* data_ind;
                    data_ind = reinterpret_cast<const GLushort*>(&buffer.data[bv_ind.byteOffset + acc_ind.byteOffset]);
                    std::memcpy(indices.data(), data_ind, acc_ind.count * sizeof(GLushort));
                    break;
                /*
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                    const GLuint* data_ind;
                    data_ind = reinterpret_cast<const GLuint*>(&buffer.data[bv_ind.byteOffset + acc_ind.byteOffset]);
                    std::memcpy(indices.data(), data_ind, acc_ind.count * sizeof(GLuint));
                    break;
                */
                default:
                    printf("Unsupported index component type %d\n", acc_ind.componentType);
                    unload_mesh();
                    return false;
            }
        }
    }

    printf("positions.size() = %zu\n", positions.size());
    printf("normals.size() = %zu\n", normals.size());
    printf("texCoords.size() = %zu\n", texCoords.size());
    printf("indices.size() = %zu\n", indices.size());

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    glGenBuffers(1, &VBO_pos);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * positions.size(), positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    if (!normals.empty()) {
        glGenBuffers(1, &VBO_norm);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_norm);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(NORMAL_LOCATION);
        glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }
    
    if (!texCoords.empty()) {
        glGenBuffers(1, &VBO_tc);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_tc);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * texCoords.size(), texCoords.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(TEX_COORD_LOCATION);
        glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    return true;
}

void Mesh::render()
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);
}

void Mesh::unload_mesh()
{
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO_pos != 0) glDeleteBuffers(1, &VBO_pos);
    if (VBO_norm != 0) glDeleteBuffers(1, &VBO_norm);
    if (VBO_tc != 0) glDeleteBuffers(1, &VBO_tc);
    if (EBO != 0) glDeleteBuffers(1, &EBO);

    indices.clear();
    indices.shrink_to_fit();

    positions.clear();
    positions.shrink_to_fit();

    normals.clear();
    normals.shrink_to_fit();

    texCoords.clear();
    texCoords.shrink_to_fit();
}
