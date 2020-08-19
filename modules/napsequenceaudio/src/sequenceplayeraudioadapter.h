#pragma once

// local includes
#include "sequenceplayer.h"
#include "sequenceplayeradapter.h"
#include "sequenceplayeraudiooutput.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	class SequencePlayerAudioAdapter : public SequencePlayerAdapter
	{
	public:
		SequencePlayerAudioAdapter(SequenceTrack& track, SequencePlayerAudioOutput& output, const SequencePlayer& player);

		/**
		* Deconstructor
		*/
		virtual ~SequencePlayerAudioAdapter();

		/**
		 * called from sequence player thread
		 * @param time time in sequence player
		 */
		virtual void tick(double time);
	private:
		// reference to track linked to adapter
		SequenceTrack& 			mTrack;

		// reference to receiver linked to adapter
		SequencePlayerAudioOutput& 	mOutput;
	};
}