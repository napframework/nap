#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

in vec3	in_Position;		// Vertex Positions
in vec4	in_Color;			// Vertex Color
in vec3	in_UV0;				// Vertex Uvs
in vec3 in_Normals;			// Vertex Normals
in int  in_Channels;		// Artnet Channels
in int  in_Universes;		// Artnet Universes
in int  in_Subnets;			// Artnet Subnets

out vec4 pass_Color;
out vec3 pass_Uvs;
out vec3 pass_Normals;
out vec3 pass_Vert;
out mat4 pass_ModelMatrix;
out vec3 pass_Artnet;

void main(void)
{
	// Calculate position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

	// Pass colors 
	pass_Color = in_Color;
	
	// Pass uvs
	pass_Uvs = in_UV0;

	// Pass along vertex position (object space)
	pass_Vert = in_Position;

	// Pass along model matrix for light calculations
	pass_ModelMatrix = modelMatrix;

	// Pass along normals for light calculations
	pass_Normals = in_Normals;

	// Pass along artnet addresses as rgb values
	float r = float(in_Channels)  / 511.0;
	float g = float(in_Universes) / 15.0;
	float b = float(in_Subnets)   / 15.0;
	pass_Artnet = vec3(r,g,b);
}