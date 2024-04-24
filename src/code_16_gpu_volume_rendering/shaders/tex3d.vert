#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uModelPlane;
uniform mat4 uView;
uniform mat4 uProj;
uniform vec3 uNCells;
out vec3 TexCoords;

void main()
{
    TexCoords =   ((uModelPlane*vec4(aPos, 1.0)).xyz);
    gl_Position = uProj*uView*uModel*vec4(aPos, 1.0);
}
