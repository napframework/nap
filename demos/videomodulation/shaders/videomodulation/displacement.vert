#version 330 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

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
uniform sampler2D	videoTexture;

const float displacement = 0.3;
const float randomDisplacement = 0.05;

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
	vec3 tex_color = texture(videoTexture, in_CenterUV.xy).rgb;

	// Get displacement value
	float greyscale = (tex_color.r + tex_color.g + tex_color.b) / 3.0;
	greyscale = fit(greyscale, 0.1,1.0,0.0,1.0);

	// Get vertex displacement value
	float ver_greyscale = (((in_Color.r + in_Color.g + in_Color.b) / 3.0) * 2.0) - 1;
	float tgreyscale = greyscale + (ver_greyscale *randomDisplacement);

	float displacement_value = mix(greyscale, tgreyscale, greyscale);

	// Modify position
	vec3 new_pos = in_Position + (in_DisplacementDirection * displacement_value * displacement);
	
	// Forward uvs to fragment shader
	passUVs = in_UV0;

	// pass object space normal
    passNormal = in_Normal;

    // calculate vert in world coordinates
    passVert = vec3(modelMatrix * vec4(new_pos, 1));

    // Pass model matrix to frag shader
    passModelMatrix = modelMatrix;

    // Pass along color
    passColor = in_Color;

    // Calculate frag position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(new_pos, 1.0);
}