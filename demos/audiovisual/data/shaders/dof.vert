#version 450 core

out vec2 passUV0;

void main()
{
	passUV0 = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);	
	gl_Position = vec4(passUV0 * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 0.0, 1.0);
}
