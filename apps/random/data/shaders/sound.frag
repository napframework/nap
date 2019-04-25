#version 330

// uniforms
uniform float uTemperature;
uniform float uSourceWidth;
uniform vec3 uSourcePosition;

// input
in vec3 passUVs;

// output
out vec4 out_Color;

void main()
{
	float intensity = passUVs.x > uSourcePosition.x - uSourceWidth * 0.5 && passUVs.x < uSourcePosition.x + uSourceWidth * 0.5 ? 1.0 : 0.0;

	// set fragment color
	out_Color =  vec4(intensity * uTemperature, intensity * (1.0 - uTemperature), 0.0, 1.0);
}
