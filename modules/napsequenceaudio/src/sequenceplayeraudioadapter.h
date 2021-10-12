/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// nap includes
#include <sequenceplayer.h>
#include <sequenceplayeradapter.h>

// local include
#include "sequencetracksegmentaudio.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequencePlayerAudioOutput;

	/**
	 * The SequencePlayerAudioAdapter is responsible for handling ticks from the sequence player and looking up which
	 * audio buffer should be played by the audio output and when.
	 */
	class SequencePlayerAudioAdapter final : public SequencePlayerAdapter
	{
	public:
		/**
		 * Constructor
		 * @param track reference to track
		 * @param output reference to audio output
		 * @param player reference to player
		 */
		SequencePlayerAudioAdapter(const SequenceTrack& track, SequencePlayerAudioOutput& output, const SequencePlayer& player);

		/**
		 * called by sequence player
		 * @param time the time in the sequence
		 */
		void tick(double time) override;

		/**
		 * called before deconstruction of the adapter
		 */
		void destroy() override;
	private:
		// reference to track linked to adapter
		const SequenceTrack& mTrack;

		// reference to output linked to adapter
		SequencePlayerAudioOutput& 	mOutput;

		// reference to player
		const SequencePlayer& mPlayer;

		// the current segment ID being player
		std::string mCurrentStartedSegmentID;

		// previous time in segment
		double mPrevTimeInSegment = 0.0;

		// previous time in sequence player, used to calculate delta time
		double mPrevTime = 0.0;
	};
}