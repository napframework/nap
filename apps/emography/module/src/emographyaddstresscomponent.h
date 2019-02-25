#pragma once

// Local Includes
#include "datamodel.h"

// External Includes
#include <component.h>
#include <nap/resourceptr.h>

namespace nap
{
	namespace emography
	{
		class AddStressSampleComponentInstance;

		/**
		 * Resource part of the store stress component. 
		 * Stores a stress related sample in the database.
		 */
		class NAPAPI AddStressSampleComponent : public Component
		{
			RTTI_ENABLE(Component)
			DECLARE_COMPONENT(AddStressSampleComponent, AddStressSampleComponentInstance)
		public:

			/**
			* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
			* @param components the components this object depends on
			*/
			virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

			ResourcePtr<DataModel> mDataModel = nullptr;	///< Property: 'DataModel' the data-model that manages all emography related data.
		};


		/**
		 * Instance part of the store stress component.
		 * Stores a stress related sample in the database.
		 */
		class NAPAPI AddStressSampleComponentInstance : public ComponentInstance
		{
			RTTI_ENABLE(ComponentInstance)
		public:
			AddStressSampleComponentInstance(EntityInstance& entity, Component& resource) :
				ComponentInstance(entity, resource) { }

			/**
			* Initialize StoreStressComponentInstance based on the resource.
			* @param errorState holds the error message when initialization fails
			* @return if the StoreStressComponentInstance is initialized successfully
			*/
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			 * Stores a new sample in the database
			 * @param timeStamp the time associated with the sample
			 * @param stressValue measured stress value
			 * @param stressState state associated with this reading
			 * @param error contains the error if adding fails
			 */
			bool addSample(const TimeStamp& timeStamp, float stressValue, int stressState, utility::ErrorState& error);

		private:
			DataModelInstance* mDataModel = nullptr;			///< The data-model that manages all emography related data
		};
	}
}
