#pragma once

#include "sequencecontroller.h"

namespace nap
{
	class NAPAPI SequenceControllerPlayer : public SequenceController
	{
	public:
		SequenceControllerPlayer(SequencePlayer & player) : SequenceController(player) { }
	private:
	};
}
