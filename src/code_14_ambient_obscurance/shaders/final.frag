#version 430 core  
out vec4 color; 

 
in vec2  vTexCoord;

uniform sampler2D uNormalMap;
uniform sampler2D uAOMap;
uniform int uUseAO;
uniform vec3 uLVS;

void main(void) 
{	
	float ao = texture(uAOMap,vTexCoord).x;
	vec3 N = texture(uNormalMap,vTexCoord).xyz;

	N = N*2.0-1.0;
	N = normalize(N);

	float NL = max(0.0,dot(normalize(uLVS),N)); 
	if(uUseAO == 1)
	 color = vec4(vec3(NL*ao),1.0);
		else
	 color = vec4(vec3(NL),1.0);
} 