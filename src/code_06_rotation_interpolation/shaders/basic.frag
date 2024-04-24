#version 460 core  
out vec4 color; 
in vec3 vColor;

uniform float uScale;
 
void main(void) 
{    
   	
   color = vec4(vColor*uScale , 1.0); 
 } 