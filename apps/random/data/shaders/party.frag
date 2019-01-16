#version 330
#define PI 3.1415926535897932384626433832795

// uniforms
uniform vec3 uCenter;
uniform float uBeat;
uniform int uWaveCount;
uniform float uWaveLength;
uniform float uWaveFalloffStart;
uniform float uWaveFalloffEnd;

// input
in vec3 passUVs;

// output
out vec4 out_Color;

void main()
{
	vec2 centerDelta = passUVs.xy - uCenter.xy;
	float centerDistance = length(centerDelta);

	// waves
	float waveSize = 1.0 / uWaveCount;
	float nearestWave = uBeat / uWaveCount;
	while (nearestWave < centerDistance) {
		nearestWave += waveSize;
	}
	float nearestWaveInfluence = 1.0 - ((nearestWave - centerDistance) / waveSize / uWaveLength);
	float waveFalloffInfluence = 1.0 - clamp((centerDistance - uWaveFalloffStart) / (uWaveFalloffEnd - uWaveFalloffStart), 0.0, 1.0);
	float wave = nearestWaveInfluence * waveFalloffInfluence;


	// set fragment color
	out_Color = vec4(wave, 0.0, 0.0, 1.0);
}
