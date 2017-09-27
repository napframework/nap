#version 150 core

in vec4 pass_Color;
in vec3 pass_Uvs0;			// The global vinyl uvs
in vec3 pass_Normals;		// Normals
in mat4 pass_ModelMatrix;	// Matrix
in vec3 pass_Vert;			// The vertex position
in vec3 pass_Tangent;		// The tangent
in vec3 pass_Bitangent;		// The bitangent

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
	vec3 label_color = texture(vinylLabel, pass_Uvs0.xy).rgb;
	vec3 color = mix(recordColor.rgb, label_color.rgb, mask_color);

	vec3 groove_color = texture(grooveNormalMap, pass_Uvs0.xy).rgb;
	vec3 groove_normal = (groove_color *2.0) - 1.0;
	groove_normal.rg = groove_normal.rg * grooveScale;

	// Convert normals to world
	vec3 normal_world = normalize(transpose(inverse(mat3(pass_ModelMatrix))) * pass_Normals);
	vec3 tan_world = normalize(mat3(pass_ModelMatrix) * pass_Tangent);
	vec3 bitan_world = cross(normal_world, tan_world) * 1.0;

	// Get normal transform matrix
	mat3 tbn_matrix = mat3(tan_world, bitan_world, normal_world);

	// Get final normal
    vec3 normal = normalize(tbn_matrix * groove_normal);

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