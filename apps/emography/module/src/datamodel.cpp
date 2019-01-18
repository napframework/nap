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
						if (!prevTable->query(utility::stringFormat("\"TimeStamp.Time\" >= %llu", prev_chunk_start_time_seconds), objects, errorState))
							return false;

						std::unique_ptr<ReadingBase> collapsedObject = mSummaryFunction(objects);
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

			bool getLast(int inLODIndex, int inCount, std::vector<std::unique_ptr<rtti::Object>>& objects, utility::ErrorState& errorState)
			{
				if (inLODIndex == -1)
					return mRawTable->getLast(inCount, objects, errorState);

				return mLODs[inLODIndex].mTable->getLast(inCount, objects, errorState);
			}

		private:
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

		private:
			struct ReadingLOD
			{
				int				mMaxNumSeconds = 0;
				uint64_t		mCurrentChunkIndex = -1;
				DatabaseTable*	mTable = nullptr;
			};

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
	}
}