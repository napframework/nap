#version 150 core

in vec3 pass_Uvs;
out vec4 out_Color;
uniform vec4 inColor;

uniform sampler2D	mTexture;

void main(void)
{
	vec2 uvs = vec2(pass_Uvs.x, pass_Uvs.y);
	uvs.y = 1.0 - uvs.y;
	out_Color = texture(mTexture, uvs) * inColor;
}