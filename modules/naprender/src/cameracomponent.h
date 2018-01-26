#pragma once

// External Includes
#include <glm/glm.hpp>
#include <component.h>
#include <utility/dllexport.h>
#include <renderwindow.h>
#include <rect.h>

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
		virtual void setRenderTargetSize(const glm::ivec2& size)												{ mRenderTargetSize = size; }

		/**
		 * Maps a pixel coordinate to a world space coordinate using the camera project and view matrix
		 * The z component is generally acquired by sampling the window depth buffer
		 * When screenPos.z has a value of 0 the result is relative to this camera, ie: 
		 * the return value is the location of this camera in the world.
		 * @param screenPos window coordinates, where 0,0 is the lower left corner
		 * @param viewport rectangle that defines the viewport
		 * @return converted screen to world space pixel coordinate
		 */
		glm::vec3 screenToWorld(const glm::vec3& screenPos, const math::Rect& viewport);

		/**
		 * Maps a pixel coordinate to a world space coordinate using the camera projection and view matrix
		 * The viewport is extracted from the render target
		 * This call performs a depth sample in to the render target to acquire the z value of the screen position
		 * It's important that the window associated with the target is active. By default this is the first (primary window)
		 * @param screenPos horizontal and vertical screen coordinates, where 0,0 is the lower left corner
		 * @param target render target (window / texture target etc.) that defines the viewport
		 * @return converted screen to world space pixel coordinate
		 */
		glm::vec3 screenToWorld(const glm::vec2& screenPos, opengl::RenderTarget& target);

		/**
		 * Maps a world space coordinate to a screen space coordinate using the camera projection and view matrix
		 * @param worldPos the point position in world space
		 * @param viewport rectangle that defines the viewport
		 * @return the converted world to screen space coordinate
		 */
		glm::vec3 worldToScreen(const glm::vec3& worldPos, const math::Rect& viewport);

		/**
		* Maps a world space coordinate to a screen space coordinate using the camera projection and view matrix
		* @param worldPos the point position in world space
		* @param target render target (window / texture target etc.) that defines the viewport
		* @return the converted world to screen space coordinate
		*/
		glm::vec3 worldToScreen(const glm::vec3& worldPos, const opengl::RenderTarget& target);

		/**
		 * Computes a ray directed outwards from the camera based on a screen space position
		 * The ray is normalized
		 * @param screenPos, horizontal and vertical screen coordinates, where 0,0 is the lower left corner
		 * @param viewport rectangle that defines the viewport
		 * @return a normalized ray pointing outwards from the camera in to the scene
		 */
		glm::vec3 rayFromScreen(const glm::vec2& screenPos, const math::Rect& viewport);

		/**
		* Computes a ray directed outwards from the camera based on a screen space position
		* The ray is normalized
		* @param screenPos horizontal and vertical screen coordinates, where 0,0 is the lower left corner
		* @param target render target (window / texture target etc.) that defines the viewport
		* @return a ray pointing outwards from the camera in to the scene
		*/
		glm::vec3 rayFromScreen(const glm::vec2& screenPos, const opengl::RenderTarget& target);

		/**
		 * @return RenderTarget size
		 */
		const glm::ivec2& getRenderTargetSize() const					{ return mRenderTargetSize; }

	private:
		glm::ivec2	mRenderTargetSize;			// The size of the render target we're rendering to

	};
}
