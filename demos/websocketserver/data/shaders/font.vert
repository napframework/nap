#version 330 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

// Input Vertex Attributes
in vec3	in_Position;
in vec3 in_UV0;

// Output to fragment shader
out vec3 passUVs;					//< vetex uv's

void main(void)
{
	// Calculate frag position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

	// Forward uvs to fragment shader
	passUVs = in_UV0;
}