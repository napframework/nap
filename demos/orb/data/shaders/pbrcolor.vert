#version 450 core

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

uniform VERTUBO
{
	mat4 lightSpaceMatrix;
	vec3 lightPosition;
} ubo;

// Input Vertex Attributes
in vec3 in_Position;
in vec3 in_Normals;
in vec3 in_UV0;
in vec4 in_Color0;

out vec3 passPosition;
out vec3 passNormal;
out vec3 passUV;
out vec4 passColor;
out vec4 passShadowCoord;

void main()
{
	// Normal
	mat3 normal_matrix = transpose(inverse(mat3(mvp.modelMatrix)));
	vec3 normal = normalize(normal_matrix * in_Normals);
	passNormal = normal;

	// Position
	vec4 world_position = mvp.modelMatrix * vec4(in_Position, 1.0);
	vec4 view_position = mvp.viewMatrix * world_position;

	passPosition = world_position.xyz;
	gl_Position = mvp.projectionMatrix * view_position;

	// UV
	passUV = in_UV0;

	// Color
	passColor = in_Color0;

	// Shadow
	passShadowCoord = ubo.lightSpaceMatrix * world_position;
}
