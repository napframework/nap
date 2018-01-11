#version 330 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform float blendValue;
uniform float normalBlendValue;

// Input Vertex Attributes
in vec3	in_Position;
in vec3 in_UV0;
in vec3 in_Normal;
in vec3 in_OriginalNormal;
in vec3 in_OriginalPosition;

// Output to fragment shader
out vec3 passUVs;					//< vetex uv's
out vec3 passNormal;				//< vertex normal in world space
out vec3 passPosition;				//< vertex world space position

void main(void)
{
	// Blend original normal with target normal
	vec3 blend_nor = mix(in_OriginalNormal, in_Normal, normalBlendValue);

	//rotate normal based on model matrix and set
    mat3 normal_matrix = transpose(inverse(mat3(modelMatrix)));
	passNormal = normalize(normal_matrix * blend_nor);

	// Blend original position with target position
	vec3 blend_pos = mix(in_OriginalPosition, in_Position, blendValue);

	// Multiply with modelmatrix to position in world space and pass to frag
	passPosition = vec3(modelMatrix * vec4(blend_pos, 1));

	// Forward uvs to fragment shader
	passUVs = in_UV0;

	// Calculate frag position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(blend_pos, 1.0);
}