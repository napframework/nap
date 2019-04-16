#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

in vec3	in_Position;
in vec4 in_Color0;
in float in_Tip;

out float pass_Tip;
out vec4 pass_Color;

void main(void)
{
	// Calculate position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

	// Pass tip
	pass_Tip = in_Tip;

	// Pass color
	pass_Color = in_Color0;
}