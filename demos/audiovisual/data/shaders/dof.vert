#version 450

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

in vec3 in_Position;
in vec3 in_UV0;

out vec2 pass_UV;

void main()
{
	pass_UV = in_UV0.xy;
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);
}
