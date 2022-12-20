/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "parametergroup.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * Represents a group of parameters that can be blended over time by a nap::ParameterBlendComponent.
	 * All given parameters need to be part of the root group, either as a direct sibling or 
	 * contained within a group that is part of the root group.
	 */
	class NAPAPI ParameterBlendGroup : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~ParameterBlendGroup();

		/**
		 * Ensures that every parameter specified in the 'Parameters' property is part of the 'RootGroup'.
		 * Either as a direct sibling or contained within a group that is part of the root group.
		 * @param errorState contains the error message when initialization fails
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		std::vector<ResourcePtr<Parameter>> mParameters;				///< Property: 'Parameters' list of parameters that are blended
		nap::ResourcePtr<ParameterGroup> mRootGroup = nullptr;			///< Property: 'RootGroup' the group all blend 'Parameters' belong to, either direct or in a child group.
		bool mBlendAll = false;											///< Property: 'BlendAll' recursively traverses the specified parameter group to include all parameters in this blend group.
	};
}
