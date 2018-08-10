#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

in vec3	in_Position;
in vec3 in_Normal;
in vec3 in_UV0;
in vec3 in_UV1;

out vec3 passUVs0;					//< The unwrapped normalized texture
out vec3 passUVs1;					//< The polar unwrapped texture
out vec3 passNormal;				//< vertex normal in object space
out vec3 passPosition;				//< vertex position in object space
out mat4 passModelMatrix;			//< model matrix

void main(void)
{
	// Calculate position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

	// Pass uvs
    passUVs0 = in_UV0;
    passUVs1 = in_UV1;

	// Pass normal in object space
	passNormal = in_Normal;

	// Pass position in object space
	passPosition = in_Position;

	// Pass model matrix for calculations
	passModelMatrix = modelMatrix;
}