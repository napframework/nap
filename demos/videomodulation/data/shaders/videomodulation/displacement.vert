// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

// Input Vertex Attributes
in vec3	in_Position;
in vec3 in_UV0;
in vec3 in_Normal;
in vec3 in_CenterUV;
in vec3 in_DisplacementDirection;
in vec4 in_Color;

// Output to fragment shader
out vec3 passUVs;					//< vetex uv's
out vec3 passNormal;				//< Vertex normal object space
out vec3 passVert;					//< Vertex position world space
out mat4 passModelMatrix;			//< Model matrix
out vec4 passColor;					//< Vertex color

// uniform inputs
uniform DisplacementUBO
{
	float value;
	float randomness;
} displacement;

uniform sampler2D	videoTextureVert;

float fit(float value, float inMin, float inMax, float outMin, float outMax)
{
	float v = clamp(value, inMin, inMax);
	float m = inMax - inMin;
	if(m == 0.0)
		m = 0.00001;
	return (v - inMin) / (m) * (outMax - outMin) + outMin;
}

void main(void)
{
	// Get texture rgba value
	vec3 tex_color = texture(videoTextureVert, in_CenterUV.xy).rgb;

	// Get displacement value
	float tex_greyscale = (tex_color.r + tex_color.g + tex_color.b) / 3.0;
	tex_greyscale = fit(tex_greyscale, 0.1,1.0,0.0,1.0);

	// Compute vertex displacement value
	float ver_greyscale = (((in_Color.r + in_Color.g + in_Color.b) / 3.0) * 2.0) - 1;
	float temp_greyscale = tex_greyscale + (ver_greyscale * displacement.randomness);
	float displacement_value = mix(tex_greyscale, temp_greyscale, tex_greyscale);

	// Modify position
	vec3 new_pos = in_Position + (in_DisplacementDirection * displacement_value * displacement.value);
	
	// Forward uvs to fragment shader
	passUVs = in_UV0;

	// pass object space normal
    passNormal = in_Normal;

    // calculate vert in world coordinates
    passVert = vec3(mvp.modelMatrix * vec4(new_pos, 1));

    // Pass model matrix to frag shader
    passModelMatrix = mvp.modelMatrix;

    // Pass along color
    passColor = in_Color;

    // Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(new_pos, 1.0);
}