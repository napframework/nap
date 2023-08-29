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

uniform UBO
{
	vec2	fresnel;						//< Fresnel [scale, power]
} ubo;

// Fragment Input
in vec3 	passPosition;					//< Fragment position in world space
in vec3 	passNormal;						//< Fragment normal in world space
in float 	passFresnel;					//< Fresnel term

// Fragment Output
out vec4 out_Color;

// Environment map
uniform samplerCube environmentMap;

void main()
{
	// Sample environment map
	vec3 I = normalize(passPosition - mvp.cameraPosition);
	vec3 R = reflect(I, normalize(passNormal));
	vec4 color = texture(environmentMap, R);

	// Add fresnel
	color.rgb = mix(color.rgb, vec3(1.0), passFresnel);

	// Final color output
	out_Color = color;
}
