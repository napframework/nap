#include "parameterblendgroup.h"

// nap::animablendparameters run time class definition 
RTTI_BEGIN_CLASS(nap::ParameterBlendGroup)
	RTTI_PROPERTY("Parameters",		&nap::ParameterBlendGroup::mParameters,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ParameterGroup",	&nap::ParameterBlendGroup::mParameterGroup, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	ParameterBlendGroup::~ParameterBlendGroup()			{ }


	bool ParameterBlendGroup::init(utility::ErrorState& errorState)
	{
		// Ensure the declared parameters are part of the parameter or parameter group children
		for (const auto& parameter : mParameters)
		{
			ResourcePtr<Parameter> found_param = mParameterGroup->findParameterRecursive(parameter);
			if (!errorState.check(found_param != nullptr, "%s: parameter %s not part of group or child of group: %s",
				mID.c_str(), parameter->mID.c_str(), mParameterGroup->mID.c_str()))
			{
				return false;
			}
		}
		return true;
	}
}