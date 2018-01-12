#version 330 core

// Vertex shader uniforms
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

// Input attributes
in vec3	in_Position;			// Vertex position
in vec4	in_Color0;				// Vertex color
in vec3 in_Normals;				// Vertex normal
in vec3	in_UV0;					// Vertex uv
in vec3 in_Tangent;				// Vertex tangent
in vec3 in_Bitangent;			// Vertex bi-tangent

// Output attributes to fragment shader
out vec3 pass_Uvs0;
out vec3 pass_Normals;
out mat4 pass_ModelMatrix;
out vec3 pass_Vert;
out vec3 pass_Tangent;
out vec3 pass_Bitangent;

void main(void)
{
	// Calculate position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

	// Pass all important attributes to frag shader
	pass_Uvs0 = in_UV0;
	pass_Normals = in_Normals;
	pass_ModelMatrix = modelMatrix;
	pass_Vert = in_Position;
	pass_Tangent = in_Tangent;
	pass_Bitangent = in_Bitangent;
}