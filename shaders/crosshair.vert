#version 330 core

const vec2 vertices[4] = vec2[4](
    vec2(0.0, 0.04),
    vec2(0.0, -0.04),
    vec2(0.02, 0.0),
    vec2(-0.02, 0.0)
);

void main() {
    gl_Position = vec4(vertices[gl_VertexID], 0.0, 1.0);
}