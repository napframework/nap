#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passPosition;					//< frag world space position

// uniforms
uniform float uOuterSize;
uniform float uInnerSize;

// output
out vec4 out_Color;

void main()
{
	vec2 center = vec2(0.5, 0.5);
	float distance = distance(passUVs.xy, center);
	float innerSize = uInnerSize * uOuterSize;
	float intensity = 1.0 - clamp((distance - innerSize) / (uOuterSize - innerSize), 0.0, 1.0);
	out_Color =  vec4(0.0, intensity, 0.0, 1.0);
}
