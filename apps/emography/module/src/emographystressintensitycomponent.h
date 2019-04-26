#pragma once

// Local Includes
#include "emographyrangedataviewcomponent.h"
#include "emographyreading.h"

// External Includes

namespace nap
{
	namespace emography 
	{
		class StressIntensityComponentInstance;

		/**
		 * StressDataViewComponent
		 */
		class NAPAPI StressIntensityComponent : public RangeDataViewComponent
		{
			RTTI_ENABLE(RangeDataViewComponent)
			DECLARE_COMPONENT(StressIntensityComponent, StressIntensityComponentInstance)
		public:

			/**
			 * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
			 * @param components the components this object depends on
			 */
			virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

			std::string mReplyName = "StressIntensityReply";	///< Property: 'ReplyName' name of the reply that is send as a message.
		};


		/**
		 * StressDataViewComponentInstance
		 */
		class NAPAPI StressIntensityComponentInstance : public RangeDataviewComponentInstance
		{
			RTTI_ENABLE(RangeDataviewComponentInstance)
		public:
			StressIntensityComponentInstance(EntityInstance& entity, Component& resource) :
				RangeDataviewComponentInstance(entity, resource) { }

			/**
			* Initialize StressDataViewComponentInstance based on the StressDataViewComponent resource
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
			 * Occurs when the sample count or times frame changes 
			 * update container size, get records and push view	
			 */
			virtual void onQuery() override;

		private:
			std::string mReplyName = "StressIntensityReply";
		};
	}
}
