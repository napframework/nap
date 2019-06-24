#version 150 core

in vec4 pass_Color;
in vec3 pass_Uvs;

out vec4 out_Color;

uniform sampler2D	pigTexture;
uniform sampler2D	testTexture;
uniform vec4		mColor;
uniform int			mTextureIndex;

struct Point
{
	float x;
	float y;
};

struct Light
{
	Point		mPoint;
	float 		mTest[2];
	sampler2D 	mTestSampler[2];
};

uniform Light mLight[2];

void main(void)
{
	vec2 uvs = vec2(pass_Uvs.x + mLight[0].mPoint.x, pass_Uvs.y + mLight[0].mPoint.y);
	uvs.x -= mLight[1].mPoint.x;
	uvs.y -= mLight[1].mPoint.y;

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

	color.r += mLight[0].mTest[0];
	color.g += mLight[0].mTest[1];
	color *= texture(mLight[0].mTestSampler[0], uvs).rgb * texture(mLight[0].mTestSampler[1], uvs).rgb;

	color.r -= mLight[1].mTest[0];
	color.g -= mLight[1].mTest[1];
	color /= texture(mLight[1].mTestSampler[0], uvs).rgb * texture(mLight[1].mTestSampler[1], uvs).rgb;
		
	// Set output color
	out_Color = vec4(color, 1.0) * mColor;
}