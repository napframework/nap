#version 450 core

// vertex shader input 
in vec3 passNormals;									// Normals
in mat4 passModelMatrix;								// Matrix
in vec3 passVert;										// The vertex position
in vec3 passPosition;									//< frag world space position 

// Point light structure
struct PointLight
{
	vec3		mPosition;
	vec3 		mIntensity;
};

uniform UBO
{
	// uniform inputs
	uniform vec3		cameraLocation;							// World Space location of the camera
	uniform PointLight	lights[2];								// All lights in the scene
	uniform vec3 		meshColor;								// Color or the mesh
} ubo;

// Light Uniforms
const float			ambientIntensity = 0.3;					// Ambient light intensity
const float			shininess = 4.0;						// Specular angle shininess
const float			specularIntensity = 0.5;				// Amount of added specular

// output
out vec4 out_Color;


vec3 computeLightContribution(int lightIndex, vec3 color)
{
	//calculate normal in world coordinates
    mat3 normal_matrix = transpose(inverse(mat3(passModelMatrix)));
    vec3 normal = normalize(normal_matrix * passNormals);
	
	//calculate the location of this fragment (pixel) in world coordinates
    vec3 frag_position = vec3(passModelMatrix * vec4(passVert, 1));

	//calculate the vector from this pixels surface to the light source
	vec3 surfaceToLight = normalize(ubo.lights[lightIndex].mPosition - frag_position);

	// calculate vector that defines the distance from camera to the surface
	vec3 cameraPosition = ubo.cameraLocation;
	vec3 surfaceToCamera = normalize(cameraPosition - frag_position);
	
	//diffuse
    float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * color.rgb * ubo.lights[lightIndex].mIntensity;
    
	//specular
	vec3 specularColor = vec3(1.0,1.0,1.0);
	float specularCoefficient = 0.0;
    if(diffuseCoefficient > 0.0)
        specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-surfaceToLight, normal))), shininess);
    vec3 specular = specularCoefficient * specularColor * ubo.lights[lightIndex].mIntensity * specularIntensity;

    // return combination
    return specular + diffuse;
}


void main() 
{        
	//linear color (color before gamma correction)
    vec3 outColor = vec3(0,0,0);
    for(int i=0; i<ubo.lights.length(); i++)
    {
    	outColor = outColor + computeLightContribution(i, ubo.meshColor);
    }

    // Add ambient color
	vec3 ambient = ubo.meshColor.rgb * ambientIntensity;
    outColor = outColor + ambient;
    out_Color = vec4(outColor, 1.0);
}
