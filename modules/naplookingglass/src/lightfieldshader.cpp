// Local includes
#include "lightfieldshader.h"

// External includes
#include <nap/core.h>

// Third-party includes
// The third-party lightfield shaders cannot be used right out of the headers as Vulkan requires
// uniforms to be enclosed inside UBO declarations.
#include <HoloPlayShaders.h>

// nap::LightFieldShader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LightFieldShader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// Constant LightFieldShader
//////////////////////////////////////////////////////////////////////////

static const char sLightfieldVertShaderGLSL[] = R"glslang(
#version 450

out vec2 texCoords;

void main()
{
	texCoords = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(texCoords * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 0.0, 1.0);
})glslang";

static const char sLightfieldFragShaderGLSL[] = R"glslang(
#version 450


in vec2 texCoords;
out vec4 fragColor;

// Calibration values

uniform UBO
{
	float pitch;
	float tilt;
	float center;
	int invView;
	float subp;
	float displayAspect;
	int ri;
	int bi;

// Quilt settings

	vec3 tile;
	vec2 viewPortion;
	float quiltAspect;
	int overscan;
	int quiltInvert;
	
	int debug;
};

uniform sampler2D screenTex;

vec2 texArr(vec3 uvz)
{
	// decide which section to take from based on the z.
	float x = (mod(uvz.z, tile.x) + uvz.x) / tile.x;
	float y = (floor(uvz.z / tile.x) + uvz.y) / tile.y;
	return vec2(x, y) * viewPortion.xy;
}

// recreate CG clip function (clear pixel if any component is negative)
void clip(vec3 toclip)
{
	if (any(lessThan(toclip, vec3(0,0,0)))) discard;
}

void main()
{
	if (debug == 1)
	{
		fragColor = texture(screenTex, texCoords.xy);
	}
	else {
		float invert = 1.0;
		if (invView + quiltInvert == 1) invert = -1.0;
		vec3 nuv = vec3(texCoords.xy, 0.0);
		nuv -= 0.5;
		float modx = clamp (step(quiltAspect, displayAspect) * step(float(overscan), 0.5) + step(displayAspect, quiltAspect) * step(0.5, float(overscan)), 0, 1);
		nuv.x = modx * nuv.x * displayAspect / quiltAspect + (1.0-modx) * nuv.x;
		nuv.y = modx * nuv.y + (1.0-modx) * nuv.y * quiltAspect / displayAspect; 
		nuv += 0.5;
		clip (nuv);
		clip (1.0-nuv);
		vec4 rgb[3];
		for (int i=0; i < 3; i++)
		{
			nuv.z = (texCoords.x + i * subp + texCoords.y * tilt) * pitch - center;
			nuv.z = mod(nuv.z + ceil(abs(nuv.z)), 1.0);
			nuv.z *= invert;
			nuv.z *= tile.z;
			vec3 coords1 = nuv;
			vec3 coords2 = nuv;
			coords1.y = coords2.y = clamp(nuv.y, 0.005, 0.995);
			coords1.z = floor(nuv.z);
			coords2.z = ceil(nuv.z);
			vec4 col1 = texture(screenTex, texArr(coords1));
			vec4 col2 = texture(screenTex, texArr(coords2));
			rgb[i] = mix(col1, col2, nuv.z - coords1.z);
		}
		fragColor = vec4(rgb[ri].r, rgb[1].g, rgb[bi].b, 1.0);
	}
})glslang";


//////////////////////////////////////////////////////////////////////////
// LightFieldShader
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	LightFieldShader::LightFieldShader(Core& core) : Shader(core) { }


	bool LightFieldShader::init(utility::ErrorState& errorState)
	{
		// Number of characters = number of bytes minus null termination character of string literal.
		auto vert_size = sizeof(sLightfieldVertShaderGLSL) - 1;
		auto frag_size = sizeof(sLightfieldFragShaderGLSL) - 1;
		return load("lightfieldshader", sLightfieldVertShaderGLSL, vert_size, sLightfieldFragShaderGLSL, frag_size, errorState);
	}
}
