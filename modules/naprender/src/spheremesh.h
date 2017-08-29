#pragma once

#include "mesh.h"
#include <utility/dllexport.h>

namespace nap
{
	/**
	 * Predefined sphere mesh
	 */
	class NAPAPI SphereMesh : public Mesh
	{
		RTTI_ENABLE(Mesh)

	public:
		/**
 		 * Load the mesh
 		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		void initSphere();

	public:
		float mRadius	= 1.0f;		// The radius of the mesh
		float mRings	= 50.0f;	// The number of rings in the mesh
		float mSectors	= 50.0f;	// The number of sectors in the mesh
	};
}
