#include "sequenceplayercurveinput.h"

#include <nap/logger.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerCurveInput)
RTTI_PROPERTY("Parameter", &nap::SequencePlayerCurveInput::mParameter, nap::rtti::EPropertyMetaData::Default)
RTTI_PROPERTY("Use Main Thread", &nap::SequencePlayerCurveInput::mUseMainThread, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	SequencePlayerCurveInput::SequencePlayerCurveInput(SequenceService& service)
		: mSequenceService(&service)
	{
	}
}