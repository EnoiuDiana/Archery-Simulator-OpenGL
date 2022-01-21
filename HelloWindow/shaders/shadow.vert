#version 410 core
layout(location=0) in vec3 vPosition;
uniform mat4 lightSpaceTrMatrix;
uniform mat4 model;
layout(location=2) in vec2 vTexCoords;
out vec2 fTexCoords;
void main()
{
 gl_Position = lightSpaceTrMatrix * model * vec4(vPosition, 1.0f);
 fTexCoords = vTexCoords;
}