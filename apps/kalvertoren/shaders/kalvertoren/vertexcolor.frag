#version 150 core

// Passed vertex attributes
in vec3 pass_Uvs;						// Uvs
in vec4 pass_Color;						// Color
in vec3 pass_Normals;					// Normals
in mat4 pass_ModelMatrix;				// Matrix
in vec3 pass_Vert;						// The vertex position

// Light Uniforms
uniform vec3		lightPosition;		// World position of the light
uniform vec3		lightIntensity;		// Light intensity
uniform float		ambientIntensity;	// Ambient light intensity
uniform float		shininess;			// Specular angle shininess
uniform float		specularIntensity;	// Amount of added specular
uniform float		attenuationScale;	// Light Falloff
uniform vec3		cameraLocation;		// World Space location of the camera

// Video Texture
uniform sampler2D	videoTexture;		// Lookup texture from video

out vec4 out_Color;

void main(void)
{
	// Get video color
	vec2 uvs = vec2(pass_Uvs.x, pass_Uvs.y);
	vec3 color = texture(videoTexture, uvs).rgb;
	//vec3 color = vec3(1.0,0.0,0.0);

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
    float attenuation = 1.0 / (1.0 + attenuationScale * pow(distanceToLight, 2));

	//linear color (color before gamma correction)
    vec3 linearColor = ambient + attenuation*(diffuse + specular);

	// Set as output color
	out_Color = vec4(linearColor, 1.0);
}