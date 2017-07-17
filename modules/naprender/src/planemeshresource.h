#pragma once

// Local Includes
#include "meshresource.h"

namespace nap
{
	/**
	 * Predefined plane mesh with size 1x1, centered at origin.
	 */
	class NAPAPI PlaneMeshResource : public MeshResource
	{
		RTTI_ENABLE(MeshResource)

	public:
		/**
		 * Creates the mesh.
		 */
		virtual bool init(utility::ErrorState& errorState) override;
	};
}
