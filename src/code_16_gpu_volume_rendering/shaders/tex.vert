#version 460 core
layout (location = 0) in vec3 aPos;
	
uniform mat4 uModel;
out vec2 TexCoords;
	
void main()
{
    TexCoords = ((uModel*vec4(aPos, 1.0)).xy+vec2(1.f))*0.5;
    gl_Position = uModel*vec4(aPos, 1.0);
}