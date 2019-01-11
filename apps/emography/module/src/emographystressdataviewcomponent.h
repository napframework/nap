#pragma once

// Local Includes
#include "emographyrangedataviewcomponent.h"
#include "emographysnapshot.h"

// External Includes

namespace nap
{
	namespace emography 
	{
		class StressDataViewComponentInstance;

		/**
		 * StressDataViewComponent
		 */
		class NAPAPI StressDataViewComponent : public RangeDataViewComponent
		{
			RTTI_ENABLE(RangeDataViewComponent)
			DECLARE_COMPONENT(StressDataViewComponent, StressDataViewComponentInstance)
		public:

			/**
			 * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
			 * @param components the components this object depends on
			 */
			virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

			std::vector<StressSnapshot> mStressData;	///< Property: 'StressData' Example: stress related data snapshot
		};


		/**
		 * StressDataViewComponentInstance
		 */
		class NAPAPI StressDataViewComponentInstance : public RangeDataviewComponentInstance
		{
			RTTI_ENABLE(RangeDataviewComponentInstance)
		public:
			StressDataViewComponentInstance(EntityInstance& entity, Component& resource) :
				RangeDataviewComponentInstance(entity, resource) { }

			/**
			* Initialize emographystressdataviewcomponentInstance based on the emographystressdataviewcomponent resource
			* @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
			* @param errorState should hold the error message when initialization fails
			* @return if the emographystressdataviewcomponentInstance is initialized successfully
			*/
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			 * update emographystressdataviewcomponentInstance. This is called by NAP core automatically
			 * @param deltaTime time in between frames in seconds
			 */
			virtual void update(double deltaTime) override;

		protected:
			/**
			 * Occurs when the sample count changes, update container size, get records and push view	
			 */
			virtual void settingsChanged() override;

		private:
			std::vector<StressSnapshot> mStressData;	///< Property: 'Data' Example: stress related data snapshot
		};
	}
}
