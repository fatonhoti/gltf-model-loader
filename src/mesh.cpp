#include "mesh.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include <filesystem>
#include <assert.h>

static const auto to_glm_vec3 = [](const float* data, const size_t size, std::vector<glm::vec3>& out) -> void {
    out.reserve(size);
    for (size_t i = 0; i < size * 3; i += 3)
        out.emplace_back(data[i + 0], data[i + 1], data[i + 2]);
};

static const auto to_glm_vec2 = [](const float* data, const size_t size, std::vector<glm::vec2>& out) -> void {
    out.reserve(size);
    for (size_t i = 0; i < size * 3; i += 2)
        out.emplace_back(data[i + 0], data[i + 1]);
};

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
    unload_mesh();
}

bool Mesh::load_mesh(const char *filename)
{
    if (!meshes.empty())
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

    std::vector<glm::vec3> positions{};
    std::vector<glm::vec3> normals{};
    std::vector<glm::vec2> texCoords{};

    uint32_t nof_indices = 0;
    for (const auto& mesh : model.meshes) {
        for (const auto& primitive : mesh.primitives) {
            const auto& acc_ind = model.accessors[primitive.indices];
            nof_indices += acc_ind.count;
        }
    }
    printf("Found %d indices\n", nof_indices);

    std::vector<uint32_t> indices{};
    indices.reserve(nof_indices);

    meshes.resize(model.meshes.size());

    for (size_t i = 0; i < model.meshes.size(); i++) {
        const auto& mesh = model.meshes[i];

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texCoords;
        std::vector<uint32_t> indices;

        uint32_t index_offset = 0;

        for (const auto& primitive : mesh.primitives) {
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

            // Handling indices correctly
            if (primitive.indices >= 0) {
                const auto& acc_ind = model.accessors[primitive.indices];
                const auto& bv_ind = model.bufferViews[acc_ind.bufferView];
                const auto& buffer = model.buffers[bv_ind.buffer];

                switch(acc_ind.componentType) {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                        indices.insert(indices.end(),
                            reinterpret_cast<const GLubyte*>(&buffer.data[bv_ind.byteOffset + acc_ind.byteOffset]),
                            reinterpret_cast<const GLubyte*>(&buffer.data[bv_ind.byteOffset + acc_ind.byteOffset]) + acc_ind.count);
                        break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                        indices.insert(indices.end(),
                            reinterpret_cast<const GLushort*>(&buffer.data[bv_ind.byteOffset + acc_ind.byteOffset]),
                            reinterpret_cast<const GLushort*>(&buffer.data[bv_ind.byteOffset + acc_ind.byteOffset]) + acc_ind.count);
                        break;
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                        indices.insert(indices.end(),
                            reinterpret_cast<const GLuint*>(&buffer.data[bv_ind.byteOffset + acc_ind.byteOffset]),
                            reinterpret_cast<const GLuint*>(&buffer.data[bv_ind.byteOffset + acc_ind.byteOffset]) + acc_ind.count);
                        break;
                    default:
                        printf("Unsupported index component type %d\n", acc_ind.componentType);
                        return false;
                }
            }
        }

        meshes[i].offset = index_offset;
        meshes[i].count = indices.size();
        index_offset += indices.size();

        glGenVertexArrays(1, &meshes[i].VAO);
        glBindVertexArray(meshes[i].VAO);

        glGenBuffers(1, &meshes[i].VBO_pos);
        glBindBuffer(GL_ARRAY_BUFFER, meshes[i].VBO_pos);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(POSITION_LOCATION);
        glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

        if (!normals.empty()) {
            glGenBuffers(1, &meshes[i].VBO_norm);
            glBindBuffer(GL_ARRAY_BUFFER, meshes[i].VBO_norm);
            glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(NORMAL_LOCATION);
            glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);
        }

        if (!texCoords.empty()) {
            glGenBuffers(1, &meshes[i].VBO_tc);
            glBindBuffer(GL_ARRAY_BUFFER, meshes[i].VBO_tc);
            glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(TEX_COORD_LOCATION);
            glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);
        }

        glGenBuffers(1, &meshes[i].EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[i].EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    // Handle materials
    for (size_t i = 0; i < model.meshes.size(); i++) {
        const auto& mesh = model.meshes[i];
        for (const auto& primitive : mesh.primitives) {
            printf("\n@@@\n");
            printf("material = %d\n", primitive.material);
            const auto& mat = model.materials[primitive.material];

            printf("mat.name = %s\n", mat.name.c_str());
            
            const auto& pbrMR = mat.pbrMetallicRoughness;
            printf("metallicFactor = %f\n", pbrMR.metallicFactor);
            printf("roughnessFactor = %f\n", pbrMR.roughnessFactor);
            
            printf("baseColorTexture:\n");
            printf("\tbaseColorTexture.index = %d\n", pbrMR.baseColorTexture.index);
            printf("\tbaseColorTexture.texCoord = %d\n", pbrMR.baseColorTexture.texCoord);
            const auto& baseColorTexture = model.textures[pbrMR.baseColorTexture.index];
            printf("baseColorTexture.name = %s\n", baseColorTexture.name.c_str());
            const auto& baseColorTextureImage = model.images[baseColorTexture.source];
            printf("\timage.uri = %s\n", baseColorTextureImage.uri.c_str());
            const auto& baseColorTextureSampler = model.samplers[baseColorTexture.sampler];
            printf("\t\tmagFilter = %d\n", baseColorTextureSampler.magFilter);
            printf("\t\tmagFilter = %d\n", baseColorTextureSampler.magFilter);
            printf("\t\twrapS = %d\n", baseColorTextureSampler.wrapS);
            printf("\t\twrapT = %d\n", baseColorTextureSampler.wrapT);

            printf("metallicRoughnessTexture:\n");
            printf("\tmetallicRoughnessTexture.index = %d\n", pbrMR.metallicRoughnessTexture.index);
            printf("\tmetallicRoughnessTexture.texCoord = %d\n", pbrMR.metallicRoughnessTexture.texCoord);
            const auto& metallicRoughnessTexture = model.textures[pbrMR.metallicRoughnessTexture.index];
            printf("metallicRoughnessTexture.name = %s\n", metallicRoughnessTexture.name.c_str());
            const auto& metallicRoughnessTextureImage = model.images[metallicRoughnessTexture.source];
            printf("\timage.uri = %s\n", metallicRoughnessTextureImage.uri.c_str());
            const auto& metallicRoughnessTextureSampler = model.samplers[metallicRoughnessTexture.sampler];
            printf("\t\tmagFilter = %d\n", metallicRoughnessTextureSampler.magFilter);
            printf("\t\tmagFilter = %d\n", metallicRoughnessTextureSampler.magFilter);
            printf("\t\twrapS = %d\n", metallicRoughnessTextureSampler.wrapS);
            printf("\t\twrapT = %d\n", metallicRoughnessTextureSampler.wrapT);

        }
    }

    return true;
}

void Mesh::render()
{
    for (const auto& mesh : meshes) {
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, (void*)(mesh.offset * sizeof(uint32_t)));
    }
}

void Mesh::unload_mesh()
{
    for (const auto& mesh : meshes) {
        if (mesh.VAO != 0) glDeleteVertexArrays(1, &mesh.VAO);
        if (mesh.VBO_pos != 0) glDeleteBuffers(1, &mesh.VBO_pos);
        if (mesh.VBO_norm != 0) glDeleteBuffers(1, &mesh.VBO_norm);
        if (mesh.VBO_tc != 0) glDeleteBuffers(1, &mesh.VBO_tc);
        if (mesh.EBO != 0) glDeleteBuffers(1, &mesh.EBO);
    }
}
