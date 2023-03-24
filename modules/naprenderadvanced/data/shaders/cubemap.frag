#version 450 core

// Uniforms
uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraPosition;
} mvp;

// Fragment Input
in vec3 	passPosition;			//< Fragment position in world space
in vec3 	passUV0;				//< Texture UVs

// Fragment Output
out vec4 out_Color;

// Texture Sampler
uniform sampler2D equiTexture;


void main()
{
	out_Color = texture(equiTexture, passUV0.xy);
}
