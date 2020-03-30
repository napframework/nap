#pragma once

#include "utility/dllexport.h"
#include "glm/glm.hpp"

namespace nap
{
	/**
	 * 
	 */
	class NAPAPI IRenderTarget
	{
	public:
		virtual void beginRendering() = 0;
		virtual void endRendering() = 0;
		virtual const glm::ivec2 getSize() const = 0;
		virtual void setClearColor(const glm::vec4& color) = 0;
		virtual const glm::vec4& getClearColor() const = 0;
	};
}
