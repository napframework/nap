#pragma once

// Local Includes
#include "resource.h"
#include "resourceptr.h"

namespace nap
{
	/**
	 * Groups together a set of resources.
	 */
	class NAPAPI Group : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::vector<nap::ResourcePtr<Resource>> mResources;		///< Property: 'Resources' The resources that belong to this group
	};
}
