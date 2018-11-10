#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's

// uniform inputs
uniform sampler2D glyph;				//< Glyph Texture
uniform vec3 textColor;

// output
out vec4 out_Color;

void main() 
{
	vec2 uv = vec2(passUVs.x, 1.0-passUVs.y);
	float alpha = texture(glyph, uv).r;
    out_Color = vec4(textColor, alpha);
}
