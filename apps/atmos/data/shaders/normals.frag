#version 330

// input  
in float passTip;			// If the fragment is a tip or not
in vec3 passUVs0;			// The unwrapped normalized texture
in vec3 passUVs1;			// The polar unwrapped texture
in vec3 passNormal;			//< frag normal in world space
in vec3 passPosition;		//< frag position in world space

// output
out vec4 out_Color;

// Uniform used for setting color
uniform float opacity;			//< Opacity

// shared uniforms
uniform sampler2D	colorTextureOne;
uniform sampler2D	colorTextureTwo;
uniform sampler2D	videoTextureOne;
uniform float		colorTexScaleOne;
uniform float		colorTexScaleTwo;
uniform float		videoTexScaleOne;
uniform float		colorTexMix;
uniform vec3		diffuseColor;
uniform float		diffuseColorMix;
uniform float		videoColorMix;
uniform vec3 		cameraPosition;							//< Camera World Space Position
uniform vec3		lightPos;		
uniform float 		lightIntensity;		
uniform float 		ambientIntensity;
uniform float		preMultiplyTexValue;
uniform float		diffuseIntensity;
uniform vec3		fogColor;
uniform float		fogMin;
uniform float		fogMax;
uniform float		fogInfluence;
uniform float		fogPower;

// Unshared uniforms		
uniform float 		specularIntensity;		
uniform vec3  		specularColor;
uniform float 		shininess;
uniform float		diffuseSpecularInfluence;
uniform float		textureTimeU;
uniform float		textureTimeV;
uniform float		videoTimeU;
uniform float		videoTimeV;

float fit(float value, float min, float max, float outMin, float outMax)
{
  float v = clamp(value, min, max);
  float m = max - min;
  if(m==0.0)
    m = 0.00000001;
  return (v - min) / (m) * (outMax - outMin) + outMin;
}

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
	//calculate the vector from this pixels surface to the light source
	vec3 surfaceToLight = normalize(lightPos - position);

	// calculate vector that defines the distance from camera to the surface
	vec3 surfaceToCamera = normalize(cameraPosition - position);

	// Ambient color
	vec3 ambient = color * ambientIntensity;

	// diffuse
    float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * color * lightIntensity * diffuseIntensity;

	// Calculate alternative normal for specular
    vec3 cam_normal = normalize(cameraPosition - position);
    vec3 cro_normal = normalize(cross(cam_normal, normal));
    float angle = acos(dot(normal, cro_normal));

	mat3 rotationMatrix = mat3(rotationMatrix(normal, angle));
	vec3 alt_normal = normalize(mix(rotationMatrix * cro_normal, normal, 0.00));

	// Scale specular based on vert color (greyscale)
	float spec_intensity = specularIntensity;

	// Compute specularf
    float specularCoefficient = pow(max(0.0, dot(normalize(reflect(-alt_normal, normal)), surfaceToCamera)), shininess);
    vec3 specular = specularCoefficient * specularColor * lightIntensity * spec_intensity;

    // Compensate using tip
    specular = specular * pow((1.0-passTip),1.0) * mix(1.0, pow(diffuseCoefficient,0.75), diffuseSpecularInfluence);

	//linear color (color before gamma correction)
    return diffuse + specular + ambient;
}

void main() 
{
	// Get color from texture
	vec2 tex_time = vec2(textureTimeU, textureTimeV);
	vec3 tex_color_one = texture(colorTextureOne, (passUVs0.xy  * colorTexScaleOne)).rgb;
	vec3 tex_color_two = texture(colorTextureTwo, ((passUVs1.xy * colorTexScaleTwo)+tex_time)).rgb;
	
	vec2 vid_time = vec2(videoTimeU, videoTimeV);
	vec3 vid_color_one = texture(videoTextureOne, ((passUVs1.xy  * videoTexScaleOne)+vid_time)).rgb;

	vec3 tex_color_mul = tex_color_one *  tex_color_two;
	tex_color_one = mix(tex_color_one, tex_color_mul, preMultiplyTexValue);  

	// Mix into texture color
	vec3 tex_color = mix(tex_color_one, tex_color_two, colorTexMix);
	tex_color = mix(tex_color, vid_color_one, videoColorMix);
	tex_color = mix(tex_color, diffuseColor, diffuseColorMix);

	// Get shading
	vec3 lit_color = applyLight(tex_color, passNormal, passPosition);

	// Apply alpha based on tip interpolated tip value
	float v = fit(1.0-passTip, 0.9,1.0,1.0,0.0);

	// Fog blend value
	float fog_blend_v = smoothstep(fogMin, fogMax, pow(gl_FragCoord.z,fogPower));
	vec3 fog_color  = mix(lit_color, fogColor, fog_blend_v);
	lit_color = mix(lit_color, fog_color, fogInfluence);

	// Set output color
	out_Color = vec4(lit_color, opacity * v);
}
