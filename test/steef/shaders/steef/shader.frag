#version 150 core

in vec4 pass_Color;
in vec3 pass_Uvs0;			// The global vinyl uvs
in vec3 pass_Uvs1;			// The label uvs
in vec3 pass_Normals;		// Normals
in mat4 pass_ModelMatrix;	// Matrix
in vec3 pass_Vert;			// The vertex position

out vec4 out_Color;

uniform sampler2D	vinylLabel;
uniform sampler2D	vinylMask;
uniform vec3		recordColor;
uniform vec3		lightPosition;
uniform vec3		lightIntensity;

void main(void)
{
	// Get color of mask based on first uv set
	float mask_color = texture(vinylMask, pass_Uvs0.xy).r;

	// Label color
	vec3 label_color = texture(vinylLabel, pass_Uvs1.xy).rgb;

	//calculate normal in world coordinates
    mat3 normal_matrix = transpose(inverse(mat3(pass_ModelMatrix)));
    vec3 normal = normalize(normal_matrix * pass_Normals);

	//calculate the location of this fragment (pixel) in world coordinates
    vec3 frag_position = vec3(pass_ModelMatrix * vec4(pass_Vert, 1));

	//calculate the vector from this pixels surface to the light source
    vec3 light_position = lightPosition;
	vec3 surfaceToLight = light_position - frag_position;

	//calculate the cosine of the angle of incidence
    float brightness = dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));
    brightness = clamp(brightness, 0, 1);

	vec3 light_intensity = lightIntensity;
	vec3 label_color_lit = vec3(brightness * light_intensity * label_color.rgb);
	vec3 record_color_lit = vec3(brightness * light_intensity * recordColor.rgb);

	// Blend based on mask color
	vec3 out_color = mix(record_color_lit, label_color_lit, mask_color);

	// Set output color
	out_Color = vec4(out_color, 1.0);
}