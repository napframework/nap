#pragma once

#include <component.h>
#include <nap/timestamp.h>

namespace nap
{
	namespace emography
	{
		class RangeDataviewComponentInstance;

		/**
		 * Settings associated with every range data component that is serializable
		 */
		struct RangeDataSettings
		{
			RangeDataSettings() = default;

			/**
			 * Construct settings based on start / stop time	
			 */
			RangeDataSettings(const TimeStamp& start, const TimeStamp& stop) :
				mStartTime(start),
				mEndTime(stop)	{ }

			int			mSamples = 1024;		///< Property: 'Samples' number of samples associated with this data view
			TimeStamp	mStartTime;				///< Property: 'StartTime' sample record start time
			TimeStamp	mEndTime;				///< Property: 'EndTime' sample record end time
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
			* Initialize emographydataviewcomponentInstance based on the emographydataviewcomponent resource
			* @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
			* @param errorState should hold the error message when initialization fails
			* @return if the emographydataviewcomponentInstance is initialized successfully
			*/
			virtual bool init(utility::ErrorState& errorState) override;

			/**
			 * @return total number of samples associated with this data view	
			 */
			int getSampleCount() const						{ return mSettings.mSamples; }

			/**
			 * @return the settings associated with this component
			 */
			const RangeDataSettings& getSettings() const	{ return mSettings; }

			/**
			 * Update settings associated with this ranged data view
			 * @param settings the new settings
			 */
			void setSettings(const RangeDataSettings& settings);

		protected:
			RangeDataSettings mSettings;	///< Ranged data settings

			/**
			 * Needs to be implemented by derived classes.
			 * Called when sample count changes, ie: resolution of the buffer containing records
			 */
			virtual void settingsChanged() = 0;
		};
	}
}
