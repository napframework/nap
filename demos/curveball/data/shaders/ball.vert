#version 450 core

// Uniform inputs
uniform nap
{
	uniform mat4 projectionMatrix;
	uniform mat4 viewMatrix;
	uniform mat4 modelMatrix;
} mvp;

in vec3	in_Position;
in vec3	in_UV0;
in vec4 in_Color0;
in vec3 in_Normal;

// Output to fragment shader
out vec3 passUVs;					//< vetex uv's
out vec3 passNormal;				//< vertex normal in object space
out vec3 passPosition;				//< vertex position in object space
out mat4 passModelMatrix;			//< model matrix
out vec2 passColor;

void main(void)
{
	// Calculate frag position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);

    // Pass normal in object space
	passNormal = in_Normal;

	// Pass position in object space
	passPosition = in_Position;

	// Pass model matrix for blob light calculations
	passModelMatrix = mvp.modelMatrix;

	// Forward uvs to fragment shader
	passUVs = in_UV0;
}