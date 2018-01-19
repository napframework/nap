#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< normal object space
in vec3 passVert;						//< vertex world space coordinate
in mat4 passModelMatrix;				//< Used model matrix
in vec4 passColor;						//< Vertex color

// uniform inputs
uniform sampler2D	videoTexture;
uniform vec3		cameraPosition;		//< world space camera position

// output
out vec4 out_Color;

const vec3  lightPostition = vec3(0.0,3.0,5.0);
const float lightIntensity = 1.0;
const float specularIntensity = 0.5;
const vec3  haloColor = vec3(0.545, 0.549, 0.627);
const vec3  specularColor = vec3(0.545, 0.549, 0.627);
const float shininess = 4;
const float ambientIntensity = 0.45f;
const vec3	colorOne = vec3(0.066, 0.078, 0.149);
const vec3	colorTwo = vec3(0.545, 0.549, 0.627);
const vec3 	colorThr = vec3(0.784, 0.411, 0.411);
const vec3	colorFor = vec3(0.176, 0.180, 0.2588);



void main() 
{
	//calculate the vector from this pixels surface to the light source
	vec3 surfaceToLight = normalize(lightPostition - passVert);

	// calculate vector that defines the distance from camera to the surface
	vec3 surfaceToCamera = normalize(cameraPosition - passVert);

	//calculate normal in world coordinates
    mat3 normal_matrix = transpose(inverse(mat3(passModelMatrix)));
    vec3 ws_normal = normalize(normal_matrix * passNormal);

	// Get texture rgba value
	vec3 tex_color = texture(videoTexture, passUVs.xy).rgb;

	// Get both mix colors
	vec3 ver_color = passColor.rgb;
	float ver_greyscale = (ver_color.r + ver_color.g + ver_color.b) / 3.0;
	ver_color = mix(colorFor, colorTwo, pow(ver_greyscale,2.0));
	vec3 ver_color_one = mix(ver_color, colorThr, 0.5);
	vec3 ver_color_two = mix(ver_color, colorFor, 0.75);

	// Get textured color
	float greyscale = (tex_color.r + tex_color.g + tex_color.b) / 3.0; 

	// Mix vertex with texture color
	tex_color = mix(ver_color_one, ver_color_two, pow(greyscale,1.0));

	// Ambient color
	vec3 ambient = tex_color.rgb * ambientIntensity;

	// diffuse
    float diffuseCoefficient = max(0.0, dot(ws_normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * tex_color.rgb * lightIntensity;

	// Scale specular based on vert color (greyscale)
	float spec_intensity = min(specularIntensity + (pow(ver_greyscale,2) / 2.0),1.0);

	// Compute specularf
    float specularCoefficient = pow(max(0.0, dot(normalize(reflect(-surfaceToLight, ws_normal)), surfaceToCamera)), shininess);
    vec3 specular = specularCoefficient * specularColor * lightIntensity * spec_intensity;

	//linear color (color before gamma correction)
    vec3 linearColor = diffuse + specular + ambient;

	// Set fragment color output to be texture color
	out_Color =  vec4(linearColor, 0.1);
}