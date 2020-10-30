#version 450 core

in vec3 pass_Uvs;

out vec4 out_Color;

uniform sampler2D	pigTexture;
//layout(binding=1) uniform sampler2D	testTexture;

struct Data
{
	vec4		mColor;
	int			mTextureIndex;
};

uniform UBO
{
	Data mData[2];
}pig;

void main(void)
{
	vec2 uvs = vec2(pass_Uvs.x, pass_Uvs.y);
	vec3 color = texture(pigTexture, uvs).rgb;

	// Set output color
	out_Color = vec4(color, 1.0) * pig.mData[0].mColor * pig.mData[1].mColor;
}