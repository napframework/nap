/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <glm/glm.hpp>
#include <component.h>
#include <utility/dllexport.h>
#include <renderwindow.h>
#include <rect.h>

namespace nap
{
	// Forward declares
	class CameraComponentInstance;


	/**
	 * Base class for perspective and orthographic camera resource.
	 */
	class NAPAPI CameraComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(CameraComponent, CameraComponentInstance)
	public:
		/**
		 * The perspective camera needs on a transform to calculate it's view matrix
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


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
		 * Returns the matrix that is used to transform a 3d scene in to a 2d projection.
		 * @return default camera projection matrix
		 */
		virtual const glm::mat4& getProjectionMatrix() const = 0;

		/**
		 * Returns the world space position and rotation of the camera in 3D space.
		 * @return camera view matrix
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
		 * The z component is generally acquired by sampling the window depth buffer.
		 * When screenPos.z has a value of 0 the result is relative to this camera, ie: 
		 * the return value is the location of this camera in the world.
		 * @param screenPos window coordinates, where 0,0 is the lower left corner
		 * @param viewport rectangle that defines the viewport
		 * @return converted screen to world space pixel coordinate
		 */
		glm::vec3 screenToWorld(const glm::vec3& screenPos, const math::Rect& viewport);

		/**
		 * Maps a world space coordinate to a screen space coordinate using the camera projection and view matrix
		 * @param worldPos the point position in world space
		 * @param viewport rectangle that defines the viewport
		 * @return the converted world to screen space coordinate
		 */
		glm::vec3 worldToScreen(const glm::vec3& worldPos, const math::Rect& viewport);

		/**
		 * Computes a ray directed outwards from the camera based on a screen space position.
		 * The ray is normalized.
		 * @param screenPos horizontal and vertical screen coordinates: where 0, 0 is the lower left corner
		 * @param viewport rectangle that defines the viewport
		 * @return a normalized ray pointing outwards from the camera in to the scene
		 */
		glm::vec3 rayFromScreen(const glm::vec2& screenPos, const math::Rect& viewport);

		/**
		 * @return RenderTarget size
		 */
		const glm::ivec2& getRenderTargetSize() const					{ return mRenderTargetSize; }

		/**
		 * @return the near clipping plane of the camera
		 */
		virtual float getNearClippingPlane() const = 0;

		/**
		 * @return the far clipping plane of the camera
		 */
		virtual float getFarClippingPlane() const = 0;

		/**
		 * Returns the matrix that is used to transform a 3d scene in to a 2d projection by the renderer.
		 * This can be different from the default if the renderer uses a different coordinate system.
		 * Use this matrix to transform a 3d scene in to a 2d projection.
		 * @return the projection matrix used by the renderer
		 */
		virtual const glm::mat4& getRenderProjectionMatrix() const = 0;

	private:
		glm::ivec2	mRenderTargetSize = { 1, 1 };						// The size of the render target we're rendering to
	};
}
