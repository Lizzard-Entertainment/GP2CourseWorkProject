#version 150

// Uniforms
uniform vec4 ambientColour;
uniform vec4 diffuseColour;
uniform vec4 specularColour;
uniform sampler2D diffuseMap; // This is the original texture
uniform sampler2D normalMap; // This is the normal-mapped version of our texture

// Input from our vertex shader
smooth in vec3 VaryingNormal;
smooth in vec3 VaryingLightDir;
smooth in vec2 TexCoords;

// Output fragments
out vec4 FragColour;

void main(void)
{
	const float maxVariance = 2.0; // Mess around with this value to increase/decrease normal perturbation
	const float minVariance = maxVariance / 2.0;

	// Create a normal which is our standard normal + the normal map perturbation (which is going to be either positive or negative)
	vec3 normalAdjusted = VaryingNormal + normalize(texture2D(normalMap, TexCoords.st).rgb * maxVariance - minVariance);

	// Calculate diffuse intensity
	float diffuseIntensity = max(0.0, dot(normalize(normalAdjusted), normalize(VaryingLightDir)));

	// Add the diffuse contribution blended with the standard texture lookup and add in the ambient light on top
	vec3 colour = (diffuseIntensity * diffuseColour.rgb) * texture2D(diffuseMap, TexCoords.st).rgb + ambientColour.rgb;

	// Set the almost final output color as a vec4 - only specular to go!
	FragColour = vec4(colour, 1.0);

	// Calc and apply specular contribution
	vec3 vReflection = normalize(reflect(-normalize(normalAdjusted), normalize(VaryingLightDir)));
	float specularIntensity = max(0.0, dot(normalize(normalAdjusted), vReflection));

	// If the diffuse light intensity is over a given value, then add the specular component
	// Only calc the pow function when the diffuseIntensity is high (adding specular for high diffuse intensities only runs faster)
	// Put this as 0 for accuracy, and something high like 0.98 for speed
	if (diffuseIntensity > 0.98)
	{
		float fSpec = pow(specularIntensity, 64.0);
		FragColour.rgb += vec3(fSpec * specularColour.rgb);
	}
}