#pragma once

// Local Includes
#include "mesh.h"

namespace nap
{
	/**
	 * Predefined plane mesh with size 1x1, centered at origin.
	 */
	class NAPAPI PlaneMesh : public Mesh
	{
		RTTI_ENABLE(Mesh)

	public:
		/**
		 * Creates the mesh.
		 */
		virtual bool init(utility::ErrorState& errorState) override;
	};
}
