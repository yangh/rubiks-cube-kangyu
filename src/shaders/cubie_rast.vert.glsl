#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in mat4 aInstanceModel;
layout(location = 6) in vec3 aInstanceColor;

uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vColor;

void main() {
    vec4 worldPos = aInstanceModel * vec4(aPos, 1.0);
    vWorldPos = worldPos.xyz;
    vNormal = normalize(mat3(aInstanceModel) * aNormal);
    vColor = aInstanceColor;
    gl_Position = uProjection * uView * worldPos;
}
