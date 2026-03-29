#version 330 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vColor;

uniform vec3 uCameraPos;
uniform vec3 uLightPos[2];
uniform vec3 uLightColor;

out vec4 fragColor;

void main() {
    vec3 n = normalize(vNormal);

    float ambient = 0.15;
    vec3 color = vColor * ambient;

    for (int li = 0; li < 2; li++) {
        vec3 lightDir = normalize(uLightPos[li] - vWorldPos);
        vec3 viewDir = normalize(uCameraPos - vWorldPos);
        vec3 reflectDir = reflect(-lightDir, n);

        float diff = max(dot(n, lightDir), 0.0);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

        color += vColor * diff * 0.7 + uLightColor * spec * 0.3;
    }

    fragColor = vec4(color, 1.0);
}
