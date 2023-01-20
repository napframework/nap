#version 450 core

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

in vec3	in_Position;
in vec3 in_Normals;
in vec3 in_UV0;

// Output to fragment shader
out vec3 passPosition;				//< vertex position in object space
out vec3 passNormal;				//< vertex normal in object space
out vec3 passUV0;					//< UVs

void main()
{
	// Calculate frag position
	vec4 position = vec4(in_Position, 1.0);
	vec4 world_position = mvp.modelMatrix * position;
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * world_position;

	passPosition = world_position.xyz;
	passUV0 = in_UV0;

	// Rotate normal based on model matrix and set
    mat3 normal_matrix = transpose(inverse(mat3(mvp.modelMatrix)));
	vec3 world_normal = normalize(normal_matrix * in_Normals * scaleNormals);

	passNormal = world_normal;
}
