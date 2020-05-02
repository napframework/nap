#pragma once

// local includes
#include "sequenceplayerinput.h"
#include "sequenceeventreceiver.h"

// nap includes
#include <nap/resourceptr.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	class NAPAPI SequencePlayerEventInput : public SequencePlayerInput
	{
		RTTI_ENABLE(SequencePlayerInput)
	public:
		ResourcePtr<SequenceEventReceiver> mReceiver;
	};
}