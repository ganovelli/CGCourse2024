#version 430 core  
out vec4 color; 

in vec4 vCoordLS;
in vec3 vNormalWS;
in vec3 vVWS;
in vec3 vLWS;

uniform int uRenderMode;
uniform vec3 uDiffuseColor;

uniform sampler2D  uShadowMap;
uniform ivec2 uShadowMapSize;
uniform float uBias;

// this produce the Hue for v:0..1 (for debug purposes)
vec3 hsv2rgb(float  v)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(vec3(v,v,v) + K.xyz) * 6.0 - K.www);
    return   mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0),1.0);
}
float linstep(float low, float high, float v){
    return clamp((v-low)/(high-low), 0.0, 1.0);
}
 	
void main(void) 
{	
	vec3 N = normalize(vNormalWS);
	vec3 L = normalize(vLWS);
	vec3 V = normalize(vVWS);

	vec3 diffuse_ligth = vec3(max(0.0,dot(L,N)))*uDiffuseColor;
	vec3 ambient_ligth = vec3(0.3,0.3,0.3);

	float lit = 1.0;

	if(uRenderMode==0) // just diffuse gray
		{
			color = vec4(diffuse_ligth+ambient_ligth,1.0);
		}
	else
	if(uRenderMode==1) // basic shadow
	{
		vec4 pLS = (vCoordLS/vCoordLS.w)*0.5+0.5;
		float depth = texture(uShadowMap,pLS.xy).x;
		if(depth < pLS.z)
			lit = 0.0;
		color = vec4(diffuse_ligth*lit+ambient_ligth,1.0);
	}
	else
	if(uRenderMode==2) // bias
	{
		vec4 pLS = (vCoordLS/vCoordLS.w)*0.5+0.5;
		float depth = texture(uShadowMap,pLS.xy).x;
		if(depth + uBias < pLS.z )
			lit = 0.0;
		color = vec4(diffuse_ligth*lit+ambient_ligth,1.0);
	}else
	if(uRenderMode==3) // slope bias
	{
		float bias = clamp(uBias*tan(acos(dot(N,L))),uBias,0.05); 
		vec4 pLS = (vCoordLS/vCoordLS.w)*0.5+0.5;
		float depth = texture(uShadowMap,pLS.xy).x;
		if(depth + bias < pLS.z )
			lit = 0.0;
		color = vec4(diffuse_ligth*lit+ambient_ligth,1.0);
	}else
	if(uRenderMode==4) // backfaces for watertight objects
	{
		vec4 pLS = (vCoordLS/vCoordLS.w)*0.5+0.5;
		float depth = texture(uShadowMap,pLS.xy).x;
		if(depth  < pLS.z  || dot(N,L)< 0.f )
			lit = 0.0;
		color = vec4(diffuse_ligth*lit+ambient_ligth,1.0);
	}else
	if(uRenderMode==5) // Percentage Closest Filtering
	{
	float storedDepth;
	vec4 pLS = (vCoordLS/vCoordLS.w)*0.5+0.5;
	for( float  x = 0.0; x < 5.0;x+=1.0)
		for( float y = 0.0; y < 5.0;y+=1.0)
			{
				storedDepth =  texture(uShadowMap,pLS.xy+vec2(-2.0+x,-2.0+y)/uShadowMapSize).x;
				if(storedDepth + uBias < pLS.z )    
					lit  -= 1.0/25.0;
			}
		color = vec4(diffuse_ligth*lit+ambient_ligth,1.0);
	}else
	if(uRenderMode==6) // VARIANCE SHADOW MAPPING
		{
			vec4 pLS = (vCoordLS/vCoordLS.w)*0.5+0.5;
			vec2 m = texture( uShadowMap, pLS.xy).xy;
 			float mu = m.x;
			/* do not trust 0 variance */
			float sigma = max(m.y-mu*mu,0.001);
			float diff = pLS.z - mu; 
			if(diff > 0.0){
			 lit = sigma / (sigma+diff*diff);

			 /* remap the formula between 0.001 and 1.0*/
			 lit = linstep(0.001, 1.0, sigma / (sigma + diff*diff));
			}
		color = vec4(diffuse_ligth*lit+ambient_ligth,1.0);
		}
} 