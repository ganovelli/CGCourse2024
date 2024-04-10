#version 430 core 
layout (location = 0) in vec3 aPosition; 
layout (location = 2) in vec3 aNormal; 

out vec3  vNVS;
uniform mat4 uModel;
uniform mat4 uProj;
uniform mat4 uView;
uniform int uRenderMode;


void main(void) 
{ 
    gl_Position = uProj*uView*uModel*vec4(aPosition, 1.0);
	vNVS = (uView*uModel*vec4(aNormal,0.0)).xyz; 
}
