#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's

// Blob detection structure
struct Blob
{
	vec2	mCenter;
	float	mSize;
};

// uniform inputs
uniform Blob blobs[20];					//< All detected blobs
uniform int blobCount;					//< Total number of detected blobs
uniform sampler2D captureTexture;		//< Classify texture 

// output
out vec4 out_Color;


// maps value from min / max to output
float fit(float value, float min, float max, float outMin, float outMax)
{
    float v = clamp(value, min, max);
    float m = max - min;
    if(m==0.0)
        m = 0.00000001;
    return (v - min) / (m) * (outMax - outMin) + outMin;
}


// Computes the distance to a certain blob
float getDistance(int blobID)
{
	return length(blobs[blobID].mCenter - passUVs.xy);
}


void main() 
{
	// Iterate over every blob, for every blob compute the distance to current uv coordinate.
	// Store the blob with the lowest distance to current sample. 
	float current_dist = 100.0;
	int closest_blob = -1;

	for(int i=0; i < blobCount; i++)
	{
		float blob_dist = getDistance(i);
		if(blob_dist < current_dist)
		{
			closest_blob = i;
			current_dist = blob_dist;
		}
	}

	// For the closest blob, compute lerp value, where 0.0 is outer bound
	float lerp_v = 0.0;
	if (closest_blob >= 0)
	{
		lerp_v = fit(current_dist, 0.0, blobs[closest_blob].mSize, 1.0, 0.0);
	}

	vec3 tex_color = texture(captureTexture, passUVs.xy).bgr;
	vec3 mix_color = mix(tex_color, vec3(1.0,1.0,1.0), lerp_v);

    out_Color = vec4(mix_color, 1.0);
}
