#pragma once

#include "sequencecontroller.h"
#include "sequenceplayer.h"

namespace nap
{
	class NAPAPI SequenceControllerPlayer : public SequenceController
	{
	public:
		SequenceControllerPlayer(SequencePlayer & player) : SequenceController(player) { }

		SequencePlayer& getSequencePlayer() { return mPlayer; }
	private:
	};
}
