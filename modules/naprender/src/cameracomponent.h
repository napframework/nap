#pragma once

// External Includes
#include <glm/glm.hpp>
#include <nap/component.h>
#include <utility/dllexport.h>

namespace nap
{
	/**
	 * Base class for perspective and orthographic cameras.
	 */
	class NAPAPI CameraComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Default constructor
		CameraComponentInstance(EntityInstance& entity, Component& resource);

		/**
		* @return camera projection matrix
		* Use this matrix to transform a 3d scene in to a 2d projection
		*/
		virtual const glm::mat4& getProjectionMatrix() const = 0;

		/**
		 * Returns the view matrix of the camera
		 * The view is determined by a number of factors including the camera's position
		 * and possible look at objects
		 * @return The populated view matrix
		 */
		virtual const glm::mat4 getViewMatrix() const = 0;

		/**
		 * This is used by derived classes for extracting information like aspect ratio
		 * or the the size of the screen, in case of orthographic camera's (where pixels coordinates
		 * can be used).
		 * @param size: size of the render target in pixels.
		 */
		virtual void setRenderTargetSize(glm::ivec2 size) { mRenderTargetSize = size; }


		/**
		 * @return RenderTarget size
		 */
		glm::ivec2 getRenderTargetSize() const { return mRenderTargetSize; }

	private:
		glm::ivec2	mRenderTargetSize;			// The size of the render target we're rendering to

	};
}
