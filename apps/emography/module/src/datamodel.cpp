
// Local Includes
#include "datamodel.h"
#include "emographyreading.h"
#include "emographyservice.h"
#include "emographystress.h"
#include "emographysummaryfunctions.h"

// External Includes
#include <rtti/rttiutilities.h>
#include <nap/core.h>
#include <emographystress.h>
#include <emographysummaryfunctions.h>

namespace nap
{
	namespace emography
	{
		/**
		 * Runtime state for ReadingProcessor. This is separated in an object so that it can be serialized to a database table.
		 */
		class ReadingProcessorState : public rtti::Object
		{
			RTTI_ENABLE(rtti::Object)
		public:
			/** 
			 * Default constructor for RTTI. Should not be used.
			 */
			ReadingProcessorState() = default;

			ReadingProcessorState(size_t rttiVersion) :
				mRTTIVersion(rttiVersion)
			{
			}

		public:
			TimeStamp	mLastReadingTime;
			size_t		mRTTIVersion = -1;
		};

		/**
		 * Runtime state for ReadingProcessorLOD. This is separated in an object so that it can be serialized to a database table.
		 */
		class ReadingProcessorLODState : public rtti::Object
		{
			RTTI_ENABLE(rtti::Object)
		public:
			/** 
			 * Default constructor for RTTI. Should not be used.
			 */
			ReadingProcessorLODState() = default;

			ReadingProcessorLODState(size_t rttiVersion) :
				mRTTIVersion(rttiVersion)
			{
			}

		public:
			uint64_t	mCurrentChunkIndex = -1;
			size_t		mRTTIVersion = -1;
		};
	}
}


RTTI_BEGIN_CLASS(nap::emography::ReadingProcessorState)
	RTTI_PROPERTY("LastReadingTime", &nap::emography::ReadingProcessorState::mLastReadingTime, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RTTIVersion", &nap::emography::ReadingProcessorState::mRTTIVersion, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::emography::ReadingProcessorLODState)
	RTTI_PROPERTY("CurrentChunkIndex", &nap::emography::ReadingProcessorLODState::mCurrentChunkIndex, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RTTIVersion", &nap::emography::ReadingProcessorLODState::mRTTIVersion, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_ENUM(nap::emography::DataModel::EKeepRawReadings)
	RTTI_ENUM_VALUE(nap::emography::DataModel::EKeepRawReadings::Enabled, "Enabled"),
	RTTI_ENUM_VALUE(nap::emography::DataModel::EKeepRawReadings::Disabled, "Disabled")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::emography::DataModel)
	RTTI_PROPERTY("KeepRawReadings", &nap::emography::DataModel::mKeepRawReadings, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Database", &nap::emography::DataModel::mDatabaseFile, nap::rtti::EPropertyMetaData::Required)
	RTTI_CONSTRUCTOR(nap::EmographyService&)
RTTI_END_CLASS


namespace nap
{
	namespace emography
	{
		/**
		 * This summary function is intended to summarize StressState readings. It does this by maintaining a count (per StressState) of how many times
		 * that StressState has been seen in the list of WeightedObjects
		 */
		std::unique_ptr<StressStateReadingSummary> stressStateCountingSummary(const std::vector<DataModelInstance::WeightedObject>& inObjects)
		{
			std::unique_ptr<StressStateReadingSummary> new_summary = std::make_unique<StressStateReadingSummary>();
			for (int index = 0; index < inObjects.size(); ++index)
			{
				rtti::Object* object = inObjects[index].mObject.get();
				StressStateReadingSummary* reading_summary = rtti_cast<StressStateReadingSummary>(object);
				assert(reading_summary != nullptr);

				new_summary->add(*reading_summary);
			}

			return new_summary;
		}


		DataModel::DataModel(EmographyService& service) : mService(service)
		{ }

		
		DataModel::~DataModel()								
		{ 
			mInstance.reset(nullptr); 
		}


		bool DataModel::init(utility::ErrorState& error)	
		{
			// Get factory
			nap::rtti::Factory& factory = mService.getCore().getResourceManager()->getFactory();

			// Create instance
			mInstance = std::make_unique<DataModelInstance>(factory);

			// Initialize data model
			if (!mInstance->init(mService.getDBSourceDir() + mDatabaseFile, mKeepRawReadings, error))
				return false;

			// Register types
			// TODO: Should not happen here, but in a deferred class
            if(!mInstance->registerType<StressIntensity>(&gAveragingSummary<StressIntensity>, error))
                return false;
		
			if (!mInstance->registerType(RTTI_OF(StressStateReading), RTTI_OF(StressStateReadingSummary), &stressStateCountingSummary, error))
				return false;

			return true; 
		}
	}
}


namespace nap
{
	namespace emography
	{
		/**
		 * Internally the database works with seconds, this converts whatever unit was used in TimeStamp into seconds.
		 */
		static TimeStamp secondsToTimeStamp(uint64_t seconds)
		{
			return TimeStamp(SystemTimeStamp(Seconds(seconds)));
		}


		/**
		 * Internally the database works with seconds, this converts whatever unit was used in TimeStamp to seconds.
		 */
		static uint64_t timeStampToSeconds(TimeStamp timeStamp)
		{
			return std::chrono::time_point_cast<Seconds>(timeStamp.toSystemTime()).time_since_epoch().count();
		}


		/**
		 * Represents a single Level Of Detail in a ReadingProcessor.
		 */
		class ReadingProcessorLOD final
		{
		public:
			ReadingProcessorLOD() = default;

			ReadingProcessorLOD(const ReadingProcessorLOD&) = delete;
			ReadingProcessorLOD& operator=(const ReadingProcessorLOD&) = delete;


			ReadingProcessorLOD(ReadingProcessorLOD&& rhs)
			{
				mMaxNumSeconds = rhs.mMaxNumSeconds;
				mState = std::move(rhs.mState);
				mTable = rhs.mTable;
				mStateTable = rhs.mStateTable;

				rhs.mTable = nullptr;
				rhs.mStateTable = nullptr;
				rhs.mMaxNumSeconds = 0;
			}


			ReadingProcessorLOD& operator=(ReadingProcessorLOD&& rhs)
			{
				if (&rhs != this)
				{
					mMaxNumSeconds = rhs.mMaxNumSeconds;
					mState = std::move(rhs.mState);
					mTable = rhs.mTable;
					mStateTable = rhs.mStateTable;

					rhs.mTable = nullptr;
					rhs.mStateTable = nullptr;
					rhs.mMaxNumSeconds = 0;
				}
				return *this;
			}

		public:
			int											mMaxNumSeconds = 0;			///< The amount of seconds stored in a single bucket for this LOD
			std::unique_ptr<ReadingProcessorLODState>	mState;						///< Runtime state for this LOD
			DatabaseTable*								mTable = nullptr;			///< Database table for this LOD's reading summaries
			DatabaseTable*								mStateTable = nullptr;		///< Database table for runtime state
		};


		/**
		 * Internal class for processing a reading of a certain type. Converts data from input to output using the summary function and stores the result in LODs in the database.
		 */
		class ReadingProcessor final
		{
		public:
			ReadingProcessor(Database& database, const rtti::TypeInfo& readingType, const rtti::TypeInfo& summaryType, DataModel::EKeepRawReadings keepRawReadings, const DataModelInstance::SummaryFunction& summaryFunction) :
				mDatabase(&database),
				mReadingType(readingType),
				mSummaryType(summaryType),
				mKeepRawReadings(keepRawReadings),
				mSummaryFunction(summaryFunction)
			{
			}


			bool init(utility::ErrorState& errorState)
			{
				rtti::Path timestamp_path;
				timestamp_path.pushAttribute("TimeStamp");
				timestamp_path.pushAttribute("Time");

				rtti::Path id_path;
				id_path.pushAttribute(rtti::sIDPropertyName);

				// Create a path to the timestamp field for the Reading type for creating the index on the timestamp field
				mRawTimeStampPath = DatabasePropertyPath::sCreate(mReadingType, timestamp_path, errorState);
				if (mRawTimeStampPath == nullptr)
					return false;

				// Create a path to the timestamp field for the ReadingSummary type for creating the index on the timestamp field
				mLODTimeStampPath = DatabasePropertyPath::sCreate(mSummaryType, timestamp_path, errorState);
				if (mLODTimeStampPath == nullptr)
					return false;

				// Create a path to the ID field of Reading type for ignoring the property when serializing
				std::unique_ptr<DatabasePropertyPath> raw_id_path = DatabasePropertyPath::sCreate(mReadingType, id_path, errorState);
				if (raw_id_path == nullptr)
					return false;

				// Create processor state table first so we can detect RTTI changes
				{
					// Create a path to the ID field of ReadingProcessorState type for ignoring the property when serializing
					std::unique_ptr<DatabasePropertyPath> processor_id_path = DatabasePropertyPath::sCreate(RTTI_OF(ReadingProcessorState), id_path, errorState);
					if (processor_id_path == nullptr)
						return false;

					// Get/create the state table for the processor
					std::string state_table_name = utility::stringFormat("%s_state", mReadingType.get_name().data());
					mStateTable = mDatabase->getOrCreateTable(state_table_name, RTTI_OF(ReadingProcessorState), { *processor_id_path }, errorState);
					if (mStateTable == nullptr)
						return false;

					// Retrieve the state object from the state table
					std::vector<std::unique_ptr<rtti::Object>> state_object;
					if (!mStateTable->query("", state_object, errorState))
						return false;

					if (!errorState.check(state_object.size() <= 1, "Found invalid number of state objects"))
						return false;

					// If the object does not exist, create a default one. Otherwise, use the one from the table
					size_t rtti_version = rtti::getRTTIVersion(mReadingType);
					if (state_object.empty())
						mState = std::make_unique<ReadingProcessorState>(rtti_version);
					else
						mState = rtti_cast<ReadingProcessorState>(state_object[0]);

					if (!errorState.check(mState->mRTTIVersion == rtti_version, "The structure of type '%s' has changed and is no longer compatible with the existing data", mReadingType.get_name().data()))
						return false;
				}

				// Create raw table and index
				std::string raw_table_name(mReadingType.get_name());
				mRawTable = mDatabase->getOrCreateTable(raw_table_name, mReadingType, { *raw_id_path }, errorState);
				if (mRawTable == nullptr)
					return false;

				// Create index for the raw table on the timestamp
				if (!mRawTable->getOrCreateIndex(*mRawTimeStampPath, errorState))
					return false;

				if (!addLOD(utility::stringFormat("%s_%s", raw_table_name.c_str(), "Seconds"), 1, errorState))
					return false;

				if (!addLOD(utility::stringFormat("%s_%s", raw_table_name.c_str(), "Minutes"), 60, errorState))
					return false;

				if (!addLOD(utility::stringFormat("%s_%s", raw_table_name.c_str(), "Hours"), 3600, errorState))
					return false;

				if (!addLOD(utility::stringFormat("%s_%s", raw_table_name.c_str(), "Days"), 86400, errorState))
					return false;

				if (!addLOD(utility::stringFormat("%s_%s", raw_table_name.c_str(), "Weeks"), 604800, errorState))
					return false;

				return true;
			}


			bool add(const ReadingBase& object, utility::ErrorState& errorState)
			{
				// Make sure that the LODs are updated up until this timestamp that we just received
				if (!flush(object.mTimeStamp, errorState))
					return false;

				// Add raw reading if enabled
				if (mKeepRawReadings == DataModel::EKeepRawReadings::Enabled)
				{
					if (!mRawTable->add(object, errorState))
						return false;
				}

				// We maintain a small window of raw data for creating the first LOD level. We only want to convert from 
				// summary to summary objects in the summary function, so we create an initial summary object based on the
				// raw reading here through the summary constructor.
				std::unique_ptr<rtti::Object> raw_summary(mSummaryType.create<ReadingSummaryBase>({ object }));
				mRawReadingCache.emplace_back(std::move(raw_summary));

				// Store the last reading time. This means we have updated up until this point. This state is used when flushing
				// manually using flush(): we will then update all LODs up until the last known processed timestamp.
				mState->mLastReadingTime = object.mTimeStamp;

				return true;
			}


			bool flush(utility::ErrorState& errorState)
			{
				if (!mState->mLastReadingTime.isValid())
					return true;

				// We take the last processed reading time and add a single second to it. We do this to ensure that a second is guaranteed
				// to pass it's bucket size. This will flush the second LOD and any higher LODs that are aligned to that second LOD as well. 
				// Because we artificially progress in time a little bit to ensure the LOD is flushed, this does mean that one could, in theory,
				// call flush again within that same second, causing the data to go back in time. To solve this, the flush ignores any data
				// that goes back in time.
				auto nextTimeStamp = mState->mLastReadingTime.toSystemTime() + Seconds(1);
				return flush(nextTimeStamp, errorState);
			}


			/** 
			 * This is the main 'LOD generation' routine. Each LOD is separated into chunks of a certain length, indicated by mMaxNumSeconds.
			 * The chunk 'index' is an ID that represent a single chunk in the LOD. It is calculated by:
			 *
			 *			floor(TimeStampSecs / mMaxNumSeconds)
			 *
			 * For each LOD we store the current chunk index, and when a new flush is called, we check whether we transitioned from the 
			 * previous to a new chunk. This means we need to update the LOD level. 
			 *
			 * Updating is performed by gathering data from the lower LOD for the timerange that we need, and summarizing that data into
			 * a new, single value. That value is then stored in the database. If there is no lower LOD, the raw cache is used to gather
			 * data, otherwise, a database query is performed on the lower LOD.
			 */
			bool flush(TimeStamp timestamp, utility::ErrorState& errorState)
			{
				bool any_lod_flushed = false;

				uint64_t timestamp_seconds = timeStampToSeconds(timestamp);

				std::vector<std::unique_ptr<rtti::Object>> objects;
				std::vector<DataModelInstance::WeightedObject> weighted_objects;

				DatabaseTable* prevTable = nullptr;
				for (int lod_index = 0; lod_index < mLODs.size(); ++lod_index)
				{
					ReadingProcessorLOD& lod = mLODs[lod_index];

					// Calculate what chunk this lod is currently in
					uint64_t chunk_index = timestamp_seconds / lod.mMaxNumSeconds;

					// If we are still in the same chunk, we don't need to continue processing. Notice that this is a break and not a continue, because
					// we know for sure that when this chunk didn't transition, that any higher LODs will also not transition, because its hierarchical nature.
					if (chunk_index == lod.mState->mCurrentChunkIndex)
						break;

					// If this is not the first chunk, process it
					if (lod.mState->mCurrentChunkIndex != -1)
					{
						// If we went back in time, ignore data (this is theoretically possible when flush is called manually, see comments in flush()).
						if (chunk_index < lod.mState->mCurrentChunkIndex)
							break;

						// We have to convert from seconds to Timestamp's time format, as this is the format that was written to the database
						TimeStamp prev_chunk_start_timestamp = secondsToTimeStamp(lod.mState->mCurrentChunkIndex * lod.mMaxNumSeconds);

						objects.clear();
						weighted_objects.clear();

						if (prevTable == nullptr)
						{
							// If this is the lowest LOD, grab data from the raw cache
							objects = std::move(mRawReadingCache);
						}
						else
						{
							// For any non-zero LOD, query the 'parent' LOD for data on this LOD's timerange
							const std::string timestamp_column_name = prevTable->getColumnName(*mRawTimeStampPath, errorState);
							if (timestamp_column_name.empty())
								return false;

							if (!prevTable->query(utility::stringFormat("%s >= %llu", timestamp_column_name.c_str(), prev_chunk_start_timestamp.mTimeStamp), objects, errorState))
								return false;
						}

						// Here we are going to convert the data from the parent LOD into a list of weighted objects. Although weighting (as described in
						// the DataModel header) is only relevant when querying from multiple LOD levels (as is performed in getRange), we still perform
						// the same procedure here because we use the same Summary function for generating the LOD as we do for querying the LODs.
						// The weight that is calculated here is simply an even distribution of the amount of values we received from the parent LOD:
						// which is 1 / numvalues.
						// The numSecondsActive field is processed here as well. We simply add all the NumSecondsActive from the higher LOD together, making
						// the value a summary of the higher LODs. Notice though that we always reset the value to 1 for the lowest LOD in the collapsed object.
						int num_active_seconds = 0;
						weighted_objects.reserve(objects.size());
						float weight = 1.0f / objects.size();
						for (auto& object : objects)
						{
							ReadingSummaryBase* reading = rtti_cast<ReadingSummaryBase>(object.get());
							num_active_seconds += reading->mNumSecondsActive;

							DataModelInstance::WeightedObject weighted_object{ weight, std::move(object) };
							weighted_objects.emplace_back(std::move(weighted_object));
						}

						// Create the new summarized object by calling the Summarize function with the weighted objects
						std::unique_ptr<ReadingSummaryBase> collapsedObject = mSummaryFunction(weighted_objects);
						assert(collapsedObject->get_type() == mSummaryType);

						// Set timestamp to start of the chunk and update seconds active. Notice that we reset the seconds active for the lowest LOD.
						collapsedObject->mTimeStamp = prev_chunk_start_timestamp;
						collapsedObject->mNumSecondsActive = lod_index == 0 ? 1 : num_active_seconds;

						if (!lod.mTable->add(*collapsedObject, errorState))
							return false;
					}

					// Always store the last processed chunk
					lod.mState->mCurrentChunkIndex = chunk_index;

					// We replace the state object in the state table by clearing the entire table and re-adding the value. A more elegant and possibly
					// faster way would be to update the row in the database table.
					if (!lod.mStateTable->clear(errorState))
						return false;

					if (!lod.mStateTable->add(*lod.mState, errorState))
						return false;

					any_lod_flushed = true;

					prevTable = lod.mTable;
				}

				// To avoid flushing the generic processor state for each reading, we only update it when any of the LODs is updated.
				if (any_lod_flushed)
				{
					if (!mStateTable->clear(errorState))
						return false;

					if (!mStateTable->add(*mState, errorState))
						return false;
				}

				return true;
			}


			/**
			 * This is the main algorithm for querying and summarizing a range of data. It's recommended to first read the generic documentation in DataModel.h.
			 * 
			 * See the following state and query range:
			 * 
			 *	Query range	                     #----------------------------------------------#
			 *	LOD 0		   [ 1  ][ 1  ][ 4  ][ 5  ][ 2  ][ 5  ][ 6  ][ 4  ][ 3  ][ 3  ][ 6  ][ 4  ]
			 *	LOD 1		   [    0.5   ][   4.5    ][   3.5    ][    5     ][     3    ][    5     ]
			 *  LOD 2		   [         2.5          ][         4.25         ][           4          ]
			 *  LOD 3								   [                     4.125                    ]
			 *
			 * As explained in the DataModel comments, we wish to process the minimum of data, and calculate the correct weights for the data,
			 * which, for this example, would be the following data:
			 *
			 *	LOD 0		   [ 5  ]                                   [ 6  ]
			 *	LOD 1		                                 [     3    ]
			 *  LOD 2		         [         4.25         ]
			 *
			 * The algorithm is split into two parts: 
			 * 1) The part where the algorithm processes blocks that lead to higher LODs in This example that is only the block with value [5].
			 * 2) The part where the algorithm tries to fit in as many blocks as possible, from low detail (high LOD) to high detail (lowest LOD).
			 * 
			 * Part 1) We process all LODs, starting from the lowest LOD. We want to answer the question: is there a higher LOD that we want to process? (Is there a higher 
			 * that fits within our time range?) And if so, where does it start? To answer this question, we calculate the start time of the chunk in the next, higher LOD. 
			 * This start time can either be exactly equal to the current time or somewhere in the future. From the start time of the next LOD, we calculate the end time of 
			 * a single chunk in that LOD. If the end time of the block is within the time range, we know we want to process at least one chunk in a higher LOD, so we don't 
			 * want to go through all the current LOD's separate blocks. However, we do want to process any blocks leading to the next LOD's start time. After processing
			 * these blocks, we proceed to the next LOD level and perform the same routine. In our example, this means the following:
			 * 
			 * - We start at LOD 5, at the block with the value [5]. The next LOD chunk is LOD1 with value [3.5]. The start time of that chunk lies one second ahead of the 
			 *   current block. The end time of that chunk is well within the range of our query, so we know we want to process the next LOD. There is a single block at LOD0 
			 *   that we need to process that lies before the next LOD, so we process the block with value [5].
			 * - We are now at LOD1. The next LOD is LOD2. The time aligns perfectly with our LOD, and the end time of the block with value [4.25] is within the time range,
			 *   so we want to process it. There are no blocks leading to that LOD as the time aligns perfectly, so we don't process any blocks on this level.
			 * - We are at LOD2. The next LOD level aligns perfectly, but the end of the block passes beyond the time range, so we can't use it. We stop processing here.
			 *
			 * Part 2) The remainder of the algorithm is simple: we know what LODs we already have processed. We can start by the last processed LOD level and go down until we  
			 * processed all LOD levels. The only thing we need to do is see how many blocks fit within our time range, and process all of them. In our example:
			 *
			 * - We were at LOD2 so we start there. There is only one block to process here that is within range, so we process the block with value [4.25]. We proceed one level higher
			 * - We are at LOD1 and process only the block with value [3].
			 * - We are now at LOD0 and process only the block with value [6].
			 *
			 * As far as the weighting goes, each summarized block already has a value for the amount of seconds it was active. We know the full range of seconds that was queried,
			 * so each block receives a weight that corresponds to the active number of seconds: 
			 * 
			 *		weight = activeSecs / totalQuerySecs
			 */
			bool getRange(uint64_t startTime, uint64_t endTime, std::vector<std::unique_ptr<ReadingSummaryBase>>& readings, utility::ErrorState& errorState)
			{
				uint64_t cur_time_seconds = startTime;
				int total_active_seconds = 0;

				std::vector<DataModelInstance::WeightedObject> weighted_objects;

				// Part one of the algorithm: process all blocks leading to higher LODs
				int lod_index = 0;
				for (; lod_index < mLODs.size() - 1; ++lod_index)
				{
					// Find the start and end time of the next chunk of the next LOD
					uint64_t next_lod_duration = mLODs[lod_index + 1].mMaxNumSeconds;
					uint64_t next_lod_start_time = (cur_time_seconds / next_lod_duration) * next_lod_duration;
					if (next_lod_start_time < cur_time_seconds) 
						next_lod_start_time += next_lod_duration;
					uint64_t next_lod_end_time = next_lod_start_time + next_lod_duration;

					// If the next LOD level is out of the time range, we stop processing the first part of the algorithm
					if (next_lod_end_time >= endTime)
						break;

					// We do want to process the next LOD level, find how much time is leading to that level
					uint64_t delta = next_lod_start_time - cur_time_seconds;

					// If there is no time, we're perfectly aligned. There's nothing to do at this LOD level, proceed to next level
					if (delta <= 0)
						continue;

					// There is time leading up to the next LOD. Ask the database for all data in this timerange.
					if (!getWeightedObjects(mLODs[lod_index], cur_time_seconds, next_lod_start_time, total_active_seconds, weighted_objects, errorState))
						return false;

					// Progress time to the next LOD's start time
					cur_time_seconds = next_lod_start_time;
				}

				// Part two of the algorithm: process all blocks that fit within the time range, going from low detail to higher detail
				for (lod_index = lod_index; lod_index >= 0; --lod_index)
				{
					uint64_t current_lod_end_time = endTime / mLODs[lod_index].mMaxNumSeconds * mLODs[lod_index].mMaxNumSeconds;

					if (!getWeightedObjects(mLODs[lod_index], cur_time_seconds, current_lod_end_time, total_active_seconds, weighted_objects, errorState))
						return false;

					cur_time_seconds = current_lod_end_time;
				}

				// Fill in the correct weight based on the number of seconds the summary was active
				for (DataModelInstance::WeightedObject& weighted_object : weighted_objects)
				{
					ReadingSummaryBase* summary_base = rtti_cast<ReadingSummaryBase>(weighted_object.mObject.get());
					weighted_object.mWeight = (float)summary_base->mNumSecondsActive / (float)total_active_seconds;
				}

				// Create the final object. Call the summary object. Set start time to the start time of the summary
				// and set amount of active seconds to total amount of active seconds in this query
				std::unique_ptr<ReadingSummaryBase> collapsedObject = mSummaryFunction(weighted_objects);
				assert(collapsedObject->get_type() == mSummaryType);
				collapsedObject->mTimeStamp = secondsToTimeStamp(startTime);
				collapsedObject->mNumSecondsActive = total_active_seconds;

				readings.emplace_back(std::move(collapsedObject));
				return true;
			}


			/**
			 * Outer loop of the getRange function. For a detailed description, see the inner getRange function.
			 * This function simply cuts the requested range into a number of smaller queries with separate time ranges.
			 */
			bool getRange(TimeStamp startTime, TimeStamp endTime, int numValues, std::vector<std::unique_ptr<ReadingSummaryBase>>& readings, utility::ErrorState& errorState)
			{
				uint64_t start_seconds = timeStampToSeconds(startTime);
				uint64_t end_seconds = timeStampToSeconds(endTime);
				uint64_t full_range = end_seconds - start_seconds;

				// We count the offset and step size in doubles because step sizes may have fractions
				double step = (double)full_range / numValues;
				double curOffset = (double)start_seconds;

				for (int i = 0; i != numValues; ++i)
				{
					uint64_t start_time = std::round(curOffset);
					uint64_t end_time = std::round(curOffset + step);
					if (!getRange(start_time, end_time, readings, errorState))
						return false;

					curOffset += step;
				}

				return true;
			}


			TimeStamp getLastReadingTime() const
			{
				return mState->mLastReadingTime;
			}


			bool clearData(utility::ErrorState& errorState)
			{
				if (!mRawTable->clear(errorState))
					return false;

				if (!mStateTable->clear(errorState))
					return false;

				mState = std::make_unique<ReadingProcessorState>(rtti::getRTTIVersion(mReadingType));
				mRawReadingCache.clear();

				for (ReadingProcessorLOD& lod : mLODs)
				{
					if (!lod.mTable->clear(errorState))
						return false;

					if (!lod.mStateTable->clear(errorState))
						return false;

					lod.mState = std::make_unique<ReadingProcessorLODState>(rtti::getRTTIVersion(mSummaryType));
				}

				return true;
			}

		private:
			bool addLOD(const std::string& id, int maxNumSeconds, utility::ErrorState& errorState)
			{
				std::string state_table_name = id + "_state";

				rtti::Path id_path;
				id_path.pushAttribute(rtti::sIDPropertyName);

				std::unique_ptr<DatabasePropertyPath> lod_id_path = DatabasePropertyPath::sCreate(mSummaryType, id_path, errorState);
				if (lod_id_path == nullptr)
					return false;

				// Create LOD and summary table
				ReadingProcessorLOD lod;
				lod.mMaxNumSeconds = maxNumSeconds;

				std::unique_ptr<DatabasePropertyPath> state_id_path = DatabasePropertyPath::sCreate(RTTI_OF(ReadingProcessorLODState), id_path, errorState);
				if (state_id_path == nullptr)
					return false;

				// Initialize the state first so we can detect RTTI changes
				{
					lod.mStateTable = mDatabase->getOrCreateTable(state_table_name, RTTI_OF(ReadingProcessorLODState), { *state_id_path }, errorState);
					if (lod.mStateTable == nullptr)
						return false;

					std::vector<std::unique_ptr<rtti::Object>> state_object;
					if (!lod.mStateTable->query("", state_object, errorState))
						return false;

					if (!errorState.check(state_object.size() <= 1, "Found invalid number of state objects"))
						return false;

					size_t rtti_version = rtti::getRTTIVersion(mSummaryType);
					if (state_object.empty())
						lod.mState = std::make_unique<ReadingProcessorLODState>(rtti_version);
					else
						lod.mState = rtti_cast<ReadingProcessorLODState>(state_object[0]);

					if (!errorState.check(lod.mState->mRTTIVersion == rtti_version, "The structure of type '%s' has changed and is no longer compatible with the existing data", mSummaryType.get_name().data()))
						return false;
				}

				lod.mTable = mDatabase->getOrCreateTable(id, mSummaryType, { *lod_id_path }, errorState);
				if (lod.mTable == nullptr)
					return false;

				if (!lod.mTable->getOrCreateIndex(*mLODTimeStampPath, errorState))
					return false;				

				mLODs.emplace_back(std::move(lod));
				return true;
			}


			/**
			 * Retrieve objects from the database and creates a list of weightedObjects from them. The initial weight will be zero, the correct weight is filled in later.
			 */
			bool getWeightedObjects(ReadingProcessorLOD& lod, uint64_t startTime, uint64_t endTime, int& totalActiveSeconds, std::vector<DataModelInstance::WeightedObject>& weightedObjects, utility::ErrorState& errorState)
			{
				std::vector<std::unique_ptr<rtti::Object>> objects;
				const std::string timestamp_column_name = lod.mTable->getColumnName(*mLODTimeStampPath, errorState);
				if (timestamp_column_name.empty())
					return false;

				// Convert to TimeStamp time format, because this is the format that was serialized to the database
				TimeStamp start_timestamp = secondsToTimeStamp(startTime);
				TimeStamp end_timestamp = secondsToTimeStamp(endTime);

				std::string query = utility::stringFormat("%s >= %llu AND %s < %llu", timestamp_column_name.c_str(), start_timestamp.mTimeStamp, timestamp_column_name.c_str(), end_timestamp.mTimeStamp);
				if (!lod.mTable->query(query, objects, errorState))
					return false;

				// We create weighted objects here with a default weight of 0.0f. The real weight will be filled in at a later stage
				for (auto& object : objects)
				{
					ReadingSummaryBase* summary_base = rtti_cast<ReadingSummaryBase>(object.get());
					totalActiveSeconds += summary_base->mNumSecondsActive;

					DataModelInstance::WeightedObject weighted_object { 0.0f, std::move(object) };
					weightedObjects.emplace_back(std::move(weighted_object));
				}

				return true;
			}

		private:
			Database*									mDatabase;					///< Database object, owned by DataModel
			rtti::TypeInfo								mReadingType;				///< ReadingType for this processor
			rtti::TypeInfo								mSummaryType;				///< SummaryType for this processor
			DataModel::EKeepRawReadings					mKeepRawReadings;			///< Flag indicating whether to store raw reading in the database
			std::vector<ReadingProcessorLOD>			mLODs;						///< All LODs for this processor
			DataModelInstance::SummaryFunction			mSummaryFunction;			///< The summary function used to convert readings to summaries
			DatabaseTable*								mRawTable = nullptr;		///< The raw table used to store raw reading
			DatabaseTable*								mStateTable = nullptr;		///< The database table for storing state data
			std::vector<std::unique_ptr<rtti::Object>>	mRawReadingCache;			///< Window of raw readings, large enough to build one chunk of LOD0 data
			std::unique_ptr<DatabasePropertyPath>		mRawTimeStampPath;			///< Path to timestamp property for raw database table
			std::unique_ptr<DatabasePropertyPath>		mLODTimeStampPath;			///< Path to timestamp property for LOD database table
			std::unique_ptr<ReadingProcessorState>		mState;						///< Runtime state, serialized to db
		};

		//////////////////////////////////////////////////////////////////////////

		DataModelInstance::DataModelInstance(rtti::Factory& factory) :
			mDatabase(factory)
		{
		}

		DataModelInstance::~DataModelInstance()
		{
			utility::ErrorState errorState;
			flush(errorState);
		}

		bool DataModelInstance::init(const std::string& path, DataModel::EKeepRawReadings keepRawReadings, utility::ErrorState& errorState)
		{
			mKeepRawReadings = keepRawReadings;
			return mDatabase.init(path, errorState);
		}

		bool DataModelInstance::registerType(const rtti::TypeInfo& readingType, const rtti::TypeInfo& summaryType, const SummaryFunction& summaryFunction, utility::ErrorState& errorState)
		{
			std::unique_lock<std::mutex> lock(mLock);

			ReadingProcessorMap::iterator pos = mReadingProcessors.find(readingType);
			assert(pos == mReadingProcessors.end());

			std::unique_ptr<ReadingProcessor> processor = std::make_unique<ReadingProcessor>(mDatabase, readingType, summaryType, mKeepRawReadings, summaryFunction);
			if (!processor->init(errorState))
				return false;

			mReadingProcessors.insert(std::make_pair(readingType, std::move(processor)));
			return true;
		}

		std::vector<rtti::TypeInfo> DataModelInstance::getRegisteredTypes() const
		{
			std::vector<rtti::TypeInfo> types;
			std::unique_lock<std::mutex> lock(mLock);
			types.reserve(mReadingProcessors.size());
			for (const auto& processor : mReadingProcessors)
				types.emplace_back(processor.first);
			return types;
		}

		bool DataModelInstance::add(const ReadingBase& object, utility::ErrorState& errorState)
		{
			std::unique_lock<std::mutex> lock(mLock);

			ReadingProcessorMap::iterator processorPos = mReadingProcessors.find(object.get_type());
			assert(processorPos != mReadingProcessors.end());
			return processorPos->second->add(object, errorState);
		}

		bool DataModelInstance::flush(utility::ErrorState& errorState)
		{
			std::unique_lock<std::mutex> lock(mLock);

			for (auto& kvp : mReadingProcessors)
			{
				if (!kvp.second->flush(errorState))
					return false;
			}

			return true;
		}

		bool DataModelInstance::getRange(const rtti::TypeInfo& inReadingType, TimeStamp startTime, TimeStamp endTime, int numValues, std::vector<std::unique_ptr<ReadingSummaryBase>>& readings, utility::ErrorState& errorState)
		{
			std::unique_lock<std::mutex> lock(mLock);

			ReadingProcessorMap::iterator pos = mReadingProcessors.find(inReadingType);
			assert(pos != mReadingProcessors.end());

			return pos->second->getRange(startTime, endTime, numValues, readings, errorState);
		}

		TimeStamp DataModelInstance::getLastReadingTime(const rtti::TypeInfo& inReadingType) const
		{
			std::unique_lock<std::mutex> lock(mLock);

			ReadingProcessorMap::const_iterator pos = mReadingProcessors.find(inReadingType);
			assert(pos != mReadingProcessors.end());

			return pos->second->getLastReadingTime();
		}

		bool DataModelInstance::clearData(const rtti::TypeInfo& inReadingType, utility::ErrorState& errorState)
		{
			std::unique_lock<std::mutex> lock(mLock);

			ReadingProcessorMap::const_iterator pos = mReadingProcessors.find(inReadingType);
			assert(pos != mReadingProcessors.end());

			return pos->second->clearData(errorState);
		}
	}
}