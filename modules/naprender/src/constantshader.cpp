// Local includes
#include "constantshader.h"

// External includes
#include <nap/core.h>

// nap::VideoShader run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ConstantShader)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////
// Font Vertex Shader
//////////////////////////////////////////////////////////////////////////

static const char constantVertShader[] = R"glslang(
#version 450 core
uniform nap
{
	uniform mat4 projectionMatrix;
	uniform mat4 viewMatrix;
	uniform mat4 modelMatrix;
} mvp;

in vec3	in_Position;

void main(void)
{
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * mvp.modelMatrix * vec4(in_Position, 1.0);
}
)glslang";


//////////////////////////////////////////////////////////////////////////
// Font Fragment Shader
//////////////////////////////////////////////////////////////////////////

static const char constantFragShader[] = R"glslang(
#version 450 core
in vec3 passUVs;
uniform sampler2D glyph;

uniform UBO
{
	uniform vec3 color;
	uniform float alpha;
} ubo;

out vec4 out_Color;
void main() 
{
    out_Color = vec4(ubo.color, ubo.alpha);
}
)glslang";


//////////////////////////////////////////////////////////////////////////
// ConstantShader
//////////////////////////////////////////////////////////////////////////

namespace nap
{
	ConstantShader::ConstantShader(Core& core) : Shader(core) { }


	bool ConstantShader::init(utility::ErrorState& errorState)
	{
		// Number of characters = number of bytes minus null termination character of string literal.
		auto vert_size = sizeof(constantVertShader) - 1;
		auto frag_size = sizeof(constantFragShader) - 1;
		return load("ConstantShader", constantVertShader, vert_size, constantFragShader, frag_size, errorState);
	}
}