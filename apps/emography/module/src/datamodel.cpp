#include "datamodel.h"
#include "emographysnapshot.h"

namespace nap
{
	namespace emography
	{
		class ReadingProcessor
		{
		public:
			ReadingProcessor(Database& database, const rtti::TypeInfo& readingType, const DataModel::SummaryFunction& summaryFunction) :
				mDatabase(&database),
				mReadingType(readingType),
				mSummaryFunction(summaryFunction)
			{
			}

			bool init(utility::ErrorState& errorState)
			{
				std::string raw_table_name(mReadingType.get_name());
				mRawTable = mDatabase->createTable(raw_table_name, mReadingType, errorState);
				if (mRawTable == nullptr)
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
				if (!mRawTable->add(object, errorState))
					return false;

				uint64_t timestamp_in_seconds = object.mTimeStamp.mTimeStamp / 1000;

				std::vector<std::unique_ptr<rtti::Object>> objects;
				std::vector<DataModel::WeightedObject> weighted_objects;

				DatabaseTable* prevTable = mRawTable;
				for (int index = 0; index < mLODs.size(); ++index)
				{
					ReadingLOD& lod = mLODs[index];
					uint64_t chunk_index = timestamp_in_seconds / lod.mMaxNumSeconds;
					if (chunk_index == lod.mCurrentChunkIndex)
						break;

					if (lod.mCurrentChunkIndex != -1)
					{
						uint64_t prev_chunk_start_time_seconds = lod.mCurrentChunkIndex * lod.mMaxNumSeconds;
						
						objects.clear();
						weighted_objects.clear();

						if (!prevTable->query(utility::stringFormat("\"TimeStamp.Time\" >= %llu", prev_chunk_start_time_seconds * 1000), objects, errorState))
							return false;

						weighted_objects.reserve(objects.size());
						float weight = 1.0f / objects.size();
						for (auto& object : objects)
						{
							DataModel::WeightedObject weighted_object{ weight, std::move(object) };
							weighted_objects.emplace_back(std::move(weighted_object));
						}

						std::unique_ptr<ReadingBase> collapsedObject = mSummaryFunction(weighted_objects);
						assert(collapsedObject->get_type() == mReadingType);
						collapsedObject->mTimeStamp.mTimeStamp = prev_chunk_start_time_seconds * 1000;

						if (!lod.mTable->add(*collapsedObject, errorState))
							return false;
					}

					lod.mCurrentChunkIndex = chunk_index;
					prevTable = lod.mTable;
				}

				return true;
			}

			bool getRange(uint64_t startTime, uint64_t endTime, std::vector<std::unique_ptr<ReadingBase>>& readings, utility::ErrorState& errorState)
			{
				uint64_t cur_time_seconds = startTime;
				uint64_t full_range = endTime - startTime;

				std::vector<DataModel::WeightedObject> weighted_objects;

				int lod_index = 0;
				for (; lod_index < mLODs.size() - 1; ++lod_index)
				{
					uint64_t next_lod_duration = mLODs[lod_index + 1].mMaxNumSeconds;
					uint64_t next_lod_start_time = (cur_time_seconds / next_lod_duration) * next_lod_duration + next_lod_duration;
					uint64_t next_lod_end_time = next_lod_start_time + next_lod_duration;

					if (next_lod_end_time < endTime)
					{
						uint64_t delta = next_lod_start_time - cur_time_seconds;
						if (delta <= 0)
							continue;

						if (!getWeightedObjects(mLODs[lod_index], cur_time_seconds, next_lod_start_time, full_range, weighted_objects, errorState))
							return false;

						cur_time_seconds = next_lod_start_time;
					}
					else
					{
						uint64_t current_lod_end_time = endTime / mLODs[lod_index].mMaxNumSeconds * mLODs[lod_index].mMaxNumSeconds;

						if (!getWeightedObjects(mLODs[lod_index], cur_time_seconds, current_lod_end_time, full_range, weighted_objects, errorState))
							return false;

						cur_time_seconds = current_lod_end_time;
						break;
					}
				}

				for (lod_index = lod_index - 1; lod_index >= 0; --lod_index)
				{
					uint64_t current_lod_end_time = endTime / mLODs[lod_index].mMaxNumSeconds * mLODs[lod_index].mMaxNumSeconds;

					if (!getWeightedObjects(mLODs[lod_index], cur_time_seconds, current_lod_end_time, full_range, weighted_objects, errorState))
						return false;

					cur_time_seconds = current_lod_end_time;
				}

				std::unique_ptr<ReadingBase> collapsedObject = mSummaryFunction(weighted_objects);
				assert(collapsedObject->get_type() == mReadingType);
				collapsedObject->mTimeStamp.mTimeStamp = startTime * 1000;

				readings.emplace_back(std::move(collapsedObject));
				return true;
			}

			bool getRange(TimeStamp startTime, TimeStamp endTime, int numValues, std::vector<std::unique_ptr<ReadingBase>>& readings, utility::ErrorState& errorState)
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

			bool getLast(int inLODIndex, int inCount, std::vector<std::unique_ptr<rtti::Object>>& objects, utility::ErrorState& errorState)
			{
				if (inLODIndex == -1)
					return mRawTable->getLast(inCount, objects, errorState);

				return mLODs[inLODIndex].mTable->getLast(inCount, objects, errorState);
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
				lod.mTable = mDatabase->createTable(id, mReadingType, errorState);
				if (lod.mTable == nullptr)
					return false;

				mLODs.push_back(lod);
				return true;
			}

			bool getWeightedObjects(ReadingLOD& lod, uint64_t startTime, uint64_t endTime, uint64_t fullQueryRange, std::vector<DataModel::WeightedObject>& weightedObjects, utility::ErrorState& errorState)
			{
				std::vector<std::unique_ptr<rtti::Object>> objects;
				std::string query = utility::stringFormat("\"TimeStamp.Time\" >= %llu AND \"TimeStamp.Time\" < %llu", startTime * 1000, endTime * 1000);
				if (!lod.mTable->query(query, objects, errorState))
					return false;

				float weight = (float)lod.mMaxNumSeconds / (float)fullQueryRange;
				for (auto& object : objects)
				{
					DataModel::WeightedObject weighted_object { weight, std::move(object) };
					weightedObjects.emplace_back(std::move(weighted_object));
				}

				return true;
			}

		private:


			Database*					mDatabase;
			rtti::TypeInfo				mReadingType;
			std::vector<ReadingLOD>		mLODs;
			DataModel::SummaryFunction	mSummaryFunction;
			DatabaseTable*				mRawTable;
		};

		//////////////////////////////////////////////////////////////////////////

		DataModel::DataModel()
		{
		}

		DataModel::~DataModel()
		{
		}

		bool DataModel::init(utility::ErrorState& errorState)
		{
			return mDatabase.init(errorState);
		}

		bool DataModel::registerType(const rtti::TypeInfo& readingType, const SummaryFunction& summaryFunction, utility::ErrorState& errorState)
		{
			ReadingProcessorMap::iterator pos = mReadingProcessors.find(readingType);
			assert(pos == mReadingProcessors.end());

			std::unique_ptr<ReadingProcessor> processor = std::make_unique<ReadingProcessor>(mDatabase, readingType, summaryFunction);
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

		bool DataModel::getLast(const rtti::TypeInfo& readingType, int lodIndex, int count, std::vector<std::unique_ptr<rtti::Object>>& objects, utility::ErrorState& errorState)
		{
			ReadingProcessorMap::iterator pos = mReadingProcessors.find(readingType);
			assert(pos != mReadingProcessors.end());

			return pos->second->getLast(lodIndex, count, objects, errorState);
		}

		bool DataModel::getRange(const rtti::TypeInfo& inReadingType, TimeStamp startTime, TimeStamp endTime, int numValues, std::vector<std::unique_ptr<ReadingBase>>& readings, utility::ErrorState& errorState)
		{
			ReadingProcessorMap::iterator pos = mReadingProcessors.find(inReadingType);
			assert(pos != mReadingProcessors.end());

			return pos->second->getRange(startTime, endTime, numValues, readings, errorState);
		}
	}
}