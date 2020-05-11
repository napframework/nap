#pragma once

// local includes
#include "sequenceplayer.h"
#include "sequenceplayeradapter.h"
#include "sequenceplayereventinput.h"
#include "sequenceplayereventadapter.h"
#include "sequencetracksegmentevent.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Adapter responsible for handling events from an event track and sync them with the main thread using a
	 * sequence event receiver intermediate class.
	 */
	class SequencePlayerEventAdapter : public SequencePlayerAdapter
	{
	public:
		/**
		 * Constructor
		 * @param track reference to sequence event track
		 * @param receiver reference to event receiver
		 */
		SequencePlayerEventAdapter(SequenceTrack& track, SequenceEventReceiver& receiver);

		/**
		 * Deconstructor
		 */
		virtual ~SequencePlayerEventAdapter() {}

		/**
		 * called from sequence player thread
		 * @param time time in sequence player
		 */
		virtual void update(double time);
	private:
		// reference to track linked to adapter
		SequenceTrack& 			mTrack;

		// reference to receiver linked to adapter
		SequenceEventReceiver& 	mReceiver;

		// list of dispatched events
		std::unordered_set<SequenceTrackSegmentEventBase*> mDispatchedEvents;
	};
}