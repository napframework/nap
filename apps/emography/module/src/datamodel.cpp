#include "datamodel.h"
#include "emographyreading.h"

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

		public:
			TimeStamp	mLastReadingTime;
		};

		/**
		 * Runtime state for ReadingProcessorLOD. This is separated in an object so that it can be serialized to a database table.
		 */
		class ReadingProcessorLODState : public rtti::Object
		{
			RTTI_ENABLE(rtti::Object)
		public:

		public:
			uint64_t	mCurrentChunkIndex = -1;
		};
	}
}

RTTI_BEGIN_CLASS(nap::emography::ReadingProcessorState)
	RTTI_PROPERTY("LastReadingTime", &nap::emography::ReadingProcessorState::mLastReadingTime, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::emography::ReadingProcessorLODState)
	RTTI_PROPERTY("CurrentChunkIndex", &nap::emography::ReadingProcessorLODState::mCurrentChunkIndex, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

namespace nap
{
	namespace emography
	{
		/**
		 * Internally the database works with seconds, this converts whatever unit was used in TimeStamp into seconds.
		 */
		static TimeStamp sSecondsToTimeStamp(uint64_t seconds)
		{
			return TimeStamp(SystemTimeStamp(Seconds(seconds)));
		}


		/**
		 * Internally the database works with seconds, this converts whatever unit was used in TimeStamp to seconds.
		 */
		static uint64_t sTimeStampToSeconds(TimeStamp timeStamp)
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
			ReadingProcessor(Database& database, const rtti::TypeInfo& readingType, const rtti::TypeInfo& summaryType, DataModel::EKeepRawReadings keepRawReadings, const DataModel::SummaryFunction& summaryFunction) :
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

				// Create raw table and index
				std::string raw_table_name(mReadingType.get_name());
				mRawTable = mDatabase->getOrCreateTable(raw_table_name, mReadingType, { *raw_id_path }, errorState);
				if (mRawTable == nullptr)
					return false;

				// Create index for the raw table on the timestamp
				if (!mRawTable->getOrCreateIndex(*mRawTimeStampPath, errorState))
					return false;

				// Create processor state table
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
					if (state_object.empty())
						mState = std::make_unique<ReadingProcessorState>();
					else
						mState = rtti_cast<ReadingProcessorState>(state_object[0]);
				}

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

				uint64_t timestamp_seconds = sTimeStampToSeconds(timestamp);

				std::vector<std::unique_ptr<rtti::Object>> objects;
				std::vector<DataModel::WeightedObject> weighted_objects;

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
						TimeStamp prev_chunk_start_timestamp = sSecondsToTimeStamp(lod.mState->mCurrentChunkIndex * lod.mMaxNumSeconds);

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
							const std::string timestamp_column_name = mRawTimeStampPath->toString();
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

							DataModel::WeightedObject weighted_object{ weight, std::move(object) };
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


			bool getRange(uint64_t startTime, uint64_t endTime, std::vector<std::unique_ptr<ReadingSummaryBase>>& readings, utility::ErrorState& errorState)
			{
				uint64_t cur_time_seconds = startTime;
				int total_active_seconds = 0;

				std::vector<DataModel::WeightedObject> weighted_objects;

				int lod_index = 0;
				for (; lod_index < mLODs.size() - 1; ++lod_index)
				{
					uint64_t next_lod_duration = mLODs[lod_index + 1].mMaxNumSeconds;
					uint64_t next_lod_start_time = (cur_time_seconds / next_lod_duration) * next_lod_duration;
					if (next_lod_start_time < cur_time_seconds) 
						next_lod_start_time += next_lod_duration;
					uint64_t next_lod_end_time = next_lod_start_time + next_lod_duration;

					if (next_lod_end_time < endTime)
					{
						uint64_t delta = next_lod_start_time - cur_time_seconds;
						if (delta <= 0)
							continue;

						if (!getWeightedObjects(mLODs[lod_index], cur_time_seconds, next_lod_start_time, total_active_seconds, weighted_objects, errorState))
							return false;

						cur_time_seconds = next_lod_start_time;
					}
					else
					{
						uint64_t current_lod_end_time = endTime / mLODs[lod_index].mMaxNumSeconds * mLODs[lod_index].mMaxNumSeconds;

						if (!getWeightedObjects(mLODs[lod_index], cur_time_seconds, current_lod_end_time, total_active_seconds, weighted_objects, errorState))
							return false;

						cur_time_seconds = current_lod_end_time;
						break;
					}
				}

				for (lod_index = lod_index - 1; lod_index >= 0; --lod_index)
				{
					uint64_t current_lod_end_time = endTime / mLODs[lod_index].mMaxNumSeconds * mLODs[lod_index].mMaxNumSeconds;

					if (!getWeightedObjects(mLODs[lod_index], cur_time_seconds, current_lod_end_time, total_active_seconds, weighted_objects, errorState))
						return false;

					cur_time_seconds = current_lod_end_time;
				}

				for (DataModel::WeightedObject& weighted_object : weighted_objects)
				{
					ReadingSummaryBase* summary_base = rtti_cast<ReadingSummaryBase>(weighted_object.mObject.get());
					weighted_object.mWeight = (float)summary_base->mNumSecondsActive / (float)total_active_seconds;
				}

				std::unique_ptr<ReadingSummaryBase> collapsedObject = mSummaryFunction(weighted_objects);
				assert(collapsedObject->get_type() == mSummaryType);
				collapsedObject->mTimeStamp = sSecondsToTimeStamp(startTime);
				collapsedObject->mNumSecondsActive = total_active_seconds;

				readings.emplace_back(std::move(collapsedObject));
				return true;
			}

			bool getRange(TimeStamp startTime, TimeStamp endTime, int numValues, std::vector<std::unique_ptr<ReadingSummaryBase>>& readings, utility::ErrorState& errorState)
			{
				uint64_t start_seconds = sTimeStampToSeconds(startTime);
				uint64_t end_seconds = sTimeStampToSeconds(endTime);
				uint64_t full_range = end_seconds - start_seconds;
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

				mState = std::make_unique<ReadingProcessorState>();
				mRawReadingCache.clear();

				for (ReadingProcessorLOD& lod : mLODs)
				{
					if (!lod.mTable->clear(errorState))
						return false;

					if (!lod.mStateTable->clear(errorState))
						return false;

					lod.mState = std::make_unique<ReadingProcessorLODState>();
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

				ReadingProcessorLOD lod;
				lod.mMaxNumSeconds = maxNumSeconds;
				lod.mTable = mDatabase->getOrCreateTable(id, mSummaryType, { *lod_id_path }, errorState);
				if (lod.mTable == nullptr)
					return false;

				if (!lod.mTable->getOrCreateIndex(*mLODTimeStampPath, errorState))
					return false;

				std::unique_ptr<DatabasePropertyPath> state_id_path = DatabasePropertyPath::sCreate(RTTI_OF(ReadingProcessorLODState), id_path, errorState);
				if (state_id_path == nullptr)
					return false;

				lod.mStateTable = mDatabase->getOrCreateTable(state_table_name, RTTI_OF(ReadingProcessorLODState), { *state_id_path }, errorState);
				if (lod.mStateTable == nullptr)
					return false;

				std::vector<std::unique_ptr<rtti::Object>> state_object;
				if (!lod.mStateTable->query("", state_object, errorState))
					return false;

				if (!errorState.check(state_object.size() <= 1, "Found invalid number of state objects"))
					return false;

				if (state_object.empty())
					lod.mState = std::make_unique<ReadingProcessorLODState>();
				else
					lod.mState = rtti_cast<ReadingProcessorLODState>(state_object[0]);

				mLODs.emplace_back(std::move(lod));
				return true;
			}

			bool getWeightedObjects(ReadingProcessorLOD& lod, uint64_t startTime, uint64_t endTime, int& totalActiveSeconds, std::vector<DataModel::WeightedObject>& weightedObjects, utility::ErrorState& errorState)
			{
				std::vector<std::unique_ptr<rtti::Object>> objects;
				const std::string timestamp_column_name = mLODTimeStampPath->toString();

				// Convert to TimeStamp time format, because this is the format that was serialized to the database
				TimeStamp start_timestamp = sSecondsToTimeStamp(startTime);
				TimeStamp end_timestamp = sSecondsToTimeStamp(endTime);

				std::string query = utility::stringFormat("%s >= %llu AND %s < %llu", timestamp_column_name.c_str(), start_timestamp.mTimeStamp, timestamp_column_name.c_str(), end_timestamp.mTimeStamp);
				if (!lod.mTable->query(query, objects, errorState))
					return false;

				for (auto& object : objects)
				{
					ReadingSummaryBase* summary_base = rtti_cast<ReadingSummaryBase>(object.get());
					totalActiveSeconds += summary_base->mNumSecondsActive;

					DataModel::WeightedObject weighted_object { 0.0f, std::move(object) };
					weightedObjects.emplace_back(std::move(weighted_object));
				}

				return true;
			}

		private:
			Database*									mDatabase;
			std::unique_ptr<DatabasePropertyPath>		mRawTimeStampPath;
			std::unique_ptr<DatabasePropertyPath>		mLODTimeStampPath;
			rtti::TypeInfo								mReadingType;
			rtti::TypeInfo								mSummaryType;
			DataModel::EKeepRawReadings					mKeepRawReadings;
			std::vector<ReadingProcessorLOD>			mLODs;
			DataModel::SummaryFunction					mSummaryFunction;
			DatabaseTable*								mRawTable = nullptr;
			DatabaseTable*								mStateTable = nullptr;
			std::vector<std::unique_ptr<rtti::Object>>	mRawReadingCache;

			// Runtime state, serialized to db
			std::unique_ptr<ReadingProcessorState>		mState;
		};

		//////////////////////////////////////////////////////////////////////////

		DataModel::DataModel(rtti::Factory& factory) :
			mDatabase(factory)
		{
		}

		DataModel::~DataModel()
		{
			utility::ErrorState errorState;
			flush(errorState);
		}

		bool DataModel::init(const std::string& path, EKeepRawReadings keepRawReadings, utility::ErrorState& errorState)
		{
			mKeepRawReadings = keepRawReadings;
			return mDatabase.init(path, errorState);
		}

		bool DataModel::registerType(const rtti::TypeInfo& readingType, const rtti::TypeInfo& summaryType, const SummaryFunction& summaryFunction, utility::ErrorState& errorState)
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

		bool DataModel::add(const ReadingBase& object, utility::ErrorState& errorState)
		{
			std::unique_lock<std::mutex> lock(mLock);

			ReadingProcessorMap::iterator processorPos = mReadingProcessors.find(object.get_type());
			assert(processorPos != mReadingProcessors.end());
			return processorPos->second->add(object, errorState);
		}

		bool DataModel::flush(utility::ErrorState& errorState)
		{
			std::unique_lock<std::mutex> lock(mLock);

			for (auto& kvp : mReadingProcessors)
			{
				if (!kvp.second->flush(errorState))
					return false;
			}

			return true;
		}

		bool DataModel::getRange(const rtti::TypeInfo& inReadingType, TimeStamp startTime, TimeStamp endTime, int numValues, std::vector<std::unique_ptr<ReadingSummaryBase>>& readings, utility::ErrorState& errorState)
		{
			std::unique_lock<std::mutex> lock(mLock);

			ReadingProcessorMap::iterator pos = mReadingProcessors.find(inReadingType);
			assert(pos != mReadingProcessors.end());

			return pos->second->getRange(startTime, endTime, numValues, readings, errorState);
		}

		TimeStamp DataModel::getLastReadingTime(const rtti::TypeInfo& inReadingType) const
		{
			std::unique_lock<std::mutex> lock(mLock);

			ReadingProcessorMap::const_iterator pos = mReadingProcessors.find(inReadingType);
			assert(pos != mReadingProcessors.end());

			return pos->second->getLastReadingTime();
		}

		bool DataModel::clearData(const rtti::TypeInfo& inReadingType, utility::ErrorState& errorState)
		{
			std::unique_lock<std::mutex> lock(mLock);

			ReadingProcessorMap::const_iterator pos = mReadingProcessors.find(inReadingType);
			assert(pos != mReadingProcessors.end());

			return pos->second->clearData(errorState);
		}
	}
}