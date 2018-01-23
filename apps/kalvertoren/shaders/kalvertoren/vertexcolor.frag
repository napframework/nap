#version 150 core

// Passed vertex attributes
in vec3 pass_Uvs;						// Uvs
in vec4 pass_Color;						// Color
in vec3 pass_Normals;					// Normals
in mat4 pass_ModelMatrix;				// Matrix
in vec3 pass_Vert;						// The vertex position
in vec3 pass_Artnet;					// The artnet color value

// Light Uniforms
uniform vec3		lightPosition;		// World position of the light
uniform float		ambientIntensity;	// Ambient light intensity
uniform float		shininess;			// Specular angle shininess
uniform float		specularIntensity;	// Amount of added specular
uniform vec3		cameraLocation;		// World Space location of the camera

out vec4 out_Color;

void main(void)
{
	// Get video color
	vec3 color = pass_Color.rgb;

	//calculate normal in world coordinates
    mat3 normal_matrix = transpose(inverse(mat3(pass_ModelMatrix)));
    vec3 normal = normalize(normal_matrix * pass_Normals);
	
	//calculate the location of this fragment (pixel) in world coordinates
    vec3 frag_position = vec3(pass_ModelMatrix * vec4(pass_Vert, 1));

	//calculate the vector from this pixels surface to the light source
	vec3 surfaceToLight = normalize(lightPosition - frag_position);

	// calculate vector that defines the distance from camera to the surface
	vec3 cameraPosition = cameraLocation;
	vec3 surfaceToCamera = normalize(cameraPosition - frag_position);

	// Ambient color
	vec3 ambient = color.rgb * ambientIntensity;

	//diffuse
    float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
    
	//specular
	vec3 specularColor = vec3(1.0,1.0,1.0);
	float specularCoefficient = 0.0;
    if(diffuseCoefficient > 0.0)
        specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(-surfaceToLight, normal))), shininess);
    vec3 specular = specularCoefficient * specularColor * specularIntensity;

	//linear color (color before gamma correction)
    vec3 linearColor = ambient + specular;
    linearColor.r = clamp(linearColor.x,0,1);
    linearColor.g = clamp(linearColor.y,0,1);
    linearColor.b = clamp(linearColor.z,0,1);

	// Set as output color
	out_Color = vec4(linearColor, 1.0);
}