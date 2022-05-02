#version 450 core

struct PBRMaterial
{
	vec3 albedo;
	vec3 reflectance;
	float roughness;
	float metallic;
};

struct PointLight
{
	vec3 position;
	vec3 color;
	float intensity;
};

struct DirectionalLight
{
	vec3 direction;
	vec3 color;
	float intensity;
};

uniform FRAGUBO
{
	vec3 albedo;
	float roughness;
	float metallic;

	vec3 cameraLocation;

	vec3 lightColor;
	vec3 lightPosition;
	vec3 lightDirection;
	float lightIntensity;
} frag_ubo;

// Shadow Texture Sampler
uniform sampler2DShadow shadowMap;

// Inputs
in vec3 passPosition;
in vec3 passNormal;
in vec3 passUV;
in vec4 passColor;
in vec4 passShadowCoord;

// Output
out vec4 outColor;

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
const float EPSILON 		= 0.00001;
const float PI 				= 3.14159265359;
const vec3	BASE_F0 		= vec3(0.333333);
const vec3	BASE_AMBIENT	= vec3(0.05);


// Trowbridge-Reitz GGX normal distribution function
float distributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness*roughness;
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;

	float nom   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}


// Schlick-Beckmann approximation
float geometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}


// Schmith geometry approximation from view direction (geometry obstruction) and light direction (geometry shadowing)
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = geometrySchlickGGX(NdotV, roughness);
	float ggx1 = geometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}


// Returns surface reflection ratio at specified angle
// @param cosTheta: dot product between surface normal and view direction
// @param F0: the base reflectivity parameter, determined by type of surface
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


// Calculate surface radiance for a point light
vec3 computeOutRadiance(vec3 N, vec3 V, vec3 L, vec3 radiance, PBRMaterial mtl)
{
	// Halfway vector
	vec3 H = normalize(V + L); 

	// Cook-Torrance BRDF
    float NDF = distributionGGX(N, H, mtl.roughness);   
	float G = geometrySmith(N, V, L, mtl.roughness);      
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0), mtl.reflectance);

	vec3 numerator = NDF * G * F; 
	float denominator = 1.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + EPSILON;
	vec3 specular = numerator / denominator;

	// kS is equal to Fresnel
	vec3 kS = F; 

	// for energy conservation, the diffuse and specular light can't
	// be above 1.0 (unless the surface emits light); to preserve this
	// relationship the diffuse component (kD) should equal 1.0 - kS.
	vec3 kD = 1.0 - kS;

	// multiply kD by the inverse metalness such that only non-metals 
	// have diffuse lighting, or a linear blend if partly metal (pure metals
	// have no diffuse light).
	kD *= 1.0 - mtl.metallic;

	// scale light by NdotL
	float NdotL = max(dot(N, L), 0.0);

	// add to outgoing radiance Lo
	// note that we already multiplied the BRDF by the Fresnel (kS) 
	// so we won't multiply by kS again
	vec3 Lo = (kD * mtl.albedo / PI + specular) * radiance * NdotL;

	return Lo;
}


// Calculate shadow contribution
// @param shadowCoord: the fragment position in light space
// @param lightDir: the light direction
float calcShadow(vec4 shadowCoord, vec3 lightDir)
{
	// Map coordinates to [0.0, 1.0] range
	shadowCoord = shadowCoord * 0.5 + 0.5;

	float bias = max(0.01 * (1.0 - dot(passNormal, lightDir)), 0.005);
	float frag_depth = (shadowCoord.z-bias) / shadowCoord.w;
	float shadow = 0.0;

	// Multi sample
	for (int i=4; i<16; i++) 
	{
		//int idx = int(16.0*rnd(gl_FragCoord.xy, i))%16;
		int idx = i;
		shadow += texture(shadowMap, 
			vec3(shadowCoord.xy + POISSON_DISK_16[idx]/POISSON_SPREAD, frag_depth)
		);
	}
	shadow /= 16.0;

	// Single sample
	//shadow = 1.0 - texture(shadowMap, vec3(shadowCoord.xy, frag_depth));
	return shadow;
}


vec3 pbrDirectional(vec3 N, vec3 V, DirectionalLight light, PBRMaterial mtl)
{
	vec3 L = normalize(-light.direction);
	vec3 radiance = light.color * light.intensity; 

	// Debug coordinates
	return computeOutRadiance(N, V, L, radiance, mtl);
}


vec3 pbrPoint(vec3 N, vec3 V, vec3 fragWorldPos, PointLight light, PBRMaterial mtl)
{
	vec3 surf_to_light = light.position - fragWorldPos;
	vec3 L = normalize(surf_to_light);

	float shadow = calcShadow(passShadowCoord, L);

	float distance = length(surf_to_light);
	float attenuation = 1.0 / (distance * distance);
	vec3 radiance = (1.0-shadow) * light.color * light.intensity * attenuation;

	return computeOutRadiance(N, V, L, radiance, mtl);
}


void main() 
{
	// Sample PBR Material from maps
	PBRMaterial mtl;
	mtl.albedo 			= frag_ubo.albedo;
	mtl.metallic 		= frag_ubo.metallic;
	mtl.roughness 		= frag_ubo.roughness;

	// Calculate reflectance at normal incidence; F0 is base reflectance 
	// If metal, use albedo as F0 (metallic workflow)    
	vec3 F0 			= mix(BASE_F0, mtl.albedo, mtl.metallic);
	mtl.reflectance 	= F0;

	// Surface and View Normal
	vec3 N = passNormal;
	vec3 V = normalize(frag_ubo.cameraLocation - passPosition);

	// Radiance
	vec3 Lo = vec3(0.0);
	{
		// Directional light
		DirectionalLight dl = {frag_ubo.lightDirection, frag_ubo.lightColor, frag_ubo.lightIntensity};
		Lo += pbrDirectional(N, V, dl, mtl);

		vec3 L = normalize(-dl.direction);
		Lo *= calcShadow(passShadowCoord, L);
		
		// Point light
		// PointLight pl = {frag_ubo.lightPosition, frag_ubo.lightColor, frag_ubo.lightIntensity};
		// Lo += pbrPoint(N, V, passPosition, pl, mtl);
	}

	vec3 ambient = BASE_AMBIENT * mtl.albedo;
	vec3 color = Lo;

	// HDR tonemapping
    color = color / (color + vec3(1.0));
    
    // Gamma correction
    color = pow(color, vec3(1.0/2.2)); 

    // Output
	outColor = vec4(color, 1.0);
}
