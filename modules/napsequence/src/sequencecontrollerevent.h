#pragma once

#include "sequencecontroller.h"

#include <nap/logger.h>

namespace nap
{
	class NAPAPI SequenceControllerEvent : public SequenceController
	{
	public:
		SequenceControllerEvent(SequencePlayer & player) : SequenceController(player) { }

		void test() { nap::Logger::info("hello"); }
	private:
	};
}
