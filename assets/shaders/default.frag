#version 460 core

uniform sampler2D baseColorTexture;
uniform sampler2D metallicRoughnessTexture;

uniform int hasBaseColorTexture;
uniform int hasMetallicRoughnessTexture;

struct Material {
    vec4 base_color;
    float metalness;
    float roughness;
};
uniform Material mat;

uniform vec3 u_CameraPosition;

in vec3 worldSpacePos_;
in vec3 normal_;
in vec2 texCoord_;

out vec4 FragColor;

const float PI = 3.14159265359;
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const vec3 lightDirection = normalize(vec3(1.0, 1.0, 1.0));
const float lightIntensity = 2.0;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec4 albedo = mat.base_color;
    if (hasBaseColorTexture == 1) {
        albedo = texture(baseColorTexture, texCoord_);
    }

    float metallic = mat.metalness;
    float roughness = mat.roughness;
    if (hasMetallicRoughnessTexture == 1) {
        vec4 t = texture(metallicRoughnessTexture, texCoord_);
        roughness *= t.g;
        metallic *= t.b;
    }

    vec3 N = normalize(normal_);
    vec3 V = normalize(u_CameraPosition - worldSpacePos_);
    vec3 L = normalize(-lightDirection);
    vec3 H = normalize(V + L);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo.rgb, metallic);

    float NDF = DistributionGGX(N, H, roughness);   
    float G = GeometrySmith(N, V, L, roughness);      
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    float NdotL = max(dot(N, L), 0.0);        
    vec3 radiance = lightColor * lightIntensity;
    vec3 Lo = (kD * albedo.rgb / PI + specular) * radiance * NdotL;

    vec3 ambient = vec3(0.1) * albedo.rgb;
    vec3 color = ambient + Lo;

    // Tone mapping and gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, albedo.a);
}

/*
#version 460 core

uniform sampler2D baseColorTexture;
uniform sampler2D metallicRoughnessTexture;

uniform int hasBaseColorTexture;
uniform int hasMetallicRoughnessTexture;

struct Material {
    vec4 base_color;
    float metalness;
    float roughness;
};
uniform Material mat;

uniform vec3 u_CameraPosition;

in vec3 worldSpacePos_;
in vec3 normal_;
in vec2 texCoord_;

out vec4 FragColor;

void main() {
    vec4 color = mat.base_color;
    if (hasBaseColorTexture == 1) {
        color = texture(baseColorTexture, texCoord_);
    }

    float metalness = mat.metalness;
    float roughness = mat.roughness;
    if (hasMetallicRoughnessTexture == 1) {
        vec4 t = texture(metallicRoughnessTexture, texCoord_);
        roughness *= t.g;
        metalness *= t.b;
    }

    if (gl_FragCoord.x < 400) {
        FragColor = color;
    } else if (gl_FragCoord.x < 800) {
        FragColor = vec4(vec3(metalness), 1.0);
    } else {
        FragColor = vec4(vec3(roughness), 1.0);
    }
    
    // Uncomment to visualize normals (debug)
    // FragColor = vec4(normal_*0.5 + 0.5, 1.0f);
}
*/