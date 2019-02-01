#pragma once

// Local Includes
#include "emographystressdataviewcomponent.h"

// External Includes
#include <component.h>
#include <apicomponent.h>
#include <nap/signalslot.h>
#include <componentptr.h>

namespace nap
{
	namespace emography
	{
		class APIControlComponentInstance;

		/**
		 *	APIHandleComponent
		 */
		class NAPAPI APIControlComponent : public Component
		{
			RTTI_ENABLE(Component)
			DECLARE_COMPONENT(APIControlComponent, APIControlComponentInstance)
		public:

			/**
			 * This component depends on an api component.
			 * @param components the components this object depends on
			 */
			virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

			// Ptr to the component that can perform the query
			ComponentPtr<StressDataViewComponent> mStressViewComponent;		///< Property: 'StressViewComponent'
		};


		/**
		* apihandlecomponentInstance
		*/
		class NAPAPI APIControlComponentInstance : public ComponentInstance
		{
			RTTI_ENABLE(ComponentInstance)
		public:
			APIControlComponentInstance(EntityInstance& entity, Component& resource) :
				ComponentInstance(entity, resource) { }

			/**
			* Initialize apihandlecomponentInstance based on the apihandlecomponent resource
			* @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
			* @param errorState should hold the error message when initialization fails
			* @return if the apihandlecomponentInstance is initialized successfully
			*/
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			* update apihandlecomponentInstance. This is called by NAP core automatically
			* @param deltaTime time in between frames in seconds
			*/
			virtual void update(double deltaTime) override;

			// Resolved runtime instance ptr to stress view component
			ComponentInstancePtr<StressDataViewComponent> mStressViewComponent = { this, &APIControlComponent::mStressViewComponent };

		private:
			APIComponentInstance* mComponentInstance = nullptr;

			// All callbacks
			void updateView(const nap::APIEvent& apiEvent);
			nap::Slot<const nap::APIEvent&> mUpateViewSlot = { this, &APIControlComponentInstance::updateView };
		};
	}
}
