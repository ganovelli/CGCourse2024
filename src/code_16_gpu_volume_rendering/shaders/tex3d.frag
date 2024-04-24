#version 460 core
out vec4 FragColor;

in vec3 TexCoords;
out vec4 color;
uniform sampler3D uVolume;

vec3 hsv2rgb(vec3 c)
{
        vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
        vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
        return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
vec4 transfer_function(float v){
        return vec4(hsv2rgb(vec3(v,v,v)),v*v*v);
}

void main()
{
    vec4 texCol = texture(uVolume, TexCoords*0.5+0.5);
    color = vec4(vec3(texCol.x), 1.0);
        color  = transfer_function(texCol.x);
}
