#version 460 core 
layout (location = 0) in vec3 aPosition; 
 
out vec3 vPos;
uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;
uniform mat4 uRot;

void main(void) 
{ 
	vPos = (uView*uRot*uModel*vec4(aPosition, 1.0)).xyz; 
    gl_Position = uProj*uView*uRot*uModel*vec4(aPosition, 1.0); 
}