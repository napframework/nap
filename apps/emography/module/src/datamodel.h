#include "utility/errorstate.h"
#include "database.h"

namespace nap
{
	namespace emography
	{
		class ReadingProcessor;
		class ReadingBase;

		class NAPAPI DataModel
		{
		public:
			using SummaryFunction = std::function<std::unique_ptr<ReadingBase>(const std::vector<std::unique_ptr<rtti::Object>>&)>;

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

		private:
			bool registerType(const rtti::TypeInfo& readingType, const SummaryFunction& summaryFunction, utility::ErrorState& errorState);
			bool getLast(const rtti::TypeInfo& inReadingType, int lodIndex, int count, std::vector<std::unique_ptr<rtti::Object>>& objects, utility::ErrorState& errorState);

		private:
			using ReadingProcessorMap = std::unordered_map<rtti::TypeInfo, std::unique_ptr<ReadingProcessor>>;
			Database			mDatabase;
			ReadingProcessorMap mReadingProcessors;
		};
	}
}