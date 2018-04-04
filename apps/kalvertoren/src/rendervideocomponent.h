#pragma once

#include <component.h>
#include <video.h>
#include <rendertarget.h>
#include <nap/resourceptr.h>
#include <material.h>
#include <orthocameracomponent.h>
#include <renderablemeshcomponent.h>
#include <componentptr.h>

namespace nap
{
	class RenderVideoComponentInstance;
	class RenderService;

	/**
	 *	rendervideocomponent
	 */
	class NAPAPI RenderVideoComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(RenderVideoComponent, RenderVideoComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<nap::Video>					mVideoPlayer = nullptr;
		ResourcePtr<nap::RenderTarget>			mTarget = nullptr;
		ComponentPtr<RenderableMeshComponent>	mRenderableComponent = nullptr;
		ComponentPtr<OrthoCameraComponent>		mCameraComponent = nullptr;
	};


	/**
	 * rendervideocomponentInstance	
	 */
	class NAPAPI RenderVideoComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		RenderVideoComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize rendervideocomponentInstance based on the rendervideocomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the rendervideocomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update rendervideocomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 *	Renders the video into it's target texture
		 */
		void render();

		ComponentInstancePtr<RenderableMeshComponent> mRenderableComponent =	{ this, &RenderVideoComponent::mRenderableComponent };
		ComponentInstancePtr<OrthoCameraComponent> mCameraComponent =			{ this, &RenderVideoComponent::mCameraComponent };

	private:
		RenderService*	mRenderService = nullptr;
		RenderTarget*	mTarget = nullptr;
		Video*			mVideoPlayer = nullptr;
	};
}
