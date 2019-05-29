#include "flexblockstance.h"

#include <mathutils.h>

// nap::flexblockstance run time class definition 
RTTI_BEGIN_CLASS(nap::FlexBlockStance)
	// Put additional properties here
	RTTI_PROPERTY("Inputs", &nap::FlexBlockStance::mInputs, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	FlexBlockStance::~FlexBlockStance()			{ }


	bool FlexBlockStance::init(utility::ErrorState& errorState)
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