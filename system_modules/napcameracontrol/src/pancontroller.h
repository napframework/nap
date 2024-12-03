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
	class PanControllerInstance;

	/**
	 * 2D texture pan and zoom orthographic camera controller 
	 */
	class NAPAPI PanController : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(PanController, PanControllerInstance)
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
	 * 2D texture pan and zoom orthographic camera controller 
	 */
	class NAPAPI PanControllerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		PanControllerInstance(EntityInstance& entity, Component& resource) :
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
		 * Scales and positions a texture to perfectly fit in the given window, excluding pan and zoom levels
		 * The adjusted transform must be applied to a uniform 1m2 (default) plane to position it correctly in this frame.
		 * Note that this call does *not* reset the current zoom and pan levels, call reset() if you want the camera planes to match the current window.
		 * @param texture the texture to fit
		 * @param ioTextureTransform the texture transform to update
		 * @param scale multiplication factor, defaults to 1 (perfect fit)
		 */
		void frameTexture(const Texture2D& texture, nap::TransformComponentInstance& ioTextureTransform, float scale = 1.0f);

		/**
		 * Scales and positions a texture to perfectly fit in the window.
		 * The adjusted transform must be applied to a uniform 1m2 (default) plane to position it correctly in this frame.
		 * Note that this call does *not* reset the current zoom and pan levels, call reset() if you want the camera planes to match the current window.
		 * @param textureSize size of the texture
		 * @param ioTextureTransform the texture transform to update
		 * @param scale multiplication factor, defaults to 1 (perfect fit)
		 */
		void frameTexture(const glm::vec2& textureSize, nap::TransformComponentInstance& ioTextureTransform, float scale = 1.0f);

		/**
		 * Resets the planes to match size of the window, clearing all zoom and pan settings.
		 * You probably want to call this after you frame a texture to reset all zoom and pan controls.
		 */
		void reset();

		/**
		 * Returns the current zoom level,
		 * where 0 = completely zoomed in, 1 = framed in window and anything higher is zoomed out. 
		 * @return current zoom level.
		 */
		float getZoomLevel() const;

	private:
		// Default orthographic camera and texture (plane) position
		static constexpr glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 1.0f);
		static constexpr glm::vec2 zoomLevels = glm::vec2(math::epsilon<float>(), 10.0f);

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

		/**
		 * Apply transform
		 */
		void transform(glm::vec2&& transform);

		TransformComponentInstance* mTransformComponent = nullptr;
		OrthoCameraComponentInstance* mOrthoCameraComponent = nullptr;
		RenderWindow* mWindow = nullptr;

		bool mPan  = false;
		bool mZoom = false;
		glm::vec2 mClickCoordinates;		///< Window click coordinates
		glm::vec3 mXFormCoordinates;		///< Camera transform click coordinates

		float mZoomSpeed = 1.0f;			///< Camera zoom speed
		nap::OrthoCameraProperties mCameraProperties;
	};
}
