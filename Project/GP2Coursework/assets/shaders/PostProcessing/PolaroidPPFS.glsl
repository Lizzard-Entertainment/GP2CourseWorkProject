#version 150

out vec4 FragColor;
in vec2 textureCoords;

uniform sampler2D texture0;

uniform mat4 colourFilter = mat4(1.438, -0.062, -0.062, 0,
	0.122, 1.378, -0.122, 0,
	-0.016, -0.016, 1.483, 0, 
	-0.03, 0.05, -0.02, 1);

void main()
{
	vec4 colour = texture(texture0, textureCoords);
	colour = colour*colourFilter;
	FragColor = vec4(colour);
}