#version 330 core
uniform sampler2D image;

layout(location = 0) 
out vec4 o_Color;

in VS_OUT {
	vec2 texcoord;
} vs_out;

void
main()
{
	o_Color = texture(image, vs_out.texcoord);
}
