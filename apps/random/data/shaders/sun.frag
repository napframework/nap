#version 330

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passPosition;					//< frag world space position

// uniforms
uniform vec3 uOrbitCenter;
uniform float uOuterSize;
uniform float uInnerSize;
uniform float uStretch;

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

mat3 rotate(float angle)
{
  return mat3(
    cos(angle), -sin(angle), 0.0,
    sin(angle), cos(angle), 0.0,
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

void main()
{
	vec2 sunCenter = vec2(0.5, 0.5);
	vec2 sunToOrbit = uOrbitCenter.xy - sunCenter;

	mat3 toSunCenter = translate(-sunCenter);
	mat3 fromSunCenter = translate(sunCenter);

	mat3 rotation = rotate(-atan(sunToOrbit.y / sunToOrbit.x));
	mat3 scaling = scale(vec2(1.0 / uStretch, 1.0));
	vec3 position = vec3(passUVs.xy, 1.0) * toSunCenter * rotation * scaling * fromSunCenter;

	vec2 offset = position.xy - sunCenter;
	float innerSize = uInnerSize * uOuterSize;
	float intensity = 1.0 - clamp((length(offset) - innerSize) / (uOuterSize - innerSize), 0.0, 1.0);
	out_Color =  vec4(0.0, intensity, 0.0, 1.0);
}
