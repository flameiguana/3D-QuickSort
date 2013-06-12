#version 400

in vec2 UV;
in vec4 color;
out vec4 fColor;

uniform sampler2D textureSampler;
void main()
{
	//Combine texture and lighting colors multiplicatively
    fColor = vec4(texture(textureSampler, UV).rgb * color.rgb, 1.0);
}
