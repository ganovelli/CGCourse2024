#version 430 core 
layout (location = 0) in vec3 aPosition; 
layout (location = 2) in vec3 aNormal;
 
out vec4 vCoordLS;
out vec3 vNormalVS;
out vec3 vNormalWS;
out vec3 vVWS;
out vec3 vLWS;

uniform mat4 uP;
uniform mat4 uV;
uniform mat4 uT;
uniform mat4	 uLightMatrix;

uniform int uRenderMode;

void main(void) 
{ 
    gl_Position = uP*uV*uT*vec4(aPosition, 1.0); 
	
	vNormalWS = normalize((  inverse(transpose(uT))*vec4(aNormal,0.0)).xyz);
	vVWS = (inverse(uV)*(vec4(0.0,0.0,0.0,1.0)-uV*uT*vec4(aPosition, 1.0))).xyz;

	vCoordLS =  uLightMatrix*uT*vec4(aPosition, 1.0);


	vec4 vCoordLS_NDC = (vCoordLS/vCoordLS.w);
	vec4 f = vec4(vCoordLS_NDC.xy, 1,1.0);
	vec4 n = vec4(vCoordLS_NDC.xy,-1,1.0);
	vec4 fWS = inverse(uLightMatrix)*f;
	fWS/=fWS.w;

	vec4 nWS = inverse(uLightMatrix)*n;
	nWS/=nWS.w;

	vLWS = normalize((nWS-fWS).xyz);

}
