#pragma once

// Local Includes
#include "emographystressintensitycomponent.h"
#include "emographyclearcachecomponent.h"
#include "emographypopulatecachecomponent.h"
#include "emographyaddstresscomponent.h"

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
		 * Handles API events that are given to the API service from the app or an external environment.
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
			ComponentPtr<StressIntensityComponent> mStressViewComponent;		///< Property: 'StressViewComponent'
			ComponentPtr<ClearCacheComponent> mClearCacheComponent;			///< Property: 'ClearCacheComponent'
			ComponentPtr<PopulateCacheComponent> mPopulateCacheComponent;	///< Property: 'PopulateCacheComponent'
			ComponentPtr<AddStressSampleComponent> mAddStressComponent;		///< Property: 'AddStressComponent'
		};


		/**
		 * Handles API events that are given to the API service from the app or an external environment.
		 * When an event is received by this component the system verified the api event and signature.
		 * The right callback is called based on the api event id.
		 */
		class NAPAPI APIControlComponentInstance : public ComponentInstance
		{
			RTTI_ENABLE(ComponentInstance)
		public:
			APIControlComponentInstance(EntityInstance& entity, Component& resource) :
				ComponentInstance(entity, resource) { }

			/**
			* Initialize APIControlComponentInstance based on the APIControlComponent resource
			* @param errorState should hold the error message when initialization fails
			* @return if the apihandlecomponentInstance is initialized successfully
			*/
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			* update APIControlComponentInstance. This is called by NAP core automatically
			* @param deltaTime time in between frames in seconds
			*/
			virtual void update(double deltaTime) override;

			// Resolved runtime instance ptr to stress view component
			ComponentInstancePtr<StressIntensityComponent> mStressViewComponent		= { this, &APIControlComponent::mStressViewComponent };

			// Can clear the database cache
			ComponentInstancePtr<ClearCacheComponent> mClearCacheComponent			= { this, &APIControlComponent::mClearCacheComponent };

			// Can populate the database cache
			ComponentInstancePtr<PopulateCacheComponent> mPopulateCacheComponent	= { this, &APIControlComponent::mPopulateCacheComponent };

			// Stores new stress samples in the database cache
			ComponentInstancePtr<AddStressSampleComponent> mAddStressComponent		= { this, &APIControlComponent::mAddStressComponent };

		private:
			APIComponentInstance* mComponentInstance = nullptr;

			// All callbacks
			void updateView(const nap::APIEvent& apiEvent);
			nap::Slot<const nap::APIEvent&> mUpateViewSlot = { this, &APIControlComponentInstance::updateView };

			void clearCache(const nap::APIEvent& apiEvent);
			nap::Slot<const nap::APIEvent&> mClearCacheSlot = { this, &APIControlComponentInstance::clearCache };

			void populateCache(const nap::APIEvent& apiEvent);
			nap::Slot<const nap::APIEvent&> mPopulateCacheSlot = { this, &APIControlComponentInstance::populateCache };

			void populateCacheParam(const nap::APIEvent& apiEvent);
			nap::Slot<const nap::APIEvent&> mPopulateCacheParamSlot = { this, &APIControlComponentInstance::populateCacheParam };

			void addStressSample(const nap::APIEvent& apiEvent);
			nap::Slot<const nap::APIEvent&> mAddStressSlot = { this, &APIControlComponentInstance::addStressSample };
		};
	}
}
