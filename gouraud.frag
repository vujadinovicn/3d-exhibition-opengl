#version 330 core

in vec3 fragNormal;
in vec3 fragColor;
uniform int isMaterial;
out vec4 outCol;
uniform sampler2D uTex;
in vec2 chTex;

void main() {
    if (isMaterial == 0)
        outCol = vec4(fragColor * texture(uTex, chTex).rgb, 1.0); 
    else 
        outCol = vec4(fragColor, 1.0);
}