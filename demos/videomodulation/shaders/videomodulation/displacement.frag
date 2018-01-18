#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< normal object space
in vec3 passVert;						//< vertex world space coordinate
in mat4 passModelMatrix;				//< Used model matrix

// uniform inputs
uniform sampler2D	videoTexture;
uniform vec3		cameraPosition;		//< world space camera position

// output
out vec4 out_Color;

const vec3  lightPostition = vec3(0.0,3.0,5.0);
const float lightIntensity = 1.0;
const float specularIntensity = 0.25;
const vec3  haloColor = vec3(0.545, 0.549, 0.627);
const vec3  specularColor = vec3(0.545, 0.549, 0.627);
const float shininess = 3.5;
const float ambientIntensity = 0.5;
const vec3	colorOne = vec3(0.066, 0.078, 0.149);
const vec3	colorTwo = vec3(0.545, 0.549, 0.627);



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

	float greyscale = (tex_color.r + tex_color.g + tex_color.b) / 3.0; 
	tex_color = mix(colorOne, colorTwo, greyscale);

	// Ambient color
	vec3 ambient = tex_color.rgb * ambientIntensity;

	// diffuse
    float diffuseCoefficient = max(0.0, dot(ws_normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * tex_color.rgb * lightIntensity;

	float specularCoefficient = 0.0;
	float spec_weight = 1.0 - min(specularIntensity,1.0);

    specularCoefficient = pow(max(0.0, dot(normalize(reflect(-surfaceToLight, ws_normal)), surfaceToCamera)), shininess);
    vec3 specular = specularCoefficient * specularColor * lightIntensity * (specularIntensity + (spec_weight * greyscale));

	//linear color (color before gamma correction)
    vec3 linearColor = diffuse + specular + ambient;

	// Dot product gives us the 'angle' between the surface and cam vector
	// The result is that normals pointing away from the camera at an angle of 90* are getting a higer value
	// Normals pointing towards the camera (directly) get a value of 0
	float cam_surface_dot = clamp(dot(normalize(ws_normal), surfaceToCamera),0.0,1.0);
	cam_surface_dot = clamp((1.0-cam_surface_dot) + 0.1, 0, 1);
	cam_surface_dot = pow(cam_surface_dot, 5.0);

	// Mix in the halo
	tex_color = mix(linearColor, haloColor, cam_surface_dot);

	// Set fragment color output to be texture color
	out_Color =  vec4(tex_color, 0.1);
}