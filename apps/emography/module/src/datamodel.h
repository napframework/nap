#include "utility/errorstate.h"
#include "database.h"

namespace nap
{
	class ReadingProcessor;

	class NAPAPI DataModel
	{
	public:
		using SummaryFunction = std::function<std::unique_ptr<rtti::Object>(const std::vector<rtti::Object*>&)>;
		
		DataModel();
		~DataModel();

		bool init(utility::ErrorState& errorState);
		bool add(const rtti::Object& object, utility::ErrorState& errorState);

		template<class ReadingType>
		bool registerType(const SummaryFunction& summaryFunction, utility::ErrorState& errorState)
		{
			return registerType(RTTI_OF(Reading<ReadingType>), RTTI_OF(ReadingSummary<ReadingType>), summaryFunction, errorState);
		}

		template<class ReadingType>
		std::vector<rtti::Object*> getLast(int lodIndex, int count)
		{
			return getLast(RTTI_OF(ReadingType), lodIndex, count);
		}

	private:
		bool registerType(const rtti::TypeInfo& readingType, const rtti::TypeInfo& readingSummaryType, const SummaryFunction& summaryFunction, utility::ErrorState& errorState);
		std::vector<rtti::Object*> getLast(const rtti::TypeInfo& inReadingType, int lodIndex, int count);

	private:
		using ReadingProcessorMap = std::unordered_map<rtti::TypeInfo, std::unique_ptr<ReadingProcessor>>;
		Database			mDatabase;
		ReadingProcessorMap mReadingProcessors;
	};
}