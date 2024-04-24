#version 460 core 
layout (location = 0) in vec3 aPosition; 
layout (location = 2) in vec3 aNormal; 
layout (location = 4) in vec2 aTexCoord; 
 
out vec3 vLDirVS;
out vec3 vPosVS;
out vec3 vNormalVS;
out vec3 vColor;
out vec2 vTexCoord0;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;
uniform vec3 uLDir;

uniform int	 uShadingMode;
uniform vec3 uDiffuseColor;
uniform vec3 uAmbientColor;
uniform vec3 uSpecularColor;
uniform vec3 uEmissiveColor;
uniform vec3 uLightColor;
uniform float uShininess;

/* phong lighting */
vec3 phong ( vec3 L, vec3 V, vec3 N){
	float LN = max(0.0,dot(L,N));

	vec3 R = -L+2*dot(L,N)*N;

	float spec = ((LN>0.f)?1.f:0.f) * max(0.0,pow(dot(V,R),uShininess));

	return (uAmbientColor+LN*uDiffuseColor + spec * uSpecularColor)*uLightColor;
}



void main(void) 
{ 
	
	vLDirVS   =  (uView*vec4(uLDir,0.f)).xyz; 
	vNormalVS =  (uView*uModel*vec4(aNormal, 0.0)).xyz; 

	vPosVS = (uView*uModel*vec4(aPosition, 1.0)).xyz; 
	
	/* compute lighiting in the vertex shader (Gauraud shading) */
	vColor    = phong(vLDirVS,normalize(-vPosVS),normalize(vNormalVS));
	vTexCoord0    = aTexCoord;

    gl_Position = uProj*uView*uModel*vec4(aPosition, 1.0); 
}