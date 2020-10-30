// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

// vertex shader input  
in vec3 passUVs;						//< frag Uv's

// Blob detection structure
struct Blob
{
	vec2	mCenter;
	float	mSize;
};


// All uniform inputs
uniform UBO
{
	// uniform inputs
	uniform Blob blobs[20];					//< All detected ubo.blobs
	uniform int blobCount;					//< Total number of detected ubo.blobs
	uniform vec2 captureSize;				//< Size of captureTexture in pixels
} ubo;

uniform sampler2D captureTexture;			//< Classify texture 

const float ringSize = 3;
const vec3 edgeColor = vec3(1.0, 1.0, 1.0);
const vec3 innerColor = vec3(0.545,0.549,0.627);
	
// output
out vec4 out_Color;


// maps value from min / max to output
float fit(float value, float min, float max, float outMin, float outMax)
{
    float v = clamp(value, min, max);
    float m = max - min;
    return (v - min) / (m) * (outMax - outMin) + outMin;
}


// Applies bell curve over 0-1 value
float bell(float t, float strength)
{
	return pow(4.0f, strength) * pow(t *(1.0f - t), strength);
}


// Computes the distance to a certain blob
float getDistance(int blobID)
{
	// Flip blob vertical
	vec2 center = vec2(ubo.blobs[blobID].mCenter.x, ubo.captureSize.y - ubo.blobs[blobID].mCenter.y); 
	
	// Return distance from pixel to center of blob
	return length(center - (passUVs.xy * ubo.captureSize));
}


void main() 
{
	// Iterate over every blob, for every blob compute the distance to current uv coordinate.
	// Store the blob with the lowest distance to current sample. 
	float current_dist = 1000000.0;
	int closest_blob = -1;

	for(int i=0; i < ubo.blobCount; i++)
	{
		float blob_dist = getDistance(i);
		if(blob_dist < current_dist)
		{
			closest_blob = i;
			current_dist = blob_dist;
		}
	}

	// For the closest blob, compute lerp values, 1.0 = inside, 0.0 = outside
	float edge_lerp_v = 0.0;
	float inne_lerp_v = 0.0;

	if (closest_blob >= 0)
	{
		// Get blob size and create gradient on edge
		float blob_size = ubo.blobs[closest_blob].mSize;
		edge_lerp_v = fit(current_dist, blob_size-ringSize , blob_size+ringSize, 0.0, 1.0);
		edge_lerp_v = bell(edge_lerp_v, 0.33);

		// Compute inner circle
		inne_lerp_v = fit(current_dist, blob_size-ringSize, blob_size, 1.0, 0.0);
	}

	// Mix based on interpolation values
	vec3 tex_color = texture(captureTexture, vec2(passUVs.x, 1.0-passUVs.y)).bgr;
	vec3 mix_color = mix(tex_color, innerColor, inne_lerp_v * 0.5);
	mix_color = mix(mix_color, edgeColor, edge_lerp_v * 1.0);

    //out_Color = vec4(lerp_v, lerp_v, lerp_v, 1.0);
    out_Color = vec4(mix_color, 1.0);
}
