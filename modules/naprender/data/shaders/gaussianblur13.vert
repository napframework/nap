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

out vec2 pass_UVs[7];

// Apply horizontally and vertically for a 9x9 kernel
void main()
{
	vec2 off1 = (vec2(1.411764705882353) * ubo.direction) / ubo.textureSize;
	vec2 off2 = (vec2(3.2941176470588234) * ubo.direction) / ubo.textureSize;
	vec2 off3 = (vec2(5.176470588235294) * ubo.direction) / ubo.textureSize;

	pass_UVs[0] = in_UV0.xy;
	pass_UVs[1] = in_UV0.xy + off1;
	pass_UVs[2] = in_UV0.xy - off1;
	pass_UVs[3] = in_UV0.xy + off2;
	pass_UVs[4] = in_UV0.xy - off2;
	pass_UVs[5] = in_UV0.xy + off3;
	pass_UVs[6] = in_UV0.xy - off3;

    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);
}
