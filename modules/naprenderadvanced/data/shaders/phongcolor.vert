#version 450 core

struct DirectionalLight
{
	vec3 direction;
	vec3 color;
	float intensity;
};

struct DirectionalLightShadow
{
	mat4 lightViewProjection;
	vec3 direction;
	vec3 color;
	float intensity;
};

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
	DirectionalLightShadow lights[16];
	uint count;
} lit;

in vec3	in_Position;
in vec3 in_Normals;
in vec3 in_UV0;

// Output to fragment shader
out vec3 passPosition;				//< vertex position in object space
out vec3 passNormal;				//< vertex normal in object space
out vec3 passUV0;					//< UVs
out float passFresnel;				//< Fresnel
out vec4 passShadowCoord[16];		//< Shadow

// Constants
const float fresnelScale = 1.0;
const float fresnelPower = 1.5;


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
	float fresnel = 0.95 * pow(clamp(1.0 + dot(eye_to_surface, world_normal), 0.0, 1.0), fresnelPower);
	passFresnel = fresnelScale * fresnel;

	// Shadow
	for (uint i = 0; i < lit.count; i++)
	{
		passShadowCoord[i] = lit.lights[i].lightViewProjection * world_position;
	}
}
