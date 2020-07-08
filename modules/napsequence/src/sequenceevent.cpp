// local includes
#include "sequenceevent.h"

// Define base class
RTTI_DEFINE_BASE(nap::SequenceEventBase)

// Define sequence events
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceEventString)
	RTTI_CONSTRUCTOR(const std::string&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceEventFloat)
		RTTI_CONSTRUCTOR(const float&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceEventInt)
		RTTI_CONSTRUCTOR(const int&)
RTTI_END_CLASS

