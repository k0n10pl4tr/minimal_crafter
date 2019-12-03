#version 330 core

uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix; 

layout (location = 0)
in vec4 position;

layout (location = 1)
in vec2 texcoord;

out VS_OUT {
	vec2 texcoord;
} vs_out;

void
main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * position;
	vs_out.texcoord = texcoord;
}
