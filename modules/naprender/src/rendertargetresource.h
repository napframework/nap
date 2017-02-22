#pragma once

// Local Includes
#include "renderattributes.h"

// External Includes
#include <nap/resource.h>
#include <nframebuffer.h>

namespace nap
{
	/**
	 * Represents a render target
	 * Render target can be any type of frame-buffer and are used
	 * to render objects off screen in to their own buffer
	 */
	class RenderTargetResource : public Resource
	{
		RTTI_ENABLE_DERIVED_FROM(Resource)
	public:
		/**
		 * Virtual override to be implemented by derived classes
		 */
		virtual const opengl::FramebufferBase& getTarget() const = 0;

		/**
		 * Non const framebuffer accessor
		 */
		opengl::FramebufferBase& getTarget();

		/**
		 * Binds the framebuffer
		 */
		virtual bool bind()				{ return getTarget().bind(); };

		/**
		 * Unbinds the framebuffer
		 */
		virtual bool unbind()			{ return getTarget().unbind(); }
	};


	/**
	 * Frame buffer specialization of the render target resource
	 * Wraps an opengl frame buffer (RGBA + DEPTH)
	 */
	class FrameBufferResource : public RenderTargetResource
	{
		RTTI_ENABLE_DERIVED_FROM(RenderTargetResource)
	public:
		/**
		 * Default constructor
		 */
		FrameBufferResource();

		/**
		* @return opengl base frame buffer object
		* Note that this implicitly initializes the frame buffer
		*/
		virtual const opengl::FramebufferBase& getTarget() const override;

		/**
		* @return human readable display name
		*/
		virtual std::string getDisplayName() const override;

		/**
		 * Holds the buffer size
		 */
		Attribute<glm::ivec2> size =	{ this, "width", {512, 512} };

	private:
		// Framebuffer to draw to
		mutable opengl::FrameBuffer mFrameBuffer;

		// If the framebuffer has been loaded
		mutable bool mLoaded = false;

		/**
		 * Called when the frame buffer size changed, re-allocates resources
		 */
		void onDimensionsChanged(AttributeBase& attr)		{ mDirty = true; }
		
		// Slot
		nap::Slot<AttributeBase&> dimensionsChanged =		{this, &FrameBufferResource::onDimensionsChanged };

		// Dirty flag, used to change framebuffer dimensions
		mutable bool mDirty = true;
	};
}

RTTI_DECLARE_BASE(nap::RenderTargetResource)
RTTI_DECLARE(nap::FrameBufferResource)