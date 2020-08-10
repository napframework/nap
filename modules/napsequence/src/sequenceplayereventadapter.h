#pragma once

// local includes
#include "sequenceplayer.h"
#include "sequenceplayeradapter.h"
#include "sequenceplayereventoutput.h"
#include "sequencetracksegmentevent.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	//
	class SequencePlayerEventOutput;

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
		 * @param time at which adapter is created
		 */
		SequencePlayerEventAdapter(SequenceTrack& track, SequencePlayerEventOutput& output, const SequencePlayer& player);

		/**
		 * Deconstructor
		 */
		virtual ~SequencePlayerEventAdapter() {}

		/**
		 * called from sequence player thread
		 * @param time time in sequence player
		 */
		virtual void tick(double time);
	private:
		// reference to track linked to adapter
		SequenceTrack& 			mTrack;

		// reference to receiver linked to adapter
		SequencePlayerEventOutput& 	mOutput;

		// list of dispatched events
		std::unordered_set<SequenceTrackSegmentEventBase*> mDispatchedEvents;

		//
		bool mPlayingBackwards = false;

		//
		double mPrevTime = 0.0;
	};
}