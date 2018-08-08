#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

in vec3	in_Position;
in vec3 in_Normal;
in vec3 in_UV0;
in vec3 in_UV1;

out vec3 passUVs0;		//< The unwrapped normalized texture
out vec3 passUVs1;		//< The polar unwrapped texture

void main(void)
{
	// Calculate position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

    passUVs0 = in_UV0;
    passUVs1 = in_UV1;
}