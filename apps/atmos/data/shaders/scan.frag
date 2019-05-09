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
uniform sampler2D	videoTextureOne;
uniform float		colorTexScaleOne;
uniform float		colorTexScaleTwo;
uniform float		videoTexScaleOne;
uniform float		colorTexMix;							//< Primage Color Texture Scale
uniform vec3		diffuseColor;
uniform float		diffuseColorMix;
uniform float		videoColorMix;
uniform vec3 		cameraPosition;							//< Camera World Space Position
uniform vec3		lightPos;		
uniform float 		lightIntensity;		
uniform float 		ambientIntensity;
uniform float		preMultiplyTexValue;
uniform float		diffuseIntensity;
uniform float		fogInfluence;
uniform vec3		fogColor;
uniform float		fogMin;
uniform float		fogMax;
uniform float		fogPower;
uniform float		textureTimeU;
uniform float		textureTimeV;
uniform float		videoTimeU;
uniform float		videoTimeV;
uniform float		videoContrast;
uniform float		videoMaskValue;
uniform vec3		maskColor;

// Unshared uniforms					
uniform float 		specularIntensity;			
uniform float       specularColorBlend;	
uniform vec3  		specularColor;
uniform float 		shininess; 


float toGreyscale(vec3 color)
{
	return (color.r * 0.2126) + (color.g * 0.7152) + (color.b * 0.0722);
}


float applyContrast(float value, float contrast)
{
	return clamp((value - 0.5) * contrast + 0.5,0,1);
}


float fit(float value, float min, float max, float outMin, float outMax)
{
  float v = clamp(value, min, max);
  float m = max - min;
  if(m==0.0)
    m = 0.00000001;
  return (v - min) / (m) * (outMax - outMin) + outMin;
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
	vec3 diffuse = diffuseCoefficient * color * lightIntensity * diffuseIntensity;

	// Scale specular based on vert color (greyscale)
	float spec_intensity = specularIntensity;

	// Compute specularf
	vec3 mix_spec_color = mix(specularColor, color, specularColorBlend);
    float specularCoefficient = pow(max(0.0, dot(normalize(reflect(-surfaceToLight, ws_normal)), surfaceToCamera)), shininess);
    vec3 specular = specularCoefficient * mix_spec_color * lightIntensity * spec_intensity;

	//linear color (color before gamma correction)
    return diffuse + specular + ambient;
}


void main() 
{	
	// Get color from texture
	vec2 tex_time = vec2(textureTimeU, textureTimeV);
	vec3 tex_color_one = texture(colorTextureOne, (passUVs0.xy * colorTexScaleOne)).rgb;
	vec3 tex_color_two = texture(colorTextureTwo, ((passUVs1.xy * colorTexScaleTwo)+tex_time)).rgb;

	// Get video color
	vec2 vid_time = vec2(videoTimeU, videoTimeV);
	vec3 vid_color_one = texture(videoTextureOne, ((passUVs1.xy  * videoTexScaleOne)+vid_time)).rgb;

	vec3 tex_color_mul = tex_color_one *  tex_color_two;
	tex_color_one = mix(tex_color_one, tex_color_mul, preMultiplyTexValue);  

	// First mix the two colors
	vec3 tex_color = mix(tex_color_one, tex_color_two, colorTexMix);

	// Mix in diffuse
	tex_color = mix(tex_color, diffuseColor, diffuseColorMix);

	// Mask current pixel value with video greyscale
	float vid_greyscale = fit(toGreyscale(vid_color_one),0.025,1.0,0.0,1.0);
	vid_greyscale = applyContrast(vid_greyscale, videoContrast);
	vec3 tex_color_masked = mix(maskColor, tex_color * vid_greyscale, vid_greyscale);
	tex_color = mix(tex_color, tex_color_masked, videoMaskValue);

	// Mix in video
	tex_color = mix(tex_color, vid_color_one, videoColorMix);

	// Apply light
	vec3 lit_color = applyLight(tex_color, passNormal, passPosition);

	// Apply fog
	float fog_blend_v = smoothstep(fogMin, fogMax, pow(gl_FragCoord.z,fogPower));
	vec3 fog_color  = mix(lit_color, fogColor, fog_blend_v);
	lit_color = mix(lit_color, fog_color, fogInfluence);

	// Set to frag output color
	out_Color = vec4(lit_color,1.0);
}
