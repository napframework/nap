#include "sequencecontroller.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	std::unordered_map<rttr::type, SequenceControllerFactoryFunc>& SequenceController::getControllerFactory()
	{
		static std::unordered_map<rttr::type, SequenceControllerFactoryFunc> factory;
		return factory;
	}

	bool SequenceController::registerControllerFactory(rttr::type type, SequenceControllerFactoryFunc func)
	{
		auto& factory = getControllerFactory();
		auto it = factory.find(type);
		assert(it == factory.end()); // duplicate entry
		if (it == factory.end())
		{
			factory.emplace(type, func);

			return true;
		}

		return false;
	}
}