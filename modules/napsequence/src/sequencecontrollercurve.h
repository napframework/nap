#pragma once

#include "sequencecontroller.h"

namespace nap
{
	class NAPAPI SequenceControllerCurve : public SequenceController
	{
	public:
		SequenceControllerCurve(SequencePlayer & player) : SequenceController(player) { }
	};
}