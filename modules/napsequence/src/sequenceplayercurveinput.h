#pragma once

// local includes
#include "sequenceplayerinput.h"
#include "sequenceservice.h"

// nap includes
#include <nap/resourceptr.h>
#include <parameter.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	class NAPAPI SequencePlayerCurveInput : public SequencePlayerInput
	{
		RTTI_ENABLE(SequencePlayerInput)
	public:
		SequencePlayerCurveInput(SequenceService& service);

		ResourcePtr<Parameter>	mParameter;
		bool					mUseMainThread;
		SequenceService*		mSequenceService;
	};
}
