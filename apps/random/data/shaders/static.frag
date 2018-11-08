#version 330

// uniforms
uniform float uWarmth;

// output
out vec4 out_Color;

void main()
{
  // set fragment color
	out_Color =  vec4(uWarmth, 1.0 - uWarmth, 0.0, 1.0);
}
