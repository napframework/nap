#pragma once

#include <component.h>
#include <nap/datetime.h>
#include <apiservice.h>

namespace nap
{
	namespace emography
	{
		class RangeDataviewComponentInstance;

		/**
		 * Settings associated with every range data component that is serializable
		 */
		struct NAPAPI RangeDataSettings
		{
			RangeDataSettings() = default;

			/**
			 * Construct settings based on start / stop time	
			 */
			RangeDataSettings(const Date& start, const Date& stop) :
				mStartTime(start),
				mEndTime(stop)	{ }

			int			mSamples = 1024;		///< Property: 'Samples' number of samples associated with this data view
			Date		mStartTime;				///< Property: 'StartTime' sample record start time
			Date		mEndTime;				///< Property: 'EndTime' sample record end time
		};


		/**
		 * DataViewComponent
		 */
		class NAPAPI RangeDataViewComponent : public Component
		{
			RTTI_ENABLE(Component)
			DECLARE_COMPONENT(RangeDataViewComponent, RangeDataviewComponentInstance)
		public:
			RangeDataSettings mSettings;		///< Property: 'Settings' settings associated with this ranged data view
		};


		/**
		 * DataViewComponentInstance
		 */
		class NAPAPI RangeDataviewComponentInstance : public ComponentInstance
		{
			RTTI_ENABLE(ComponentInstance)
		public:
			RangeDataviewComponentInstance(EntityInstance& entity, Component& resource) :
				ComponentInstance(entity, resource) { }

			/**
			* Initialize emographydataviewcomponentInstance based on the resource
			* @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
			* @param errorState should hold the error message when initialization fails
			* @return if the emographydataviewcomponentInstance is initialized successfully
			*/
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			 * @return total number of samples associated with this data view	
			 */
			int getSampleCount() const						{ return mSampleCount; }

			/**
			 * Sets the number of samples to take, updates settings internally
			 * @param count number of samples to take from start to end range
			 */
			void query(const TimeStamp& startTime, const TimeStamp& endTime, int samples);

		protected:

			/**
			 * Needs to be implemented by derived classes.
			 * Called when sample count changes, ie: resolution of the buffer containing records
			 */
			virtual void onQuery() = 0;

			int mSampleCount = -1;					///< Total number of samples
			TimeStamp mStartTime;					///< Sample start time
			TimeStamp mEndTime;						///< Sample end time
			nap::APIService* mAPIService = nullptr;	///< The API Service
		};
	}
}
