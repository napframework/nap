#version 330
#define PI 3.1415926535897932384626433832795

// uniforms
uniform vec3 uCenter;
uniform float uBeat;
uniform int uWaveCount;
uniform float uWaveLength;
uniform float uWaveFalloffStart;
uniform float uWaveFalloffEnd;
uniform float uWaveNoiseZ;
uniform float uWaveNoiseScale;
uniform float uWaveNoiseInfluence;
uniform float uWaveCenter;
uniform float uWaveHighlight;

// input
in vec3 passUVs;

// output
out vec4 out_Color;

mat3 translate(vec2 v)
{
  return mat3(
    1.0, 0.0, v.x,
    0.0, 1.0, v.y,
    0.0, 0.0, 1.0
  );
}

mat3 scale(vec2 scale)
{
  return mat3(
    scale.x, 0.0, 0.0,
    0.0, scale.y, 0.0,
    0.0, 0.0, 1.0
  );
}

// Simplex 3D noise: https://github.com/hughsk/glsl-noise/blob/master/simplex/3d.glsl

vec3 mod289(vec3 x)
{
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x)
{
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x)
{
	return mod289(((x * 34.0) + 1.0) * x);
}

vec4 taylorInvSqrt(vec4 r)
{
	return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
{
	const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0);
	const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

	// first corner
	vec3 i = floor(v + dot(v, C.yyy));
	vec3 x0 = v - i + dot(i, C.xxx);

	// other corners
	vec3 g = step(x0.yzx, x0.xyz);
	vec3 l = 1.0 - g;
	vec3 i1 = min(g.xyz, l.zxy);
	vec3 i2 = max(g.xyz, l.zxy);
	vec3 x1 = x0 - i1 + C.xxx;
	vec3 x2 = x0 - i2 + C.yyy;
	vec3 x3 = x0 - D.yyy;

	// permutations
	i = mod289(i);
	vec4 p = permute(permute(permute(i.z + vec4(0.0, i1.z, i2.z, 1.0)) + i.y + vec4(0.0, i1.y, i2.y, 1.0)) + i.x + vec4(0.0, i1.x, i2.x, 1.0));
	float n_ = 0.142857142857;
	vec3 ns = n_ * D.wyz - D.xzx;
	vec4 j = p - 49.0 * floor(p * ns.z * ns.z);
	vec4 x_ = floor(j * ns.z);
	vec4 y_ = floor(j - 7.0 * x_ );
	vec4 x = x_ *ns.x + ns.yyyy;
	vec4 y = y_ *ns.x + ns.yyyy;
	vec4 h = 1.0 - abs(x) - abs(y);
	vec4 b0 = vec4( x.xy, y.xy );
	vec4 b1 = vec4( x.zw, y.zw );
	vec4 s0 = floor(b0) * 2.0 + 1.0;
	vec4 s1 = floor(b1) * 2.0 + 1.0;
	vec4 sh = -step(h, vec4(0.0));
	vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy ;
	vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww ;
	vec3 p0 = vec3(a0.xy, h.x);
	vec3 p1 = vec3(a0.zw, h.y);
	vec3 p2 = vec3(a1.xy, h.z);
	vec3 p3 = vec3(a1.zw, h.w);

	// normalise gradients
	vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

	// mix final noise value
	vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
	m = m * m;
	return 42.0 * dot(m * m, vec4(dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3)));
}

void main()
{
	vec2 center = vec2(0.5, 0.5);
	mat3 toCenter = translate(-center);
	mat3 fromCenter = translate(center);
	vec2 centerDelta = passUVs.xy - uCenter.xy;
	float centerDistance = length(centerDelta);

	// waves
	mat3 waveNoiseScale = scale(1.0 / vec2(uWaveNoiseScale, uWaveNoiseScale));
	vec3 waveNoisePosition = vec3(passUVs.xy, 1.0) * toCenter * waveNoiseScale * fromCenter;
	float waveNoise = snoise(vec3(waveNoisePosition.xy, uWaveNoiseZ));
	float waveCenterDistance = centerDistance + waveNoise * uWaveNoiseInfluence;
	float waveSize = 1.0 / uWaveCount;
	float nearestWave = uBeat / uWaveCount;
	while (nearestWave < waveCenterDistance) {
		nearestWave += waveSize;
	}
	float nearestWaveInfluence = 1.0 - ((nearestWave - waveCenterDistance) / waveSize / uWaveLength);
	if (nearestWaveInfluence > 0.0) {
		float nearestWaveX = nearestWaveInfluence > uWaveCenter
			? ((nearestWaveInfluence - uWaveCenter) / (1.0 - uWaveCenter)) * PI
			: (1.0 - nearestWaveInfluence / uWaveCenter) * -PI;
		nearestWaveInfluence = (1.0 + cos(nearestWaveX)) * 0.5;
	}
	float waveFalloffX = clamp((waveCenterDistance - uWaveFalloffStart) / (uWaveFalloffEnd - uWaveFalloffStart), 0.0, 1.0) * PI;
	float waveFalloffInfluence = (1.0 + cos(waveFalloffX)) * 0.5;
	float waveInfluence = nearestWaveInfluence * waveFalloffInfluence;
	float waveHighlightX = clamp((waveInfluence - (1.0 - uWaveHighlight)) / uWaveHighlight, 0.0, 1.0) * PI;
	float waveHighlightCurve = (1.0 + cos(waveHighlightX)) * 0.5;
	float waveWarm = waveHighlightCurve * waveInfluence;
	float waveCold = (1.0 - waveHighlightCurve) * waveInfluence;

	// set fragment color
	out_Color = vec4(waveWarm, waveCold, 0.0, 1.0);
}
