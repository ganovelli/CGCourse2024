#version 430 core  
out vec4 color; 

in vec2 vTexCoord;

uniform sampler2D uDepthMap;
uniform float uRadius;
uniform float uDepthScale;
uniform vec2 uSize;
uniform vec2 uRND;
uniform vec3 uSamples[64];

/* currently not used: try it */
uniform sampler2D uNoise;

bool test_sample(vec3 s){
	float z = texture2D(uDepthMap,s.xy).x;
	return   z > s.z;
}

void main(void) 
{ 
	int n_samples = 64;
	float ao = 0.0;

	/* currently not used: try it */
	vec3 randomVec = texture(uNoise, vTexCoord * vec2(800.f) ).xyz; 
				
	vec3 center = vec3(vTexCoord,texture2D(uDepthMap,vTexCoord).x);
 	if(center.z>0.99 ){
 	ao=1.0;}
 	else{ 
		vec3 s;
		for(int i=0; i < n_samples; ++i)
			{
				s = center + vec3(uSamples[i].x*uRadius/uSize.x,uSamples[i].y*uRadius/uSize.x,(uSamples[i].z)*uDepthScale);
  				if( test_sample(s))
  					ao+=1.0 / float(n_samples);
			}	
		ao =clamp(2.0*ao,0.0,1.0);
	}
 	
	color = vec4(vec3(ao) ,1.0);
}