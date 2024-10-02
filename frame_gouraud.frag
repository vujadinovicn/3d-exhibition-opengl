#version 330 core

in vec3 fragNormal;
in vec3 fragColor;
uniform int isTexture;
out vec4 outCol;
uniform sampler2D uTex;
in vec2 chTex; 
uniform vec3 objectColor;

void main() {
    if (isTexture == 1)
        outCol = vec4(fragColor * texture(uTex, chTex).rgb, 1.0); 
    else 
        outCol = vec4(fragColor * objectColor, 1.0);
    
}