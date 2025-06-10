/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <component.h>
#include <transformcomponent.h>
#include <inputevent.h>
#include <orthocameracomponent.h>
#include <texture.h>
#include <renderwindow.h>

namespace nap
{
	class ZoomPanControllerInstance;

	/**
	 * 2D texture zoom and pan camera controller.
	 *
	 * Allows for freely moving around and zooming into a 2D texture using an orthographic camera.
	 * Use the 'frameTexture' function to adjust the scale and position of your texture, ensuring it fits perfectly in the viewport.
	 *
	 * This component updates the projection and transform matrix of an orthographic camera, based on pointer input events.
	 * It therefore requires the following components to be present on the same entity.
	 *
	 * - a nap::Transform component
	 * - a nap::PointerInputComponent
	 * - a nap::OrthoCameraComponent
	 *
	 * After calling 'frameTexture':
	 *     - The texture is placed at a depth of (z)0.
	 *     - The camera is placed at (xy)0 at a depth of (z)5.
	 *     - The camera planes will match the size of the viewport.
	 *     - The near and far clipping planes are setup to capture the framed texture.
	 *
	 * Not calling 'frameTexture' allows you to use your own texture scale and orthographic camera settings.
	 * Note that for the component to work properly it can't have any parent transform, if it does the the result is undefined.
	 */
	class NAPAPI ZoomPanController : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ZoomPanController, ZoomPanControllerInstance)
	public:
		/**
		 * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		 * @param components the components this object depends on
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		nap::ResourcePtr<RenderWindow> mRenderWindow = nullptr;		///< Property: 'Window' The window that displays the texture
		float mZoomSpeed = 1.0f;									///< Property: "ZoomSpeed" The speed with which to zoom
	};


	/**
	 * 2D texture zoom and pan camera controller.
	 *
	 * Allows for freely moving around and zooming into a 2D texture using an orthographic camera.
	 * Use the 'frameTexture' function to adjust the scale and position of your texture, ensuring it fits perfectly in the viewport.
	 *
	 * This component updates the projection and transform matrix of an orthographic camera, based on pointer input events.
	 * It therefore requires the following components to be present on the same entity.
	 *
	 * - a nap::Transform component
	 * - a nap::PointerInputComponent
	 * - a nap::OrthoCameraComponent
	 *
	 * After calling 'frameTexture':
	 *     - The texture is placed at a depth of (z)0.
	 *     - The camera is placed at (xy)0 at a depth of (z)5.
	 *     - The camera planes will match the size of the viewport.
	 *     - The near and far clipping planes are setup to capture the framed texture.
	 *
	 * Not calling 'frameTexture' allows you to use your own texture scale and orthographic camera settings.
	 * Note that for the component to work properly it can't have any parent transform, if it does the the result is undefined.
	 */
	class NAPAPI ZoomPanControllerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		ZoomPanControllerInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initializes the component based on the resource.
		 * @param errorState the error message when initialization fails
		 * @return if the component initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update pancontrollerInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Scales and positions a texture to perfectly fit in the viewport.
		 * The adjusted transform can be applied to a uniform 1m2 (default) plane to position it correctly in this frame.
		 * The current zoom and pan levels are reset and the camera position, viewport, and
		 * clipping planes are adjusted to capture the texture:
		 * 
		 *     - The texture is placed at a depth of (z)0, with dimensions adjusted to fit in the current viewport.
		 *     - The camera is placed at (xy)0 at a depth of (z)5.
		 *     - The camera planes will match the size of the viewport.
		 *     - The near and far clipping planes are setup to include the framed texture.
		 *
		 * Avoid calling this function if you need to manually control the orthographic camera.
		 *
		 * @param textureSize size of the texture
		 * @param ioTextureTransform the texture transform to update
		 * @param scale multiplication factor, defaults to 1 (perfect fit)
		 */
		void frameTexture(const Texture2D& texture, nap::TransformComponentInstance& ioTextureTransform, float scale = 1.0f);

		/**
		 * Scales and positions a texture to perfectly fit in the viewport.
		 * The adjusted transform can be applied to a uniform 1m2 (default) plane to position it correctly in this frame.
		 * The current zoom and pan levels are reset and the camera position, viewport, and
		 * clipping planes are adjusted to capture the texture:
		 *
		 *     - The texture is placed at a depth of (z)0, with dimensions adjusted to fit in the current viewport.
		 *     - The camera is placed at (xy)0 at a depth of (z)5.
		 *     - The camera planes will match the size of the viewport.
		 *     - The near and far clipping planes are setup to include the framed texture.
		 *
		 * Avoid calling this function if you need to manually control the orthographic camera.
		 * 
		 * @param textureSize size of the texture
		 * @param ioTextureTransform the texture transform to update
		 * @param scale multiplication factor, defaults to 1 (perfect fit)
		 */
		void frameTexture(const glm::vec2& textureSize, nap::TransformComponentInstance& ioTextureTransform, float scale = 1.0f);

		/**
		 * Updates the camera planes to match size of the viewport.
		 */
		void reset();

		/**
		 * Returns the current zoom level,
		 * where 0 = completely zoomed in, 1 = framed in window and anything higher is zoomed out. 
		 * @return current zoom level.
		 */
		float getZoomLevel() const;

		/**
		 * @return orthographic camera
		 */
		nap::OrthoCameraComponentInstance& getCamera()				{ return *mOrthoCameraComponent; }

	private:
		// Default orthographic camera and texture (plane) position
		static constexpr glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 5.0f);

		/**
		 * Handler for mouse down events
		 */
		void onMouseDown(const PointerPressEvent& pointerPressEvent);

		/**
		 * Handler for mouse up events
		 */
		void onMouseUp(const PointerReleaseEvent& pointerReleaseEvent);

		/**
		 * Handler for mouse move events
		 */
		void onMouseMove(const PointerMoveEvent& pointerMoveEvent);

		/**
		 * Handle panning
		 */
		void panCamera(const glm::vec2& clickPosition, glm::vec2&& position, glm::vec2&& relMovement);

		/**
		 * Handle zooming
		 */
		void zoomCamera(const glm::vec2& clickPosition, glm::vec2&& position, glm::vec2&& relMovement);

		TransformComponentInstance* mTransformComponent = nullptr;
		OrthoCameraComponentInstance* mOrthoCameraComponent = nullptr;
		RenderWindow* mViewport = nullptr;

		bool mPan = false;								///< If we're currently panning
		bool mZoom = false;								///< If we're currently zooming
		glm::vec2 mClickCoordinates;					///< Last known window click coordinates
		float mZoomSpeed = 1.0f;						///< Camera zoom speed
		nap::OrthoCameraProperties mCameraProperties;	///< Computed camera projection settings
	};
}
