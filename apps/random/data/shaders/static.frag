#version 330

// uniforms
uniform float uTemperature;
uniform float uIntensity;

// output
out vec4 out_Color;

void main()
{
  // set fragment color
	out_Color =  vec4(uTemperature * uIntensity, (1.0 - uTemperature) * uIntensity, 0.0, 1.0);
}
