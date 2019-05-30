#include "flexblockkeyframe.h"

#include <mathutils.h>

// nap::flexblockstance run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockKeyFrame)
	// Put additional properties here
	RTTI_PROPERTY("Inputs", &nap::FlexBlockKeyFrame::mInputs, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	FlexBlockKeyFrame::~FlexBlockKeyFrame()			{ }


	bool FlexBlockKeyFrame::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mInputs.size() == 8,
			"invalid number of inputs: %s, need 8", this->mID.c_str()))
			return false;

		for(int i = 0 ; i < 8; i++)
		{
			if (!errorState.check(mInputs[i] > 0.0f - math::epsilon<float>() && mInputs[i] < 1.0f + math::epsilon<float>() ,
				"input is out of range %s", this->mID.c_str()))
				return false;
		}

		return true;
	}
}