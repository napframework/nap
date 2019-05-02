#pragma once

#include <parameterenum.h>
#include <nglutils.h>

namespace nap
{
	enum class EControlMethod : uint8_t
	{
		Orbit = 0,
		FirstPerson = 1
	};

	using ParameterControlMethod = ParameterEnum<EControlMethod>;
	using ParameterPolygonMode = ParameterEnum<opengl::EPolygonMode>;
}