#version 450 core

uniform nap
{
	uniform mat4 projectionMatrix;
	uniform mat4 viewMatrix;
	uniform mat4 modelMatrix;
} mvp;

// Input Vertex Attributes
in vec3	in_Position;
in vec3 in_UV0;
in vec3 in_Normal;

// Output to fragment shader
out vec3 passUVs;					//< vetex uv's
out vec3 passNormal;				//< vertex normal in world space
out vec3 passPosition;				//< vertex world space position

void main(void)
{
	// Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);

	//rotate normal based on model matrix and set
    mat3 normal_matrix = transpose(inverse(mat3(mvp.modelMatrix)));
	passNormal = normalize(normal_matrix * in_Normal);

	// calculate vertex world space position and set
	passPosition = vec3(mvp.modelMatrix * vec4(in_Position, 1));

	// Forward uvs to fragment shader
	passUVs = in_UV0;
}