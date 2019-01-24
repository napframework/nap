#pragma once

// Local Includes
#include "datamodel.h"
#include "emographyreading.h"

namespace nap
{
	namespace emography
	{
		template<typename T>
		std::unique_ptr<ReadingSummary<T>> gAveragingSummary(const std::vector<DataModel::WeightedObject>& inObjects)
		{
			using Type = decltype(ReadingSummary<T>().mObject.mValue);

			double total = 0.0;
			for (int index = 0; index < inObjects.size(); ++index)
			{
				rtti::Object* object = inObjects[index].mObject.get();
				ReadingSummary<T>* reading_summary = rtti_cast<ReadingSummary<T>>(object);
				assert(reading_summary != nullptr);

				total += reading_summary->mObject.mValue * inObjects[index].mWeight;
			}

			return std::make_unique<StressIntensityReadingSummary>(Reading<T>((Type)total));
		}
	}
}
