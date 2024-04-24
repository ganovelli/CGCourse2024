#version 460 core  
out vec4 color; 
in vec3 vColor;
in vec3 vPosVS;
in vec3 vNormalVS;
in vec3 vLDirVS;
in vec2 vTexCoord0;

uniform sampler2D uTexColor;

uniform vec3 uLDir;
uniform vec3 uColor;

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
	if(uShadingMode == 1){
		vec3 N = normalize(cross(dFdx(vPosVS),dFdy(vPosVS)));
		color = vec4(phong(vLDirVS,normalize(-vPosVS),N),1.0);
	}
 	else
	if(uShadingMode == 2){
		color = vec4(vColor,1.0);
	}
 	else
	if(uShadingMode == 3){
		color = vec4(phong(vLDirVS,normalize(-vPosVS),normalize(vNormalVS)),1.0);
	}
	else
	if(uShadingMode == 4 ){
			color = vec4(phong(vLDirVS,normalize(-vPosVS),normalize(vNormalVS)),1.0);
			color = texture2D(uTexColor,vTexCoord0.xy)*(color.x+color.y+color.z)/3.f;
	}
	else
	/* just output the interpolated vertex normal as color		*/
	/* Note: normal is a vector with values in [-1,-1,-1][1,1,1]*/
	/* and  must be remapped in  in [0,0,0][1,1,1]				*/ 
	color = vec4(normalize(vNormalVS)*0.5+0.5,1.0);
} 