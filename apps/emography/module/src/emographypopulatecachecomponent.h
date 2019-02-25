#pragma once

// Local Includes
#include "datamodel.h"
#include "emographyclearcachecomponent.h"
#include "apiservice.h"

// External Includes
#include <component.h>
#include <nap/resourceptr.h>
#include <componentptr.h>

namespace nap
{
	namespace emography
	{
		class PopulateCacheComponentInstance;

		/**
		* Populates the data model with mock sample-data.
		 */
		class NAPAPI PopulateCacheComponent : public Component
		{
			RTTI_ENABLE(Component)
			DECLARE_COMPONENT(PopulateCacheComponent, PopulateCacheComponentInstance)
		public:

			/**
			 * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
			 * @param components the components this object depends on
			 */
			virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

			int mNumberOfDays = 7;												///< Property: 'NumberOfDays' number of days to populate the cache (data-model) with.
			int mSamplesPerSecond = 1;											///< Property: 'SamplesPerSecond' number of samples for every second to record.
			ResourcePtr<DataModel> mDataModel = nullptr;						///< Property: 'DataModel' the data-model that manages all emography related data.
			ComponentPtr<ClearCacheComponent> mClearCacheComponent = nullptr;	///< Property: 'ClearCacheComponent' the component that can clear the data-model cache.
		};


		/**
		 * Generate data for the specified number of days and samples per second.
		 * The generated data is a sine wave with noise added on top, with periods of no data to simulate 'no activity'.
		 * The data is generated for the current date minus the number of days up to now.
		 */
		class NAPAPI PopulateCacheComponentInstance : public ComponentInstance
		{
			RTTI_ENABLE(ComponentInstance)
		public:
			PopulateCacheComponentInstance(EntityInstance& entity, Component& resource) :
				ComponentInstance(entity, resource) { }

			/**
			 * Initialize PopulateCacheComponentInstance based on the PopulateCacheComponent resource
			 * @param errorState should hold the error message when initialization fails
			 * @return if the emographypopulatecomponentInstance is initialized successfully
			 */
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			 * Populates the data-model with mock data, note that all previously recorded data is destroyed	
			 * @param error contains the error when population fails
			 * @return if population succeeded.
			 */
			bool populate(nap::utility::ErrorState& error);

			/**
			 * Updates settings associated with mock-data generation.
			 * @param numberOfDays total number of days to generate data for.
			 * @param samplesPerSecond number of samples generated for every second.
			 */
			void setParameters(int numberOfDays, int samplesPerSecond);

			// Pointer to the clear cache component, set after de-serialization
			ComponentInstancePtr<ClearCacheComponent> mClearCacheComponent = { this, &PopulateCacheComponent::mClearCacheComponent };

		private:
			int mNumberOfDays = 7;
			int mSamplesPerSecond = 1;
			DataModelInstance* mDataModel = nullptr;
			APIService* mAPIService = nullptr;

			/**
		 	 * Populates the data-model with mock data based on the given parameters.
			 * Note that all previously recorded data is destroyed
			 * @param numberOfDays number of days to populate data for
			 * @param samplesPerSecond number of samples generated for every second.
			 * @param error contains the error when population fails.
			 * @return if population succeeded.
			 */
			bool populate(int numberOfDays, int samplesPerSecond, nap::utility::ErrorState& error);

		};
	}
}
