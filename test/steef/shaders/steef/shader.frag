#version 150 core

in vec3 pass_Uvs0;			// The global vinyl uvs
in vec3 pass_Uvs1;			// The label uvs
in vec3 pass_Normals;		// Normals
in mat4 pass_ModelMatrix;	// Matrix
in vec3 pass_Vert;			// The vertex position

out vec4 out_Color;

uniform sampler2D	vinylLabel;
uniform sampler2D	vinylMask;
uniform sampler2D	grooveNormalMap;
uniform vec3		recordColor;
uniform vec3		lightPosition;
uniform vec3		lightIntensity;
uniform float		ambientIntensity;	// Ambient light intensity
uniform float		shininess;			// Specular angle shininess
uniform float		specularIntensity;	// Amount of added specular
uniform float		attenuationScale;	// Light Falloff
uniform float		grooveScale;		// Amount of groove highlight
uniform vec3		cameraLocation;		// World space location of the camera

void main(void)
{
	// Get color of mask based on first uv set
	float mask_color = texture(vinylMask, pass_Uvs0.xy).r;

	// Label color
	vec3 label_color = texture(vinylLabel, pass_Uvs1.xy).rgb;
	vec3 color = mix(recordColor.rgb, label_color.rgb, mask_color);

	vec3 groove_color = texture(grooveNormalMap, pass_Uvs0.xy).rgb;
	vec3 groove_normal_tangent_space = (groove_color *2.0 - 1.0) * grooveScale;
	vec3 adjusted_vinyl_normal = pass_Normals + vec3(groove_normal_tangent_space.x,0.0, groove_normal_tangent_space.y);

	//calculate normal in world coordinates
    mat3 normal_matrix = transpose(inverse(mat3(pass_ModelMatrix)));
    vec3 normal = normalize(normal_matrix * adjusted_vinyl_normal);

	//calculate the location of this fragment (pixel) in world coordinates
    vec3 frag_position = vec3(pass_ModelMatrix * vec4(pass_Vert, 1));

	//calculate the vector that defines the direction of the light to the surface
	vec3 surfaceToLight = normalize(lightPosition - frag_position);

	// calculate vector that defines the direction of camera to the surface
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

	//final color (after gamma correction)
	out_Color = vec4(linearColor, 1.0);
}
