#version 330

// input
in vec3 passUVs0;			// The unwrapped normalized texture
in vec3 passUVs1;			// The polar unwrapped texture
in vec3 passNormal;			//< frag normal in object space
in vec3 passPosition;		//< frag position in object space
in mat4 passModelMatrix;	//< modelMatrix

// output
out vec4 out_Color;

// uniforms
uniform sampler2D	colorTexture;							//< Primary Color Texture
uniform float		colorTexScale;							//< Primage Color Texture Scale
uniform vec3 		cameraPosition;							//< Camera World Space Position

// constants
const vec3			lightPos = vec3(0.0, 100.0, 100.0);		
const float 		lightIntensity = 1.0;					
const float 		specularIntensity = 0.25;				
const vec3  		specularColor = vec3(1.0,1.0,1.0);
const float 		shininess = 10;
const float 		ambientIntensity = 0.5f;


// Shades a color based on a light, incoming normal and position should be in object space
vec3 applyLight(vec3 color, vec3 normal, vec3 position)
{
	// Calculate normal to world
	mat3 normal_matrix = transpose(inverse(mat3(passModelMatrix)));
	vec3 ws_normal = normalize(normal * normal_matrix);

	// Calculate frag to world
	vec3 ws_position = vec3(passModelMatrix * vec4(position, 1.0));

	//calculate the vector from this pixels surface to the light source
	vec3 surfaceToLight = normalize(lightPos - ws_position);

	// calculate vector that defines the distance from camera to the surface
	vec3 surfaceToCamera = normalize(cameraPosition - ws_position);

	// Ambient color
	vec3 ambient = color * ambientIntensity;

	// diffuse
    float diffuseCoefficient = max(0.0, dot(ws_normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * color * lightIntensity;

	// Scale specular based on vert color (greyscale)
	float spec_intensity = specularIntensity;

	// Compute specularf
    float specularCoefficient = pow(max(0.0, dot(normalize(reflect(-surfaceToLight, ws_normal)), surfaceToCamera)), shininess);
    vec3 specular = specularCoefficient * specularColor * lightIntensity * spec_intensity;

	//linear color (color before gamma correction)
    return diffuse + specular + ambient;
}


void main() 
{	
	// Get color from texture
	vec3 tex_color = texture(colorTexture, (passUVs0.xy * colorTexScale)).rgb;

	vec3 lit_color = applyLight(tex_color, passNormal, passPosition);

	// Set to frag output color
	out_Color = vec4(lit_color,1.0);
}
