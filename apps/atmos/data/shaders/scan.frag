#version 330

// input
in vec3 passUVs0;			// The unwrapped normalized texture
in vec3 passUVs1;			// The polar unwrapped texture
in vec3 passNormal;			//< frag normal in object space
in vec3 passPosition;		//< frag position in object space
in mat4 passModelMatrix;	//< modelMatrix

// output
out vec4 out_Color;

// shared uniforms
uniform sampler2D	colorTextureOne;						//< Primary Color Texture
uniform sampler2D	colorTextureTwo;
uniform float		colorTexScaleOne;
uniform float		colorTexScaleTwo;
uniform float		colorTexMix;							//< Primage Color Texture Scale
uniform vec3		diffuseColor;
uniform float		diffuseColorMix;
uniform vec3 		cameraPosition;							//< Camera World Space Position
uniform vec3		lightPos;		
uniform float 		lightIntensity;		
uniform float 		ambientIntensity;
uniform float		preMultiplyTexValue;

// Unshared uniforms					
uniform float 		specularIntensity;				
uniform vec3  		specularColor;
uniform float 		shininess; 
uniform float		textureTimeU;
uniform float		textureTimeV;

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
	vec2 tex_time = vec2(textureTimeU, textureTimeV);
	vec3 tex_color_one = texture(colorTextureOne, (passUVs0.xy * colorTexScaleOne)).rgb;
	vec3 tex_color_two = texture(colorTextureTwo, ((passUVs1.xy * colorTexScaleTwo)+tex_time)).rgb;

	vec3 tex_color_mul = tex_color_one *  tex_color_two;
	tex_color_one = mix(tex_color_one, tex_color_mul, preMultiplyTexValue);  

	// Mix into texture color
	vec3 tex_color = mix(tex_color_one, tex_color_two, colorTexMix);
	tex_color = mix(tex_color, diffuseColor, diffuseColorMix);

	vec3 lit_color = applyLight(tex_color, passNormal, passPosition);

	// Set to frag output color
	out_Color = vec4(lit_color,1.0);
}
