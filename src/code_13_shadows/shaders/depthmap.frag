#version 430 core  
out vec4 color; 

uniform int uRenderMode;
uniform float uPlaneApprox;

// this produce the Hue for v:0..1 (for debug purposes)
vec3 hsv2rgb(float  v)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(vec3(v,v,v) + K.xyz) * 6.0 - K.www);
    return   mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0),1.0);
}

float  PlaneApprox() {   
	// Compute partial derivatives of depth.    
	float dx = dFdx(gl_FragCoord.z);   
	float dy = dFdy(gl_FragCoord.z);

	// Compute second moment over the pixel extents.   
	return  gl_FragCoord.z*gl_FragCoord.z + uPlaneApprox*0.5*(dx*dx + dy*dy);   
} 

void main(void) 
{ 
//	color = vec4(vec3((gl_FragCoord.z)),1.0);
//	color = vec4(1.0,0.0,0.0,1.0);
 	color = vec4(gl_FragCoord.z,PlaneApprox(),0.0,1.0);
} 