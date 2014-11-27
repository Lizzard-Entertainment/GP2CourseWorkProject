#version 150

out vec4 FragColor;
in vec2 textureCoords;

uniform sampler2D texture0;

vec2 boxFilter[4] = vec2[4]( vec2(0.0001f, 0.001f), vec2(-0.001f, 0.001f),vec2(0.001f,-0.001f),vec2(-0.001f,-0.001f) );

void main()
{
	vec3 colour = vec3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 4; i++)
	{
		colour += texture(texture0, textureCoords + boxFilter[i]).xyz;
	}
	FragColor = vec4(colour, 1.0);
}