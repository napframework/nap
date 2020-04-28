// local includes
#include "sequenceevent.h"

// Define base class
RTTI_DEFINE_BASE(nap::SequenceEvent)

// Define sequence event
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceEventString)
	RTTI_CONSTRUCTOR(const std::string&)
RTTI_END_CLASS

