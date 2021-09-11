#version 330 core

uniform sampler2D map;
uniform samplerCube fogMap;
uniform vec3 cameraPosition;

// in float vAo;
in vec2 vUv;
in vec4 vAo;
in float vUvOffsetX;

in vec3 worldPosition;

out vec4 fragColor;

void main() {
    vec3 color = vec3(0.8, 0.4, 0.3);

    float top = mix(vAo.x, vAo.y, vUv.x);
    float bottom = mix(vAo.z, vAo.w, vUv.x);
    float ao = mix(bottom, top, vUv.y);
    // ao = ao * 0.5 + 0.5;

    color = texture(map, vec2(vUvOffsetX, 1.0 - vUv.y)).rgb; // Flipped uv y because texture is flipped

    fragColor = vec4(color * ao, 1.0);
    // fragColor = vec4(vUv, 0.0, 1.0);
    
    vec3 worldToEye = cameraPosition - worldPosition;
    float fogFactor = smoothstep(50.0, 200.0, length(worldToEye));
    // vec3 fogColor = vec3(1.0);
    vec3 fogColor = texture(fogMap, -worldToEye).rgb;
    fragColor.rgb = mix(fragColor.rgb, fogColor, fogFactor);
}