#pragma once

// local includes
#include "sequenceplayerinput.h"
#include "sequenceeventreceiver.h"

// nap includes
#include <nap/resourceptr.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * SequencePlayerEventInput is used to link an SequenceEventReceiver to a SequenceEventTrack
	 */
	class NAPAPI SequencePlayerEventInput : public SequencePlayerInput
	{
		RTTI_ENABLE(SequencePlayerInput)
	public:
		ResourcePtr<SequenceEventReceiver> mReceiver; ///< Property: 'Event Receiver' event receiver resource
	};
}