#pragma once

// Local Includes
#include "apivalue.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * Represents the signature of a method.
	 * This object is a resource that can be serialized from and to an external environment.
	 */
	class NAPAPI APISignature : public Resource
	{
		RTTI_ENABLE(Resource)
	public:

		// Default destructor
		virtual ~APISignature();

		std::vector<ResourcePtr<APIBaseValue>> mArguments;	///< Property: 'Arguments': All input arguments associated with this signature
	};
}
