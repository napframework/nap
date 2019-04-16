// Local Includes
#include "emographystress.h"

// nap::emography::EState enum definition 
RTTI_BEGIN_ENUM(nap::emography::EStressState)
	RTTI_ENUM_VALUE(nap::emography::EStressState::Under,	"Under"),
	RTTI_ENUM_VALUE(nap::emography::EStressState::Normal,	"Normal"),
	RTTI_ENUM_VALUE(nap::emography::EStressState::Over,		"Over"),
	RTTI_ENUM_VALUE(nap::emography::EStressState::Unknown,	"Unknown")
RTTI_END_ENUM

// nap::emography::Intensity class definition
RTTI_BEGIN_STRUCT(nap::emography::StressIntensity)
	RTTI_VALUE_CONSTRUCTOR(float)
	RTTI_PROPERTY("Value", &nap::emography::StressIntensity::mValue, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

DEFINE_READING_RTTI(nap::emography::StressIntensity)

RTTI_BEGIN_CLASS(nap::emography::Reading<nap::emography::EStressState>)
	RTTI_CONSTRUCTOR(const nap::emography::EStressState&, const nap::TimeStamp&)
	RTTI_CONSTRUCTOR(const nap::emography::EStressState&)
	RTTI_PROPERTY("Object", &nap::emography::Reading<nap::emography::EStressState>::mObject, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::emography::StressStateReadingSummary)
	RTTI_CONSTRUCTOR(const nap::emography::ReadingBase&)
	RTTI_PROPERTY("UnderCount", &nap::emography::StressStateReadingSummary::mUnderCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("NormalCount", &nap::emography::StressStateReadingSummary::mNormalCount, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OverCount", &nap::emography::StressStateReadingSummary::mOverCount, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	namespace emography
	{
		StressStateReadingSummary::StressStateReadingSummary(const ReadingBase& readingBase) :
			ReadingSummaryBase(readingBase)
		{
			const StressStateReading* reading = rtti_cast<const StressStateReading>(&readingBase);
			assert(reading);

			switch (reading->mObject)
			{
			case EStressState::Under:
				mUnderCount = 1;
				break;
			case EStressState::Normal:
				mNormalCount = 1;
				break;
			case EStressState::Over:
				mOverCount = 1;
				break;
			default:
				assert(false);
				break;
			}
		}


		void StressStateReadingSummary::add(const StressStateReadingSummary& other)
		{
			mUnderCount  += other.mUnderCount;
			mNormalCount += other.mNormalCount;
			mOverCount	 += other.mOverCount;
		}


		int StressStateReadingSummary::getCount(EStressState inState) const
		{
			switch (inState)
			{
			case EStressState::Normal:
				return mNormalCount;
			case EStressState::Over:
				return mOverCount;
			default:
				return mUnderCount;
			}
		}


		int StressStateReadingSummary::getTotalCount() const
		{
			return mUnderCount + mNormalCount + mOverCount;
		}


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
	}
}