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
		 * Scales and positions texture to perfectly fit in current frame
		 * @param texture the texture to fit
		 * @param ioTextureTransform the texture transform to update
		 * @param scale multiplication factor, defaults to 1 (perfect fit)
		 */
		void frameTexture(const Texture2D& texture, nap::TransformComponentInstance& ioTextureTransform, float scale = 1.0f);

		/**
		 * Scales and positions texture to perfectly fit in current frame
		 * @param textureSize size of the texture
		 * @param ioTextureTransform the texture transform to update
		 * @param scale multiplication factor, defaults to 1 (perfect fit)
		 */
		void frameTexture(const glm::vec2& textureSize, nap::TransformComponentInstance& ioTextureTransform, float scale = 1.0f);

	private:
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

		TransformComponentInstance* mTransformComponent = nullptr;
		OrthoCameraComponentInstance* mOrthoCameraComponent = nullptr;
		RenderWindow* mRenderTarget = nullptr;
	};
}
