#version 410 core

layout (location = 0) in vec3 vertexPosition;
out vec3 textureCoordinates;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vec4 tempPos = projection * view * vec4(vertexPosition, 1.0f);
    gl_Position = tempPos.xyww;
    textureCoordinates = vertexPosition;
}
