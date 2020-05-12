// local includes
#include "sequenceplayerinput.h"
#include "sequenceservice.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerInput)
RTTI_END_CLASS

namespace  nap
{
	SequencePlayerInput::SequencePlayerInput(SequenceService& service)
		: mService(&service)
	{
	}


	bool SequencePlayerInput::init(utility::ErrorState& errorState)
	{
		mService->registerInput(*this);

		return true;
	}


	void SequencePlayerInput::onDestroy()
	{
		mService->removeInput(*this);
	}
}
