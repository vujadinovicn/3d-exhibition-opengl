#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNor; // Normale
layout(location = 2) in vec2 aTexCoords;

out vec3 chFragPos; // Interpolated fragment position
out vec3 chNor;     // Interpolated normals

uniform vec2 uPos;
uniform float uAngle; // Rotation angle in degrees

uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;
out vec2 TexCoords;

void main()
{
    // Apply rotation around the Z-axis
    vec3 t = vec3(inPos.x-uPos.x , inPos.y+uPos.y, inPos.z);
    TexCoords = aTexCoords;
    chFragPos = vec3(uM * vec4(t, 1.0));
    gl_Position = uP * uV * vec4(chFragPos, 1.0);
    chNor = mat3(transpose(inverse(uM))) * inNor;
}
