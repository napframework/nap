#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< normal world space
in vec3 passVert;						//< vertex world space coordinate

// uniform inputs
uniform sampler2D	videoTexture;

// output
out vec4 out_Color;

void main() 
{
	// Get texture rgba value
	//vec3 tex_color = texture(videoTexture, passUVs.xy).rgb;
	vec3 tex_color = vec3(0,0,0);

	vec3 cameraPosition = vec3(0.0,0.0,5.0);

	// Calculate mesh to camera angle for halo effect
	vec3 cam_normal = normalize(cameraPosition - passVert);

	// Dot product gives us the 'angle' between the surface and cam vector
	// The result is that normals pointing away from the camera at an angle of 90* are getting a higer value
	// Normals pointing towards the camera (directly) get a value of 0
	float cam_surface_dot = clamp(dot(normalize(passNormal), cam_normal),0.0,1.0);
	cam_surface_dot = clamp((1.0-cam_surface_dot) + 0.1, 0, 1);
	cam_surface_dot = pow(cam_surface_dot, 5.0);

	// Mix in the halo
	tex_color = mix(vec3(0.0,0.0,0.0), vec3(0.545, 0.549, 0.627), cam_surface_dot);

	// Set fragment color output to be texture color
	out_Color =  vec4(tex_color, 1.0);
}