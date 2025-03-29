#version 460 core

layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;

uniform mat4 u_PV;
uniform mat4 u_ModelMatrix;

out vec3 worldSpacePos_;
out vec3 normal_;
out vec2 texCoord_;

void main() {
    const vec4 worldSpacePos = u_ModelMatrix * vec4(aPos, 1.0f);
    gl_Position = u_PV * worldSpacePos;
    worldSpacePos_ = vec3(worldSpacePos);
    normal_ = normalize(transpose(inverse(mat3(u_ModelMatrix))) * aNormal);
    texCoord_ = aTexCoord;
}
