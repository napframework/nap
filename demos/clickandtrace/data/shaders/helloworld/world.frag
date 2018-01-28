#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< frag normal in world space
in vec3 passPosition;					//< frag world space position 

// uniform inputs
uniform sampler2D inWorldTexture;		//< World Texture
uniform vec3 inCameraPosition;			//< Camera World Space Position
uniform vec3 inClickPosition;			//< Click position in uv space
uniform float inTime;					//< Modulation time

// output
out vec4 out_Color;

float fit(float value, float inMin, float inMax, float outMin, float outMax)
{
	float v = clamp(value, inMin, inMax);
	float m = inMax - inMin;
	if(m == 0.0)
		m = 0.00001;
	return (v - inMin) / (m) * (outMax - outMin) + outMin;
}

void main() 
{
	// Cast click position and get distance
	vec2 click_pos = inClickPosition.xy;
	float click_distance = distance(click_pos, passUVs.xy);

	float falloff = fit(click_distance, 0.1, 0.2, 1.0, 0.0);

	float sin_color = (sin((click_distance*200.0)+ ((inTime * 4.0) * -1.0)) + 1.0) / 2.0;
	sin_color = sin_color * falloff;

	// Use texture alpha to blend between two colors
	float alpha = texture(inWorldTexture, passUVs.xy).a;
	vec3 color = vec3(sin_color,sin_color,sin_color);

	// Calculate mesh to camera angle for halo effect
	vec3 cam_normal = normalize(inCameraPosition - passPosition);

	// Dot product gives us the 'angle' between the surface and cam vector
	// The result is that normals pointing away from the camera at an angle of 90* are getting a higer value
	// Normals pointing towards the camera (directly) get a value of 0
	float cam_surface_dot = clamp(dot(normalize(passNormal), cam_normal),0.0,1.0);
	cam_surface_dot = clamp((1.0-cam_surface_dot) + 0.1, 0, 1);
	cam_surface_dot = pow(cam_surface_dot, 5.0);

	// Mix in the halo
	color = mix(color, vec3(0.545, 0.549, 0.627), cam_surface_dot);

	// Set fragment color output
	out_Color =  vec4(color,1.0);
}
