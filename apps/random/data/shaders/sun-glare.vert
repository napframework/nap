#version 330 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

// Input Vertex Attributes
in vec3	in_Position;
in vec3 in_UV0;

// Output to fragment shader
out vec3 passUVs;					//< vetex uv's
out vec3 passPosition;				//< vertex world space position

void main(void)
{
	// Calculate frag position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

	// calculate vertex world space position and set
	passPosition = vec3(modelMatrix * vec4(in_Position, 1));

	// Forward uvs to fragment shader
	passUVs = in_UV0;
}
