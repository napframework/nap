#version 450 core

layout(binding=0) uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

// Input Vertex Attributes
layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normals;

// Output to fragment shader
layout(location=0) out vec3 passNormals;				//< Vertex normal
layout(location=1) out mat4 passModelMatrix;			//< Matrix to transform vertex from object to world space
layout(location=5) out vec3 passVert;					//< Vertex position in object space 
layout(location=6) out vec3 passPosition;				//< vertex world space position

void main(void)
{
	// Pass along model matrix for light calculations
	passModelMatrix = mvp.modelMatrix;

	// Pass along normals for light calculations
	passNormals = in_Normals;

	// Pass along vertex position in object space
	passVert = in_Position;

	// calculate vertex world space position and set
	passPosition = vec3(mvp.modelMatrix * vec4(in_Position, 1));

	// Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);
}