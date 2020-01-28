#pragma once

#include "nrendertarget.h"

namespace opengl
{
	/**
	* Render target that represents a backbuffer.
	*/
	class BackbufferRenderTarget : public RenderTarget
	{
	public:
		/**
		* @param size Size of the render target
		*/
		void setSize(const glm::ivec2& size) { mSize = size; }

		/**
		* @return size of render target.
		*/
		virtual const glm::ivec2 getSize() const override { return mSize; }

	private:
		glm::ivec2 mSize;
	};

} // opengl
