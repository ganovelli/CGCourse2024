#version 330 core  

out vec4 color;

uniform vec2 uBlur;	
uniform sampler2D uTexture;		
in vec2 vTexCoord;
		
void main(void)									
{									
	color = vec4(0.0);

	color += texture2D(uTexture, vTexCoord + (vec2(-3.0) * uBlur.xy)) * (1.0/64.0);
	color += texture2D(uTexture, vTexCoord + (vec2(-2.0) * uBlur.xy)) * (6.0/64.0);
	color += texture2D(uTexture, vTexCoord + (vec2(-1.0) * uBlur.xy)) * (15.0/64.0);
	color += texture2D(uTexture, vTexCoord + (vec2(0.0)  * uBlur.xy)) * (20.0/64.0);
	color += texture2D(uTexture, vTexCoord + (vec2(1.0)  * uBlur.xy)) * (15.0/64.0);
	color += texture2D(uTexture, vTexCoord + (vec2(2.0)  * uBlur.xy)) * (6.0/64.0);
	color += texture2D(uTexture, vTexCoord + (vec2(3.0)  * uBlur.xy)) * (1.0/64.0);
	color = color;
}