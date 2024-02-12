#version 330 core 
layout (location = 0) in vec2 aPosition; 
layout (location = 1) in vec3 aColor; 
uniform float uDelta;
uniform float a[10];


out vec3 vColor; 
void main(void) 
{ 
	// change the position of the vertex with a sin function
	float d  = (uDelta>0.25)?0.5-uDelta:uDelta;
	float dy =  0.2*sin(d*3.14*4.0);
    gl_Position = vec4(aPosition+vec2(d,dy), 0.0, 1.0); 

	// perturb the color with the position
    vColor = aColor + vec3(d,dy,0.0); 
}