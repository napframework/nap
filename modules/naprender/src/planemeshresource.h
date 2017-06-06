#pragma once

// Local Includes
#include "meshresource.h"

namespace nap
{
	/**
	 * Predefined plane mesh
	 */
	class PlaneMeshResource : public MeshResource
	{
		RTTI_ENABLE(MeshResource)

	public:
		virtual bool init(utility::ErrorState& errorState) override;

		int mDummyProperty = 0;
	};
}
