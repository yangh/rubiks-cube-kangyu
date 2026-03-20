#version 330 core

in vec3 vWorldPos;
out vec4 fragColor;

uniform vec3 cameraPos;
uniform vec3 cubiePositions[27];
uniform float cubieSize;
uniform float animAngle;
uniform vec3 animAxis;
uniform float animSliceMask[27];
uniform vec3 faceColors[54];
uniform float gap;
uniform vec3 lightPos[2];
uniform vec3 lightColor;
uniform vec2 resolution;

float sdRoundBox(vec3 p, vec3 b, float r) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0) - r;
}

float sdRoundRect(vec2 p, vec2 b, float r) {
    vec2 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

mat3 rotateAroundAxis(vec3 axis, float angle) {
    float c = cos(angle);
    float s = sin(angle);
    float t = 1.0 - c;
    vec3 a = normalize(axis);
    return mat3(
        t * a.x * a.x + c,       t * a.x * a.y - s * a.z, t * a.x * a.z + s * a.y,
        t * a.x * a.y + s * a.z, t * a.y * a.y + c,       t * a.y * a.z - s * a.x,
        t * a.x * a.z - s * a.y, t * a.y * a.z + s * a.x, t * a.z * a.z + c
    );
}

float sceneSDF(vec3 p, out int cubieIndex, mat3 animRot, bool hasAnim) {
    float minDist = 1e10;
    cubieIndex = -1;

    for (int i = 0; i < 27; i++) {
        vec3 pos = cubiePositions[i];

        if (hasAnim && animSliceMask[i] > 0.5) {
            pos = animRot * pos;
        }

        vec3 localP = p - pos;

        if (hasAnim && animSliceMask[i] > 0.5) {
            localP = transpose(animRot) * localP;
        }

        float d = sdRoundBox(localP, vec3(cubieSize), cubieSize * 0.1);

        if (d < minDist) {
            minDist = d;
            cubieIndex = i;
        }
    }
    return minDist;
}

vec3 calcNormal(vec3 p, mat3 animRot, bool hasAnim) {
    const float h = 0.0005;
    const vec2 k = vec2(1.0, -1.0);
    int dummy;
    return normalize(
        k.xyy * sceneSDF(p + k.xyy * h, dummy, animRot, hasAnim) +
        k.yyx * sceneSDF(p + k.yyx * h, dummy, animRot, hasAnim) +
        k.yxy * sceneSDF(p + k.yxy * h, dummy, animRot, hasAnim) +
        k.xxx * sceneSDF(p + k.xxx * h, dummy, animRot, hasAnim)
    );
}

int getFaceIndex(vec3 n) {
    vec3 an = abs(n);
    if (an.x >= an.y && an.x >= an.z) {
        return n.x > 0.0 ? 4 : 5;
    } else if (an.y >= an.z) {
        return n.y > 0.0 ? 2 : 3;
    } else {
        return n.z > 0.0 ? 0 : 1;
    }
}

vec2 projectToFace(vec3 localP, vec3 n) {
    vec3 an = abs(n);
    if (an.x >= an.y && an.x >= an.z) {
        return localP.yz;
    } else if (an.y >= an.z) {
        return localP.xz;
    } else {
        return localP.xy;
    }
}

void main() {
    vec3 ro = cameraPos;
    vec3 rd = normalize(vWorldPos - cameraPos);

    mat3 animRot;
    bool hasAnim = (animAngle != 0.0);
    if (hasAnim) {
        animRot = rotateAroundAxis(animAxis, radians(animAngle));
    }

    float t = 0.0;
    int cubieIdx = -1;
    bool hit = false;
    float closestDist = 1e10;
    float tClosest = 0.0;
    int closestCubie = -1;

    for (int i = 0; i < 48; i++) {
        vec3 p = ro + t * rd;
        int ci;
        float d = sceneSDF(p, ci, animRot, hasAnim);
        if (d < closestDist) {
            closestDist = d;
            closestCubie = ci;
            tClosest = t;
        }
        if (d < 0.001) { hit = true; cubieIdx = ci; break; }
        if (t > 20.0) break;
        if (t > 5.0 && d > t * 0.5) break;
        t += d;
    }

    float coverage = 1.0;
    vec3 p;

    if (hit) {
        p = ro + t * rd;
    } else {
        float pixelSize = 2.0 * tClosest * tan(radians(22.5)) / resolution.y;
        coverage = 1.0 - smoothstep(pixelSize * 0.3, pixelSize * 1.0, closestDist);
        if (coverage < 0.01) { discard; }
        p = ro + tClosest * rd;
        cubieIdx = closestCubie;
    }

    vec3 n = calcNormal(p, animRot, hasAnim);

    int ci = cubieIdx;

    vec3 pos = cubiePositions[ci];
    if (hasAnim && animSliceMask[ci] > 0.5) {
        pos = animRot * pos;
    }
    vec3 localP = p - pos;
    if (hasAnim && animSliceMask[ci] > 0.5) {
        localP = transpose(animRot) * localP;
    }

    vec3 localN = n;
    if (hasAnim && animSliceMask[ci] > 0.5) {
        localN = transpose(animRot) * n;
    }

    int fi = getFaceIndex(localN);

    int layer = ci / 9;
    int posInLayer = ci % 9;
    int row = posInLayer / 3;
    int col = posInLayer % 3;

    int stickerIdx = -1;
    if (fi == 0 && layer == 2) stickerIdx = (2 - row) * 3 + col;
    else if (fi == 1 && layer == 0) stickerIdx = 9 + (2 - row) * 3 + (2 - col);
    else if (fi == 2 && row == 2) stickerIdx = 18 + layer * 3 + col;
    else if (fi == 3 && row == 0) stickerIdx = 27 + (2 - layer) * 3 + col;
    else if (fi == 4 && col == 2) stickerIdx = 36 + (2 - row) * 3 + (2 - layer);
    else if (fi == 5 && col == 0) stickerIdx = 45 + (2 - row) * 3 + layer;

    vec3 stickerColor = (stickerIdx >= 0) ? faceColors[stickerIdx] : vec3(0.02);

    float stickerSize = cubieSize * 0.80;  // sticker coverage ratio on cubie face; decrease this value to widen the black gap between sticker edge and cubie edge
    float stickerRadius = cubieSize * 0.20;  // sticker corner radius

    vec2 uv = projectToFace(localP, localN);
    float sd = sdRoundRect(uv, vec2(stickerSize), stickerRadius);

    float edgeSoft = 0.015 * cubieSize;
    float aa = 1.0 - smoothstep(-edgeSoft, edgeSoft, sd);

    vec3 baseColor = vec3(0.02, 0.02, 0.02);
    vec3 surfaceColor = (stickerIdx >= 0) ? mix(baseColor, stickerColor, aa) : baseColor;

    float ambient = 0.15;
    vec3 color = surfaceColor * ambient;

    for (int li = 0; li < 2; li++) {
        vec3 lightDir = normalize(lightPos[li] - p);
        vec3 viewDir = normalize(cameraPos - p);
        vec3 reflectDir = reflect(-lightDir, n);

        float diff = max(dot(n, lightDir), 0.0);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

        color += surfaceColor * diff * 0.7 + lightColor * spec * 0.3;
    }

    fragColor = vec4(color * coverage, 1.0);
}
