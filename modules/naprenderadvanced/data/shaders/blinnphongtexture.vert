#version 450 core

// Extensions
#extension GL_GOOGLE_include_directive : enable

// Includes
#include "light.glslinc"
#include "utils.glslinc"
#include "shadow.glslinc"

// Uniforms
uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
	mat4 normalMatrix;
	vec3 cameraPosition;
} mvp;

uniform light
{
	Light lights[8];
	uint count;
} lit;

uniform UBO
{
	vec4	ambient;				//< Ambient
	vec3	diffuse;				//< Diffuse
	vec3	specular;				//< Specular
	vec2	fresnel;				//< Fresnel [scale, power]
	float	shininess;				//< Shininess
	float	alpha;					//< Alpha
	uint	environment;			//< Whether to sample an environment map
} ubo;

// Vertex Input
in vec3		in_Position;			//< Vertex position in object space
in vec3 	in_Normals;				//< Vertex normal in object space
in vec3 	in_UV0;					//< Texture UVs

// Vertex Output
out vec3 	passPosition;			//< Vertex position in world space
out vec3 	passNormal;				//< Vertex normal in world space
out vec3 	passUV0;				//< UVs
out float 	passFresnel;			//< Fresnel
out vec4 	passShadowCoords[8];	//< Shadow Coordinates

void main()
{
	// Calculate frag position
	vec4 world_position = mvp.modelMatrix * vec4(in_Position, 1.0);
	gl_Position = mvp.projectionMatrix * mvp.viewMatrix * world_position;

	passPosition = world_position.xyz;
	passUV0 = in_UV0;

	// Rotate normal based on model matrix and set
	vec3 world_normal = normalize((mvp.normalMatrix * vec4(in_Normals, 1.0)).xyz);
	passNormal = world_normal;

	// Calculate fresnel term
	vec3 eye_to_surface = normalize(world_position.xyz - mvp.cameraPosition);
	passFresnel = pow(clamp(1.0 + dot(eye_to_surface, world_normal), 0.0, 1.0), ubo.fresnel.y) * ubo.fresnel.x;

	// Shadow
	for (uint i = 0; i < lit.count; i++)
	{
		// Check if shadow is enabled on this light, else skip
		uint flags = lit.lights[i].flags;
		if (!hasShadow(flags))
			continue;

		// Compute current shadow coordinate: the world position in lightviewspace
		vec4 coord = lit.lights[i].viewProjectionMatrix * world_position;
		
		// Flip y (Vulkan coordinates are [-1, 1], refer to NAP RenderProjectionMatrix)
		coord.y = -coord.y;
		
		// Pass coordinates
		passShadowCoords[i] = coord;
	}
}
