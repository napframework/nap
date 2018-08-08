#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform float blendValue;
uniform float normalBlendValue;

in vec3	in_Position;				//< Vertex position
in vec3 in_UV0;						//< First uv coordinate set
in vec3 in_UV1;						//< Second uv coordinate set
in float in_Tip;					//< If the vertex is a tip or not

out float passTip;
out vec3 passUVs0;					//< The unwrapped normalized texture
out vec3 passUVs1;					//< The polar unwrapped texture

void main(void)
{
	// Calculate position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

	// Pass color
	passTip = in_Tip;

	// Pass uvs
	passUVs0 = in_UV0;
    passUVs1 = in_UV1;
}