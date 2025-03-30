#include "asset_manager.hpp"

#include "glad.h"

#include <assert.h>
#include <stack>

namespace AssetManager {

    std::unordered_map<std::string, Model> models;

    bool load_model(const std::string name, const FILE_FORMAT format)
    {
        switch (format) {
            case FILE_FORMAT::GLB:
                return load_glb(name);
            default:
                printf("Unsupported fileformat.");
                break;
        }
        return false;
    }

    bool load_glb(const std::string &name)
    {
        const auto path_model = path_models / name;

        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;
    
        bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path_model);
    
        if (!warn.empty())
            printf("Warn: %s\n", warn.c_str());
    
        if (!err.empty())
            printf("Err: %s\n", err.c_str());
    
        if (!ret) {
            printf("Failed to parse glTF (glb).\n");
            return false;
        }

        models.emplace(name, Model{});

        if (!load_glb_meshes(models[name], model)) {
            printf("Failed to load model meshes.\n");
            return false;
        }

        if (!load_glb_transformations(models[name], model)) {
            printf("Failed to load transformations.\n");
            return false;
        }

        return true;
    }

    bool load_glb_meshes(Model& m, const tinygltf::Model& model)
    {
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
    
        std::vector<uint32_t> indices{};
        indices.reserve(nof_indices);
    
        m.meshes.resize(model.meshes.size());
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
    
            m.meshes[i].offset = index_offset;
            m.meshes[i].count = indices.size();
            index_offset += indices.size();
    
            glGenVertexArrays(1, &m.meshes[i].VAO);
            glBindVertexArray(m.meshes[i].VAO);
    
            glGenBuffers(1, &m.meshes[i].VBO_pos);
            glBindBuffer(GL_ARRAY_BUFFER, m.meshes[i].VBO_pos);
            glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(POSITION_LOCATION);
            glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
            if (!normals.empty()) {
                glGenBuffers(1, &m.meshes[i].VBO_norm);
                glBindBuffer(GL_ARRAY_BUFFER, m.meshes[i].VBO_norm);
                glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
                glEnableVertexAttribArray(NORMAL_LOCATION);
                glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);
            }
    
            if (!texCoords.empty()) {
                glGenBuffers(1, &m.meshes[i].VBO_tc);
                glBindBuffer(GL_ARRAY_BUFFER, m.meshes[i].VBO_tc);
                glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);
                glEnableVertexAttribArray(TEX_COORD_LOCATION);
                glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);
            }
    
            glGenBuffers(1, &m.meshes[i].EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.meshes[i].EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
    
            glBindVertexArray(0);
        }

        return true;
    }

    bool load_glb_transformations(Model& m, const tinygltf::Model &model)
    {
        assert(model.scenes.size() == 1 && "Found several scenes. Only supporting single scene.");
        const tinygltf::Scene& scene = model.scenes[0];
        const auto& nodes = scene.nodes;

        struct StkEntry {
            const tinygltf::Node& node;
            glm::mat4x4 model_matrix{1.0f};
        };

        std::stack<StkEntry> stk{};
        stk.push(StkEntry{
            .node = model.nodes[nodes[0]],
            .model_matrix = vec_to_glm_mat4x4(model.nodes[nodes[0]].matrix)
        });

        while (!stk.empty()) {
            const auto parent = stk.top();
            stk.pop();

            if (parent.node.mesh != -1) {
                m.meshes[parent.node.mesh].model_matrix = parent.model_matrix;
            }

            // process children
            for (const auto& child_idx : parent.node.children) {
                const auto& child_node = model.nodes[child_idx];
                const auto& child_matrix = vec_to_glm_mat4x4(child_node.matrix);
                stk.push(StkEntry{
                    .node = model.nodes[child_idx],
                    .model_matrix = parent.model_matrix * child_matrix
                });
            }
        }

        return true;
    }

    glm::mat4x4 vec_to_glm_mat4x4(const std::vector<double> &mat)
    {
        if (mat.size() == 0)
            return glm::mat4x4(1.0f);

        assert(mat.size() == 16 && "Supplied matrix must have size 16.");
        glm::mat4x4 res{};
        for (size_t row = 0; row < 4; row++) {
            for (size_t col = 0; col < 4; col++) {
                res[row][col] = mat[row * 4 + col];
            }
        }
        return res;
    }

}; // end namespace 'AssetManager'