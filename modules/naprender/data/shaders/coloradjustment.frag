// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform UBO
{
	float contrast;		// Contrast adjustment [-1.0, 1.0]
	float brightness;	// Brightness adjustment [-x, x]
	float saturation;	// Saturation adjustment [0.0, x]
} ubo;

uniform sampler2D colorTexture;

in vec3 pass_UV;
out vec4 out_Color;

const float PI 				= 3.141592;
const float PIVOT			= 0.5;

// @param color: input color
// @param value: [-1, 1] where negative reduces contrast and positive increases it
vec3 contrast(vec3 color, float value) 
{	
	float s_curve = tan((value) * PI * 0.25) + 1.0;
	return clamp(s_curve * (color - PIVOT) + PIVOT, 0.0, 1.0);
}

// @param color: input color
// @param value: [-1, 1] where negative reduces brightness and positive increases it
vec3 brightness(vec3 color, float value) 
{
	return clamp(color + value, 0.0, 1.0);
} 

// @param color: input color
// @param value: [0, x] increases saturation
vec3 saturation(vec3 color, float value)
{
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    vec3 luminance = vec3(dot(color, W));
    return mix(luminance, color, value);
}

void main(void)
{
	vec4 color = texture(colorTexture, pass_UV.xy);

	// Apply adjustments to RGB
	vec3 color_mod = brightness(color.rgb, ubo.brightness);
	color_mod = contrast(color_mod, ubo.contrast);
	color_mod = saturation(color_mod, ubo.saturation);

	out_Color = vec4(color_mod, color.a);
}
