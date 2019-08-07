#version 330

// vertex shader input 
in vec3 passPosition;										//< frag world space position 
in vec3 passNormals;										// Normals
in mat4 passModelMatrix;									// Matrix
in vec3 passVert;											// The vertex position

// Point light structure
struct PointLight
{
	vec3		mPosition;
	vec3 		mIntensity;
};

// uniform inputs
uniform vec3 		color;
uniform vec3		cameraLocation;							// World Space location of the camera
uniform PointLight	light;

// Light Uniforms
const float			ambientIntensity = 0.4;					// Ambient light intensity
const float			shininess = 4.0;						// Specular angle shininess
const float			specularIntensity = 0.5;				// Amount of added specular

// output
out vec4 out_Color;

void main() 
{
	//calculate normal in world coordinates
    mat3 normal_matrix = transpose(inverse(mat3(passModelMatrix)));
    vec3 normal = normalize(normal_matrix * passNormals);
	
	//calculate the location of this fragment (pixel) in world coordinates
    vec3 frag_position = vec3(passModelMatrix * vec4(passVert, 1));

	//calculate the vector from this pixels surface to the light source
	vec3 surfaceToLight = normalize(light.mPosition - frag_position);

	// calculate vector that defines the distance from camera to the surface
	vec3 cameraPosition = cameraLocation;
	vec3 surfaceToCamera = normalize(cameraPosition - frag_position);

	// Ambient color
	vec3 ambient = color.rgb * light.mIntensity * ambientIntensity;
	
	//diffuse
    float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * color.rgb * light.mIntensity;
    
	//specular
	vec3 specularColor = vec3(1.0,1.0,1.0);
	float specularCoefficient = 0.0;
    if(diffuseCoefficient > 0.0)
        specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-surfaceToLight, normal))), shininess);
    vec3 specular = specularCoefficient * specularColor * light.mIntensity * specularIntensity;
    
	//linear color (color before gamma correction)
    vec3 linearColor = ambient + specular + diffuse;

    out_Color = vec4(linearColor, 1.0);
}
