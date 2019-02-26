#include "emographypopulatecachecomponent.h"
#include "emographystress.h"

// External Includes
#include <entity.h>
#include <mathutils.h>
#include <nap/logger.h>
#include <apiservice.h>
#include <nap/core.h>

// nap::emographypopulatecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::emography::PopulateCacheComponent)
	RTTI_PROPERTY("NumberOfDays",			&nap::emography::PopulateCacheComponent::mNumberOfDays,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SamplesPerSecond",		&nap::emography::PopulateCacheComponent::mSamplesPerSecond,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DataModel",				&nap::emography::PopulateCacheComponent::mDataModel,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ClearCacheComponent",	&nap::emography::PopulateCacheComponent::mClearCacheComponent,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::emographypopulatecomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::emography::PopulateCacheComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace emography
	{
		void PopulateCacheComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
		{
		}


		bool PopulateCacheComponentInstance::init(utility::ErrorState& errorState)
		{
			PopulateCacheComponent& resource = *getComponent<PopulateCacheComponent>();
			mAPIService = getEntityInstance()->getCore()->getService<APIService>();
			if (!errorState.check(mAPIService != nullptr, "%s: unable to acquire handle to API service", resource.mID.c_str()))
				return false;

			setParameters(resource.mNumberOfDays, resource.mSamplesPerSecond);
			mDataModel = &resource.mDataModel->getInstance();
			return true;
		}


		bool PopulateCacheComponentInstance::populate(nap::utility::ErrorState& error)
		{
			return populate(mNumberOfDays, mSamplesPerSecond, error);
		}


		bool PopulateCacheComponentInstance::populate(int numberOfDays, int samplesPerSecond, nap::utility::ErrorState& error)
		{
			// Clear cache using the clear cache component
			if (!mClearCacheComponent->clearCache(error))
			{
				error.fail("failed to populate cache");
				return false;
			}

			// Start generating data from the last timestamp in the model, if available. Otherwise generate data starting at the current time.
			SystemTimeStamp current_time = getCurrentTime() - (Hours(24) * numberOfDays);
			int num_samples_added = 0;

			// Used for tracking progress (0-100)
			int step_inc = (numberOfDays * 24 * 60 * 60 * samplesPerSecond) / 100;
			int curr_pro = 0;
			int next_inc = step_inc;

			// Notify listeners we're starting
			APIEventPtr progress_event = std::make_unique<APIEvent>("PopulateCache");
			progress_event->addArgument<APIBool>("Status", true);
			mAPIService->dispatchEvent(std::move(progress_event));

			nap::SystemTimer timer;
			timer.start();

			for (int days = 0; days != numberOfDays; ++days)
			{
				for (int hours = 0; hours != 24; ++hours)
				{
					for (int minutes = 0; minutes != 60; ++minutes)
					{
						float minute_bias = 0.5f + 0.5f * sin(((float)minutes / 60.0f) * math::pi());
						for (int seconds = 0; seconds != 60; ++seconds)
						{
							float seconds_bias = 0.5f + sin(((float)minutes / 60.0f) * math::pi());

							for (int seconds_samples = 0; seconds_samples != samplesPerSecond; ++seconds_samples)
							{
								utility::ErrorState errorState;
								float intensity_value = (float)(math::random<int>(0, 99)) * minute_bias * seconds_bias;
								EStressState state_value;
								if (intensity_value > 66)
									state_value = EStressState::Over;
								else if (intensity_value > 33)
									state_value = EStressState::Normal;
								else
									state_value = EStressState::Under;

								current_time += Milliseconds(1000 / samplesPerSecond);

								std::unique_ptr<StressIntensityReading> intensityReading = std::make_unique<StressIntensityReading>(intensity_value, current_time);
								if (!mDataModel->add(*intensityReading, errorState))
								{
									error.fail("failed to populate cache");
									return false;
								}

								std::unique_ptr<StressStateReading> stateReading = std::make_unique<StressStateReading>(state_value, current_time);
								if (!mDataModel->add(*stateReading, errorState))
								{
									error.fail("failed to populate cache");
									return false;
								}

								num_samples_added++;

								// Report progress
								if (num_samples_added == next_inc)
								{
									progress_event = std::make_unique<APIEvent>("PopulateCacheProgress");
									progress_event->addArgument<APIInt>("Value", ++curr_pro);
									mAPIService->dispatchEvent(std::move(progress_event));
									nap::Logger::info("Cache Progress: %d", curr_pro);
									next_inc += step_inc;
								}
							}
						}
					}
				}
			}

			float pop_duration_sec = timer.getElapsedTimeFloat();
			Logger::info("Generating %d days of data (%d samples) took %.2fs (%.2f ms/sample)",
				numberOfDays,
				num_samples_added,
				pop_duration_sec,
				(pop_duration_sec * 1000.0f) / float(num_samples_added));

			// Notify listeners we've finished
			progress_event = std::make_unique<APIEvent>("PopulateCache");
			progress_event->addArgument<APIBool>("Status", false);
			mAPIService->dispatchEvent(std::move(progress_event));

			return true;
		}


		void PopulateCacheComponentInstance::setParameters(int numberOfDays, int samplesPerSecond)
		{
			mNumberOfDays = math::max<int>(numberOfDays, 1);
			mSamplesPerSecond = math::max<int>(samplesPerSecond, 1);
		}
	}
}