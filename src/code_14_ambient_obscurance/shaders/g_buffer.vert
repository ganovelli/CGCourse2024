#version 430 core 
layout (location = 0) in vec3 aPosition; 
layout (location = 2) in vec3 aNormal; 

out vec3  vNVS;
uniform mat4 uT;
uniform mat4 uP;
uniform mat4 uV;
uniform int uRenderMode;


void main(void) 
{ 
    gl_Position = uP*uV*uT*vec4(aPosition, 1.0);
	vNVS = (uV*uT*vec4(aNormal,0.0)).xyz; 
}
