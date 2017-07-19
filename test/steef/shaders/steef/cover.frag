#version 150 core

in vec4 pass_Color;			// Vertex colors
in vec3 pass_Uvs0;			// The global cover uvs
in vec3 pass_Normals;		// Normals
in mat4 pass_ModelMatrix;	// Matrix
in vec3 pass_Vert;			// The vertex position

out vec4 out_Color;

uniform sampler2D	coverImage;
uniform vec3		lightPosition;
uniform vec3		lightIntensity;

void main(void)
{
	// Fetch both colors
	vec3 color = texture(coverImage, pass_Uvs0.xy).rgb;

	//calculate normal in world coordinates
    mat3 normal_matrix = transpose(inverse(mat3(pass_ModelMatrix)));
    vec3 normal = normalize(normal_matrix * pass_Normals);

	//calculate the location of this fragment (pixel) in world coordinates
    vec3 frag_position = vec3(pass_ModelMatrix * vec4(pass_Vert, 1));

	//calculate the vector from this pixels surface to the light source
	vec3 surfaceToLight = lightPosition - frag_position;

	//calculate the cosine of the angle of incidence
    float brightness = dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));
    brightness = clamp(brightness, 0, 1);

	// Get diffuse color
	color = vec3(brightness * lightIntensity * color.rgb);

	// Set output color
	out_Color = vec4(color, 1.0);
}