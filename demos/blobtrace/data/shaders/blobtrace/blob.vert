#version 330 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

// Input Vertex Attributes
in vec3	in_Position;
in vec3 in_UV0;
in vec3 in_Normal;

// Output to fragment shader
out vec3 passUVs;					//< vetex uv's
out vec3 passNormal;				//< vertex normal in object space
out vec3 passPosition;				//< vertex position in object space
out mat4 passModelMatrix;			//< model matrix

void main(void)
{
	// Calculate frag position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

    // Pass normal in object space
	passNormal = in_Normal;

	// Pass position in object space
	passPosition = in_Position;

	// Pass model matrix for blob light calculations
	passModelMatrix = modelMatrix;

	// Forward uvs to fragment shader
	passUVs = in_UV0;
}