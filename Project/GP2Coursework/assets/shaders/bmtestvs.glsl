#version 150

// Incoming per-vertex attribute values
in vec4 VertexPosition;
in vec3 VertexNormal;
in vec4 VertexTexture; 

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;
uniform vec3 LightPosition;

// Outgoing normal and light direction to fragment shader
smooth out vec3 VaryingNormal;
smooth out vec3 VaryingLightDir;
smooth out vec2 TexCoords;

void  main(void)
{
	// Get surface normal in eye coordinates and pass them through to the fragment shader
	VaryingNormal = normalMatrix * VertexNormal;

	// Get vertex position in eye coordinates, and convert to vector 3
	vec4 v4Position = mvMatrix * VertexPosition;
	vec3 v3Position = v4Position.xyz / v4Position.w;

	// Get vector to light source
	VaryingLightDir = normalize(LightPosition - v3Position);

	// Pass the texture coordinates through the vertex shader so they get smoothly interpolated
	TexCoords = VertexTexture.st;

	// Transform the geometry through the modelview-projection matrix
	gl_Position = mvpMatrix * VertexPosition;
}