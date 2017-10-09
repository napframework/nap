#version 150 core

in vec3 pass_Uvs;
in vec4 pass_Color;

out vec4 out_Color;

// Video Texture
uniform sampler2D	mVideoTexture;

void main(void)
{
	// Get video color
	vec2 uvs = vec2(pass_Uvs.x, pass_Uvs.y);
	vec3 color = texture(mVideoTexture, uvs).rgb;

	// Set as output color
	out_Color = vec4(color, 1.0);
}