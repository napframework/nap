#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

in vec3	in_Position;
in vec4 in_Color0;

// Output to fragment shader
out vec3 passPosition;				//< vertex position in object space
out mat4 passModelMatrix;			//< model matrix
out vec2 passColor;

void main(void)
{
	// Calculate frag position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

	// Pass position in object space
	passPosition = in_Position;

	// Pass model matrix for blob light calculations
	passModelMatrix = modelMatrix;

}