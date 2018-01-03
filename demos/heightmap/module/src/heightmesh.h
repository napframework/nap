#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <planemesh.h>
#include <image.h>

namespace nap
{
	/**
	 * heightmesh
	 */
	class HeightMesh : public PlaneMesh
	{
		RTTI_ENABLE(nap::PlaneMesh)
	public:
		virtual ~HeightMesh();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		// Heightmap Properties
		nap::ObjectPtr<nap::Image> mHeightmap;					///< Property: "Heightmap" image resource
		float mHeight = 1.0f;									///< Property: "Height" max elevation level applied to the mesh with a pixel value of 1 
	};
}
