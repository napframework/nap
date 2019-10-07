#version 450 core

layout(location=0) in vec4 pass_Color;
layout(location=1) in vec3 pass_Uvs;

layout(location=0) out vec4 out_Color;

layout(binding=0) uniform sampler2D	pigTexture;
layout(binding=1) uniform sampler2D	testTexture;

layout(binding=2) uniform UniformBufferObject
{
	uniform vec4		mColor;
	uniform int			mTextureIndex;
}matBuf;

void main(void)
{
	vec2 uvs = vec2(pass_Uvs.x, pass_Uvs.y);

	// Fetch both colors
	vec3 color = vec3(1.0,1.0,1.0);
	if(matBuf.mTextureIndex == 0)
	{
		color = texture(pigTexture, uvs).rgb;
	}
	else
	{
		color = texture(testTexture, uvs).rgb;
	}

	// Set output color
	out_Color = vec4(color, 1.0) * matBuf.mColor;
}