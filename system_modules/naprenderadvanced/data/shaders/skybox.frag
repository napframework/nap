#version 450 core

// Uniforms
uniform UBO
{
	vec3 color;
} ubo;

// Fragment Input
in vec3 passPosition;				//< Fragment position in world space

// Fragment Output
out vec4 out_Color;

// Texture Sampler
uniform samplerCube cubeTexture;

void main()
{
	vec4 cube = texture(cubeTexture, normalize(passPosition));
	out_Color = vec4(cube.rgb * ubo.color, cube.a);
}
