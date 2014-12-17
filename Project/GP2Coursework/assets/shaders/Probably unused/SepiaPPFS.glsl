#version 150

out vec4 FragColor;
in vec2 textureCoords;

uniform sampler2D texture0;

uniform mat4 colourFilter = mat4(0.393, 0.769, 0.189, 0,
	0.349, 0.686, 0.168, 0,
	0.272, 0.534, 0.131, 0,
	0, 0, 0, 1);

void main()
{
	vec4 colour = texture(texture0, textureCoords);
	colour = colour*colourFilter;
	FragColor = vec4(colour);
}