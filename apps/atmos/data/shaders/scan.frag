#version 330

// input
in vec3 passUVs0;	// The unwrapped normalized texture
in vec3 passUVs1;	// The polar unwrapped texture

// output
out vec4 out_Color;

// uniforms
uniform sampler2D colorTexture;
uniform float colorTexScale;

void main() 
{	
	// Get color from texture
	vec3 tex_color = texture(colorTexture, (passUVs0.xy * colorTexScale)).rgb;

	// Set to frag output color
	out_Color = vec4(tex_color,1.0);
}
