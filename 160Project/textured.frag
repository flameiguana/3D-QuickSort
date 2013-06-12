#version 400

in vec2 UV;
in vec4 color;
out vec4 fColor;

uniform sampler2D textureSampler;
void main()
{
	//for now only use texture colors.
    fColor = vec4(texture(textureSampler, UV).rgb, 1.0) * color;
}
