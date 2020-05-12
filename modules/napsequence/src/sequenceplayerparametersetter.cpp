#include "sequenceplayerparametersetter.h"
#include "sequenceservice.h"

namespace nap
{
	SequencePlayerParameterSetterBase::SequencePlayerParameterSetterBase(SequencePlayerCurveInput& input)
		: mInput(input)
	{
		mInput.registerParameterSetter(this);
	}


	SequencePlayerParameterSetterBase::~SequencePlayerParameterSetterBase()
	{
		mInput.removeParameterSetter(this);
	}
}
