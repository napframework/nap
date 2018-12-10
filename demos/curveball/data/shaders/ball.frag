#version 330

// vertex shader input
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< frag normal in object space
in vec3 passPosition;					//< frag position in object space
in mat4 passModelMatrix;				//< modelMatrix

// uniform inputs
uniform vec3 	inCameraPosition;		//< Camera World Space Position

// output
out vec4 out_Color;

// uniforms
uniform sampler2D	mCanvas;
uniform vec3		mColor;


const vec3		lightPos = vec3(0.0, 20.0, 10.0);
const float 	lightIntensity = 1.0;
const float 	specularIntensity = 0.5;
const vec3  	specularColor = vec3(0.545, 0.549, 0.627);
const float 	shininess = 10;
const float 	ambientIntensity = 0.5f;
const vec3		colorTwo = vec3(0.066, 0.078, 0.149);
const vec3		colorOne = vec3(0.784, 0.411, 0.411);
const vec3		colorThr = vec3(0.176, 0.180, 0.258);
const vec3		colorFor = vec3(0.321, 0.329, 0.415);
const float		uvOffset = 0.015;

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
	vec3 surfaceToCamera = normalize(inCameraPosition - ws_position);

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
	vec2 uvs = vec2(passUVs.x, passUVs.y);
//	vec4 canvas_color = texture(mCanvas, uvs);
    vec3 col = vec3(1.0, 0.5, 0.0);
	vec3 output_color = applyLight(col, passNormal, passPosition);

	out_Color =  vec4(output_color.rgb, 1.0);
//    out_Color = output_color;
}
