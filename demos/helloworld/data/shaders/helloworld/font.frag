#version 450 core

// vertex shader input  
in vec3 passUVs;						//< frag Uv's

// uniform inputs
uniform sampler2D glyph;				//< Glyph Texture

uniform UBO
{
	uniform vec3 textColor;
} ubo;

// output
out vec4 out_Color;

void main() 
{
	float alpha = texture(glyph, passUVs.xy).r;
    out_Color = vec4(ubo.textColor, alpha);
}
