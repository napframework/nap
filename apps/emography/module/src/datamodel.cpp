#include "datamodel.h"
#include "emographysnapshot.h"

namespace nap
{
	namespace emography
	{
		class ReadingProcessor
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

				mRawTimeStampPath = DatabasePropertyPath::sCreate(mReadingType, timestamp_path, errorState);
				if (mRawTimeStampPath == nullptr)
					return false;

				mLODTimeStampPath = DatabasePropertyPath::sCreate(mSummaryType, timestamp_path, errorState);
				if (mLODTimeStampPath == nullptr)
					return false;

				std::string raw_table_name(mReadingType.get_name());
				mRawTable = mDatabase->createTable(raw_table_name, mReadingType, errorState);
				if (mRawTable == nullptr)
					return false;

				if (!mRawTable->createIndex(*mRawTimeStampPath, errorState))
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
				if (!flush(object.mTimeStamp, errorState))
					return false;

				if (mKeepRawReadings == DataModel::EKeepRawReadings::Enabled)
				{
					if (!mRawTable->add(object, errorState))
						return false;
				}

				std::unique_ptr<rtti::Object> raw_summary(mSummaryType.create<ReadingSummaryBase>({ object }));
				mRawReadingCache.emplace_back(std::move(raw_summary));

				mLastReadingTime = object.mTimeStamp;

				return true;
			}

			bool flush(utility::ErrorState& errorState)
			{
				if (!mLastReadingTime.isValid())
					return true;

				auto nextTimeStamp = mLastReadingTime.toSystemTime() + Seconds(1);
				return flush(nextTimeStamp, errorState);
			}

			bool flush(TimeStamp timestamp, utility::ErrorState& errorState)
			{
				uint64_t timestamp_in_seconds = timestamp.mTimeStamp / 1000;

				std::vector<std::unique_ptr<rtti::Object>> objects;
				std::vector<DataModel::WeightedObject> weighted_objects;

				DatabaseTable* prevTable = nullptr;
				for (int lod_index = 0; lod_index < mLODs.size(); ++lod_index)
				{
					ReadingLOD& lod = mLODs[lod_index];
					uint64_t chunk_index = timestamp_in_seconds / lod.mMaxNumSeconds;
					if (chunk_index == lod.mCurrentChunkIndex)
						break;

					if (lod.mCurrentChunkIndex != -1 && chunk_index < lod.mCurrentChunkIndex)
						break;

					if (lod.mCurrentChunkIndex != -1)
					{
						uint64_t prev_chunk_start_time_seconds = lod.mCurrentChunkIndex * lod.mMaxNumSeconds;

						objects.clear();
						weighted_objects.clear();

						if (prevTable == nullptr)
						{
							objects = std::move(mRawReadingCache);
						}
						else
						{
							const std::string timestamp_column_name = mRawTimeStampPath->toString();
							if (!prevTable->query(utility::stringFormat("%s >= %llu", timestamp_column_name.c_str(), prev_chunk_start_time_seconds * 1000), objects, errorState))
								return false;
						}

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

						std::unique_ptr<ReadingSummaryBase> collapsedObject = mSummaryFunction(weighted_objects);
						assert(collapsedObject->get_type() == mReadingType);
						collapsedObject->mTimeStamp.mTimeStamp = prev_chunk_start_time_seconds * 1000;
						collapsedObject->mNumSecondsActive = num_active_seconds;

						if (!lod.mTable->add(*collapsedObject, errorState))
							return false;
					}

					lod.mCurrentChunkIndex = chunk_index;
					prevTable = lod.mTable;
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
				assert(collapsedObject->get_type() == mReadingType);
				collapsedObject->mTimeStamp.mTimeStamp = startTime * 1000;
				collapsedObject->mNumSecondsActive = total_active_seconds;

				readings.emplace_back(std::move(collapsedObject));
				return true;
			}

			bool getRange(TimeStamp startTime, TimeStamp endTime, int numValues, std::vector<std::unique_ptr<ReadingSummaryBase>>& readings, utility::ErrorState& errorState)
			{
				uint64_t start_seconds = startTime.mTimeStamp / 1000;
				uint64_t end_seconds = endTime.mTimeStamp / 1000;
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

		private:
			struct ReadingLOD
			{
				int				mMaxNumSeconds = 0;
				uint64_t		mCurrentChunkIndex = -1;
				DatabaseTable*	mTable = nullptr;
			};

			bool addLOD(const std::string& id, int maxNumSeconds, utility::ErrorState& errorState)
			{
				std::string raw_table_name(mReadingType.get_name());

				ReadingLOD lod;
				lod.mMaxNumSeconds = maxNumSeconds;
				lod.mTable = mDatabase->createTable(id, mSummaryType, errorState);
				if (lod.mTable == nullptr)
					return false;

				if (!lod.mTable->createIndex(*mLODTimeStampPath, errorState))
					return false;

				mLODs.push_back(lod);
				return true;
			}

			bool getWeightedObjects(ReadingLOD& lod, uint64_t startTime, uint64_t endTime, int& totalActiveSeconds, std::vector<DataModel::WeightedObject>& weightedObjects, utility::ErrorState& errorState)
			{
				std::vector<std::unique_ptr<rtti::Object>> objects;
				const std::string timestamp_column_name = mLODTimeStampPath->toString();
				std::string query = utility::stringFormat("%s >= %llu AND %s < %llu", timestamp_column_name.c_str(), startTime * 1000, timestamp_column_name.c_str(), endTime * 1000);
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
			std::vector<ReadingLOD>						mLODs;
			DataModel::SummaryFunction					mSummaryFunction;
			DatabaseTable*								mRawTable;
			std::vector<std::unique_ptr<rtti::Object>>	mRawReadingCache;
			TimeStamp									mLastReadingTime;
		};

		//////////////////////////////////////////////////////////////////////////

		DataModel::DataModel()
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
			ReadingProcessorMap::iterator processorPos = mReadingProcessors.find(object.get_type());
			assert(processorPos != mReadingProcessors.end());
			return processorPos->second->add(object, errorState);
		}

		bool DataModel::flush(utility::ErrorState& errorState)
		{
			for (auto& kvp : mReadingProcessors)
			{
				if (!kvp.second->flush(errorState))
					return false;
			}

			return true;
		}

		bool DataModel::getRange(const rtti::TypeInfo& inReadingType, TimeStamp startTime, TimeStamp endTime, int numValues, std::vector<std::unique_ptr<ReadingSummaryBase>>& readings, utility::ErrorState& errorState)
		{
			ReadingProcessorMap::iterator pos = mReadingProcessors.find(inReadingType);
			assert(pos != mReadingProcessors.end());

			return pos->second->getRange(startTime, endTime, numValues, readings, errorState);
		}
	}
}