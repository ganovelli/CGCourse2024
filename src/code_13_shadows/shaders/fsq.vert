#version 430 core 
layout (location = 0) in vec3 aPosition; 

out vec2 vTexCoord;

void main(void) 
{ 
    gl_Position = vec4(aPosition, 1.0); 
	vTexCoord = aPosition.xy*0.5f + 0.5f;
}
