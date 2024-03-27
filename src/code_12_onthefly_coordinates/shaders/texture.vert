#version 330 core 
layout (location = 0) in vec3 aPosition; 
layout (location = 2) in vec3 aNormal;
 
out vec2 vTexCoord;
out vec4 vProjTexCoord;
out vec4 vSkyboxTexCoord;
out vec3 vLdirVS;
out vec3 vNormalVS;
out vec3 vNormalWS;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;
uniform vec4 uLdir;

uniform mat4 uLPView;
uniform mat4 uLPProj;

uniform int uRenderMode;


void main(void) 
{ 
    gl_Position = uProj*uView*uModel*vec4(aPosition, 1.0); 
	
	vLdirVS   = (uView*uLdir).xyz;
	vNormalVS = normalize((uView*uModel*vec4(aNormal,0.0)).xyz);
	vNormalWS = normalize(( uModel*vec4(aNormal,0.0)).xyz);
	vProjTexCoord  = uLPProj* uLPView*uModel*vec4(aPosition, 1.0);
	vSkyboxTexCoord =  inverse(uView)*(uView*uModel*vec4(aPosition, 1.0)-vec4(0,0,0,1.0));
}
