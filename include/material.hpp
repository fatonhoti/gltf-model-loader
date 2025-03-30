#pragma once

#include "glm/vec4.hpp"

struct Material {
    Material() = default;

    enum class AlphaMode {OPAQUE, MASK, BLEND};
    bool double_sided{false};

    AlphaMode mode{AlphaMode::OPAQUE};
    float alpha_cutoff{0.5f};
    glm::vec4 base_color{1.0};
    float metalness{1.0};
    float roughness{0.0};
    
    int32_t base_color_texture_idx{-1};
    int32_t metallic_roughness_texture_idx{-1};
    int32_t normal_texture_idx{-1};
    int32_t emissive_texture_idx{-1};
};