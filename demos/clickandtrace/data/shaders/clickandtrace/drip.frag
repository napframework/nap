#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< frag normal in world space
in vec3 passPosition;					//< frag world space position 
in mat4 passModelMatrix;				//< modelMatrix

// uniform inputs
uniform vec3 inCameraPosition;			//< Camera World Space Position
uniform vec3 inClickPosition;			//< Click position in uv space
uniform float inTime;					//< Modulation time

// output
out vec4 out_Color;

const vec2		location = vec2(0.5, 0.5);
const float		maxDistance = 0.33;
const float		speed = 0.01	;
const float		fade = 0.1;
const float		frequency = 400;
const vec3		colorOne = vec3(0.0, 1, 0);
const vec3		colorTwo = vec3(1.0, 0.0, 0.0);
const float		distribution = 2.5;
const vec3		lightPos = vec3(0.0, 2.0, 1.0);

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

float calculateDisplacement(vec2 uv)
{
	// Distance in uv space from click to frag
	float uv_dist = distance(inClickPosition.xy, uv);

	// Get mapped normalized value
	float uv_dist_norm = fit(uv_dist, 0.0, maxDistance, 0.0, 1.0, false);
	uv_dist_norm = pow(uv_dist_norm, distribution);

	// Fit distribution based on distance
	float max_dist_weight = 1.0 / distribution;
	uv_dist_norm = fit(uv_dist_norm, 0.0, 1.0, 0.15, max_dist_weight, false);

	// Multiply distance with weighted freq scale over distance
	float weighted_dist = uv_dist * uv_dist_norm;

	// Apply phase
	weighted_dist += ((inTime * speed) * -1.0);

	// Apply freq
	weighted_dist *= frequency;
	
	// Get sin over distance
	float displacement_v = (sin(weighted_dist) + 1.0) / 2.0;

	// Get fade distance
	float min_fade = fade * maxDistance;
	min_fade = clamp(maxDistance - min_fade, 0, 1);
	float fade_mult = fit(uv_dist, min_fade, maxDistance, 1, 0, true);

	// Multiply over displacement
	displacement_v *= fade_mult;

	return displacement_v;

}


void main()
{
	// Cast click position and get distance
	vec2 uvpos_n = vec2(passUVs.x, passUVs.y);
	vec2 uvpos_x = vec2(passUVs.x+0.01, passUVs.y);
	vec2 uvpos_y = vec2(passUVs.x, passUVs.y+0.01);

	float sin_color = calculateDisplacement(uvpos_n);
	float sin_color_x = calculateDisplacement(uvpos_x);
	float sin_color_y = calculateDisplacement(uvpos_y);

	// Calculate displacement vector for both
	vec3 pos_x = vec3(uvpos_x.x, uvpos_x.y, sin_color_x * 0.2);
	vec3 pos_y = vec3(uvpos_y.x, uvpos_y.y, sin_color_y * 0.2);
	vec3 pos_n = vec3(uvpos_n.x, uvpos_n.y, sin_color * 0.2);

	// Calculate tangents
	vec3 tangent = normalize(pos_x - pos_n);
	vec3 bitangent = normalize(pos_y - pos_n);

	// Calculate fake normal
	vec3 normal = cross(tangent, bitangent);

	// Calculate normal to world
	mat3 normal_matrix = transpose(inverse(mat3(passModelMatrix)));
	normal = normalize(normal * normal_matrix);

	// Use texture alpha to blend between two colors
	vec3 color = mix(colorTwo, colorOne, sin_color);

	// Calculate mesh to camera angle for halo effect
	vec3 cam_normal = normalize(inCameraPosition - passPosition);

	// Dot product gives us the 'angle' between the surface and cam vector
	// The result is that normals pointing away from the camera at an angle of 90* are getting a higer value
	// Normals pointing towards the camera (directly) get a value of 0
	float cam_surface_dot = clamp(dot(normalize(normal), cam_normal),0.0,1.0);
	cam_surface_dot = clamp((1.0-cam_surface_dot) + 0.1, 0, 1);
	cam_surface_dot = pow(cam_surface_dot, 1.0);

	// Mix in the halo
	color = mix(color, vec3(0.545, 0.549, 0.627), cam_surface_dot);

	// Set fragment color output
	out_Color =  vec4(color,1.0);
}
