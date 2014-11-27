#version 150

out vec4 FragColor;
in vec2 textureCoords;

uniform sampler2D texture0;

uniform mat3 colourFilter = mat3(-0.7, 2.0, -0.3,
	-0.7, 2.0, -0.3,
	-0.7, 2.0, -0.3);

void main()
{
	vec3 colour = texture(texture0, textureCoords).xyz;
	colour = colour*colourFilter;
	FragColor = vec4(colour, 1.0);
}