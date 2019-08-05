#version 150 core

in vec4 pass_Color;
in vec3 pass_Uvs;

out vec4 out_Color;

uniform sampler2D	pigTexture;
uniform sampler2D	testTexture;
uniform vec4		mColor;
uniform int			mTextureIndex;

void main(void)
{
	vec2 uvs = vec2(pass_Uvs.x, pass_Uvs.y);

	// Fetch both colors
	vec3 color = vec3(1.0,1.0,1.0);
	if(mTextureIndex == 0)
	{
		color = texture(pigTexture, uvs).rgb;
	}
	else
	{
		color = texture(testTexture, uvs).rgb;
	}

	// Set output color
	out_Color = vec4(color, 1.0) * mColor;
}