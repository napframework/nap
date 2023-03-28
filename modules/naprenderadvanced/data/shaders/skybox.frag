#version 450 core

// Fragment Input
in vec3 passPosition;				//< Fragment position in world space

// Fragment Output
out vec4 out_Color;

// Texture Sampler
uniform samplerCube cubeTexture;


void main()
{
	out_Color = texture(cubeTexture, normalize(passPosition));
}
