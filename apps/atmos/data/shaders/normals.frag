#version 330

// input  
in float passTip;			// If the fragment is a tip or not
in vec3 passUVs0;			// The unwrapped normalized texture
in vec3 passUVs1;			// The polar unwrapped texture
in vec3 passNormal;			//< frag normal in object space
in vec3 passPosition;		//< frag position in object space
in mat4 passModelMatrix;	//< modelMatrix

// output
out vec4 out_Color;

// Uniform used for setting color
uniform vec3 color;				//< Normal color
uniform float opacity;			//< Opacity
uniform float length;			//< Normal length

// uniforms
uniform sampler2D	colorTexture;
uniform float		colorTexScale;
uniform vec3 		cameraPosition;							//< Camera World Space Position

// constants
const vec3			lightPos = vec3(0,100,100);		
const float 		lightIntensity = 1.0;					
const float 		specularIntensity = 0.5;				
const vec3  		specularColor = vec3(1.0,1.0,1.0);
const float 		shininess = 20;
const float 		ambientIntensity = 0.5f;
const float			diffuseSpecularInfluence = 0.0;
const float			diffuseIntensity = 1.0;


mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}


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
	vec3 diffuse = diffuseCoefficient * diffuseIntensity * color * lightIntensity;

	// Calculate alternative normal for specular
    vec3 cam_normal = normalize(cameraPosition - ws_position);
    vec3 cro_normal = normalize(cross(cam_normal, ws_normal));
    float angle = acos(dot(ws_normal, cro_normal));

	mat3 rotationMatrix = mat3(rotationMatrix(ws_normal, angle));
	vec3 alt_normal = normalize(mix(rotationMatrix * cro_normal, ws_normal, 0.00));

	// Scale specular based on vert color (greyscale)
	float spec_intensity = specularIntensity;

	// Compute specularf
    float specularCoefficient = pow(max(0.0, dot(normalize(reflect(-alt_normal, ws_normal)), surfaceToCamera)), shininess);
    vec3 specular = specularCoefficient * specularColor * lightIntensity * spec_intensity;

    // Compensate using tip
    specular = specular * pow((1.0-passTip),1.0) * mix(1.0, pow(diffuseCoefficient,0.75), diffuseSpecularInfluence);

	//linear color (color before gamma correction)
    return diffuse + specular + ambient;
}

void main() 
{
	// Get color from texture
	vec3 tex_color = texture(colorTexture, (passUVs0.xy * colorTexScale)).rgb;
	tex_color = mix(tex_color, vec3(0.0,0.0,0.0), 0.0);

	// Get shading
	vec3 lit_color = applyLight(tex_color, passNormal, passPosition);

	// Apply alpha based on tip interpolated tip value
	float v = 1.0-clamp(passTip, 0.0, 1.0);
	float m = length;
	v =  pow(1.0 - (v / (m) * 1.0),0.5);

	// Set output color
	out_Color = vec4(lit_color, opacity * v);
}
