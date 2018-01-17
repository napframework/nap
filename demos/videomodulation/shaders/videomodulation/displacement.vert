#version 330 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

// Input Vertex Attributes
in vec3	in_Position;
in vec3 in_UV0;
in vec3 in_Normal;
in vec3 in_DisplacementDirection;

// Output to fragment shader
out vec3 passUVs;					//< vetex uv's
out vec3 passNormal;				//< Vertex normal world space
out vec3 passVert;					//< Vertex position world space

// uniform inputs
uniform sampler2D	videoTexture;

void main(void)
{
	// Get texture rgba value
	vec3 tex_color = texture(videoTexture, in_UV0.xy).rgb;

	// Get displacement value
	float greyscale = (tex_color.r + tex_color.g + tex_color.b) / 3.0;

	// Modify position
	vec3 new_pos = in_Position + (in_DisplacementDirection * max(pow(greyscale,50) * 0.5, 0.05));

	// Forward uvs to fragment shader
	passUVs = in_UV0;

	//calculate normal in world coordinates
    mat3 normal_matrix = transpose(inverse(mat3(modelMatrix)));
    passNormal = normalize(normal_matrix * in_Normal);

    // calculate vert in world coordinates
    passVert = vec3(modelMatrix * vec4(new_pos, 1));

    // Calculate frag position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(new_pos, 1.0);
}