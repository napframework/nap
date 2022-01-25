#version 450

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

uniform UBO
{
	vec2 textureSize;
	vec2 direction;
	vec2 scaleBias;
} ubo;

in vec3 in_Position;
in vec3 in_UV0;

out vec2 pass_UVs[3];

void main()
{
	vec2 off1 = vec2(1.3333333333) * ubo.direction * 1.0;
 	off1 /= ubo.textureSize;

	pass_UVs[0] = in_UV0.xy;
	pass_UVs[1] = in_UV0.xy + off1;
	pass_UVs[2] = in_UV0.xy - off1;

    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);
}
