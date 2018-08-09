#version 330

// input  
in float passTip;	// If the fragment is a tip or not
in vec3 passUVs0;	// The unwrapped normalized texture
in vec3 passUVs1;	// The polar unwrapped texture

// output
out vec4 out_Color;

// Uniform used for setting color
uniform vec3 color;				//< Normal color
uniform float opacity;			//< Opacity
uniform float length;			//< Normal length

// uniforms
uniform sampler2D colorTexture;
uniform float colorTexScale;

void main() 
{
	// Get color from texture
	vec3 tex_color = texture(colorTexture, (passUVs0.xy * colorTexScale)).rgb;

	float v = 1.0-clamp(passTip, 0.0, 1.0);
	float m = length;
	v =  pow(1.0 - (v / (m) * 1.0),0.5);
	out_Color = vec4(tex_color, opacity * v);
}
