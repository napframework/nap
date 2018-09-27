#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passPosition;					//< frag world space position

// output
out vec4 out_Color;

void main()
{
	float intensity = 1.0 - clamp(distance(passUVs.xy, vec2(0.5, 0.5)) / 0.4, 0.0, 1.0);
	out_Color =  vec4(vec3(0.0, 1.0, 0.0) * intensity, 1.0);
}
