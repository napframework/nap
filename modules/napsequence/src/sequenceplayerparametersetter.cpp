#include "sequenceplayerparametersetter.h"
#include "sequenceservice.h"

namespace nap
{
	SequencePlayerParameterSetterBase::SequencePlayerParameterSetterBase(SequenceService& service)
		: mService(service)
	{
		mService.registerParameterSetter(*this);
	}


	SequencePlayerParameterSetterBase::~SequencePlayerParameterSetterBase()
	{
		mService.removeParameterSetter(*this);
	}
}
