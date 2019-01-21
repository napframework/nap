#include "utility/errorstate.h"
#include "database.h"
#include "nap/datetime.h"

namespace nap
{
	namespace emography
	{
		class ReadingProcessor;
		class ReadingBase;

		class NAPAPI DataModel
		{
		public:
			struct WeightedObject
			{
				float							mWeight;
				std::unique_ptr<rtti::Object>	mObject;
			};

			using SummaryFunction = std::function<std::unique_ptr<ReadingBase>(const std::vector<WeightedObject>&)>;

			DataModel();
			~DataModel();

			bool init(utility::ErrorState& errorState);
			bool add(const ReadingBase& object, utility::ErrorState& errorState);

			template<class ReadingType>
			bool registerType(const SummaryFunction& summaryFunction, utility::ErrorState& errorState)
			{
				return registerType(RTTI_OF(Reading<ReadingType>), summaryFunction, errorState);
			}

			template<class ReadingType>
			bool getLast(int lodIndex, int count, std::vector<std::unique_ptr<rtti::Object>>& objects, utility::ErrorState& errorState)
			{
				return getLast(RTTI_OF(ReadingType), lodIndex, count, objects, errorState);
			}

			template<class ReadingType>
			bool getRange(TimeStamp startTime, TimeStamp endTime, int numValues, std::vector<std::unique_ptr<ReadingBase>>& readings, utility::ErrorState& errorState)
			{
				return getRange(RTTI_OF(ReadingType), startTime, endTime, numValues, readings, errorState);
			}

		private:
			bool registerType(const rtti::TypeInfo& readingType, const SummaryFunction& summaryFunction, utility::ErrorState& errorState);
			bool getLast(const rtti::TypeInfo& inReadingType, int lodIndex, int count, std::vector<std::unique_ptr<rtti::Object>>& objects, utility::ErrorState& errorState);
			bool getRange(const rtti::TypeInfo& inReadingType, TimeStamp startTime, TimeStamp endTime, int numValues, std::vector<std::unique_ptr<ReadingBase>>& readings, utility::ErrorState& errorState);

		private:
			using ReadingProcessorMap = std::unordered_map<rtti::TypeInfo, std::unique_ptr<ReadingProcessor>>;
			Database			mDatabase;
			ReadingProcessorMap mReadingProcessors;
		};
	}
}