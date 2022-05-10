#version 450

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

uniform UBO
{
	vec2 textureSize;	// The size of 'colorTexture', used to pre-calculate sampling coordinates in vertex shader
	vec2 direction;		// The sampling direction
} ubo;

in vec3 in_Position;
in vec3 in_UV0;

out vec2 pass_UVs[3];

// Apply horizontally and vertically for a 5x5 kernel
void main()
{
	vec2 off1 = (vec2(1.3333333333) * ubo.direction) / ubo.textureSize;

	pass_UVs[0] = in_UV0.xy;
	pass_UVs[1] = in_UV0.xy + off1;
	pass_UVs[2] = in_UV0.xy - off1;

    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);
}
