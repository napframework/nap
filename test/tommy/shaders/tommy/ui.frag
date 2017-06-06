#version 150 core

in vec3 pass_Uvs;
out vec4 out_Color;

uniform sampler2D	mTexture;

void main(void)
{
	vec2 uvs = vec2(pass_Uvs.x, pass_Uvs.y);
	out_Color = texture(mTexture, uvs);
}