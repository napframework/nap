#pragma once

// Local Includes
#include "lightintensitycomponent.h"

// External Includes
#include <component.h>
#include <artnetcontroller.h>
#include <componentptr.h>

namespace nap
{
	class FacadeLightComponentInstance;

	/**
	 *	facadelightcomponent
	 */
	class NAPAPI FacadeLightComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FacadeLightComponent, FacadeLightComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		// Property: artnet controller
		rtti::ObjectPtr<nap::ArtNetController> mArtnetController;

		// Property: light controller
		ComponentPtr<nap::LightIntensityComponent> mIntensityComponent;

		// Property: min artnet channel
		int mMinChannel = 108;

		// Property: max artnet channel
		int mMaxChannel = 288;

		// Property: if it sends light to the facade
		bool mSend = true;
	};


	/**
	 * facadelightcomponentInstance	
	 */
	class NAPAPI FacadeLightComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FacadeLightComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize facadelightcomponentInstance based on the facadelightcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the facadelightcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update facadelightcomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 *	Resolved component instance pointer
		 */
		ComponentInstancePtr<LightIntensityComponent> mIntensityComponent = { this, &FacadeLightComponent::mIntensityComponent };

	private:
		nap::ArtNetController*	mController = nullptr;
		int						mMinChannel = 0;
		int						mMaxChannel = 0;
		bool					mSend = true;
		std::vector<uint8_t>	mOpeningData;
		std::vector<uint8_t>	mClosingData;
	};
}
