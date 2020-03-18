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
uniform Blob blobs[10];					//< All detected blobs
uniform int blobCount;					//< Total number of detected blobs
uniform sampler2D captureTexture;		//< Classify texture 

// output
out vec4 out_Color;

void main() 
{
	vec3 color = texture(captureTexture, passUVs.xy).rgb;
    out_Color = vec4(color, 1.0);
}
