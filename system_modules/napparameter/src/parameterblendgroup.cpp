/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "parameterblendgroup.h"

// nap::ParameterBlendGroup run time class definition 
RTTI_BEGIN_CLASS(nap::ParameterBlendGroup, "Group of parameters that can be blended over time by using ParameterBlendComponent")
	RTTI_PROPERTY("Parameters",		&nap::ParameterBlendGroup::mParameters,		nap::rtti::EPropertyMetaData::Required,	"List of parameters to blend")
	RTTI_PROPERTY("RootGroup",		&nap::ParameterBlendGroup::mRootGroup,		nap::rtti::EPropertyMetaData::Required,	"The group, including sub-groups, the parameters belong to")
	RTTI_PROPERTY("BlendAll",		&nap::ParameterBlendGroup::mBlendAll,		nap::rtti::EPropertyMetaData::Default,	"Add all parameters, including children, from the root group")
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////

	static void getParametersRecursive(const ParameterGroup& inGroup, std::vector<ResourcePtr<Parameter>>& outParameters)
	{
		// Recursively apply the parameters of all child groups
		for (auto& child : inGroup.mChildren)
			getParametersRecursive(*child, outParameters);

		for (auto& member : inGroup.mMembers)
			outParameters.emplace_back(member);
	}


	//////////////////////////////////////////////////////////////////////////
	// ParameterBlendGroup
	//////////////////////////////////////////////////////////////////////////

	ParameterBlendGroup::~ParameterBlendGroup()			{ }


	bool ParameterBlendGroup::init(utility::ErrorState& errorState)
	{
		if (!mBlendAll)
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
		}
		else
		{
			// Recursively collect parameters from root group
			getParametersRecursive(*mRootGroup, mParameters);
		}
		return true;
	}
}
