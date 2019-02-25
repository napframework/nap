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
		class ClearCacheComponentInstance;

		/**
		*	Clears the database cache
		*/
		class NAPAPI ClearCacheComponent : public Component
		{
			RTTI_ENABLE(Component)
				DECLARE_COMPONENT(ClearCacheComponent, ClearCacheComponentInstance)
		public:

			/**
			* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
			* @param components the components this object depends on
			*/
			virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

			ResourcePtr<DataModel> mDataModel = nullptr;	///< Property: 'DataModel' the data-model that manages all emography related data.
		};


		/**
		* emographyclearcomponentInstance
		*/
		class NAPAPI ClearCacheComponentInstance : public ComponentInstance
		{
			RTTI_ENABLE(ComponentInstance)
		public:
			ClearCacheComponentInstance(EntityInstance& entity, Component& resource) :
				ComponentInstance(entity, resource) { }

			/**
			* Initialize ClearCacheComponentInstance based on the ClearCacheComponent resource.
			* @param errorState should hold the error message when initialization fails
			* @return if the emographyclearcomponentInstance is initialized successfully
			*/
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			 * Clears the entire cache of the data-model.
			 * This includes all data for every registered type.
			 * @param error contains the error if the cache can't be cleared.
			 * @return if the cache was cleared successfully.
			 */
			bool clearCache(nap::utility::ErrorState& error);

		private:
			DataModelInstance* mDataModel = nullptr;			///< The data-model that manages all emography related data
		};
	}
}
