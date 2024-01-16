#version 450

// Extensions
#extension GL_GOOGLE_include_directive : enable

// Includes
#include "shadow.glslinc"

uniform UBO
{
	vec2 textureSize;	// The size of 'colorTexture', used to pre-calculate sampling coordinates in vertex shader
	vec2 direction;		// The sampling direction
	vec2 nearFar;		// camera near/far planes
	float aperture;
	float focusDistance;
	float focusPower;
} ubo;

uniform sampler2D colorTexture;		// The input color texture to sample from
uniform sampler2D depthTexture;		// The input color texture to sample from

in vec2 pass_UV;
out vec4 out_Color;

const float weight[] = { 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 };


// Sampler must be configured with filter VK_FILTER_LINEAR
vec4 blur(sampler2D tx, vec4 col, float s) 
{
	const vec2 offset[] = { 
		{ 0.0, 0.0 }, 
		ubo.direction * (1.0/ubo.textureSize), 
		ubo.direction * (2.0/ubo.textureSize),
		ubo.direction * (3.0/ubo.textureSize),
		ubo.direction * (4.0/ubo.textureSize)
	};

	col = col * weight[0];
	for (uint i = 1; i < offset.length(); i++)
	{
		col += texture(tx, pass_UV + offset[i] * s) * weight[i];
		col += texture(tx, pass_UV - offset[i] * s) * weight[i];
	}	
	return col;
}


void main() 
{
	// Adding this small constant to resolve sampling artifacts in cube seams
	float near = min(ubo.nearFar.x + 0.001, ubo.nearFar.y);
	float far = ubo.nearFar.y;

	vec4 frag_col = texture(colorTexture, pass_UV);
	float frag_depth = texture(depthTexture, pass_UV).x;

	// https://developer.nvidia.com/gpugems/gpugems/part-iv-image-processing/chapter-23-depth-field-survey-techniques
	const float aperture = ubo.aperture;
	const float focal = 0.5;
	const float focus = 1.0 + ubo.focusDistance*0.01;

	float coc_scale = (aperture * focal * focus * (far - near)) / ((focus - focal) * near * far);
	float coc_bias = (aperture * focal * (near - focus)) / ((focus * focal) * near);
	float coc = abs(frag_depth * coc_scale + coc_bias);

	out_Color = blur(colorTexture, frag_col, min(pow(coc, max(ubo.focusPower, 1.0)), 4.0));
}
 