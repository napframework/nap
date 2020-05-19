#version 450 core

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< frag normal in object space
in vec3 passPosition;					//< frag position in object space
in mat4 passModelMatrix;				//< modelMatrix

// Point light structure
struct PointLight
{
	vec3		mPosition;
	float 		mIntensity;
};

// uniform inputs
uniform UBOFrag
{
	uniform vec3 		inCameraPosition;		//< Camera World Space Position
	uniform vec3 		inBlobPosition;			//< Blob position in uv space
	uniform float 		inTime;					//< Modulation time
	uniform float 		inVelocity;				//< Velocity used for modulating frequency
	uniform vec3 		inMousePosition;		//< Current mouse position in uv space
	uniform PointLight	inLight;				//< Light
} ubofrag;

// output
out vec4 out_Color;

const float		minDistance = 0.4;
const float		maxDistance = 0.1;
const float		speed = 0.005;
const float		fade = 0.75;
const float		minFrequency = 499.8;
const float		maxFrequency = 500;
const float		minDistribution = 2.25;
const float		maxDistribution = 4.0;
const float 	specularIntensity = 0.5;
const vec3  	specularColor = vec3(0.545, 0.549, 0.627);
const float 	shininess = 10;
const float 	ambientIntensity = 0.5f;
const vec3		colorTwo = vec3(0.066, 0.078, 0.149);
const vec3		colorOne = vec3(0.784, 0.411, 0.411);
const vec3		colorThr = vec3(0.176, 0.180, 0.258);
const vec3		colorFor = vec3(0.321, 0.329, 0.415);
const float		uvOffset = 0.015;

// Maps a value to a new range
float fit(float value, float inMin, float inMax, float outMin, float outMax, bool doClamp)
{
	float v = value;
	if(doClamp)
		v = clamp(v, inMin, inMax);

	float m = inMax - inMin;
	if(m == 0.0)
		m = 0.00001;
	return (v - inMin) / (m) * (outMax - outMin) + outMin;
}


// Calculates blob displacement based on the blob position and movement speed
// This is just a fancy sine mnodulation function
float calculateDisplacement(vec2 uv)
{
	// Distance in uv space from click to frag
	float uv_dist = distance(ubofrag.inBlobPosition.xy, uv);
	float currentDistance = mix(minDistance, maxDistance, pow(ubofrag.inVelocity,0.75f));

	// Get mapped normalized value
	float uv_dist_norm = fit(uv_dist, 0.0, currentDistance, 0.0, 1.0, false);
	float distribution = mix(minDistribution, maxDistribution, ubofrag.inVelocity);
	uv_dist_norm = pow(uv_dist_norm, distribution);

	// Fit distribution based on distance
	float max_dist_weight = 1.0 / distribution;
	uv_dist_norm = fit(uv_dist_norm, 0.0, 1.0, 0.125, max_dist_weight, false);

	// Multiply distance with weighted freq scale over distance
	float weighted_dist = uv_dist * uv_dist_norm;

	// Apply phase
	weighted_dist += ((ubofrag.inTime * speed) * -1.0);

	// Apply freq
	weighted_dist *= mix(minFrequency, maxFrequency, ubofrag.inVelocity);
	
	// Get sin over distance
	float displacement_v = (sin(weighted_dist) + 1.0) / 2.0;

	// Get fade distance
	float min_fade = fade * currentDistance;
	min_fade = clamp(currentDistance - min_fade, 0, 1);
	float fade_mult = pow(fit(uv_dist, min_fade, currentDistance, 1, 0, true),1.33);

	// Multiply over displacement
	displacement_v *= fade_mult;

	return clamp(displacement_v,0,1);
}


// Computes the color of the mouse cursor based on distance of the mouse to the blob
float calculateMouseCursor(vec2 uv)
{
	// Distance in uv space from click to frag
	float uv_dist = distance(ubofrag.inMousePosition.xy, uv);

	// Fit to get a nice little ball in uv space where the mouse is
	float offset = 0.02 * fit(ubofrag.inVelocity, 0.0, 1.0, 1.0, 0.4, true);
	float mouse_value = fit(uv_dist, offset, offset+0.002, 1.0, 0.0, true);

	// Scale with distance to blob, when the blob is close to the mouse cursor
	// the cursor disappears
	float blob_dist = distance(ubofrag.inMousePosition.xy, ubofrag.inBlobPosition.xy);
	mouse_value *= pow(fit(blob_dist, 0.025, 0.1, 0, 1, true),2);

	return mouse_value;
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
	vec3 surfaceToLight = normalize(ubofrag.inLight.mPosition - ws_position);

	// calculate vector that defines the distance from camera to the surface
	vec3 surfaceToCamera = normalize(ubofrag.inCameraPosition - ws_position);

	// Ambient color
	vec3 ambient = color * ambientIntensity;

	// diffuse
    float diffuseCoefficient = max(0.0, dot(ws_normal, surfaceToLight));
	vec3 diffuse = diffuseCoefficient * color * ubofrag.inLight.mIntensity;

	// Scale specular based on vert color (greyscale)
	float spec_intensity = specularIntensity;

	// Compute specularf
    float specularCoefficient = pow(max(0.0, dot(normalize(reflect(-surfaceToLight, ws_normal)), surfaceToCamera)), shininess);
    vec3 specular = specularCoefficient * specularColor * ubofrag.inLight.mIntensity * spec_intensity;

	//linear color (color before gamma correction)
    return diffuse + specular + ambient;
}


// Computes a blob in uv space together with a mouse cursor
// Normals are faked because the blob is computed using a function
// We therefore can approximate the normals by taking multiple samples
void main()
{
	// Get the current position of the fragment in uv space
	vec2 uvpos_n = vec2(passUVs.x, passUVs.y);

	// Get neighbour fragments in uv space
	vec2 uvpos_x = vec2(passUVs.x+uvOffset, passUVs.y);
	vec2 uvpos_y = vec2(passUVs.x, passUVs.y+uvOffset);

	// Calculate displacement for all uv samples
	float sin_color = calculateDisplacement(uvpos_n);
	float sin_color_x = calculateDisplacement(uvpos_x);
	float sin_color_y = calculateDisplacement(uvpos_y);

	// Use the displacement information to compute a normal
	vec3 pos_x = vec3(uvpos_x.x, uvpos_x.y, sin_color_x * 0.066);
	vec3 pos_y = vec3(uvpos_y.x, uvpos_y.y, sin_color_y * 0.066);
	vec3 pos_n = vec3(uvpos_n.x, uvpos_n.y, sin_color * 0.066);

	// Calculate tangents
	vec3 tangent =   pos_x - pos_n;
	vec3 bitangent = pos_y - pos_n;

	// Calculate fake normal at frag coordinate
	vec3 normal = cross(tangent, bitangent);

	// Calculate edge
	float edge_x = abs((passUVs.x - 0.5) * 2.0);
	float edge_y = abs((passUVs.y - 0.5) * 2.0);
	edge_x = fit(edge_x, 0.985, 0.9875, 0.0, 1.0, true);
	edge_y = fit(edge_y, 0.985, 0.9875, 0.0, 1.0, true);
	float edge =  clamp(edge_x + edge_y, 0,1 );

	// Blend between two colors based on returned sin value
	vec3 color = mix(colorTwo, colorOne, sin_color);

	// Mix in cursor
	float cursor_v = calculateMouseCursor(passUVs.xy);
	color  = mix(color, colorOne, cursor_v);

	// Mix in border
	color  = mix(color,colorFor, edge);
	normal = mix(normal, passNormal,edge);

	// Apply lights and specular
	vec3 lit_color = applyLight(color, normal, passPosition);

	normal = normalize(normal);

	// Set fragment color output
	out_Color =  vec4(lit_color,1.0);
	//out_Color =  vec4(normal.x,normal.y,normal.z,1.0);
}
