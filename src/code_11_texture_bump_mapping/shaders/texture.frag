#version 410 core  
out vec4 color; 

in vec2 vTexCoord;
in vec3 vLdirVS;
in vec3 vLdirTS;
in vec3 vVdirTS;

uniform int uRenderMode;
uniform vec3 uDiffuseColor;

uniform sampler2D uColorImage;
uniform sampler2D uBumpmapImage;
uniform sampler2D uNormalmapImage;


/* Diffuse */
vec3 diffuse( vec3 L, vec3 N){
	return  max(0.0,dot(L,N))*texture2D(uColorImage,vTexCoord.xy).xyz;
}

// this produce the Hue for v:0..1 (for debug purposes)
vec3 hsv2rgb(float  v)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(vec3(v,v,v) + K.xyz) * 6.0 - K.www);
    return   mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0),1.0);
}

void main(void) 
{ 

	if(uRenderMode==0)
	color = vec4(vTexCoord,0.0,1.0);
	else
	if(uRenderMode==1)
		color = texture2D(uColorImage,vTexCoord.xy);
		else
	if(uRenderMode==2) // mip mapping levels
	{
		float rho = 512.f*max(length(dFdx(vTexCoord)),length(dFdy(vTexCoord)));
		float level = floor( log(rho)/log(2.f));
		color = texture2D(uColorImage,vec2(vTexCoord.x,vTexCoord.y)) * 0.5+
				vec4(hsv2rgb(level/9.f)* 0.5,1.0);		
	}else
	if(uRenderMode==3) // bump mapping
	{
		float dx = (texture2D(uBumpmapImage,vTexCoord.xy+vec2(1.0/512.0,0.0)).x - texture2D(uBumpmapImage,vTexCoord.xy).x);
		float dy = (texture2D(uBumpmapImage,vTexCoord.xy+vec2(0.0,1.0/512.0)).x - texture2D(uBumpmapImage,vTexCoord.xy).x);
		vec3 dx3 = normalize(vec3(1.0,0.0,4.0*dx));
		vec3 dy3 = normalize(vec3(0.0,1.0,4.0*dy));

		vec3 N = normalize(cross(dx3,dy3));
		color =  vec4(diffuse(normalize(vLdirTS),N),1.0);
	}else
	if(uRenderMode==4) // normal mapping
	{
		vec3 N =  texture2D(uNormalmapImage,vTexCoord.xy).xyz ;
		N = normalize(N*2.0-1.0);
		color =  vec4(vec3(diffuse(normalize(vLdirTS),N)),1.0);
	}
	else
	if(uRenderMode == 5){// parallax mapping
	 	float h =  4.0*  (texture2D(uBumpmapImage,vTexCoord).x) ;
      	vec3 V = normalize(vVdirTS);
      	vec3 intVh = vec3(vTexCoord,0.0) + ( h/V.z) *V/512.0 ;
      	vec2 uprime = intVh.st;
      	vec3 N = texture2D(uNormalmapImage,uprime).xyz;
		N = normalize(N*2.0-1.0);
      	color = vec4(diffuse(normalize(vLdirTS),N),1.0);
	}else
		color = vec4(1.0,.0,0.0,1.0);
} 