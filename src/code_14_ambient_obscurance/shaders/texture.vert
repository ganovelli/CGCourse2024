#version 430 core 
layout (location = 0) in vec3 aPosition; 
layout (location = 2) in vec3 aNormal;
 
out vec2 vTexCoord;
out vec4 vProjTexCoord;
out vec4 vSkyboxTexCoord;
out vec3 vLdirVS;
out vec3 vNormalVS;
out vec3 vNormalWS;

uniform mat4 uP;
uniform mat4 uV;
uniform mat4 uT;
uniform vec4 uLdir;

uniform mat4 uLPView;
uniform mat4 uLPProj;

uniform int uRenderMode;


void main(void) 
{ 
    gl_Position = uP*uV*uT*vec4(aPosition, 1.0); 
	
	vLdirVS   = (uV*uLdir).xyz;
	vNormalVS = normalize((uV*uT*vec4(aNormal,0.0)).xyz);
	vNormalWS = normalize(( uT*vec4(aNormal,0.0)).xyz);
	vProjTexCoord  = uLPProj* uLPView*uT*vec4(aPosition, 1.0);
	vSkyboxTexCoord =  inverse(uV)*(uV*uT*vec4(aPosition, 1.0)-vec4(0,0,0,1.0));
}
