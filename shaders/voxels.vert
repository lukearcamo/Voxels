#version 330 core

layout (location = 0) in vec3 position;
// layout (location = 0) in uint position;
layout (location = 1) in uint UVAO;

out vec2 vUv;
out vec4 vAo;
out float vUvOffsetX;

out vec3 worldPosition;

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
// Normal matrix is silently ignored because get uniform location returns -1

// vec3 unpackPositionCustom(uint ret) {
//     uint ix = ret & 0x3ffu;
//     uint iy = (ret >> 10) & 0x3ffu;
//     uint iz = (ret >> 20) & 0x3ffu;
//     return vec3(ix, iy, iz) / 16.0 - 1.0;
// }

void main() {
    vAo = vec4(
        (UVAO >> 16) & 0x3u,
        (UVAO >> 18) & 0x3u,
        (UVAO >> 20) & 0x3u,
        (UVAO >> 24) & 0x3u
    ) / 3.0;

    vUv = vec2(
        UVAO & 0xfu,
        (UVAO >> 4) & 0xfu
    ) / 15.0;
    float uvOffset = (UVAO >> 8) & 0xffu;
    vUvOffsetX = (vUv.x + uvOffset) / 12; // NUM_ATLAS_FACES

    gl_Position = worldMatrix * vec4(position / 16.0, 1.0);
    // vec3 realPos = unpackPositionCustom(position);
    // gl_Position = worldMatrix * vec4(realPos, 1.0);

    worldPosition = gl_Position.xyz;
    gl_Position = projectionMatrix * viewMatrix * gl_Position;
}