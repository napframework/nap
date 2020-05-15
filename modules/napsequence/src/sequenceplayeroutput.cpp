// local includes
#include "sequenceplayeroutput.h"
#include "sequenceservice.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerOutput)
RTTI_END_CLASS

namespace  nap
{
	SequencePlayerOutput::SequencePlayerOutput(SequenceService& service)
		: mService(&service)
	{
	}


	bool SequencePlayerOutput::init(utility::ErrorState& errorState)
	{
		mService->registerOutput(*this);

		return true;
	}


	void SequencePlayerOutput::onDestroy()
	{
		mService->removeOutput(*this);
	}
}
