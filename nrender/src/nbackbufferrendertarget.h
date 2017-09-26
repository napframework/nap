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
		Backbuffer is always valid. Returns true.
		*/
		virtual bool isValid() override { return true; }

		/**
		* Binds the framebuffer so it can be used by subsequent render calls
		*/
		virtual bool bind() override;

		/**
		* NOP. TODO: decide how to proceed with unbinding render targets
		*/
		virtual bool unbind() override { return true; }

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
