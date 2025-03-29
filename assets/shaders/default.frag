#version 460 core

uniform vec3 u_CameraPosition;

in vec3 worldSpacePos_;
in vec3 normal_;
in vec2 texCoord_;

out vec4 FragColor;

void main() {
    //FragColor = vec4(1.0);
    FragColor = vec4(normal_*0.5 + 0.5, 1.0f);
}
