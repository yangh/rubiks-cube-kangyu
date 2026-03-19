#version 330 core

out vec3 vWorldPos;

uniform mat4 view;
uniform mat4 projection;

const vec2 quadVertices[6] = vec2[6](
    vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(1.0, 1.0),
    vec2(-1.0, -1.0), vec2(1.0, 1.0), vec2(-1.0, 1.0)
);

void main() {
    vec2 pos = quadVertices[gl_VertexID];
    
    vec4 clipPos = vec4(pos, -1.0, 1.0);
    vec4 viewPos = inverse(projection) * clipPos;
    viewPos = viewPos / viewPos.w;
    
    vWorldPos = (inverse(view) * vec4(viewPos.xyz, 1.0)).xyz;
    
    gl_Position = clipPos;
}
