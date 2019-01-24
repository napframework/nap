#version 330

// uniforms
uniform float uTemperature;

// output
out vec4 out_Color;

void main()
{
  // set fragment color
	out_Color =  vec4(uTemperature, 1.0 - uTemperature, 0.0, 1.0);
}
