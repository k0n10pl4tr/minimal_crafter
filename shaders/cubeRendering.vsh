#version 330 core

uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix; 

layout (location = 0)
in vec4 position;

void
main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * position;
}
