// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// All mvp uniform variables
uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraPosition;
} mvp;

// All non mvp uniform variables
uniform UBOVert
{
	vec3 	inBlobPosition;			//< Blob position in uv space
	float 	inTime;					//< Modulation time
	float 	inVelocity;				//< Velocity used for modulating frequency
} ubovert;

// Input Vertex Attributes
in vec3	in_Position;
in vec3 in_UV0;
in vec3 in_Normal;

// Output to fragment shader
out vec3 passUVs;					//< vetex uv's
out vec3 passNormal;				//< vertex normal in object space
out vec3 passPosition;				//< vertex position in object space
out vec2 passColor;

const float	displacement = 0.033;
const float	minDistance = 0.4;
const float	maxDistance = 0.1;
const float	speed = 0.005;
const float	fade = 0.75;
const float	minFrequency = 499.8;
const float	maxFrequency = 500;
const float	minDistribution = 2.25;
const float	maxDistribution = 4.0;

// Maps a value to a new range
float fit(float value, float inMin, float inMax, float outMin, float outMax, bool doClamp)
{
	float v = value;
	if(doClamp)
		v = clamp(v, inMin, inMax);

	float m = inMax - inMin;

	if(m == 0.0)
		m = 0.00001;
	return (v - inMin) / (m) * (outMax - outMin) + outMin;
}


// Calculates blob displacement based on the blob position and movement speed
// This is just a fancy sine mnodulation function
float calculateDisplacement(vec2 uv)
{
	// Distance in uv space from click to frag
	float uv_dist = distance(ubovert.inBlobPosition.xy, uv);
	float currentDistance = mix(minDistance, maxDistance, pow(ubovert.inVelocity,0.75f));

	// Get mapped normalized value
	float uv_dist_norm = fit(uv_dist, 0.0, currentDistance, 0.0, 1.0, false);
	float distribution = mix(minDistribution, maxDistribution, ubovert.inVelocity);
	uv_dist_norm = pow(uv_dist_norm, distribution);

	// Fit distribution based on distance
	float max_dist_weight = 1.0 / distribution;
	uv_dist_norm = fit(uv_dist_norm, 0.0, 1.0, 0.125, max_dist_weight, false);

	// Multiply distance with weighted freq scale over distance
	float weighted_dist = uv_dist * uv_dist_norm;

	// Apply phase
	weighted_dist += ((ubovert.inTime * speed) * -1.0);

	// Apply freq
	weighted_dist *= mix(minFrequency, maxFrequency, ubovert.inVelocity);
	
	// Get sin over distance
	float displacement_v = (sin(weighted_dist) + 1.0) / 2.0;

	// Get fade distance
	float min_fade = fade * currentDistance;
	min_fade = clamp(currentDistance - min_fade, 0, 1);
	float fade_mult = pow(fit(uv_dist, min_fade, currentDistance, 1, 0, true),1.33);

	// Multiply over displacement
	displacement_v *= fade_mult;

	return clamp(displacement_v,0,1);
}


// Use the blob function to calculate displacement
// The same function is used in the fragment shader to calculate the normals for lighting
// It's important that the const values are similar for both the frag and vertex shader 
void main(void)
{
	float sin_v = calculateDisplacement(in_UV0.xy);
	vec3 displ_pos = in_Position + (in_Normal * sin_v * displacement);

	// Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(displ_pos, 1.0);

    // Pass normal in object space
	passNormal = in_Normal;

	// Pass position in object space
	passPosition = displ_pos;

	// Forward uvs to fragment shader
	passUVs = in_UV0;
}