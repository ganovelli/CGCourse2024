#version 410 core 
layout (location = 0) in vec3 aPosition; 
layout (location = 1) in vec3 aColor; 
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec2 aTexCoord;
 
out vec2 vTexCoord;
out vec3 vLdirVS;
out vec3 vLdirTS;
out vec3 vVdirTS;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;
uniform vec4 uLdir;
uniform int uRenderMode;


void main(void) 
{ 
    // computing the (inverse of the ) tangent frame
    vec3 tangent = normalize(aTangent);
    vec3 bitangent = normalize(cross(aNormal,tangent));
	
	mat3 TF;
	TF[0]	= tangent;
	TF[1]	= bitangent;
	TF[2]	= normalize(aNormal);
	TF		= transpose(TF);

	// light direction in tangent space
	vLdirTS   =    TF * (inverse(uModel)*uLdir).xyz;

	vec3 ViewVS  =  (vec4(0.0,0.0,0.0,1.0) -uView*uModel*vec4(aPosition, 1.0)).xyz; 

	// view direction in tangent space
	vVdirTS	  =    TF * (inverse(uModel)*inverse(uView)* vec4(ViewVS,0.0)).xyz;

	vLdirVS   = (uView*uLdir).xyz;
	vTexCoord = aTexCoord;
    gl_Position = uProj*uView*uModel*vec4(aPosition, 1.0);
}
