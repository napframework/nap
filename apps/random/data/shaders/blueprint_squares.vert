#version 330 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

// Input Vertex Attributes
in vec3	in_Position;		// Vertex Position
in vec3 in_Normals;			// Vertex Normals

// Output to fragment shader
out vec3 passPosition;				//< vertex world space position
out vec3 passNormals;				//< Vertex normal
out mat4 passModelMatrix;			//< Matrix to transform vertex from object to world space
out vec3 passVert;					//< Vertex position in object space 

void main(void)
{
	// Calculate frag position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

	// calculate vertex world space position and set
	passPosition = vec3(modelMatrix * vec4(in_Position, 1));

	// Pass along model matrix for light calculations
	passModelMatrix = modelMatrix;

	// Pass along normals for light calculations
	passNormals = in_Normals;

	// Pass along vertex position in object space
	passVert = in_Position;
}