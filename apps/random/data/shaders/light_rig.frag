#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passPosition;					//< frag world space position 
in vec3 passNormals;					// Normals
in mat4 passModelMatrix;				// Matrix
in vec3 passVert;						// The vertex position
in vec4 passColor;						// Vertex Color

// uniforms
uniform vec3		cameraLocation;		// World Space location of the camera

// Light Uniforms
const vec3			lightPosition  = vec3(0.0,200.0,-100.0);		// World position of the light
const vec3			lightIntensity = vec3(1.0,1.0,1.0) ;	// Light intensity
const float			ambientIntensity = 1.0;					// Ambient light intensity
const float			shininess = 10.0;						// Specular angle shininess
const float			specularIntensity = 0.5;				// Amount of added specular


// output
out vec4 out_Color;

void main() 
{
	// initial texture coordinate
	float led_intensity = 1.0;
	vec3 color = vec3(passColor.r, passColor.g, 0.0) * led_intensity;

	//calculate normal in world coordinates
    mat3 normal_matrix = transpose(inverse(mat3(passModelMatrix)));
    vec3 normal = normalize(normal_matrix * passNormals);
	
	//calculate the location of this fragment (pixel) in world coordinates
    vec3 frag_position = vec3(passModelMatrix * vec4(passVert, 1));

	//calculate the vector from this pixels surface to the light source
	vec3 surfaceToLight = normalize(lightPosition - frag_position);

	// calculate vector that defines the distance from camera to the surface
	vec3 cameraPosition = cameraLocation;
	vec3 surfaceToCamera = normalize(cameraPosition - frag_position);

	// Ambient color
	vec3 ambient = color.rgb * lightIntensity * ambientIntensity;
	
	//diffuse
    float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * color.rgb * lightIntensity;
    
	//specular
	vec3 specularColor = vec3(1.0,1.0,1.0);
	float specularCoefficient = 0.0;
    if(diffuseCoefficient > 0.0)
        specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-surfaceToLight, normal))), shininess);
    vec3 specular = specularCoefficient * specularColor * lightIntensity * specularIntensity;
    
	//attenuation based on light distance
    float distanceToLight = length(lightPosition - frag_position);

	//linear color (color before gamma correction)
    vec3 linearColor = ambient + specular;

	// set fragment color
	out_Color =  vec4(linearColor, 1.0);
}