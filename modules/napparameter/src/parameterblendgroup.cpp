/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "parameterblendgroup.h"

// nap::animablendparameters run time class definition 
RTTI_BEGIN_CLASS(nap::ParameterBlendGroup)
	RTTI_PROPERTY("Parameters",		&nap::ParameterBlendGroup::mParameters,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("RootGroup",		&nap::ParameterBlendGroup::mRootGroup,		nap::rtti::EPropertyMetaData::Required)
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
			ResourcePtr<Parameter> found_param = mRootGroup->findObjectRecursive(parameter->mID);
			if (!errorState.check(found_param != nullptr, "%s: parameter %s not part of group or child of group: %s",
				mID.c_str(), parameter->mID.c_str(), mRootGroup->mID.c_str()))
			{
				return false;
			}
		}
		return true;
	}
}
