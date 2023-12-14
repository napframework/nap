#version 450 core

// Uniforms
uniform UBO
{
	vec3 color;
	float alpha;
} ubo;

// Fragment Output
out vec4 out_Color;

void main()
{
	out_Color = vec4(ubo.color, ubo.alpha);
}