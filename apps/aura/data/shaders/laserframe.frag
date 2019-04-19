#version 330

// input  
in vec3 pass_Uvs;

// output
out vec4 out_Color;

// uniforms
uniform sampler2D	mFrame;

void main() 
{
	vec3 frame_color = texture(mFrame, pass_Uvs.xy).rgb;
	out_Color =  vec4(frame_color.rgb, 1.0);
}
