#version 450 core

// Uniforms
uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraPosition;
} mvp;

// Vertex Output
out vec3 	passPosition;			//< Vertex position in world space
out vec3 	passUV0;				//< UVs

void main()
{
	passUV0 = vec3((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2, 0.0);
	passPosition = vec3(passUV0.x, passUV0.y, 0.0);

	gl_Position = vec4(passUV0.xy * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 0.0, 1.0);
}
