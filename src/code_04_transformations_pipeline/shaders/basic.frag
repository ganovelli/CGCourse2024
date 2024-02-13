#version 330 core  
out vec4 color; 
in vec3 vColor; 
in vec3 vPos; 
void main(void) 
{ 
   // this part uses concept we haven't covered yet. Please ignore it
   vec3 N = normalize(cross(dFdx(vPos),dFdy(vPos)));
   vec3 L0 = normalize(vec3(2,2,0)-vPos);
   vec3 L1 = normalize(vec3(-2,1,0)-vPos);
   float contrib = (max(0.f,dot(N,L0))+max(0.f,dot(N,L1)))*0.5;

   color = vec4(vColor*contrib, 1.0); 
} 