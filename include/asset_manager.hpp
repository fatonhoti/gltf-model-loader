#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>

#include "glm/glm.hpp"
#include "tiny_gltf.h"

#include "material.hpp"
#include "texture.hpp"

namespace AssetManager {

    constexpr uint32_t POSITION_LOCATION = 0;
    constexpr uint32_t NORMAL_LOCATION = 1;
    constexpr uint32_t TEX_COORD_LOCATION = 2;

    const auto path_assets = std::filesystem::current_path() / "assets";
    const auto path_models = path_assets / "models";

    struct Model {
        struct Primitive {
            int32_t mat_idx{-1};
            glm::mat4x4 model_matrix{1.0f};
            uint32_t VAO, VBO_pos, VBO_norm, VBO_tc, EBO;
            uint32_t offset{0};
            uint32_t count{0};
        };
        std::vector<Primitive> meshes{};
        std::string name{};
    };
    extern std::unordered_map<std::string, Model> models;
    extern std::vector<Material> materials;
    extern std::vector<Texture> textures;

    enum class FILE_FORMAT {
        GLB,
        GLTF,
    };

    bool load_model(const std::string name, const FILE_FORMAT format);

    bool load_glb(const std::string& name);
    bool load_glb_meshes(Model& m, const tinygltf::Model& model);
    bool load_glb_materials(Model& m, const tinygltf::Model& model);
    bool load_glb_transformations(Model& m, const tinygltf::Model& model);

    glm::mat4x4 vec_to_glm_mat4x4(const std::vector<double>& mat);
    glm::mat4 get_node_transform(const tinygltf::Node& node);

}; // end namespace 'AssetManager'