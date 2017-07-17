#pragma once

#include "meshresource.h"
#include <nap/dllexport.h>

namespace nap
{
	/**
	 * Predefined sphere mesh
	 */
	class NAPAPI SphereMeshResource : public MeshResource
	{
		RTTI_ENABLE(MeshResource)

	public:
		/**
 		 * Load the mesh
 		 */
		virtual bool init(utility::ErrorState& errorState) override;

	public:
		float mRadius	= 1.0f;		// The radius of the mesh
		float mRings	= 50.0f;	// The number of rings in the mesh
		float mSectors	= 50.0f;	// The number of sectors in the mesh
	};
}
