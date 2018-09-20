#pragma once

// External Includes
#include <component.h>
#include <rendertarget.h>
#include <orthocameracomponent.h>
#include <renderablemeshcomponent.h>
#include <bitmap.h>

namespace nap
{
	class RenderCombinationComponentInstance;

	/**
	 * Component that is able to render the cloud / video combination into a back buffer
	 * But most importantly this component downloads the GPU pixel information into it's own texture
	 * This texture is used to shade various parts of the light rig
	 */
	class NAPAPI RenderCombinationComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(RenderCombinationComponent, RenderCombinationComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		// Properties
		rtti::ObjectPtr<RenderTarget> mRenderTarget = nullptr;	///< Property: 'Target' 
		rtti::ObjectPtr<Bitmap> mBitmap = nullptr;				///< Property: 'Bitmap'
	};


	/**
	 * rendercombinationcomponentInstance	
	 */
	class NAPAPI RenderCombinationComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		RenderCombinationComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize rendercombinationcomponentInstance based on the rendercombinationcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the rendercombinationcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update rendercombinationcomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Render the combination of video / clouds into back-buffer
		 * @param orthoCamera the camera used for projection
		 */
		void render(OrthoCameraComponentInstance& orthoCamera);

	private:
		nap::RenderTarget* mRenderTarget = nullptr;
		nap::Bitmap* mBitmap = nullptr;
		nap::RenderableMeshComponentInstance* mRenderableMesh = nullptr;
		nap::RenderService* mRenderService = nullptr;
	};
}
