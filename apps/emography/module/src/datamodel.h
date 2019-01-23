#pragma once

#include "utility/errorstate.h"
#include "database.h"
#include "nap/datetime.h"
#include "utility/dllexport.h"

namespace nap
{
	namespace emography
	{
		class ReadingProcessor;
		class ReadingBase;
		class ReadingSummaryBase;

		class NAPAPI DataModel final
		{
		public:
			enum class EKeepRawReadings : uint8_t
			{
				Enabled,
				Disabled
			};

			struct WeightedObject
			{
				float							mWeight;
				std::unique_ptr<rtti::Object>	mObject;
			};

			using SummaryFunction = std::function<std::unique_ptr<ReadingSummaryBase>(const std::vector<WeightedObject>&)>;

			DataModel();
			~DataModel();

			bool init(const std::string& path, EKeepRawReadings keepRawReadings, utility::ErrorState& errorState);
			bool add(const ReadingBase& object, utility::ErrorState& errorState);
			bool flush(utility::ErrorState& errorState);

			template<class ReadingType>
			bool registerType(const SummaryFunction& summaryFunction, utility::ErrorState& errorState)
			{
				return registerType(RTTI_OF(Reading<ReadingType>), RTTI_OF(ReadingSummary<ReadingType>), summaryFunction, errorState);
			}

			template<class ReadingType>
			bool getRange(TimeStamp startTime, TimeStamp endTime, int numValues, std::vector<std::unique_ptr<ReadingSummaryBase>>& readings, utility::ErrorState& errorState)
			{
				return getRange(RTTI_OF(ReadingType), startTime, endTime, numValues, readings, errorState);
			}

		private:
			bool registerType(const rtti::TypeInfo& readingType, const rtti::TypeInfo& summaryType, const SummaryFunction& summaryFunction, utility::ErrorState& errorState);
			bool getRange(const rtti::TypeInfo& inReadingType, TimeStamp startTime, TimeStamp endTime, int numValues, std::vector<std::unique_ptr<ReadingSummaryBase>>& readings, utility::ErrorState& errorState);

		private:
			using ReadingProcessorMap = std::unordered_map<rtti::TypeInfo, std::unique_ptr<ReadingProcessor>>;
			Database			mDatabase;
			ReadingProcessorMap mReadingProcessors;
			EKeepRawReadings	mKeepRawReadings;
		};
	}
}