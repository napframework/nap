#version 450 core

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

// Input Vertex Attributes
in vec3 in_Position;
in vec3 in_Normals;

// Output to fragment shader
out vec3 passNormals;				//< Vertex normal
out mat4 passModelMatrix;			//< Matrix to transform vertex from object to world space
out vec3 passVert;					//< Vertex position in object space 
out vec3 passPosition;				//< vertex world space position

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