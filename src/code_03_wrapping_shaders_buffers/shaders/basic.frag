#version 460 core  
out vec4 color; 
in vec3 vColor; 
void main(void) 
{ 
    color = vec4(vColor, 1.0); 
} 