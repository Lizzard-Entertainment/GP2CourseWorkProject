#version 150

out vec4 FragColor;
in vec2 textureCoords;

uniform sampler2D texture0;

uniform mat4 colourFilter = mat4(1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1);

void main()
{
	vec4 colour = texture(texture0, textureCoords);
	colour = colour * colourFilter;
	FragColor = vec4(colour);
}