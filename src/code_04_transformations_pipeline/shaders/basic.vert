#version 460 core 
in vec3 aPosition;
in vec3 aColor;
out vec3 vColor;
out vec3 vPos; 

uniform mat4 uRot;  
uniform mat4 uModel;
uniform mat4 uView; 
uniform mat4 uProj; 

void main(void)
{
	gl_Position = uProj*uView * uModel*uRot*vec4(aPosition, 1.0);
	vColor = aColor;

	// ignore this next line
	vPos = (uView * uModel*uRot*vec4(aPosition, 1.0)).xyz;
}