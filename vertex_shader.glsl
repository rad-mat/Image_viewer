#version 330 

layout (location = 0) in vec3 pos;
layout (location = 1) in float col;
layout (location = 2) in float frame;
uniform mat4 MVP;

out float Col;
out float Frame;

void main()
{
    Frame = frame;
    Col = col;
    gl_Position = MVP * vec4(pos, 1.0);
}