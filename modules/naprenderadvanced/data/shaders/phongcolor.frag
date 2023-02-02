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

// vertex shader input
in vec3 passPosition;					//< frag position in object space
in vec3 passNormal;						//< frag normal in object space
in vec3 passUV0;						//< UVs
in float passFresnel;					//< fresnel term
in vec4 passShadowCoord[8];			//< shadow coord

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
	DirectionalLightShadow lights[8];
	uint count;
} lit;

// uniform inputs
uniform UBO
{
	vec3 	color;						//< Color
	float	alpha;						//< Alpha
} ubo;

// Shader Output
out vec4 out_Color;

// Shadow Texture Sampler
uniform sampler2DShadow shadowMaps[8];

// Constants
const float SHADOW_STRENGTH 	= 0.8;
const float SHININESS 			= 24.0;
const float EPSILON 			= 0.00001;
const float PI 					= 3.14159265359;

// Poisson Sampling
const vec2 POISSON_DISK_4[4] = vec2[](
	vec2(-0.94201624, -0.39906216),
	vec2(0.94558609, -0.76890725),
	vec2(-0.094184101, -0.92938870),
	vec2(0.34495938, 0.29387760)
);

const vec2 POISSON_DISK_16[16] = vec2[](
	vec2( -0.94201624, -0.39906216 ),
	vec2( 0.94558609, -0.76890725 ),
	vec2( -0.094184101, -0.92938870 ),
	vec2( 0.34495938, 0.29387760 ),
	vec2( -0.91588581, 0.45771432 ),
	vec2( -0.81544232, -0.87912464 ),
	vec2( -0.38277543, 0.27676845 ),
	vec2( 0.97484398, 0.75648379 ),
	vec2( 0.44323325, -0.97511554 ),
	vec2( 0.53742981, -0.47373420 ),
	vec2( -0.26496911, -0.41893023 ),
	vec2( 0.79197514, 0.19090188 ),
	vec2( -0.24188840, 0.99706507 ),
	vec2( -0.81409955, 0.91437590 ),
	vec2( 0.19984126, 0.78641367 ),
	vec2( 0.14383161, -0.14100790 )
);

const float POISSON_SPREAD 	= 768.0;

// Calculate shadow contribution
// @param shadowCoord: the fragment position in light space
// @param lightDir: the light direction
float computeShadow(vec4 shadowCoord, vec3 lightDir, uint lightIndex)
{
	// Map coordinates to [0.0, 1.0] range
	shadowCoord = shadowCoord * 0.5 + 0.5;

	const float texelSize = 1.0/float(textureSize(shadowMaps[lightIndex], 0).x);
	float bias = max(4.0*texelSize * (1.0 - dot(passNormal, lightDir)), texelSize);
	float frag_depth = (shadowCoord.z-bias) / shadowCoord.w;
	float shadow = 0.0;

	// Multi sample
	// for (int i=0; i<16; i++) 
	// {
	// 	shadow += texture(shadowMaps[lightIndex], 
	// 		vec3(shadowCoord.xy + POISSON_DISK_16[i]/POISSON_SPREAD, frag_depth)
	// 	);
	// }
	// shadow /= 16.0;

	// Single sample
	shadow = 1.0 - texture(shadowMaps[lightIndex], vec3(shadowCoord.xy, frag_depth));
	return shadow;
}


// Shades a color based on a light, incoming normal and position should be in object space
vec3 applyLight(vec3 color, vec3 normal, vec3 position, uint lightIndex)
{
	DirectionalLightShadow li = lit.lights[lightIndex];

	// inverse light direction
	vec3 inv_dir = normalize(-li.direction);

	// calculate vector that defines the distance from camera to the surface
	vec3 surface_to_cam = normalize(mvp.cameraPosition - position);

	// lighting components
	float ambient = 1.0;
	float diffuse = max(0.0, dot(normal, inv_dir));
	float specular = 0.0;

	// compute specular value if diffuse coefficient is > 0.0
	if (diffuse > 0.0)
	{
		vec3 halfway = normalize(inv_dir + surface_to_cam);  
		specular = pow(max(dot(normal, halfway), 0.0), SHININESS);
	}

	vec3 ambient_color = ambient * li.intensity * li.color;
	vec3 diffuse_color = diffuse * li.intensity * li.color;
	vec3 specular_color = specular * li.intensity * li.color;

	// linear color (color before gamma correction)
	vec3 comp_color = (diffuse_color + specular_color + ambient_color) * ubo.color;

	// clamp
	return clamp(comp_color, 0.0, 1.0);
}


void main()
{
	vec3 color = vec3(0.0);
	for (uint i = 0; i < lit.count; i++)
	{
		color += applyLight(ubo.color.rgb, passNormal, passPosition, i);
	};

	float shadow = 0.0;
	for (uint i = 0; i < lit.count; i++)
	{
		DirectionalLightShadow li = lit.lights[i];
		shadow = max(computeShadow(passShadowCoord[i], normalize(li.direction), i), shadow);
	}
	shadow = clamp(shadow, 0.0, 1.0) * SHADOW_STRENGTH;
	
	color = mix(color, vec3(0.975), passFresnel);
	color = mix(color, vec3(0.025), shadow);

	out_Color = vec4(color, ubo.alpha);
}
