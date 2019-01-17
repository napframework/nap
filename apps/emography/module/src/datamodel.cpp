#include "datamodel.h"

namespace nap
{
	class ReadingProcessor
	{
	public:
		ReadingProcessor(Database& database, const rtti::TypeInfo& readingType, const rtti::TypeInfo& readingSummaryType, const DataModel::SummaryFunction& summaryFunction) :
			mDatabase(&database),
			mReadingType(readingType),
			mReadingSummaryType(readingSummaryType),
			mSummaryFunction(summaryFunction)
		{
		}

		bool init(utility::ErrorState& errorState)
		{
			std::string raw_table_name(mReadingType.get_name());
			mRawTable = mDatabase->createTable(raw_table_name, mReadingType, errorState);
			if (mRawTable == nullptr)
				return false;

			const int numLODs = 5;
			for (int i = 0; i != numLODs; ++i)
			{
				ReadingLOD lod;
				lod.mMaxNumSamples = (i + 1) * 4;
				lod.mCurNumSamples = 0;
				lod.mTable = mDatabase->createTable(utility::stringFormat("%s_LOD%d", raw_table_name.c_str(), i + 1), mReadingSummaryType, errorState);
				if (lod.mTable == nullptr)
					return false;

				mLODs.push_back(lod);
			}

			return true;
		}

		bool add(const rtti::Object& object, utility::ErrorState& errorState)
		{
			if (!mRawTable->add(object, errorState))
				return false;

			for (ReadingLOD& readingLOD : mLODs)
			{
				if (++readingLOD.mCurNumSamples % readingLOD.mMaxNumSamples == 0)
				{
					std::vector<rtti::Object*> lastObjects = mRawTable->getLast(readingLOD.mMaxNumSamples);
					/*
					std::unique_ptr<rtti::Object> collapsedObject = mCollapseFunction(lastObjects);
					assert(collapsedObject->get_type().is_derived_from(mReadingSummaryType));
					if (!readingLOD.mTable->add(*collapsedObject, errorState))
					return false;
					*/
				}
			}

			return true;
		}

		std::vector<rtti::Object*> getLast(int inLODIndex, int inCount)
		{
			if (inLODIndex == -1)
				return mRawTable->getLast(inCount);

			return mLODs[inLODIndex].mTable->getLast(inCount);
		}

	private:
		struct ReadingLOD
		{
			int				mMaxNumSamples = 0;
			int				mCurNumSamples = 0;
			DatabaseTable*	mTable = nullptr;
		};

		Database*					mDatabase;
		rtti::TypeInfo				mReadingType;
		rtti::TypeInfo				mReadingSummaryType;
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

	bool DataModel::registerType(const rtti::TypeInfo& readingType, const rtti::TypeInfo& readingSummaryType, const SummaryFunction& summaryFunction, utility::ErrorState& errorState)
	{
		ReadingProcessorMap::iterator pos = mReadingProcessors.find(readingType);
		assert(pos == mReadingProcessors.end());

		std::unique_ptr<ReadingProcessor> processor = std::make_unique<ReadingProcessor>(mDatabase, readingType, readingSummaryType, summaryFunction);
		if (!processor->init(errorState))
			return false;

		mReadingProcessors.insert(std::make_pair(readingType, std::move(processor)));
		return true;
	}

	bool DataModel::add(const rtti::Object& object, utility::ErrorState& errorState)
	{
		ReadingProcessorMap::iterator processorPos = mReadingProcessors.find(object.get_type());
		assert(processorPos != mReadingProcessors.end());
		return processorPos->second->add(object, errorState);
	}

	std::vector<rtti::Object*> DataModel::getLast(const rtti::TypeInfo& readingType, int lodIndex, int count)
	{
		ReadingProcessorMap::iterator pos = mReadingProcessors.find(readingType);
		assert(pos != mReadingProcessors.end());

		return pos->second->getLast(lodIndex, count);
	}
}