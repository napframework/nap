#version 450 core

// vertex shader input
in vec3 passPosition;					//< frag position in object space
in vec3 passNormal;						//< frag normal in object space
in vec3 passUV0;						//< UVs
in float passFresnel;					//< fresnel term

// uniform inputs
uniform UBO
{
	vec3 	color;						//< Color
	float	alpha;						//< Alpha
} ubo;

// output
out vec4 out_Color;

const vec3		lightPos 			= vec3(0.0, 3.0, 10.0);
const float 	lightIntensity		= 1.0;
const float 	specularIntensity 	= 0.5;
const vec3  	specularColor 		= vec3(1.0, 1.0, 1.0);
const vec3  	haloColor 			= vec3(1.0, 1.0, 1.0);
const float 	shininess 			= 24.0;
const float 	ambientIntensity 	= 0.5;

// Shades a color based on a light, incoming normal and position should be in object space
vec3 applyLight(vec3 color, vec3 normal, vec3 position)
{
	//calculate the vector from this pixels surface to the light source
	vec3 surface_to_light = normalize(lightPos - position);

	// calculate vector that defines the distance from camera to the surface
	vec3 surface_to_cam = normalize(ubo.cameraLocation - position);

	// ambient
	vec3 ambient = ubo.color.rgb * ambientIntensity;

	// compute diffuse
	float diffuse_coefficient = max(0.0, dot(normal, surface_to_light));
	vec3 diffuse = diffuse_coefficient * ubo.color.rgb * lightIntensity;

	// compute specular value if diffuse coefficient is > 0.0
	float specular_coefficient = 0.0;
	if (diffuse_coefficient > 0.0)
	{
		vec3 halfway = normalize(surface_to_light + surface_to_cam);  
		specular_coefficient = pow(max(dot(normal, halfway), 0.0), shininess);
	}

	// compute specular
	vec3 specular = specular_coefficient * specularColor * lightIntensity * specularIntensity;

	// linear color (color before gamma correction)
	vec3 comp_color = diffuse + specular + ambient;

	// clamp
	return clamp(comp_color, vec3(0.0), vec3(1.0));
}


void main()
{
	vec3 output_color = applyLight(ubo.color.rgb, passNormal, passPosition);
	out_Color = vec4(mix(output_color, haloColor, passFresnel), ubo.color.a); 
}
