#version 330
#define PI 3.1415926535897932384626433832795

// uniforms
uniform float uTime;
uniform vec3 uCenter;

// input
in vec3 passUVs;

// output
out vec4 out_Color;

void main()
{
	// waves
	float dCenter = distance(uCenter.xy, passUVs.xy);
	float wave = cos(uTime * 10 - dCenter * 100);

	// beams
	float beamAngle = -0.5 * PI;
	float beamWidth = 0.025;
	float fragAngle = atan(passUVs.y - uCenter.y, passUVs.x - uCenter.x);
	float beam = abs(beamAngle - fragAngle) > beamWidth ? 0.0 : 1.0;

	// set fragment color
	out_Color = vec4(wave, beam, 0.0, 1.0);
}
