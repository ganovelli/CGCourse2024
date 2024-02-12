#version 460 core 
in vec2 aPosition;
in vec3 aColor;
out vec3 vColor;
uniform vec2 uDelta;
void main(void)
{
gl_Position = vec4(aPosition+uDelta, 0.0, 1.0);
vColor = aColor;
}