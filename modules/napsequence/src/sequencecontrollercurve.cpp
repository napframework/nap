#include "sequencecontrollercurve.h"

namespace nap
{
	static bool sRegistered = SequenceController::registerControllerFactory(RTTI_OF(SequenceControllerCurve), [](SequencePlayer& player)->std::unique_ptr<SequenceController> { return std::make_unique<SequenceControllerCurve>(player); });
}