#pragma once

// internal includes
#include "sequencetracksegmentevent.h"

// external includes
#include <nap/resource.h>
#include <nap/signalslot.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * 
	 */
	class NAPAPI SequenceTrackEventDispatcher : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		nap::Signal<const SequenceTrackSegmentEvent&> mSignal;
	private:
		void dispatch(const SequenceTrackSegmentEvent &event)
		{
			mSignal.trigger(event);
		}
	};
}
